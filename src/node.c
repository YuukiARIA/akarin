#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "node.h"
#include "utils/memory.h"
#include "utils/array.h"

#define VARIABLE_NAME_MAX         ( 63 )
#define INITIAL_CHILDREN_CAPACITY ( 4 )

struct node_t {
  ntype_t      ntype;
  unary_op_t   uop;
  binary_op_t  bop;
  int          value;
  char         name[VARIABLE_NAME_MAX + 1];
  array_t     *children;
};

node_t *node_new(ntype_t ntype) {
  node_t *node = (node_t *)AK_MEM_MALLOC(sizeof(node_t));
  node->ntype    = ntype;
  node->uop      = UOP_INVALID;
  node->bop      = BOP_INVALID;
  node->value    = 0;
  node->children = array_new(INITIAL_CHILDREN_CAPACITY);
  return node;
}

void node_release(node_t **pnode) {
  node_t *node = *pnode;

  for (int i = 0; i < node_get_child_count(node); ++i) {
    node_t *child = node_get_child(node, i);
    node_release(&child);
  }
  array_release(&node->children);

  AK_MEM_FREE(node);
  *pnode = NULL;
}

node_t *node_new_invalid(void) {
  return node_new(NT_INVALID);
}

node_t *node_new_group(node_t *child, const char *group_label) {
  node_t *node = node_new(NT_GROUP);
  node_add_child(node, child);
  strncpy(node->name, group_label, VARIABLE_NAME_MAX);
  return node;
}

node_t *node_new_empty(void) {
  return node_new(NT_EMPTY);
}

node_t *node_new_seq(void) {
  return node_new(NT_SEQ);
}

node_t *node_new_expr(node_t *expr) {
  node_t *node = node_new(NT_EXPR);
  node_add_child(node, expr);
  return node;
}

node_t *node_new_unary(unary_op_t uop, node_t *arg) {
  node_t *node = node_new(NT_UNARY);
  node->uop = uop;
  node_add_child(node, arg);
  return node;
}

node_t *node_new_binary(binary_op_t bop, node_t *lhs, node_t *rhs) {
  node_t *node = node_new(NT_BINARY);
  node->bop = bop;
  node_add_child(node, lhs);
  node_add_child(node, rhs);
  return node;
}

node_t *node_new_assign(node_t *lhs, node_t *rhs) {
  node_t *node = node_new(NT_ASSIGN);
  node_add_child(node, lhs);
  node_add_child(node, rhs);
  return node;
}

node_t *node_new_integer(int value) {
  node_t *node = node_new(NT_INTEGER);
  node->value = value;
  return node;
}

node_t *node_new_ident(const char *name) {
  node_t *node = node_new(NT_IDENT);
  strncpy(node->name, name, VARIABLE_NAME_MAX);
  return node;
}

node_t *node_new_variable(node_t *ident) {
  node_t *node = node_new(NT_VARIABLE);
  node_add_child(node, ident);
  return node;
}

node_t *node_new_array(node_t *ident, node_t *indexer) {
  node_t *node = node_new(NT_ARRAY);
  node_add_child(node, ident);
  node_add_child(node, indexer);
  return node;
}

node_t *node_new_func_call(node_t *ident, node_t *arg) {
  node_t *node = node_new(NT_FUNC_CALL);
  node_add_child(node, ident);
  node_add_child(node, arg);
  return node;
}

node_t *node_new_func_call_arg(void) {
  return node_new(NT_FUNC_CALL_ARG);
}

node_t *node_new_if(node_t *cond, node_t *then, node_t *els) {
  node_t *node = node_new(NT_IF);
  node_add_child(node, cond);
  node_add_child(node, then);
  if (els) {
    node_add_child(node, els);
  }
  return node;
}

node_t *node_new_while(node_t *cond, node_t *body) {
  node_t *node = node_new(NT_WHILE);
  node_add_child(node, cond);
  node_add_child(node, body);
  return node;
}

node_t *node_new_loop_statement(node_t *body) {
  node_t *node = node_new(NT_LOOP_STATEMENT);
  node_add_child(node, body);
  return node;
}

node_t *node_new_for_statement(node_t *init, node_t *cond, node_t *next, node_t *body) {
  node_t *node = node_new(NT_FOR_STATEMENT);
  node_add_child(node, init);
  node_add_child(node, cond);
  node_add_child(node, next);
  node_add_child(node, body);
  return node;
}

node_t *node_new_break(void) {
  return node_new(NT_BREAK);
}

node_t *node_new_continue(void) {
  return node_new(NT_CONTINUE);
}

node_t *node_new_puti(node_t *expr) {
  node_t *node = node_new(NT_PUTI);
  node_add_child(node, expr);
  return node;
}

node_t *node_new_putc(node_t *expr) {
  node_t *node = node_new(NT_PUTC);
  node_add_child(node, expr);
  return node;
}

node_t *node_new_geti(node_t *var) {
  node_t *node = node_new(NT_GETI);
  node_add_child(node, var);
  return node;
}

node_t *node_new_getc(node_t *var) {
  node_t *node = node_new(NT_GETC);
  node_add_child(node, var);
  return node;
}

node_t *node_new_array_decl(node_t *ident, node_t *capacity) {
  node_t *node = node_new(NT_ARRAY_DECL);
  node_add_child(node, ident);
  node_add_child(node, capacity);
  return node;
}

node_t *node_new_return(node_t *expr) {
  node_t *node = node_new(NT_RETURN);
  node_add_child(node, expr);
  return node;
}

node_t *node_new_halt(void) {
  return node_new(NT_HALT);
}

node_t *node_new_func(node_t *ident, node_t *param, node_t *body) {
  node_t *node = node_new(NT_FUNC);
  node_add_child(node, ident);
  node_add_child(node, param);
  node_add_child(node, body);
  return node;
}

node_t *node_new_func_param(void) {
  return node_new(NT_FUNC_PARAM);
}

node_t *node_new_const_statement(node_t *ident, node_t *value) {
  node_t *node = node_new(NT_CONST_STATEMENT);
  node_add_child(node, ident);
  node_add_child(node, value);
  return node;
}

void node_add_child(node_t *node, node_t *child) {
  array_append(node->children, child);
}

node_t *node_get_child(node_t *node, int i) {
  return (node_t *)array_get(node->children, i);
}

int node_get_child_count(node_t *node) {
  return array_count(node->children);
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

int node_is_assignable(node_t *node) {
  return node->ntype == NT_VARIABLE || node->ntype == NT_ARRAY;
}

bool node_is_all_paths_ended_with_return(node_t *node) {
  switch (node->ntype) {
  case NT_SEQ:
    for (int i = 0; i < node_get_child_count(node); ++i) {
      node_t *child = node_get_child(node, i);
      if (node_is_all_paths_ended_with_return(child)) {
	return true;
      }
    }
    return false;
  case NT_IF:
    if (node_get_child_count(node) == 3) {
      node_t *then = node_get_child(node, 1);
      node_t *els = node_get_child(node, 2);
      return node_is_all_paths_ended_with_return(then) && node_is_all_paths_ended_with_return(els);
    }
    return false;
  case NT_RETURN:
    return true;
  default:
    return false;
  }
}
