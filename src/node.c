#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

static void print_indent(int indent, uint64_t mask) {
  if (indent == 0) {
    return;
  }

  for (int i = 0; i < indent - 1; ++i) {
    bool b = (mask >> (indent - 1 - i)) & 1;
    putchar(b ? '|' : ' ');
    putchar(' ');
  }
  putchar('+');
  putchar('-');
}

static void indent_puts(int indent, uint64_t mask, const char *s) {
  print_indent(indent, mask);
  puts(s);
}

static void indent_printf(int indent, uint64_t mask, const char *fmt, ...) {
  va_list args;

  print_indent(indent, mask);
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

static void dump_rec(node_t *node, int indent, uint64_t mask);

static void dump_children(node_t *node, int indent, uint64_t mask) {
  int count = node_get_child_count(node);
  for (int i = 0; i < count; ++i) {
    bool is_last = i == count - 1;
    dump_rec(node_get_child(node, i), indent, mask | !is_last);
  }
}

static void dump_rec(node_t *node, int indent, uint64_t mask) {
  uint64_t mask0 = mask << 1;

  switch (node->ntype) {
  case NT_INVALID:
    indent_puts(indent, mask, "Invalid");
    break;
  case NT_GROUP:
    indent_puts(indent, mask, node->name);
    break;
  case NT_EMPTY:
    break;
  case NT_SEQ:
    dump_children(node, indent, mask);
    return;
  case NT_EXPR:
    indent_puts(indent, mask, "Expr");
    break;
  case NT_UNARY:
    indent_printf(indent, mask, "Unary %s\n", unary_op_to_string(node->uop));
    break;
  case NT_BINARY:
    indent_printf(indent, mask, "Binary %s\n", binary_op_to_string(node->bop));
    break;
  case NT_ASSIGN:
    indent_puts(indent, mask, "Assign");
    break;
  case NT_INTEGER:
    indent_printf(indent, mask, "Integer %d\n", node->value);
    break;
  case NT_IDENT:
    indent_printf(indent, mask, "Ident %s\n", node->name);
    break;
  case NT_VARIABLE:
    indent_puts(indent, mask, "Variable");
    break;
  case NT_ARRAY:
    indent_puts(indent, mask, "Array");
    break;
  case NT_FUNC_CALL:
    indent_puts(indent, mask, "FuncCall");
    break;
  case NT_FUNC_CALL_ARG:
    indent_puts(indent, mask, "FuncCallArg");
    break;
  case NT_IF:
    indent_puts(indent, mask, "If-Statement");
    break;
  case NT_WHILE:
    indent_puts(indent, mask, "While-Statement");
    break;
  case NT_LOOP_STATEMENT:
    indent_puts(indent, mask, "Loop-Statement");
    break;
  case NT_FOR_STATEMENT:
    indent_puts(indent, mask, "For-Statement");
    break;
  case NT_BREAK:
    indent_puts(indent, mask, "Break-Statement");
    break;
  case NT_CONTINUE:
    indent_puts(indent, mask, "Continue-Statement");
    break;
  case NT_PUTI:
    indent_puts(indent, mask, "Puti-Statement");
    break;
  case NT_PUTC:
    indent_puts(indent, mask, "Putc-Statement");
    break;
  case NT_GETI:
    indent_puts(indent, mask, "Geti-Statement");
    break;
  case NT_GETC:
    indent_puts(indent, mask, "Getc-Statement");
    break;
  case NT_ARRAY_DECL:
    indent_puts(indent, mask, "ArrayDecl-Statement");
    break;
  case NT_RETURN:
    indent_puts(indent, mask, "Return");
    break;
  case NT_HALT:
    indent_puts(indent, mask, "Halt-Statement");
    break;
  case NT_FUNC:
    indent_puts(indent, mask, "Func");
    break;
  case NT_FUNC_PARAM:
    indent_puts(indent, mask, "FuncParam");
    break;
  case NT_CONST_STATEMENT:
    indent_puts(indent, mask, "Const-Statement");
    break;
  }
  dump_children(node, indent + 1, mask0);
}

void node_dump_tree(node_t *node) {
  dump_rec(node, 0, 0);
}
