#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"
#include "node.h"
#include "vartable.h"
#include "operator.h"
#include "inst.h"
#include "utils/memory.h"
#include "utils/array.h"

typedef struct {
  char *name;
  int   label;
  bool  resolved;
} func_def_t;

struct codegen_t {
  node_t     *root;
  int         label_count;
  vartable_t *vartable;
  array_t    *funcs;
  int         cur_label_head;
  int         cur_label_tail;
  int         stack_depth;
  array_t    *insts;
  int         error_count;
};

static void gen(codegen_t *codegen, node_t *node);
static void gen_sequence(codegen_t *codegen, node_t *node);
static void gen_expr_statement(codegen_t *codegen, node_t *node);
static void gen_if_statement(codegen_t *codegen, node_t *node);
static void gen_while_statement(codegen_t *codegen, node_t *node);
static void gen_break_statement(codegen_t *codegen, node_t *node);
static void gen_continue_statement(codegen_t *codegen, node_t *node);
static void gen_putc_statement(codegen_t *codegen, node_t *node);
static void gen_puti_statement(codegen_t *codegen, node_t *node);
static void gen_getc_statement(codegen_t *codegen, node_t *node);
static void gen_geti_statement(codegen_t *codegen, node_t *node);
static void gen_array_decl_statement(codegen_t *codegen, node_t *node);
static void gen_func_statement(codegen_t *codegen, node_t *node);
static void gen_return_statement(codegen_t *codegen, node_t *node);
static void gen_unary(codegen_t *codegen, node_t *node);
static void gen_binary(codegen_t *codegen, node_t *node);
static void gen_assign(codegen_t *codegen, node_t *node);
static void gen_variable(codegen_t *codegen, node_t *node);
static void gen_array(codegen_t *codegen, node_t *node);
static void gen_func_call(codegen_t *codegen, node_t *node);
static void emit_inst(codegen_t *codegen, opcode_t opcode, int operand);
static int  alloc_label_id(codegen_t *codegen);
static int  allocate(codegen_t *codegen, const char *name, int size);
static func_def_t *lookup_or_register_func(codegen_t *codegen, const char *name);
static void error(codegen_t *codegen, const char *fmt, ...);

codegen_t *codegen_new(node_t *root) {
  codegen_t *codegen = (codegen_t *)AK_MEM_MALLOC(sizeof(codegen_t));
  codegen->root = root;
  codegen->label_count = 0;
  codegen->vartable = vartable_new(NULL);
  codegen->funcs = array_new(64);
  codegen->cur_label_head = -1;
  codegen->cur_label_tail = -1;
  codegen->stack_depth = 0;
  codegen->insts = array_new(256);
  codegen->error_count = 0;
  return codegen;
}

void codegen_release(codegen_t **pcodegen) {
  codegen_t *c = *pcodegen;

  vartable_release(&c->vartable);

  for (int i = 0; i < array_count(c->funcs); ++i) {
    func_def_t *f = (func_def_t *)array_get(c->funcs, i);
    AK_MEM_FREE(f->name);
    AK_MEM_FREE(f);
  }
  array_release(&c->funcs);

  for (int i = 0; i < array_count(c->insts); ++i) {
    inst_t *inst = (inst_t *)array_get(c->insts, i);
    AK_MEM_FREE(inst);
  }
  array_release(&c->insts);

  AK_MEM_FREE(c);
  *pcodegen = NULL;
}

void codegen_generate(codegen_t *codegen) {
  func_def_t *func_main = lookup_or_register_func(codegen, "main");

  emit_inst(codegen, OP_CALL, func_main->label);
  emit_inst(codegen, OP_HALT, 0);

  gen(codegen, codegen->root);

  if (!func_main->resolved) {
    error(codegen, "error: function 'main' is not defined.\n");
  }
}

int codegen_get_error_count(codegen_t *codegen) {
  return codegen->error_count;
}

array_t *codegen_get_instructions(codegen_t *codegen) {
  return codegen->insts;
}

static void gen(codegen_t *codegen, node_t *node) {
  switch (node_get_ntype(node)) {
  case NT_SEQ:
    codegen->stack_depth = 0;
    gen_sequence(codegen, node);
    break;
  case NT_EXPR:
    codegen->stack_depth = 0;
    gen_expr_statement(codegen, node_get_l(node));
    break;
  case NT_IF:
    codegen->stack_depth = 0;
    gen_if_statement(codegen, node);
    break;
  case NT_WHILE:
    codegen->stack_depth = 0;
    gen_while_statement(codegen, node);
    break;
  case NT_BREAK:
    codegen->stack_depth = 0;
    gen_break_statement(codegen, node);
    break;
  case NT_CONTINUE:
    codegen->stack_depth = 0;
    gen_continue_statement(codegen, node);
    break;
  case NT_GETC:
    codegen->stack_depth = 0;
    gen_getc_statement(codegen, node);
    break;
  case NT_GETI:
    codegen->stack_depth = 0;
    gen_geti_statement(codegen, node);
    break;
  case NT_PUTC:
    codegen->stack_depth = 0;
    gen_putc_statement(codegen, node);
    break;
  case NT_PUTI:
    codegen->stack_depth = 0;
    gen_puti_statement(codegen, node);
    break;
  case NT_ARRAY_DECL:
    codegen->stack_depth = 0;
    gen_array_decl_statement(codegen, node);
    break;
  case NT_FUNC:
    codegen->stack_depth = 0;
    gen_func_statement(codegen, node);
    break;
  case NT_RETURN:
    codegen->stack_depth = 0;
    gen_return_statement(codegen, node);
    break;
  case NT_UNARY:
    gen_unary(codegen, node);
    break;
  case NT_BINARY:
    gen_binary(codegen, node);
    codegen->stack_depth--;
    break;
  case NT_ASSIGN:
    gen_assign(codegen, node);
    break;
  case NT_INTEGER:
    emit_inst(codegen, OP_PUSH, node_get_value(node));
    codegen->stack_depth++;
    break;
  case NT_VARIABLE:
    gen_variable(codegen, node);
    codegen->stack_depth++;
    break;
  case NT_ARRAY:
    gen_array(codegen, node);
    break;
  case NT_FUNC_CALL:
    gen_func_call(codegen, node);
    break;
  case NT_HALT:
    emit_inst(codegen, OP_HALT, 0);
    break;
  default:
    break;
  }
}

static void gen_sequence(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  gen(codegen, node_get_r(node));
}

static void gen_expr_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node);
  emit_inst(codegen, OP_POP, 0);
}

static void gen_if_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node);
  node_t *then = node_get_l(node);
  node_t *els = node_get_r(node);

  if (els) {
    int l1 = alloc_label_id(codegen);
    int l2 = alloc_label_id(codegen);

    gen(codegen, cond);
    emit_inst(codegen, OP_JZ, l1);
    gen(codegen, then);
    emit_inst(codegen, OP_JMP, l2);
    emit_inst(codegen, OP_LABEL, l1);
    gen(codegen, els);
    emit_inst(codegen, OP_LABEL, l2);
  }
  else {
    int l = alloc_label_id(codegen);

    gen(codegen, cond);
    emit_inst(codegen, OP_JZ, l);
    gen(codegen, then);
    emit_inst(codegen, OP_LABEL, l);
  }
}

static void gen_while_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node);
  node_t *body = node_get_l(node);
  int label_head = alloc_label_id(codegen);
  int label_tail = alloc_label_id(codegen);
  int label_head_before;
  int label_tail_before;

  emit_inst(codegen, OP_LABEL, label_head);
  gen(codegen, cond);
  emit_inst(codegen, OP_JZ, label_tail);

  label_head_before = codegen->cur_label_head;
  label_tail_before = codegen->cur_label_tail;

  codegen->cur_label_head = label_head;
  codegen->cur_label_tail = label_tail;

  gen(codegen, body);

  codegen->cur_label_head = label_head_before;
  codegen->cur_label_tail = label_tail_before;

  emit_inst(codegen, OP_JMP, label_head);
  emit_inst(codegen, OP_LABEL, label_tail);
}

static void gen_break_statement(codegen_t *codegen, node_t *node) {
  int label = codegen->cur_label_tail;
  if (label == -1) {
    error(codegen, "error: illegal break statement.\n");
    return;
  }

  emit_inst(codegen, OP_JMP, label);
}

static void gen_continue_statement(codegen_t *codegen, node_t *node) {
  int label = codegen->cur_label_head;
  if (label == -1) {
    error(codegen, "error: illegal break statement.\n");
    return;
  }

  emit_inst(codegen, OP_JMP, label);
}

static void gen_putc_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  emit_inst(codegen, OP_PUTC, 0);
}

static void gen_puti_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  emit_inst(codegen, OP_PUTI, 0);
}

static void gen_getc_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_l(node);
  const char *name = node_get_name(ident);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);

  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is readonly.\n", name);
    return;
  }

  emit_inst(codegen, OP_PUSH, varentry_get_offset(varentry));
  emit_inst(codegen, OP_GETC, 0);
}

static void gen_geti_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_l(node);
  const char *name = node_get_name(ident);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);

  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is readonly.\n", name);
    return;
  }

  emit_inst(codegen, OP_PUSH, varentry_get_offset(varentry));
  emit_inst(codegen, OP_GETI, 0);
}

static void gen_array_decl_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_l(node);
  node_t *capacity = node_get_r(node);
  allocate(codegen, node_get_name(ident), node_get_value(capacity));
}

static void gen_func_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  node_t *param = node_get_child(node, 1);
  node_t *body = node_get_child(node, 2);
  func_def_t *func = lookup_or_register_func(codegen, node_get_name(ident));
  vartable_t *vartable_local;

  if (func->resolved) {
    error(codegen, "error: function '%s' is redefined.\n", func->name);
    return;
  }
  func->resolved = true;

  vartable_local = vartable_new(codegen->vartable);

  for (int i = 0; i < node_get_child_count(param); ++i) {
    node_t *param_ident = node_get_child(param, i);
    vartable_add_var(vartable_local, node_get_name(param_ident), 1);
  }

  codegen->vartable = vartable_local;

  emit_inst(codegen, OP_LABEL, func->label);
  gen(codegen, body);

  codegen->vartable = vartable_get_parent(vartable_local);
  vartable_release(&vartable_local);
}

static void gen_return_statement(codegen_t *codegen, node_t *node) {
  node_t *expr = node_get_child(node, 0);
  gen(codegen, expr);
  emit_inst(codegen, OP_RET, 0);
}

static void gen_unary(codegen_t *codegen, node_t *node) {
  switch (node_get_uop(node)) {
  case UOP_NEGATIVE: /* implement -x as 0 - x. */
    emit_inst(codegen, OP_PUSH, 0);
    gen(codegen, node_get_l(node));
    emit_inst(codegen, OP_SUB, 0);
    break;
  case UOP_NOT:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen(codegen, node_get_l(node));
      emit_inst(codegen, OP_JZ, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  default:
    break;
  }
}

static void gen_binary(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  gen(codegen, node_get_r(node));

  switch (node_get_bop(node)) {
  case BOP_ADD:
    emit_inst(codegen, OP_ADD, 0);
    break;
  case BOP_SUB:
    emit_inst(codegen, OP_SUB, 0);
    break;
  case BOP_MUL:
    emit_inst(codegen, OP_MUL, 0);
    break;
  case BOP_DIV:
    emit_inst(codegen, OP_DIV, 0);
    break;
  case BOP_MOD:
    emit_inst(codegen, OP_MOD, 0);
    break;
  case BOP_OR:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      int l3 = alloc_label_id(codegen);
      emit_inst(codegen, OP_JZ, l1);
      emit_inst(codegen, OP_POP, 0);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_JMP, l3);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_JZ, l2);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_JMP, l3);
      emit_inst(codegen, OP_LABEL, l2);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_LABEL, l3);
    }
    break;
  case BOP_AND:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      int l3 = alloc_label_id(codegen);
      int l4 = alloc_label_id(codegen);
      emit_inst(codegen, OP_JZ, l1);
      emit_inst(codegen, OP_JZ, l2);
      emit_inst(codegen, OP_JMP, l3);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_POP, 0);
      emit_inst(codegen, OP_LABEL, l2);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_JMP, l4);
      emit_inst(codegen, OP_LABEL, l3);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_LABEL, l4);
    }
    break;
  case BOP_EQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_inst(codegen, OP_SUB, 0);
      emit_inst(codegen, OP_JZ, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  case BOP_NEQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_inst(codegen, OP_SUB, 0);
      emit_inst(codegen, OP_JZ, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  case BOP_LT: /* x < y --> x - y < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_inst(codegen, OP_SUB, 0);
      emit_inst(codegen, OP_JNEG, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  case BOP_LE: /* x <= y --> !(y - x < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_inst(codegen, OP_SWAP, 0);
      emit_inst(codegen, OP_SUB, 0);
      emit_inst(codegen, OP_JNEG, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  case BOP_GT: /* x > y --> y - x < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_inst(codegen, OP_SWAP, 0);
      emit_inst(codegen, OP_SUB, 0);
      emit_inst(codegen, OP_JNEG, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  case BOP_GE: /* x >= y --> !(x - y < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_inst(codegen, OP_SUB, 0);
      emit_inst(codegen, OP_JNEG, l1);
      emit_inst(codegen, OP_PUSH, 1);
      emit_inst(codegen, OP_JMP, l2);
      emit_inst(codegen, OP_LABEL, l1);
      emit_inst(codegen, OP_PUSH, 0);
      emit_inst(codegen, OP_LABEL, l2);
    }
    break;
  default:
    break;
  }
}

static void gen_assign(codegen_t *codegen, node_t *node) {
  node_t *lhs = node_get_l(node);
  node_t *expr = node_get_r(node);

  gen(codegen, expr);

  switch (node_get_ntype(lhs)) {
  case NT_VARIABLE:
    {
      node_t *ident = node_get_child(lhs, 0);
      const char *name = node_get_name(ident);
      varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);
      if (varentry_is_local(varentry)) {
	error(codegen, "error: function parameter '%s' is readonly.\n", name);
	return;
      }
      emit_inst(codegen, OP_PUSH, varentry_get_offset(varentry));
    }
    break;
  case NT_ARRAY:
    {
      node_t *ident = node_get_l(lhs);
      const char *name = node_get_name(ident);
      varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);
      if (varentry_is_local(varentry)) {
	error(codegen, "error: function parameter '%s' is readonly.\n", name);
	return;
      }
      emit_inst(codegen, OP_PUSH, varentry_get_offset(varentry));
      codegen->stack_depth++;
      gen(codegen, node_get_r(lhs));
      emit_inst(codegen, OP_ADD, 0);
      codegen->stack_depth--;
    }
    break;
  default:
    error(codegen, "error: invalid left hand value. (ntype=%d)\n", node_get_ntype(lhs));
    return;
  }

  emit_inst(codegen, OP_COPY, 1);
  emit_inst(codegen, OP_STORE, 0);
  codegen->stack_depth++;
}

static void gen_variable(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, node_get_name(ident));
  int offset = varentry_get_offset(varentry);

  if (varentry_is_local(varentry)) {
    emit_inst(codegen, OP_COPY, codegen->stack_depth + offset);
  }
  else {
    emit_inst(codegen, OP_PUSH, offset);
    emit_inst(codegen, OP_LOAD, 0);
  }
}

static void gen_array(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_l(node);
  const char *name = node_get_name(ident);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);

  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is not array.\n", name);
    return;
  }

  emit_inst(codegen, OP_PUSH, varentry_get_offset(varentry));
  codegen->stack_depth++;
  gen(codegen, node_get_r(node));
  emit_inst(codegen, OP_ADD, 0);
  emit_inst(codegen, OP_LOAD, 0);
  codegen->stack_depth--;
}

static void gen_func_call(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  node_t *args = node_get_child(node, 1);
  int arg_count = node_get_child_count(args);
  func_def_t *func = lookup_or_register_func(codegen, node_get_name(ident));

  for (int i = arg_count - 1; i >= 0; --i) {
    gen(codegen, node_get_child(args, i));
  }
  emit_inst(codegen, OP_CALL, func->label);
  codegen->stack_depth++;
  emit_inst(codegen, OP_SLIDE, arg_count);
  codegen->stack_depth -= arg_count;
}

static void emit_inst(codegen_t *codegen, opcode_t opcode, int operand) {
  array_append(codegen->insts, inst_new(opcode, operand));
}

static int alloc_label_id(codegen_t *codegen) {
  return codegen->label_count++;
}

static int allocate(codegen_t *codegen, const char *name, int size) {
  varentry_t *e = vartable_add_var(codegen->vartable, name, size);
  return varentry_get_offset(e);
}

static func_def_t *lookup_or_register_func(codegen_t *codegen, const char *name) {
  func_def_t *func;

  for (int i = 0; i < array_count(codegen->funcs); ++i) {
    func = (func_def_t *)array_get(codegen->funcs, i);
    if (strcmp(func->name, name) == 0) {
      return func;
    }
  }

  func = (func_def_t *)AK_MEM_MALLOC(sizeof(func_def_t));
  func->name = AK_MEM_STRDUP(name);
  func->label = alloc_label_id(codegen);
  func->resolved = false;
  array_append(codegen->funcs, func);
  return func;
}

static void error(codegen_t *codegen, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  codegen->error_count++;
}
