#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"
#include "node.h"

struct codegen_t {
  node_t *root;
};

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
