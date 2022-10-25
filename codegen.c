#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "node.h"
#include "operator.h"

struct codegen_t {
  node_t *root;
  int     label_count;
};

static void gen(codegen_t *codegen, node_t *node);
static void gen_if_statement(codegen_t *codegen, node_t *node);
static void gen_unary(codegen_t *codegen, node_t *node);
static void gen_assign(codegen_t *codegen, node_t *node);
static void gen_arith(codegen_t *codegen, node_t *node);
static void encode_integer(int n);
static void encode_uint(unsigned int n);
static void encode_uint_rec(unsigned int n);
static int  alloc_label_id(codegen_t *codegen);
static void gen_label(int label_id);

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
  default:
    break;
  }
}

static void gen_if_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node), *then = node_get_l(node), *els = node_get_r(node);
  int l1 = alloc_label_id(codegen), l2 = alloc_label_id(codegen);

  gen(codegen, cond);

  /* IF FALSE GOTO L1 */
  printf("LTS");
  encode_uint((unsigned int)l1);

  gen(codegen, then);

  /* GOTO L2 */
  printf("LSL");
  encode_uint((unsigned int)l2);

  /* L1 */
  gen_label(l1);

  if (els) {
    gen(codegen, els);
  }

  /* L2 */
  gen_label(l2);
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
  gen(codegen, node_get_r(node));
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
  default:
    break;
  }
}

static void encode_integer(int n) {
  putchar(n >= 0 ? 'S' : 'T');
  encode_uint_rec((unsigned int)abs(n));
  putchar('L');
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
