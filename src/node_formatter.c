#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include "node.h"

static void print_indent(int indent, uint64_t mask);
static void indent_puts(int indent, uint64_t mask, const char *s);
static void indent_printf(int indent, uint64_t mask, const char *fmt, ...);
static void dump_rec(node_t *node, int indent, uint64_t mask);
static void dump_children(node_t *node, int indent, uint64_t mask);

void node_dump_tree(node_t *node) {
  dump_rec(node, 0, 0);
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

static void dump_rec(node_t *node, int indent, uint64_t mask) {
  uint64_t mask0 = mask << 1;

  switch (node_get_ntype(node)) {
  case NT_INVALID:
    indent_puts(indent, mask, "Invalid");
    break;
  case NT_GROUP:
    indent_puts(indent, mask, node_get_name(node));
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
    indent_printf(indent, mask, "Unary %s\n", unary_op_to_string(node_get_uop(node)));
    break;
  case NT_BINARY:
    indent_printf(indent, mask, "Binary %s\n", binary_op_to_string(node_get_bop(node)));
    break;
  case NT_ASSIGN:
    indent_puts(indent, mask, "Assign");
    break;
  case NT_INTEGER:
    indent_printf(indent, mask, "Integer %d\n", node_get_value(node));
    break;
  case NT_IDENT:
    indent_printf(indent, mask, "Ident %s\n", node_get_name(node));
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

static void dump_children(node_t *node, int indent, uint64_t mask) {
  int count = node_get_child_count(node);
  for (int i = 0; i < count; ++i) {
    bool is_last = i == count - 1;
    dump_rec(node_get_child(node, i), indent, mask | !is_last);
  }
}
