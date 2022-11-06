#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "utils/memory.h"

#define VARIABLE_NAME_MAX           ( 63 )
#define INITIAL_NODE_ARRAY_CAPACITY ( 4 )

typedef struct {
  int      count;
  int      capacity;
  node_t **nodes;
} node_array_t;

struct node_t {
  ntype_t       ntype;
  unary_op_t    uop;
  binary_op_t   bop;
  int           value;
  char          name[VARIABLE_NAME_MAX + 1];
  node_t       *l;
  node_t       *r;
  node_t       *cond;
  node_array_t *children;
};

static node_array_t *node_array_new(void) {
  node_array_t *node_array = (node_array_t *)AK_MEM_MALLOC(sizeof(node_array_t));
  node_array->count    = 0;
  node_array->capacity = INITIAL_NODE_ARRAY_CAPACITY;
  node_array->nodes    = (node_t **)AK_MEM_CALLOC(node_array->capacity, sizeof(node_array_t *));
  return node_array;
}

static void node_array_release(node_array_t **pnode_array) {
  AK_MEM_FREE((*pnode_array)->nodes);
  AK_MEM_FREE(*pnode_array);
  *pnode_array = NULL;
}

static void node_array_add(node_array_t *node_array, node_t *node) {
  if (node_array->count == node_array->capacity) {
    node_array->capacity *= 2;
    node_array->nodes = (node_t **)AK_MEM_REALLOC(node_array->nodes, node_array->capacity);
  }
  node_array->nodes[node_array->count++] = node;
}

node_t *node_new(ntype_t ntype) {
  node_t *node = (node_t *)AK_MEM_MALLOC(sizeof(node_t));
  node->ntype    = ntype;
  node->uop      = UOP_INVALID;
  node->bop      = BOP_INVALID;
  node->value    = 0;
  node->l        = NULL;
  node->r        = NULL;
  node->children = node_array_new();
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
  node_array_release(&node->children);
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

node_t *node_new_variable(const char *name) {
  node_t *node = node_new(NT_VARIABLE);
  strncpy(node->name, name, VARIABLE_NAME_MAX);
  return node;
}

node_t *node_new_array(node_t *var, node_t *indexer) {
  node_t *node = node_new(NT_ARRAY);
  node->l = var;
  node->r = indexer;
  return node;
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

node_t *node_new_break(void) {
  return node_new(NT_BREAK);
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

node_t *node_new_halt(void) {
  return node_new(NT_HALT);
}

void node_add_child(node_t *node, node_t *child) {
  node_array_add(node->children, child);
}

node_t *node_get_child(node_t *node, int i) {
  return node->children->nodes[i];
}

int node_get_child_count(node_t *node) {
  return node->children->count;
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
    print_indent(indent);
    printf("Variable %s\n", node->name);
    break;
  case NT_ARRAY:
    puts_indent(indent, "Array");
    dump_rec(node->l, indent + 1);
    dump_rec(node->r, indent + 1);
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
  case NT_BREAK:
    puts_indent(indent, "Break-Statement");
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
  case NT_HALT:
    puts_indent(indent, "Halt-Statement");
    break;
  }
}

void node_dump_tree(node_t *node) {
  dump_rec(node, 0);
}
