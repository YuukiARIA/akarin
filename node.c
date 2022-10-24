#include <stdlib.h>
#include <string.h>
#include "node.h"

#define VARIABLE_NAME_MAX ( 63 )

struct node_t {
  ntype_t     ntype;
  unary_op_t  uop;
  binary_op_t bop;
  int         value;
  char        name[VARIABLE_NAME_MAX + 1];
  node_t     *l;
  node_t     *r;
};

node_t *node_new(ntype_t ntype) {
  node_t *node = (node_t *)malloc(sizeof(node_t));
  node->ntype = ntype;
  node->uop   = UOP_INVALID;
  node->bop   = BOP_INVALID;
  node->value = 0;
  node->l     = NULL;
  node->r     = NULL;
  return node;
}

void node_release(node_t **pnode) {
  node_t *node = *pnode;
  if (node->l) {
    node_release(&node->l);
  }
  if ((*pnode)->r) {
    node_release(&node->r);
  }
  free(node);
  *pnode = NULL;
}

node_t *node_new_unary(unary_op_t uop, node_t *arg) {
  node_t *node = node_new(NT_UNARY);
  node->uop = uop;
  node->l = arg;
  return node;
}

node_t *node_new_binary(binary_op_t bop, node_t *lhs, node_t *rhs) {
  node_t *node = node_new(NT_BINARY);
  node->bop = bop;
  node->l = lhs;
  node->r = rhs;
  return node;
}

node_t *node_new_integer(int value) {
  node_t *node = node_new(NT_INTEGER);
  node->value = value;
  return node;
}

node_t *node_new_variable(const char *name) {
  node_t *node = node_new(NT_VARIABLE);
  strncpy(node->name, name, VARIABLE_NAME_MAX);
  return node;
}

ntype_t node_get_ntype(node_t *node) {
  return node->ntype;
}

unary_op_t node_get_uop(node_t *node) {
  return node->uop;
}

binary_op_t node_get_bop(node_t *node) {
  return node->bop;
}

int node_get_value(node_t *node) {
  return node->value;
}

const char *node_get_name(node_t *node) {
  return node->name;
}

node_t *node_get_l(node_t *node) {
  return node->l;
}

node_t *node_get_r(node_t *node) {
  return node->r;
}
