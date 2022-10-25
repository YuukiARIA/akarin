#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "node.h"
#include "operator.h"

struct codegen_t {
  node_t *root;
};

static void gen(node_t *node);
static void gen_unary(node_t *node);
static void gen_assign(node_t *node);
static void gen_arith(node_t *node);
static void encode_integer(int n);
static void encode_uint_rec(unsigned int n);

codegen_t *codegen_new(node_t *root) {
  codegen_t *codegen = (codegen_t *)malloc(sizeof(codegen_t));
  codegen->root = root;
  return codegen;
}

void codegen_release(codegen_t **pcodegen) {
  free(*pcodegen);
  *pcodegen = NULL;
}

void codegen_generate(codegen_t *codegen) {
  gen(codegen->root);
  puts("LLL");
}

static void gen(node_t *node) {
  switch (node_get_ntype(node)) {
  case NT_SEQ:
    gen(node_get_l(node));
    gen(node_get_r(node));
    break;
  case NT_PUTI:
    gen(node_get_l(node));
    printf("TLST");
    break;
  case NT_PUTC:
    gen(node_get_l(node));
    printf("TLSS");
    break;
  case NT_UNARY:
    gen_unary(node);
    break;
  case NT_BINARY:
    if (node_get_bop(node) == BOP_ASSIGN) {
      gen_assign(node);
    }
    else {
      gen_arith(node);
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

static void gen_unary(node_t *node) {
  switch (node_get_uop(node)) {
  case UOP_NEGATIVE: /* implement -x as 0 - x. */
    printf("SS");
    encode_integer(0);
    gen(node_get_l(node));
    printf("TSST");
    break;
  default:
    break;
  }
}

static void gen_assign(node_t *node) {
  gen(node_get_r(node));
}

static void gen_arith(node_t *node) {
  gen(node_get_l(node));
  gen(node_get_r(node));

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

static void encode_uint_rec(unsigned int n) {
  if (n != 0) {
    encode_uint_rec(n >> 1);
    putchar((n & 1) ? 'T' : 'S');
  }
}
