#pragma once

#include "operator.h"

typedef enum {
  NT_INVALID,
  NT_SEQ,
  NT_EXPR,
  NT_UNARY,
  NT_BINARY,
  NT_ASSIGN,
  NT_INTEGER,
  NT_VARIABLE,
  NT_ARRAY,
  NT_IF,
  NT_WHILE,
  NT_BREAK,
  NT_PUTI,
  NT_PUTC,
  NT_GETI,
  NT_GETC,
  NT_ARRAY_DECL,
  NT_HALT,
} ntype_t;

typedef struct node_t node_t;

node_t     *node_new(ntype_t ntype);
node_t     *node_new_seq(node_t *first, node_t *second);
node_t     *node_new_expr(node_t *expr);
node_t     *node_new_unary(unary_op_t uop, node_t *arg);
node_t     *node_new_binary(binary_op_t bop, node_t *lhs, node_t *rhs);
node_t     *node_new_assign(node_t *lhs, node_t *rhs);
node_t     *node_new_integer(int value);
node_t     *node_new_variable(const char *name);
node_t     *node_new_array(node_t *var, node_t *indexer);
node_t     *node_new_if(node_t *cond, node_t *then, node_t *els);
node_t     *node_new_while(node_t *cond, node_t *body);
node_t     *node_new_break(void);
node_t     *node_new_puti(node_t *expr);
node_t     *node_new_putc(node_t *expr);
node_t     *node_new_geti(node_t *var);
node_t     *node_new_getc(node_t *var);
node_t     *node_new_array_decl(node_t *var, int size);
node_t     *node_new_halt(void);

void        node_release(node_t **pnode);

ntype_t     node_get_ntype(node_t *node);
unary_op_t  node_get_uop(node_t *node);
binary_op_t node_get_bop(node_t *node);
int         node_get_value(node_t *node);
const char *node_get_name(node_t *node);
node_t     *node_get_l(node_t *node);
node_t     *node_get_r(node_t *node);
node_t     *node_get_cond(node_t *node);
