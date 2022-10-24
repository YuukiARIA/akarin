#pragma once

#include "operator.h"

typedef enum {
  NT_INVALID,
  NT_UNARY,
  NT_BINARY,
  NT_INTEGER,
  NT_VARIABLE,
} ntype_t;

typedef struct node_t node_t;

node_t     *node_new(ntype_t ntype);
node_t     *node_new_unary(unary_op_t uop, node_t *arg);
node_t     *node_new_binary(binary_op_t bop, node_t *lhs, node_t *rhs);
node_t     *node_new_integer(int value);
node_t     *node_new_variable(const char *name);
void        node_release(node_t **pnode);

ntype_t     node_get_ntype(node_t *node);
unary_op_t  node_get_uop(node_t *node);
binary_op_t node_get_bop(node_t *node);
int         node_get_value(node_t *node);
node_t     *node_get_l(node_t *node);
node_t     *node_get_r(node_t *node);
