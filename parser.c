#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "operator.h"
#include "node.h"

struct parser_t {
  lexer_t *lexer;
};

static binary_op_t ttype_to_binary_op(ttype_t ttype);
static node_t *parse_block(parser_t *parser);
static node_t *parse_statement(parser_t *parser);
static node_t *parse_puti(parser_t *parser);
static node_t *parse_putc(parser_t *parser);
static node_t *parse_expr(parser_t *parser);
static node_t *parse_assign(parser_t *parser);
static node_t *parse_addsub(parser_t *parser);
static node_t *parse_muldiv(parser_t *parser);
static node_t *parse_atomic(parser_t *parser);

parser_t *parser_new(FILE *input) {
  parser_t *parser = (parser_t *)malloc(sizeof(parser_t));
  parser->lexer = lexer_new(input);
  return parser;
}

void parser_release(parser_t **pparser) {
  lexer_release(&(*pparser)->lexer);
  free(*pparser);
  *pparser = NULL;
}

node_t *parser_parse(parser_t *parser) {
  lexer_succ(parser->lexer);
  lexer_next(parser->lexer);
  return parse_statement(parser);
}

static binary_op_t ttype_to_binary_op(ttype_t ttype) {
  switch (ttype) {
  case TT_EQ:       return BOP_ASSIGN;
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

static node_t *parse_block(parser_t *parser) {
  node_t *root = NULL, *node = NULL;

  if (lexer_ttype(parser->lexer) == TT_LBRACE) {
    lexer_next(parser->lexer);
  }

  while (lexer_ttype(parser->lexer) != TT_RBRACE) {
    node = parse_statement(parser);
    if (!root) {
      root = node;
    }
    else {
      root = node_new_seq(root, node);
    }
  }

  if (lexer_ttype(parser->lexer) == TT_RBRACE) {
    lexer_next(parser->lexer);
  }

  return root;
}

static node_t *parse_statement(parser_t *parser) {
  node_t *node = NULL;

  switch (lexer_ttype(parser->lexer)) {
  case TT_LBRACE:
    return parse_block(parser);
  case TT_KW_PUTI:
    node = parse_puti(parser);
    break;
  case TT_KW_PUTC:
    node = parse_putc(parser);
    break;
  default:
    node = parse_expr(parser);
    break;
  }

  if (lexer_ttype(parser->lexer) == TT_SEMICOLON) {
    lexer_next(parser->lexer);
  }

  return node;
}

static node_t *parse_puti(parser_t *parser) {
  node_t *node;
  lexer_next(parser->lexer);
  node = node_new_puti(parse_expr(parser));
  if (lexer_ttype(parser->lexer) == TT_SEMICOLON) {
    lexer_next(parser->lexer);
  }
  return node;
}

static node_t *parse_putc(parser_t *parser) {
  node_t *node;
  lexer_next(parser->lexer);
  node = node_new_putc(parse_expr(parser));
  if (lexer_ttype(parser->lexer) == TT_SEMICOLON) {
    lexer_next(parser->lexer);
  }
  return node;
}

static node_t *parse_expr(parser_t *parser) {
  node_t *expr = parse_assign(parser);
  if (lexer_ttype(parser->lexer) == TT_SEMICOLON) {
    lexer_next(parser->lexer);
  }
  return expr;
}

static node_t *parse_assign(parser_t *parser) {
  node_t *x, *y;
  x = parse_addsub(parser);
  if (lexer_ttype(parser->lexer) == TT_EQ) {
    lexer_next(parser->lexer);
    y = parse_assign(parser);
    x = node_new_binary(BOP_ASSIGN, x, y);
  }
  return x;
}

static node_t *parse_addsub(parser_t *parser) {
  node_t *x, *y;
  x = parse_muldiv(parser);
  while (lexer_ttype(parser->lexer) == TT_PLUS || lexer_ttype(parser->lexer) == TT_MINUS) {
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
  while (lexer_ttype(parser->lexer) == TT_ASTERISK || lexer_ttype(parser->lexer) == TT_SLASH || lexer_ttype(parser->lexer) == TT_PERCENT) {
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

  switch (lexer_ttype(parser->lexer)) {
  case TT_INTEGER:
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
  case TT_SYMBOL:
    node = node_new_variable(lexer_text(parser->lexer));
    lexer_next(parser->lexer);
    return node;
  case TT_LPAREN:
    lexer_next(parser->lexer);
    node = parse_expr(parser);
    lexer_next(parser->lexer);
    return node;
  default:
    break;
  }

  printf("unexpected: ttype=%d\n", lexer_ttype(parser->lexer));
  return NULL;
}
