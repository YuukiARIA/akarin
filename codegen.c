#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "node.h"
#include "operator.h"

struct codegen_t {
  node_t *root;
  int     label_count;
  int     var_count;
  char    vars[256][64];
};

static void gen(codegen_t *codegen, node_t *node);
static void gen_if_statement(codegen_t *codegen, node_t *node);
static void gen_while_statement(codegen_t *codegen, node_t *node);
static void gen_unary(codegen_t *codegen, node_t *node);
static void gen_assign(codegen_t *codegen, node_t *node);
static void gen_arith(codegen_t *codegen, node_t *node);
static void gen_push(int value);
static void gen_jmp(int label_id);
static void gen_jz(int label_id);
static void gen_jneg(int label_id);
static void encode_integer(int n);
static void encode_uint(unsigned int n);
static void encode_uint_rec(unsigned int n);
static int  alloc_label_id(codegen_t *codegen);
static void gen_label(int label_id);
static int  get_var_index(codegen_t *codegen, const char *name);

codegen_t *codegen_new(node_t *root) {
  codegen_t *codegen = (codegen_t *)malloc(sizeof(codegen_t));
  codegen->root = root;
  codegen->label_count = 0;
  return codegen;
}

void codegen_release(codegen_t **pcodegen) {
  free(*pcodegen);
  *pcodegen = NULL;
}

void codegen_generate(codegen_t *codegen) {
  gen(codegen, codegen->root);
  puts("LLL");
}

static void gen(codegen_t *codegen, node_t *node) {
  switch (node_get_ntype(node)) {
  case NT_SEQ:
    gen(codegen, node_get_l(node));
    gen(codegen, node_get_r(node));
    break;
  case NT_IF:
    gen_if_statement(codegen, node);
    break;
  case NT_WHILE:
    gen_while_statement(codegen, node);
    break;
  case NT_PUTI:
    gen(codegen, node_get_l(node));
    printf("TLST");
    break;
  case NT_PUTC:
    gen(codegen, node_get_l(node));
    printf("TLSS");
    break;
  case NT_UNARY:
    gen_unary(codegen, node);
    break;
  case NT_BINARY:
    if (node_get_bop(node) == BOP_ASSIGN) {
      gen_assign(codegen, node);
    }
    else {
      gen_arith(codegen, node);
    }
    break;
  case NT_INTEGER:
    printf("SS");
    encode_integer(node_get_value(node));
    break;
  case NT_VARIABLE:
    {
      int var_index = get_var_index(codegen, node_get_name(node));
      printf("SS");
      encode_integer(var_index);
      printf("TTT");
    }
    break;
  default:
    break;
  }
}

static void gen_if_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node);
  node_t *then = node_get_l(node);
  node_t *els = node_get_r(node);
  int l1 = alloc_label_id(codegen);
  int l2 = alloc_label_id(codegen);

  gen(codegen, cond);
  gen_jz(l1);
  gen(codegen, then);
  gen_jmp(l2);
  gen_label(l1);
  if (els) {
    gen(codegen, els);
  }
  gen_label(l2);
}

static void gen_while_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node);
  node_t *body = node_get_l(node);
  int label_head = alloc_label_id(codegen);
  int label_tail = alloc_label_id(codegen);

  gen_label(label_head);
  gen(codegen, cond);
  gen_jz(label_tail);
  gen(codegen, body);
  gen_jmp(label_head);
  gen_label(label_tail);
}

static void gen_unary(codegen_t *codegen, node_t *node) {
  switch (node_get_uop(node)) {
  case UOP_NEGATIVE: /* implement -x as 0 - x. */
    printf("SS");
    encode_integer(0);
    gen(codegen, node_get_l(node));
    printf("TSST");
    break;
  default:
    break;
  }
}

static void gen_assign(codegen_t *codegen, node_t *node) {
  node_t *var = node_get_l(node);
  node_t *expr = node_get_r(node);
  int var_index = get_var_index(codegen, node_get_name(var));

  printf("SS"); /* PUSH */
  encode_integer(var_index);
  gen(codegen, expr);
  printf("TTS"); /* STORE */
}

static void gen_arith(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  gen(codegen, node_get_r(node));

  switch (node_get_bop(node)) {
  case BOP_ADD:
    printf("TSSS");
    break;
  case BOP_SUB:
    printf("TSST");
    break;
  case BOP_MUL:
    printf("TSSL");
    break;
  case BOP_DIV:
    printf("TSTS");
    break;
  case BOP_MOD:
    printf("TSTT");
    break;
  case BOP_OR:
    break;
  case BOP_AND:
    break;
  case BOP_EQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      printf("TSST");  /* SUB */
      gen_jz(l1);      /* JZ L1 */
      gen_push(0);     /* PUSH 0 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(1);     /* PUSH 1 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_NEQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      printf("TSST");  /* SUB */
      gen_jz(l1);      /* JZ L1 */
      gen_push(1);     /* PUSH 1 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(0);     /* PUSH 0 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_LT: /* x < y --> x - y < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      printf("TSST");  /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(0);     /* PUSH 0 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(1);     /* PUSH 1 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_LE: /* x <= y --> !(y - x < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      printf("SLT");   /* SWAP */
      printf("TSST");  /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(1);     /* PUSH 1 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(0);     /* PUSH 0 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_GT: /* x > y --> y - x < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      printf("SLT");   /* SWAP */
      printf("TSST");  /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(0);     /* PUSH 0 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(1);     /* PUSH 1 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_GE: /* x >= y --> !(x - y < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      printf("TSST");  /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(1);     /* PUSH 1 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(0);     /* PUSH 0 */
      gen_label(l2);   /* :L2 */
    }
    break;
  default:
    break;
  }
}

static void gen_push(int value) {
  printf("SS");
  encode_integer(value);
}

static void gen_jmp(int label_id) {
  printf("LSL");
  encode_uint((unsigned int)label_id);
}

static void gen_jz(int label_id) {
  printf("LTS");
  encode_uint((unsigned int)label_id);
}

static void gen_jneg(int label_id) {
  printf("LTT");
  encode_uint((unsigned int)label_id);
}

static void encode_integer(int n) {
  putchar(n >= 0 ? 'S' : 'T');
  encode_uint((unsigned int)abs(n));
}

static void encode_uint(unsigned int n) {
  if (n == 0) {
    putchar('S');
  }
  else {
    encode_uint_rec(n);
  }
  putchar('L');
}

static void encode_uint_rec(unsigned int n) {
  if (n != 0) {
    encode_uint_rec(n >> 1);
    putchar((n & 1) ? 'T' : 'S');
  }
}

static int alloc_label_id(codegen_t *codegen) {
  return codegen->label_count++;
}

static void gen_label(int label_id) {
  printf("LSS");
  encode_uint((unsigned int)label_id);
}

static int get_var_index(codegen_t *codegen, const char *name) {
  int i;
  for (i = 0; i < codegen->var_count; ++i) {
    if (strcmp(codegen->vars[i], name) == 0) {
      return i;
    }
  }

  /* register name */
  i = codegen->var_count++;
  strcpy(codegen->vars[i], name);
  return i;
}
