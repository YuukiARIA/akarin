#pragma once

#include <stdbool.h>
#include "operator.h"

typedef enum {
  NT_INVALID,
  NT_EMPTY,
  NT_SEQ,
  NT_EXPR,
  NT_UNARY,
  NT_BINARY,
  NT_ASSIGN,
  NT_INTEGER,
  NT_IDENT,
  NT_VARIABLE,
  NT_ARRAY,
  NT_FUNC_CALL,
  NT_FUNC_CALL_ARG,
  NT_IF,
  NT_WHILE,
  NT_LOOP_STATEMENT,
  NT_BREAK,
  NT_CONTINUE,
  NT_PUTI,
  NT_PUTC,
  NT_GETI,
  NT_GETC,
  NT_ARRAY_DECL,
  NT_RETURN,
  NT_HALT,
  NT_FUNC,
  NT_FUNC_PARAM,
  NT_CONST_STATEMENT
} ntype_t;

typedef struct node_t node_t;

node_t     *node_new(ntype_t ntype);
node_t     *node_new_invalid(void);
node_t     *node_new_empty(void);
node_t     *node_new_seq(void);
node_t     *node_new_expr(node_t *expr);
node_t     *node_new_unary(unary_op_t uop, node_t *arg);
node_t     *node_new_binary(binary_op_t bop, node_t *lhs, node_t *rhs);
node_t     *node_new_assign(node_t *lhs, node_t *rhs);
node_t     *node_new_integer(int value);
node_t     *node_new_ident(const char *name);
node_t     *node_new_variable(node_t *ident);
node_t     *node_new_array(node_t *var, node_t *indexer);
node_t     *node_new_func_call(node_t *ident, node_t *arg);
node_t     *node_new_func_call_arg(void);
node_t     *node_new_if(node_t *cond, node_t *then, node_t *els);
node_t     *node_new_while(node_t *cond, node_t *body);
node_t     *node_new_loop_statement(node_t *body);
node_t     *node_new_break(void);
node_t     *node_new_continue(void);
node_t     *node_new_puti(node_t *expr);
node_t     *node_new_putc(node_t *expr);
node_t     *node_new_geti(node_t *var);
node_t     *node_new_getc(node_t *var);
node_t     *node_new_array_decl(node_t *ident, node_t *capacity);
node_t     *node_new_return(node_t *expr);
node_t     *node_new_halt(void);
node_t     *node_new_func(node_t *ident, node_t *param, node_t *body);
node_t     *node_new_func_param(void);
node_t     *node_new_const_statement(node_t *ident, node_t *value);

void        node_release(node_t **pnode);

void        node_add_child(node_t *node, node_t *child);
node_t     *node_get_child(node_t *node, int i);
int         node_get_child_count(node_t *node);
ntype_t     node_get_ntype(node_t *node);
unary_op_t  node_get_uop(node_t *node);
binary_op_t node_get_bop(node_t *node);
int         node_get_value(node_t *node);
const char *node_get_name(node_t *node);
node_t     *node_get_l(node_t *node);
node_t     *node_get_r(node_t *node);
node_t     *node_get_cond(node_t *node);
int         node_is_assignable(node_t *node);

bool        node_is_all_paths_ended_with_return(node_t *node);

void        node_dump_tree(node_t *node);
