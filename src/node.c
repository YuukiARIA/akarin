#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "utils/memory.h"

#define VARIABLE_NAME_MAX         ( 63 )
#define INITIAL_CHILDREN_CAPACITY ( 4 )

struct node_t {
  ntype_t       ntype;
  unary_op_t    uop;
  binary_op_t   bop;
  int           value;
  char          name[VARIABLE_NAME_MAX + 1];
  node_t       *l;
  node_t       *r;
  node_t       *cond;
  int           children_count;
  int           children_capacity;
  node_t      **children;
};

node_t *node_new(ntype_t ntype) {
  node_t *node = (node_t *)AK_MEM_MALLOC(sizeof(node_t));
  node->ntype             = ntype;
  node->uop               = UOP_INVALID;
  node->bop               = BOP_INVALID;
  node->value             = 0;
  node->l                 = NULL;
  node->r                 = NULL;
  node->children_count    = 0;
  node->children_capacity = INITIAL_CHILDREN_CAPACITY;
  node->children          = (node_t **)AK_MEM_CALLOC(node->children_capacity, sizeof(node_t *));
  return node;
}

void node_release(node_t **pnode) {
  node_t *node = *pnode;
  if (node->l) {
    node_release(&node->l);
  }
  if (node->r) {
    node_release(&node->r);
  }
  if (node->cond) {
    node_release(&node->cond);
  }
  for (int i = 0; i < node->children_count; ++i) {
    node_release(&node->children[i]);
  }
  AK_MEM_FREE(node->children);
  AK_MEM_FREE(node);
  *pnode = NULL;
}

node_t *node_new_invalid(void) {
  return node_new(NT_INVALID);
}

node_t *node_new_empty(void) {
  return node_new(NT_EMPTY);
}

node_t *node_new_seq(node_t *first, node_t *second) {
  node_t *node = node_new(NT_SEQ);
  node->l = first;
  node->r = second;
  return node;
}

node_t *node_new_expr(node_t *expr) {
  node_t *node = node_new(NT_EXPR);
  node->l = expr;
  return node;
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

node_t *node_new_assign(node_t *lhs, node_t *rhs) {
  node_t *node = node_new(NT_ASSIGN);
  node->l = lhs;
  node->r = rhs;
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

node_t *node_new_array(node_t *var, node_t *indexer) {
  node_t *node = node_new(NT_ARRAY);
  node->l = var;
  node->r = indexer;
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
  node->cond = cond;
  node->l = then;
  node->r = els;
  return node;
}

node_t *node_new_while(node_t *cond, node_t *body) {
  node_t *node = node_new(NT_WHILE);
  node->cond = cond;
  node->l = body;
  return node;
}

node_t *node_new_loop_statement(node_t *body) {
  node_t *node = node_new(NT_LOOP_STATEMENT);
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
  node->l = expr;
  return node;
}

node_t *node_new_putc(node_t *expr) {
  node_t *node = node_new(NT_PUTC);
  node->l = expr;
  return node;
}

node_t *node_new_geti(node_t *var) {
  node_t *node = node_new(NT_GETI);
  node->l = var;
  return node;
}

node_t *node_new_getc(node_t *var) {
  node_t *node = node_new(NT_GETC);
  node->l = var;
  return node;
}

node_t *node_new_array_decl(node_t *ident, node_t *capacity) {
  node_t *node = node_new(NT_ARRAY_DECL);
  node->l = ident;
  node->r = capacity;
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
  if (node->children_count == node->children_capacity) {
    node->children_capacity *= 2;
    node->children = (node_t **)AK_MEM_REALLOC(node->children, sizeof(node_t *) * node->children_capacity);
  }
  node->children[node->children_count++] = child;
}

node_t *node_get_child(node_t *node, int i) {
  return node->children[i];
}

int node_get_child_count(node_t *node) {
  return node->children_count;
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

node_t *node_get_cond(node_t *node) {
  return node->cond;
}

int node_is_assignable(node_t *node) {
  return node->ntype == NT_VARIABLE || node->ntype == NT_ARRAY;
}

bool node_is_all_paths_ended_with_return(node_t *node) {
  switch (node->ntype) {
  case NT_SEQ:
    return node_is_all_paths_ended_with_return(node->r);
  case NT_IF:
    if (node->r) {
      return node_is_all_paths_ended_with_return(node->l) && node_is_all_paths_ended_with_return(node->r);
    }
    return false;
  case NT_RETURN:
    return true;
  default:
    return false;
  }
}

static void print_indent(int indent) {
  int i;

  if (indent == 0) {
    return;
  }

  for (i = 0; i < indent - 1; ++i) {
    putchar('|');
    putchar(' ');
  }
  putchar('+');
  putchar('-');
}

static void puts_indent(int indent, const char *s) {
  print_indent(indent);
  puts(s);
}

static void dump_rec(node_t *node, int indent) {
  switch (node->ntype) {
  case NT_INVALID:
    puts_indent(indent, "Invalid");
    break;
  case NT_EMPTY:
    break;
  case NT_SEQ:
    dump_rec(node->l, indent);
    dump_rec(node->r, indent);
    break;
  case NT_EXPR:
    puts_indent(indent, "Expr");
    dump_rec(node->l, indent + 1);
    break;
  case NT_UNARY:
    print_indent(indent);
    printf("Unary %s\n", unary_op_to_string(node->uop));
    dump_rec(node->l, indent + 1);
    break;
  case NT_BINARY:
    print_indent(indent);
    printf("Binary %s\n", binary_op_to_string(node->bop));
    dump_rec(node->l, indent + 1);
    dump_rec(node->r, indent + 1);
    break;
  case NT_ASSIGN:
    puts_indent(indent, "Assign");
    dump_rec(node->l, indent + 1);
    dump_rec(node->r, indent + 1);
  case NT_INTEGER:
    print_indent(indent);
    printf("Integer %d\n", node->value);
    break;
  case NT_IDENT:
    print_indent(indent);
    printf("Ident %s\n", node->name);
    break;
  case NT_VARIABLE:
    puts_indent(indent, "Variable");
    dump_rec(node->children[0], indent + 1);
    break;
  case NT_ARRAY:
    puts_indent(indent, "Array");
    dump_rec(node->l, indent + 1);
    dump_rec(node->r, indent + 1);
    break;
  case NT_FUNC_CALL:
    puts_indent(indent, "FuncCall");
    dump_rec(node->children[0], indent + 1);
    dump_rec(node->children[1], indent + 1);
    break;
  case NT_FUNC_CALL_ARG:
    puts_indent(indent, "FuncCallArg");
    for (int i = 0; i < node->children_count; ++i) {
      dump_rec(node->children[i], indent + 1);
    }
    break;
  case NT_IF:
    puts_indent(indent, "If-Statement");
    puts_indent(indent + 1, "Condition");
    dump_rec(node->cond, indent + 2);
    puts_indent(indent + 1, "Then-Clause");
    dump_rec(node->l, indent + 2);
    if (node->r) {
      puts_indent(indent + 1, "Else-Clause");
      dump_rec(node->r, indent + 2);
    }
    break;
  case NT_WHILE:
    puts_indent(indent, "While-Statement");
    puts_indent(indent + 1, "Condition");
    dump_rec(node->cond, indent + 2);
    puts_indent(indent + 1, "Body");
    dump_rec(node->l, indent + 2);
    break;
  case NT_LOOP_STATEMENT:
    puts_indent(indent, "Loop-Statement");
    dump_rec(node->children[0], indent + 1);
    break;
  case NT_BREAK:
    puts_indent(indent, "Break-Statement");
    break;
  case NT_CONTINUE:
    puts_indent(indent, "Continue-Statement");
    break;
  case NT_PUTI:
    puts_indent(indent, "Puti-Statement");
    dump_rec(node->l, indent + 1);
    break;
  case NT_PUTC:
    puts_indent(indent, "Putc-Statement");
    dump_rec(node->l, indent + 1);
    break;
  case NT_GETI:
    puts_indent(indent, "Geti-Statement");
    dump_rec(node->l, indent + 1);
    break;
  case NT_GETC:
    puts_indent(indent, "Getc-Statement");
    dump_rec(node->l, indent + 1);
    break;
  case NT_ARRAY_DECL:
    puts_indent(indent, "ArrayDecl-Statement");
    dump_rec(node->l, indent + 1);
    dump_rec(node->r, indent + 1);
    break;
  case NT_RETURN:
    puts_indent(indent, "Return");
    dump_rec(node->children[0], indent + 1);
    break;
  case NT_HALT:
    puts_indent(indent, "Halt-Statement");
    break;
  case NT_FUNC:
    puts_indent(indent, "Func");
    dump_rec(node->children[0], indent + 1);
    dump_rec(node->children[1], indent + 1);
    dump_rec(node->children[2], indent + 1);
    break;
  case NT_FUNC_PARAM:
    puts_indent(indent, "FuncParam");
    for (int i = 0; i < node->children_count; ++i) {
      dump_rec(node->children[i], indent + 1);
    }
    break;
  case NT_CONST_STATEMENT:
    puts_indent(indent, "Const-Statement");
    dump_rec(node->children[0], indent + 1);
    dump_rec(node->children[1], indent + 1);
    break;
  }
}

void node_dump_tree(node_t *node) {
  dump_rec(node, 0);
}
