#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "operator.h"
#include "node.h"

struct parser_t {
  lexer_t *lexer;
  int      error_count;
};

static binary_op_t ttype_to_binary_op(ttype_t ttype);
static int     is_eof(parser_t *parser);
static int     is_ttype(parser_t *parser, ttype_t ttype);
static node_t *parse_program(parser_t *parser);
static node_t *parse_block(parser_t *parser);
static node_t *parse_statement(parser_t *parser);
static node_t *parse_if_statement(parser_t *parser);
static node_t *parse_while_statement(parser_t *parser);
static node_t *parse_break_statement(parser_t *parser);
static node_t *parse_puti(parser_t *parser);
static node_t *parse_putc(parser_t *parser);
static node_t *parse_geti(parser_t *parser);
static node_t *parse_getc(parser_t *parser);
static node_t *parse_array_statement(parser_t *parser);
static node_t *parse_halt_statement(parser_t *parser);
static node_t *parse_expr_statement(parser_t *parser);
static node_t *parse_expr(parser_t *parser);
static node_t *parse_assign(parser_t *parser);
static node_t *parse_or(parser_t *parser);
static node_t *parse_and(parser_t *parser);
static node_t *parse_comparison(parser_t *parser);
static node_t *parse_addsub(parser_t *parser);
static node_t *parse_muldiv(parser_t *parser);
static node_t *parse_atomic(parser_t *parser);
static node_t *parse_variable(parser_t *parser);
static node_t *parse_array_indexer(parser_t *parser);
static node_t *parse_ident(parser_t *parser);

parser_t *parser_new(FILE *input) {
  parser_t *parser = (parser_t *)malloc(sizeof(parser_t));
  parser->lexer = lexer_new(input);
  parser->error_count = 0;
  return parser;
}

void parser_release(parser_t **pparser) {
  lexer_release(&(*pparser)->lexer);
  free(*pparser);
  *pparser = NULL;
}

node_t *parser_parse(parser_t *parser) {
  lexer_next(parser->lexer);
  return parse_program(parser);
}

int parser_get_total_error_count(parser_t *parser) {
  return lexer_get_error_count(parser->lexer) + parser->error_count;
}

static binary_op_t ttype_to_binary_op(ttype_t ttype) {
  switch (ttype) {
  case TT_EQEQ:     return BOP_EQ;
  case TT_EXCLA:    return BOP_NEQ;
  case TT_LT:       return BOP_LT;
  case TT_LE:       return BOP_LE;
  case TT_GT:       return BOP_GT;
  case TT_GE:       return BOP_GE;
  case TT_PLUS:     return BOP_ADD;
  case TT_MINUS:    return BOP_SUB;
  case TT_ASTERISK: return BOP_MUL;
  case TT_SLASH:    return BOP_DIV;
  case TT_PERCENT:  return BOP_MOD;
  case TT_AMP:      return BOP_AND;
  case TT_BAR:      return BOP_OR;
  default:          return BOP_INVALID;
  }
}

static int is_eof(parser_t *parser) {
  return is_ttype(parser, TT_EOF);
}

static int is_ttype(parser_t *parser, ttype_t ttype) {
  return lexer_ttype(parser->lexer) == ttype;
}

static int expect(parser_t *parser, ttype_t ttype) {
  location_t location;

  if (is_ttype(parser, ttype)) {
    lexer_next(parser->lexer);
    return 1;
  }

  location = lexer_get_location(parser->lexer);
  fprintf(stderr, "error: unexpected %s, but expected %s. (line:%d,column:%d)\n",
          ttype_to_string(lexer_ttype(parser->lexer)),
          ttype_to_string(ttype),
          location.line,
          location.column);
  ++parser->error_count;
  return 0;
}

static node_t *parse_program(parser_t *parser) {
  node_t *root = node_new_empty(), *node = NULL;
  while (!is_eof(parser)) {
    node = parse_statement(parser);
    root = node_new_seq(root, node);
  }
  return root;
}

static node_t *parse_block(parser_t *parser) {
  node_t *root = node_new_empty(), *node = NULL;

  expect(parser, TT_LBRACE);
  while (!is_eof(parser) && !is_ttype(parser, TT_RBRACE)) {
    node = parse_statement(parser);
    root = node_new_seq(root, node);
  }
  expect(parser, TT_RBRACE);

  return root;
}

static node_t *parse_statement(parser_t *parser) {
  switch (lexer_ttype(parser->lexer)) {
  case TT_LBRACE:
    return parse_block(parser);
  case TT_KW_IF:
    return parse_if_statement(parser);
  case TT_KW_WHILE:
    return parse_while_statement(parser);
  case TT_KW_BREAK:
    return parse_break_statement(parser);
  case TT_KW_PUTI:
    return parse_puti(parser);
  case TT_KW_PUTC:
    return parse_putc(parser);
  case TT_KW_GETI:
    return parse_geti(parser);
  case TT_KW_GETC:
    return parse_getc(parser);
  case TT_KW_ARRAY:
    return parse_array_statement(parser);
  case TT_KW_HALT:
    return parse_halt_statement(parser);
  default:
    return parse_expr_statement(parser);
  }
}

static node_t *parse_if_statement(parser_t *parser) {
  node_t *cond = NULL, *then = NULL, *els = NULL;

  expect(parser, TT_KW_IF);
  expect(parser, TT_LPAREN);
  cond = parse_expr(parser);
  expect(parser, TT_RPAREN);
  then = parse_statement(parser);

  if (is_ttype(parser, TT_KW_ELSE)) {
    lexer_next(parser->lexer);
    els = parse_statement(parser);
  }

  return node_new_if(cond, then, els);
}

static node_t *parse_while_statement(parser_t *parser) {
  node_t *cond = NULL, *body = NULL;

  expect(parser, TT_KW_WHILE);
  expect(parser, TT_LPAREN);
  cond = parse_expr(parser);
  expect(parser, TT_RPAREN);
  body = parse_statement(parser);

  return node_new_while(cond, body);
}

static node_t *parse_break_statement(parser_t *parser) {
  expect(parser, TT_KW_BREAK);
  expect(parser, TT_SEMICOLON);
  return node_new_break();
}

static node_t *parse_puti(parser_t *parser) {
  node_t *node;
  expect(parser, TT_KW_PUTI);
  node = node_new_puti(parse_expr(parser));
  expect(parser, TT_SEMICOLON);
  return node;
}

static node_t *parse_putc(parser_t *parser) {
  node_t *node;
  expect(parser, TT_KW_PUTC);
  node = node_new_putc(parse_expr(parser));
  expect(parser, TT_SEMICOLON);
  return node;
}

/*
 * <<GetIStatement>> ::= 'geti' <Variable> ';'
 */
static node_t *parse_geti(parser_t *parser) {
  node_t *node;
  expect(parser, TT_KW_GETI);
  node = node_new_geti(parse_variable(parser));
  expect(parser, TT_SEMICOLON);
  return node;
}

/*
 * <<GetCStatement>> ::= 'getc' <Variable> ';'
 */
static node_t *parse_getc(parser_t *parser) {
  node_t *node;
  expect(parser, TT_KW_GETC);
  node = node_new_getc(parse_variable(parser));
  expect(parser, TT_SEMICOLON);
  return node;
}

static node_t *parse_array_statement(parser_t *parser) {
  node_t *ident;
  int size;

  expect(parser, TT_KW_ARRAY);

  ident = parse_ident(parser);

  expect(parser, TT_LBRACKET);

  size = lexer_int_value(parser->lexer);
  lexer_next(parser->lexer);

  expect(parser, TT_RBRACKET);
  expect(parser, TT_SEMICOLON);

  return node_new_array_decl(ident, size);
}

static node_t *parse_halt_statement(parser_t *parser) {
  expect(parser, TT_KW_HALT);
  expect(parser, TT_SEMICOLON);
  return node_new_halt();
}

static node_t *parse_expr_statement(parser_t *parser) {
  node_t *expr = parse_expr(parser);
  expect(parser, TT_SEMICOLON);
  return node_new_expr(expr);
}

static node_t *parse_expr(parser_t *parser) {
  return parse_assign(parser);
}

static node_t *parse_assign(parser_t *parser) {
  node_t *x, *y;
  location_t location = lexer_get_location(parser->lexer);

  x = parse_or(parser);
  if (is_ttype(parser, TT_EQ)) {
    lexer_next(parser->lexer);

    if (!node_is_assignable(x)) {
      fprintf(stderr, "error: left hand side of assignment should be variable or array. (line:%d,column:%d)\n", location.line, location.column);
      ++parser->error_count;
    }

    y = parse_assign(parser);
    x = node_new_assign(x, y);
  }
  return x;
}

static node_t *parse_or(parser_t *parser) {
  node_t *x, *y;
  x = parse_and(parser);
  while (is_ttype(parser, TT_BAR)) {
    lexer_next(parser->lexer);
    y = parse_and(parser);
    x = node_new_binary(BOP_OR, x, y);
  }
  return x;
}

static node_t *parse_and(parser_t *parser) {
  node_t *x, *y;
  x = parse_comparison(parser);
  while (is_ttype(parser, TT_AMP)) {
    lexer_next(parser->lexer);
    y = parse_comparison(parser);
    x = node_new_binary(BOP_AND, x, y);
  }
  return x;
}

static node_t *parse_comparison(parser_t *parser) {
  node_t *x, *y;
  x = parse_addsub(parser);
  while (is_ttype(parser, TT_EQEQ) ||
         is_ttype(parser, TT_EXCLAEQ) ||
         is_ttype(parser, TT_LT) ||
         is_ttype(parser, TT_LE) ||
         is_ttype(parser, TT_GT) ||
         is_ttype(parser, TT_GE)) {
    binary_op_t bop = ttype_to_binary_op(lexer_ttype(parser->lexer));
    lexer_next(parser->lexer);
    y = parse_addsub(parser);
    x = node_new_binary(bop, x, y);
  }
  return x;
}

static node_t *parse_addsub(parser_t *parser) {
  node_t *x, *y;
  x = parse_muldiv(parser);
  while (is_ttype(parser, TT_PLUS) || is_ttype(parser, TT_MINUS)) {
    binary_op_t bop = ttype_to_binary_op(lexer_ttype(parser->lexer));
    lexer_next(parser->lexer);
    y = parse_muldiv(parser);
    x = node_new_binary(bop, x, y);
  }
  return x;
}

static node_t *parse_muldiv(parser_t *parser) {
  node_t *x, *y;
  x = parse_atomic(parser);
  while (is_ttype(parser, TT_ASTERISK) || is_ttype(parser, TT_SLASH) || is_ttype(parser, TT_PERCENT)) {
    binary_op_t bop = ttype_to_binary_op(lexer_ttype(parser->lexer));
    lexer_next(parser->lexer);
    y = parse_atomic(parser);
    x = node_new_binary(bop, x, y);
  }
  return x;
}

static node_t *parse_atomic(parser_t *parser) {
  node_t *node = NULL;
  int value;
  location_t location;

  switch (lexer_ttype(parser->lexer)) {
  case TT_INTEGER:
  case TT_CHAR:
    value = lexer_int_value(parser->lexer);
    lexer_next(parser->lexer);
    return node_new_integer(value);
  case TT_PLUS:
    /* simply ignore */
    lexer_next(parser->lexer);
    return parse_atomic(parser);
  case TT_MINUS:
    lexer_next(parser->lexer);
    node = parse_atomic(parser);
    return node_new_unary(UOP_NEGATIVE, node);
  case TT_EXCLA:
    lexer_next(parser->lexer);
    node = parse_atomic(parser);
    return node_new_unary(UOP_NOT, node);
  case TT_SYMBOL:
    node = parse_variable(parser);
    if (is_ttype(parser, TT_LBRACKET)) {
      node = node_new_array(node, parse_array_indexer(parser));
    }
    return node;
  case TT_LPAREN:
    expect(parser, TT_LPAREN);
    node = parse_expr(parser);
    expect(parser, TT_RPAREN);
    return node;
  default:
    break;
  }

  location = lexer_get_location(parser->lexer);
  fprintf(stderr, "error: unexpected %s. (line:%d,column:%d)\n", ttype_to_string(lexer_ttype(parser->lexer)), location.line, location.column);
  lexer_next(parser->lexer);
  ++parser->error_count;
  return node_new_invalid();
}

static node_t *parse_variable(parser_t *parser) {
  node_t *node = NULL;
  if (is_ttype(parser, TT_SYMBOL)) {
    node = node_new_variable(lexer_text(parser->lexer));
    lexer_next(parser->lexer);
  }
  return node;
}

static node_t *parse_array_indexer(parser_t *parser) {
  node_t *indexer = NULL;
  expect(parser, TT_LBRACKET);
  indexer = parse_expr(parser);
  expect(parser, TT_RBRACKET);
  return indexer;
}

static node_t *parse_ident(parser_t *parser) {
  if (is_ttype(parser, TT_SYMBOL)) {
    node_t *node = node_new_ident(lexer_text(parser->lexer));
    lexer_next(parser->lexer);
    return node;
  }
  return node_new_invalid();
}
