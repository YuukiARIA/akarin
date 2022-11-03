#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "node.h"
#include "operator.h"
#include "emitter.h"

struct codegen_t {
  node_t    *root;
  int        label_count;
  int        var_count;
  char       vars[256][64];
  int        cur_label_tail;
  emitter_t *emitter;
};

static void gen(codegen_t *codegen, node_t *node);
static void gen_sequence(codegen_t *codegen, node_t *node);
static void gen_expr_statement(codegen_t *codegen, node_t *node);
static void gen_if_statement(codegen_t *codegen, node_t *node);
static void gen_while_statement(codegen_t *codegen, node_t *node);
static void gen_break_statement(codegen_t *codegen, node_t *node);
static void gen_putc_statement(codegen_t *codegen, node_t *node);
static void gen_puti_statement(codegen_t *codegen, node_t *node);
static void gen_getc_statement(codegen_t *codegen, node_t *node);
static void gen_geti_statement(codegen_t *codegen, node_t *node);
static void gen_array_decl_statement(codegen_t *codegen, node_t *node);
static void gen_unary(codegen_t *codegen, node_t *node);
static void gen_binary(codegen_t *codegen, node_t *node);
static void gen_assign(codegen_t *codegen, node_t *node);
static void gen_variable(codegen_t *codegen, node_t *node);
static void gen_array(codegen_t *codegen, node_t *node);
static int  alloc_label_id(codegen_t *codegen);
static int  get_var_index(codegen_t *codegen, const char *name);
static int  allocate(codegen_t *codegen, const char *name, int size);

codegen_t *codegen_new(node_t *root, emitter_t *emitter) {
  codegen_t *codegen = (codegen_t *)malloc(sizeof(codegen_t));
  codegen->root = root;
  codegen->label_count = 0;
  codegen->cur_label_tail = -1;
  codegen->emitter = emitter;
  return codegen;
}

void codegen_release(codegen_t **pcodegen) {
  free(*pcodegen);
  *pcodegen = NULL;
}

void codegen_generate(codegen_t *codegen) {
  gen(codegen, codegen->root);
  emit_halt(codegen->emitter);
  emit_end(codegen->emitter);
}

static void gen(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;

  switch (node_get_ntype(node)) {
  case NT_SEQ:
    gen_sequence(codegen, node);
    break;
  case NT_EXPR:
    gen_expr_statement(codegen, node_get_l(node));
    break;
  case NT_IF:
    gen_if_statement(codegen, node);
    break;
  case NT_WHILE:
    gen_while_statement(codegen, node);
    break;
  case NT_BREAK:
    gen_break_statement(codegen, node);
    break;
  case NT_GETC:
    gen_getc_statement(codegen, node);
    break;
  case NT_GETI:
    gen_geti_statement(codegen, node);
    break;
  case NT_PUTC:
    gen_putc_statement(codegen, node);
    break;
  case NT_PUTI:
    gen_puti_statement(codegen, node);
    break;
  case NT_ARRAY_DECL:
    gen_array_decl_statement(codegen, node);
    break;
  case NT_UNARY:
    gen_unary(codegen, node);
    break;
  case NT_BINARY:
    gen_binary(codegen, node);
    break;
  case NT_ASSIGN:
    gen_assign(codegen, node);
    break;
  case NT_INTEGER:
    emit_push(emitter, node_get_value(node));
    break;
  case NT_VARIABLE:
    gen_variable(codegen, node);
    break;
  case NT_ARRAY:
    gen_array(codegen, node);
    break;
  case NT_HALT:
    emit_halt(emitter);
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
  emit_pop(codegen->emitter);
}

static void gen_if_statement(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  node_t *cond = node_get_cond(node);
  node_t *then = node_get_l(node);
  node_t *els = node_get_r(node);

  if (els) {
    int l1 = alloc_label_id(codegen);
    int l2 = alloc_label_id(codegen);

    gen(codegen, cond);
    emit_jz(emitter, l1);
    gen(codegen, then);
    emit_jmp(emitter, l2);
    emit_label(emitter, l1);
    gen(codegen, els);
    emit_label(emitter, l2);
  }
  else {
    int l = alloc_label_id(codegen);

    gen(codegen, cond);
    emit_jz(emitter, l);
    gen(codegen, then);
    emit_label(emitter, l);
  }
}

static void gen_while_statement(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  node_t *cond = node_get_cond(node);
  node_t *body = node_get_l(node);
  int label_head = alloc_label_id(codegen);
  int label_tail = alloc_label_id(codegen);

  emit_label(emitter, label_head);
  gen(codegen, cond);
  emit_jz(emitter, label_tail);

  codegen->cur_label_tail = label_tail;
  gen(codegen, body);
  codegen->cur_label_tail = -1;

  emit_jmp(emitter, label_head);
  emit_label(emitter, label_tail);
}

static void gen_break_statement(codegen_t *codegen, node_t *node) {
  int label = codegen->cur_label_tail;
  if (label == -1) {
    fputs("illegal break statement.", stderr);
    return;
  }

  emit_jmp(codegen->emitter, label);
}

static void gen_putc_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  emit_putc(codegen->emitter);
}

static void gen_puti_statement(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  emit_puti(codegen->emitter);
}

static void gen_getc_statement(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  node_t *var = node_get_l(node);
  int var_index = get_var_index(codegen, node_get_name(var));
  emit_push(emitter, var_index);
  emit_getc(emitter);
}

static void gen_geti_statement(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  node_t *var = node_get_l(node);
  int var_index = get_var_index(codegen, node_get_name(var));
  emit_push(emitter, var_index);
  emit_geti(emitter);
}

static void gen_array_decl_statement(codegen_t *codegen, node_t *node) {
  int size = node_get_value(node);
  allocate(codegen, node_get_name(node_get_l(node)), size);
}

static void gen_unary(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;

  switch (node_get_uop(node)) {
  case UOP_NEGATIVE: /* implement -x as 0 - x. */
    emit_push(emitter, 0);
    gen(codegen, node_get_l(node));
    emit_sub(emitter);
    break;
  case UOP_NOT:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen(codegen, node_get_l(node));
      emit_jz(emitter, l1);
      emit_push(emitter, 0);
      emit_jmp(emitter, l2);
      emit_label(emitter, l1);
      emit_push(emitter, 1);
      emit_label(emitter, l2);
    }
    break;
  default:
    break;
  }
}

static void gen_binary(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;

  gen(codegen, node_get_l(node));
  gen(codegen, node_get_r(node));

  switch (node_get_bop(node)) {
  case BOP_ADD:
    emit_add(emitter);
    break;
  case BOP_SUB:
    emit_sub(emitter);
    break;
  case BOP_MUL:
    emit_mul(emitter);
    break;
  case BOP_DIV:
    emit_div(emitter);
    break;
  case BOP_MOD:
    emit_mod(emitter);
    break;
  case BOP_OR:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      int l3 = alloc_label_id(codegen);
      emit_jz(emitter, l1);
      emit_pop(emitter);
      emit_push(emitter, 1);
      emit_jmp(emitter, l3);
      emit_label(emitter, l1);
      emit_jz(emitter, l2);
      emit_push(emitter, 1);
      emit_jmp(emitter, l3);
      emit_label(emitter, l2);
      emit_push(emitter, 0);
      emit_label(emitter, l3);
    }
    break;
  case BOP_AND:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      int l3 = alloc_label_id(codegen);
      int l4 = alloc_label_id(codegen);
      emit_jz(emitter, l1);
      emit_jz(emitter, l2);
      emit_jmp(emitter, l3);
      emit_label(emitter, l1);
      emit_pop(emitter);
      emit_label(emitter, l2);
      emit_push(emitter, 0);
      emit_jmp(emitter, l4);
      emit_label(emitter, l3);
      emit_push(emitter, 1);
      emit_label(emitter, l4);
    }
    break;
  case BOP_EQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_sub(emitter);       /* SUB */
      emit_jz(emitter, l1);      /* JZ L1 */
      emit_push(emitter, 0);     /* PUSH 0 */
      emit_jmp(emitter, l2);     /* JMP L2 */
      emit_label(emitter, l1);   /* :L1 */
      emit_push(emitter, 1);     /* PUSH 1 */
      emit_label(emitter, l2);   /* :L2 */
    }
    break;
  case BOP_NEQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_sub(emitter);       /* SUB */
      emit_jz(emitter, l1);      /* JZ L1 */
      emit_push(emitter, 1);     /* PUSH 1 */
      emit_jmp(emitter, l2);     /* JMP L2 */
      emit_label(emitter, l1);   /* :L1 */
      emit_push(emitter, 0);     /* PUSH 0 */
      emit_label(emitter, l2);   /* :L2 */
    }
    break;
  case BOP_LT: /* x < y --> x - y < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_sub(emitter);       /* SUB */
      emit_jneg(emitter, l1);    /* JNEG L1 */
      emit_push(emitter, 0);     /* PUSH 0 */
      emit_jmp(emitter, l2);     /* JMP L2 */
      emit_label(emitter, l1);   /* :L1 */
      emit_push(emitter, 1);     /* PUSH 1 */
      emit_label(emitter, l2);   /* :L2 */
    }
    break;
  case BOP_LE: /* x <= y --> !(y - x < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_swap(emitter);      /* SWAP */
      emit_sub(emitter);       /* SUB */
      emit_jneg(emitter, l1);    /* JNEG L1 */
      emit_push(emitter, 1);     /* PUSH 1 */
      emit_jmp(emitter, l2);     /* JMP L2 */
      emit_label(emitter, l1);   /* :L1 */
      emit_push(emitter, 0);     /* PUSH 0 */
      emit_label(emitter, l2);   /* :L2 */
    }
    break;
  case BOP_GT: /* x > y --> y - x < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_swap(emitter);      /* SWAP */
      emit_sub(emitter);       /* SUB */
      emit_jneg(emitter, l1);    /* JNEG L1 */
      emit_push(emitter, 0);     /* PUSH 0 */
      emit_jmp(emitter, l2);     /* JMP L2 */
      emit_label(emitter, l1);   /* :L1 */
      emit_push(emitter, 1);     /* PUSH 1 */
      emit_label(emitter, l2);   /* :L2 */
    }
    break;
  case BOP_GE: /* x >= y --> !(x - y < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      emit_sub(emitter);       /* SUB */
      emit_jneg(emitter, l1);    /* JNEG L1 */
      emit_push(emitter, 1);     /* PUSH 1 */
      emit_jmp(emitter, l2);     /* JMP L2 */
      emit_label(emitter, l1);   /* :L1 */
      emit_push(emitter, 0);     /* PUSH 0 */
      emit_label(emitter, l2);   /* :L2 */
    }
    break;
  default:
    break;
  }
}

static void gen_assign(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  node_t *lhs = node_get_l(node);
  node_t *expr = node_get_r(node);

  gen(codegen, expr);

  switch (node_get_ntype(lhs)) {
  case NT_VARIABLE:
    {
      int var_index = get_var_index(codegen, node_get_name(lhs));
      emit_push(emitter, var_index);
    }
    break;
  case NT_ARRAY:
    {
      int var_index = get_var_index(codegen, node_get_name(node_get_l(lhs)));
      emit_push(emitter, var_index);
      gen(codegen, node_get_r(lhs));
      emit_add(emitter);
    }
    break;
  default:
    fprintf(stderr, "error: invalid left hand value. (ntype=%d)\n", node_get_ntype(lhs));
    return;
  }

  emit_copy(emitter, 1);
  emit_store(emitter);
}

static void gen_variable(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  int var_index = get_var_index(codegen, node_get_name(node));

  emit_push(emitter, var_index);
  emit_load(emitter);
}

static void gen_array(codegen_t *codegen, node_t *node) {
  emitter_t *emitter = codegen->emitter;
  int var_index = get_var_index(codegen, node_get_name(node_get_l(node)));

  emit_push(emitter, var_index);
  gen(codegen, node_get_r(node));
  emit_add(emitter);
  emit_load(emitter);
}

static int alloc_label_id(codegen_t *codegen) {
  return codegen->label_count++;
}

static int get_var_index(codegen_t *codegen, const char *name) {
  int i;
  for (i = 0; i < codegen->var_count; ++i) {
    if (strcmp(codegen->vars[i], name) == 0) {
      return i;
    }
  }

  /* register name */
  return allocate(codegen, name, 1);
}

static int allocate(codegen_t *codegen, const char *name, int size) {
  int i = codegen->var_count;
  strcpy(codegen->vars[i], name);
  codegen->var_count += size;
  return i;
}
