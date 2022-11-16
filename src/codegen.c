#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"
#include "node.h"
#include "vartable.h"
#include "label.h"
#include "operator.h"
#include "inst.h"
#include "utils/memory.h"
#include "utils/array.h"

typedef struct {
  char *name;
  int   value;
} const_def_t;

typedef struct {
  char    *name;
  label_t *label;
  bool     resolved;
} func_def_t;

struct codegen_t {
  node_t     *root;
  ltable_t   *ltable;
  vartable_t *vartable;
  array_t    *consts;
  array_t    *funcs;
  label_t    *label_continue;
  label_t    *label_break;
  int         stack_depth;
  array_t    *insts;
  int         error_count;
};

static void collect_const_defs(codegen_t *codegen, node_t *node);
static void gen(codegen_t *codegen, node_t *node);
static void gen_sequence(codegen_t *codegen, node_t *node);
static void gen_expr_statement(codegen_t *codegen, node_t *node);
static void gen_if_statement(codegen_t *codegen, node_t *node);
static void gen_while_statement(codegen_t *codegen, node_t *node);
static void gen_loop_statement(codegen_t *codegen, node_t *node);
static void gen_for_statement(codegen_t *codegen, node_t *node);
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
static void emit_inst(codegen_t *codegen, inst_t *inst);
static label_t *alloc_label(codegen_t *codegen);
static int  allocate(codegen_t *codegen, const char *name, int size);
static void register_const(codegen_t *codegen, const char *name, int value);
static const_def_t *lookup_const(codegen_t *codegen, const char *name);
static func_def_t *lookup_or_register_func(codegen_t *codegen, const char *name);
static void error(codegen_t *codegen, const char *fmt, ...);

codegen_t *codegen_new(node_t *root) {
  codegen_t *codegen = (codegen_t *)AK_MEM_MALLOC(sizeof(codegen_t));
  codegen->root = root;
  codegen->ltable = ltable_new();
  codegen->vartable = vartable_new(NULL);
  codegen->consts = array_new(64);
  codegen->funcs = array_new(64);
  codegen->label_continue = NULL;
  codegen->label_break = NULL;
  codegen->stack_depth = 0;
  codegen->insts = array_new(256);
  codegen->error_count = 0;
  return codegen;
}

void codegen_release(codegen_t **pcodegen) {
  codegen_t *c = *pcodegen;

  ltable_release(&c->ltable);
  vartable_release(&c->vartable);

  for (int i = 0; i < array_count(c->consts); ++i) {
    const_def_t *cdef = (const_def_t *)array_get(c->consts, i);
    AK_MEM_FREE(cdef->name);
    AK_MEM_FREE(cdef);
  }
  array_release(&c->consts);

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

  collect_const_defs(codegen, codegen->root);

  emit_inst(codegen, inst_new_call(func_main->label));
  emit_inst(codegen, inst_new_halt());

  gen(codegen, codegen->root);

  if (!func_main->resolved) {
    error(codegen, "error: function 'main' is not defined.\n");
  }

  for (int i = 0; i < array_count(codegen->insts) - 1; ++i) {
    inst_t *inst1 = (inst_t *)array_get(codegen->insts, i);
    inst_t *inst2 = (inst_t *)array_get(codegen->insts, i + 1);
    if (inst1->opcode == OP_LABEL && inst2->opcode == OP_LABEL) {
      label_unify(inst1->label, inst2->label);
    }
  }
}

int codegen_get_error_count(codegen_t *codegen) {
  return codegen->error_count;
}

array_t *codegen_get_instructions(codegen_t *codegen) {
  return codegen->insts;
}

static void collect_const_defs(codegen_t *codegen, node_t *node) {
  switch (node_get_ntype(node)) {
  case NT_SEQ:
    for (int i = 0; i < node_get_child_count(node); ++i) {
      collect_const_defs(codegen, node_get_child(node, i));
    }
    break;
  case NT_CONST_STATEMENT:
    register_const(codegen, node_get_name(node_get_child(node, 0)), node_get_value(node_get_child(node, 1)));
    break;
  default:
    break;
  }
}

static void gen(codegen_t *codegen, node_t *node) {
  switch (node_get_ntype(node)) {
  case NT_GROUP:
    gen(codegen, node_get_child(node, 0));
    break;
  case NT_SEQ:
    codegen->stack_depth = 0;
    gen_sequence(codegen, node);
    break;
  case NT_EXPR:
    codegen->stack_depth = 0;
    gen_expr_statement(codegen, node_get_child(node, 0));
    break;
  case NT_IF:
    codegen->stack_depth = 0;
    gen_if_statement(codegen, node);
    break;
  case NT_WHILE:
    codegen->stack_depth = 0;
    gen_while_statement(codegen, node);
    break;
  case NT_LOOP_STATEMENT:
    codegen->stack_depth = 0;
    gen_loop_statement(codegen, node);
    break;
  case NT_FOR_STATEMENT:
    codegen->stack_depth = 0;
    gen_for_statement(codegen, node);
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
    emit_inst(codegen, inst_new_push(node_get_value(node)));
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
    emit_inst(codegen, inst_new_halt());
    break;
  default:
    break;
  }
}

static void gen_sequence(codegen_t *codegen, node_t *node) {
  for (int i = 0; i < node_get_child_count(node); ++i) {
    gen(codegen, node_get_child(node, i));
  }
}

static void gen_expr_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node);
  emit_inst(codegen, inst_new_pop());
}

static void gen_if_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_child(node, 0);
  node_t *then = node_get_child(node, 1);
  node_t *els = NULL;

  if (node_get_child_count(node) == 3) {
    els = node_get_child(node, 2);
  }

  if (els) {
    label_t *l1 = alloc_label(codegen);
    label_t *l2 = alloc_label(codegen);

    gen(codegen, cond);
    emit_inst(codegen, inst_new_jz(l1));
    gen(codegen, then);
    emit_inst(codegen, inst_new_jmp(l2));
    emit_inst(codegen, inst_new_label(l1));
    gen(codegen, els);
    emit_inst(codegen, inst_new_label(l2));
  }
  else {
    label_t *l = alloc_label(codegen);

    gen(codegen, cond);
    emit_inst(codegen, inst_new_jz(l));
    gen(codegen, then);
    emit_inst(codegen, inst_new_label(l));
  }
}

static void gen_while_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_child(node, 0);
  node_t *body = node_get_child(node, 1);
  label_t *label_continue = alloc_label(codegen);
  label_t *label_break = alloc_label(codegen);
  label_t *label_continue_before;
  label_t *label_break_before;

  label_continue_before = codegen->label_continue;
  label_break_before = codegen->label_break;
  codegen->label_continue = label_continue;
  codegen->label_break = label_break;

  emit_inst(codegen, inst_new_label(label_continue));
  gen(codegen, cond);
  emit_inst(codegen, inst_new_jz(label_break));
  gen(codegen, body);
  emit_inst(codegen, inst_new_jmp(label_continue));
  emit_inst(codegen, inst_new_label(label_break));

  codegen->label_continue = label_continue_before;
  codegen->label_break = label_break_before;
}

static void gen_loop_statement(codegen_t *codegen, node_t *node) {
  node_t *body = node_get_child(node, 0);
  label_t *label_continue = alloc_label(codegen);
  label_t *label_break = alloc_label(codegen);
  label_t *label_continue_before;
  label_t *label_break_before;

  label_continue_before = codegen->label_continue;
  label_break_before = codegen->label_break;
  codegen->label_continue = label_continue;
  codegen->label_break = label_break;

  emit_inst(codegen, inst_new_label(label_continue));
  gen(codegen, body);
  emit_inst(codegen, inst_new_jmp(label_continue));
  emit_inst(codegen, inst_new_label(label_break));

  codegen->label_continue = label_continue_before;
  codegen->label_break = label_break_before;
}

static void gen_for_statement(codegen_t *codegen, node_t *node) {
  node_t *init = node_get_child(node, 0);
  node_t *cond = node_get_child(node, 1);
  node_t *next = node_get_child(node, 2);
  node_t *body = node_get_child(node, 3);
  label_t *label_head = alloc_label(codegen);
  label_t *label_continue = alloc_label(codegen);
  label_t *label_break = alloc_label(codegen);
  label_t *label_continue_before;
  label_t *label_break_before;

  label_continue_before = codegen->label_continue;
  label_break_before = codegen->label_break;
  codegen->label_continue = label_continue;
  codegen->label_break = label_break;

  if (node_get_ntype(init) != NT_EMPTY) {
    codegen->stack_depth = 0;
    gen(codegen, init);
    emit_inst(codegen, inst_new_pop());
  }

  emit_inst(codegen, inst_new_label(label_head));

  if (node_get_ntype(cond) != NT_EMPTY) {
    codegen->stack_depth = 0;
    gen(codegen, cond);
    emit_inst(codegen, inst_new_jz(label_break));
  }

  codegen->stack_depth = 0;
  gen(codegen, body);

  emit_inst(codegen, inst_new_label(label_continue));

  if (node_get_ntype(next) != NT_EMPTY) {
    codegen->stack_depth = 0;
    gen(codegen, next);
    emit_inst(codegen, inst_new_pop());
  }

  emit_inst(codegen, inst_new_jmp(label_head));
  emit_inst(codegen, inst_new_label(label_break));

  codegen->label_continue = label_continue_before;
  codegen->label_break = label_break_before;
}

static void gen_break_statement(codegen_t *codegen, node_t *node) {
  label_t *label = codegen->label_break;
  if (!label) {
    error(codegen, "error: illegal break statement.\n");
    return;
  }

  emit_inst(codegen, inst_new_jmp(label));
}

static void gen_continue_statement(codegen_t *codegen, node_t *node) {
  label_t *label = codegen->label_continue;
  if (!label) {
    error(codegen, "error: illegal continue statement.\n");
    return;
  }

  emit_inst(codegen, inst_new_jmp(label));
}

static void gen_putc_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_child(node, 0));
  emit_inst(codegen, inst_new_putc());
}

static void gen_puti_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_child(node, 0));
  emit_inst(codegen, inst_new_puti());
}

static void gen_getc_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  const char *name = node_get_name(ident);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);

  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is readonly.\n", name);
    return;
  }

  emit_inst(codegen, inst_new_push(varentry_get_offset(varentry)));
  emit_inst(codegen, inst_new_getc());
}

static void gen_geti_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  const char *name = node_get_name(ident);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);

  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is readonly.\n", name);
    return;
  }

  emit_inst(codegen, inst_new_push(varentry_get_offset(varentry)));
  emit_inst(codegen, inst_new_geti());
}

static void gen_array_decl_statement(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  node_t *capacity = node_get_child(node, 1);
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

  emit_inst(codegen, inst_new_label(func->label));
  gen(codegen, body);

  codegen->vartable = vartable_get_parent(vartable_local);
  vartable_release(&vartable_local);
}

static void gen_return_statement(codegen_t *codegen, node_t *node) {
  node_t *expr = node_get_child(node, 0);
  gen(codegen, expr);
  emit_inst(codegen, inst_new_ret());
}

static void gen_unary(codegen_t *codegen, node_t *node) {
  switch (node_get_uop(node)) {
  case UOP_NEGATIVE: /* implement -x as 0 - x. */
    emit_inst(codegen, inst_new_push(0));
    gen(codegen, node_get_child(node, 0));
    emit_inst(codegen, inst_new_sub());
    break;
  case UOP_NOT:
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      gen(codegen, node_get_child(node, 0));
      emit_inst(codegen, inst_new_jz(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  default:
    break;
  }
}

static void gen_binary(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_child(node, 0));
  gen(codegen, node_get_child(node, 1));

  switch (node_get_bop(node)) {
  case BOP_ADD:
    emit_inst(codegen, inst_new_add());
    break;
  case BOP_SUB:
    emit_inst(codegen, inst_new_sub());
    break;
  case BOP_MUL:
    emit_inst(codegen, inst_new_mul());
    break;
  case BOP_DIV:
    emit_inst(codegen, inst_new_div());
    break;
  case BOP_MOD:
    emit_inst(codegen, inst_new_mod());
    break;
  case BOP_OR:
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      label_t *l3 = alloc_label(codegen);
      emit_inst(codegen, inst_new_jz(l1));
      emit_inst(codegen, inst_new_pop());
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_jmp(l3));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_jz(l2));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_jmp(l3));
      emit_inst(codegen, inst_new_label(l2));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_label(l3));
    }
    break;
  case BOP_AND:
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      label_t *l3 = alloc_label(codegen);
      label_t *l4 = alloc_label(codegen);
      emit_inst(codegen, inst_new_jz(l1));
      emit_inst(codegen, inst_new_jz(l2));
      emit_inst(codegen, inst_new_jmp(l3));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_pop());
      emit_inst(codegen, inst_new_label(l2));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_jmp(l4));
      emit_inst(codegen, inst_new_label(l3));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_label(l4));
    }
    break;
  case BOP_EQ:
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      emit_inst(codegen, inst_new_sub());
      emit_inst(codegen, inst_new_jz(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  case BOP_NEQ:
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      emit_inst(codegen, inst_new_sub());
      emit_inst(codegen, inst_new_jz(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  case BOP_LT: /* x < y --> x - y < 0 */
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      emit_inst(codegen, inst_new_sub());
      emit_inst(codegen, inst_new_jneg(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  case BOP_LE: /* x <= y --> !(y - x < 0) */
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      emit_inst(codegen, inst_new_swap());
      emit_inst(codegen, inst_new_sub());
      emit_inst(codegen, inst_new_jneg(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  case BOP_GT: /* x > y --> y - x < 0 */
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      emit_inst(codegen, inst_new_swap());
      emit_inst(codegen, inst_new_sub());
      emit_inst(codegen, inst_new_jneg(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  case BOP_GE: /* x >= y --> !(x - y < 0) */
    {
      label_t *l1 = alloc_label(codegen);
      label_t *l2 = alloc_label(codegen);
      emit_inst(codegen, inst_new_sub());
      emit_inst(codegen, inst_new_jneg(l1));
      emit_inst(codegen, inst_new_push(1));
      emit_inst(codegen, inst_new_jmp(l2));
      emit_inst(codegen, inst_new_label(l1));
      emit_inst(codegen, inst_new_push(0));
      emit_inst(codegen, inst_new_label(l2));
    }
    break;
  default:
    break;
  }
}

static void gen_assign(codegen_t *codegen, node_t *node) {
  node_t *lhs = node_get_child(node, 0);
  node_t *expr = node_get_child(node, 1);
  node_t *ident = node_get_child(lhs, 0);
  const char *name = node_get_name(ident);
  varentry_t *varentry;

  if (lookup_const(codegen, name)) {
    error(codegen, "error: cannot assign to '%s' defined as a constant.\n", name);
    return;
  }

  varentry = vartable_lookup_or_add_var(codegen->vartable, name);
  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is readonly.\n", name);
    return;
  }

  gen(codegen, expr);

  emit_inst(codegen, inst_new_push(varentry_get_offset(varentry)));

  if (node_get_ntype(lhs) == NT_ARRAY) {
    codegen->stack_depth++;
    gen(codegen, node_get_child(lhs, 1));
    emit_inst(codegen, inst_new_add());
    codegen->stack_depth--;
  }

  emit_inst(codegen, inst_new_copy(1));
  emit_inst(codegen, inst_new_store());
  codegen->stack_depth++;
}

static void gen_variable(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  const_def_t *cdef = lookup_const(codegen, node_get_name(ident));
  varentry_t *varentry;
  int offset;

  if (cdef) {
    emit_inst(codegen, inst_new_push(cdef->value));
    return;
  }

  varentry = vartable_lookup_or_add_var(codegen->vartable, node_get_name(ident));
  offset = varentry_get_offset(varentry);

  if (varentry_is_local(varentry)) {
    emit_inst(codegen, inst_new_copy(codegen->stack_depth + offset));
  }
  else {
    emit_inst(codegen, inst_new_push(offset));
    emit_inst(codegen, inst_new_load());
  }
}

static void gen_array(codegen_t *codegen, node_t *node) {
  node_t *ident = node_get_child(node, 0);
  const char *name = node_get_name(ident);
  varentry_t *varentry = vartable_lookup_or_add_var(codegen->vartable, name);

  if (varentry_is_local(varentry)) {
    error(codegen, "error: function parameter '%s' is not array.\n", name);
    return;
  }

  emit_inst(codegen, inst_new_push(varentry_get_offset(varentry)));
  codegen->stack_depth++;
  gen(codegen, node_get_child(node, 1));
  emit_inst(codegen, inst_new_add());
  emit_inst(codegen, inst_new_load());
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
  emit_inst(codegen, inst_new_call(func->label));
  codegen->stack_depth++;
  emit_inst(codegen, inst_new_slide(arg_count));
  codegen->stack_depth -= arg_count;
}

static void emit_inst(codegen_t *codegen, inst_t *inst) {
  array_append(codegen->insts, inst);
}

static label_t *alloc_label(codegen_t *codegen) {
  return ltable_alloc(codegen->ltable);
}

static int allocate(codegen_t *codegen, const char *name, int size) {
  varentry_t *e = vartable_add_var(codegen->vartable, name, size);
  return varentry_get_offset(e);
}

static void register_const(codegen_t *codegen, const char *name, int value) {
  const_def_t *cdef = lookup_const(codegen, name);

  if (cdef) {
    error(codegen, "error: constant '%s' is redefined.\n", name);
    return;
  }

  cdef = (const_def_t *)AK_MEM_MALLOC(sizeof(const_def_t));
  cdef->name = AK_MEM_STRDUP(name);
  cdef->value = value;
  array_append(codegen->consts, cdef);
}

static const_def_t *lookup_const(codegen_t *codegen, const char *name) {
  for (int i = 0; i < array_count(codegen->consts); ++i) {
    const_def_t *cdef = (const_def_t *)array_get(codegen->consts, i);
    if (strcmp(cdef->name, name) == 0) {
      return cdef;
    }
  }
  return NULL;
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
  func->label = alloc_label(codegen);
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
