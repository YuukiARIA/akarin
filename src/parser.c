#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "operator.h"
#include "node.h"
#include "utils/memory.h"

struct parser_t {
  lexer_t *lexer;
  int      error_count;
};

static int     is_eof(parser_t *parser);
static int     is_ttype(parser_t *parser, ttype_t ttype);
static node_t *parse_program(parser_t *parser);
static node_t *parse_toplevel_statement(parser_t *parser);
static node_t *parse_block(parser_t *parser);
static node_t *parse_statement(parser_t *parser);
static node_t *parse_if_statement(parser_t *parser);
static node_t *parse_while_statement(parser_t *parser);
static node_t *parse_loop_statement(parser_t *parser);
static node_t *parse_for_statement(parser_t *parser);
static node_t *parse_break_statement(parser_t *parser);
static node_t *parse_continue_statement(parser_t *parser);
static node_t *parse_puti(parser_t *parser);
static node_t *parse_putc(parser_t *parser);
static node_t *parse_geti(parser_t *parser);
static node_t *parse_getc(parser_t *parser);
static node_t *parse_array_statement(parser_t *parser);
static node_t *parse_return_statement(parser_t *parser);
static node_t *parse_halt_statement(parser_t *parser);
static node_t *parse_func_statement(parser_t *parser);
static node_t *parse_const_statement(parser_t *parser);
static node_t *parse_expr_statement(parser_t *parser);
static node_t *parse_expr(parser_t *parser);
static node_t *parse_assign(parser_t *parser);
static node_t *parse_or(parser_t *parser);
static node_t *parse_and(parser_t *parser);
static node_t *parse_comparison(parser_t *parser);
static node_t *parse_addsub(parser_t *parser);
static node_t *parse_muldiv(parser_t *parser);
static node_t *parse_atomic(parser_t *parser);
static node_t *parse_array_indexer(parser_t *parser);
static node_t *parse_func_call_arg(parser_t *parser);
static node_t *parse_ident(parser_t *parser);
static node_t *parse_integer(parser_t *parser);

parser_t *parser_new(FILE *input) {
  parser_t *parser = (parser_t *)AK_MEM_MALLOC(sizeof(parser_t));
  parser->lexer = lexer_new(input);
  parser->error_count = 0;
  return parser;
}

void parser_release(parser_t **pparser) {
  lexer_release(&(*pparser)->lexer);
  AK_MEM_FREE(*pparser);
  *pparser = NULL;
}

node_t *parser_parse(parser_t *parser) {
  lexer_next(parser->lexer);
  return parse_program(parser);
}

int parser_get_total_error_count(parser_t *parser) {
  return lexer_get_error_count(parser->lexer) + parser->error_count;
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
  fprintf(stderr, "error: unexpected '%s' (%s), but expected %s. (line:%d,column:%d)\n",
	  lexer_text(parser->lexer),
          ttype_to_string(lexer_ttype(parser->lexer)),
          ttype_to_string(ttype),
          location.line,
          location.column);
  ++parser->error_count;
  return 0;
}

static node_t *parse_program(parser_t *parser) {
  node_t *seq = node_new_seq();
  while (!is_eof(parser)) {
    node_add_child(seq, parse_toplevel_statement(parser));
  }
  return seq;
}

static node_t *parse_block(parser_t *parser) {
  node_t *seq = node_new_seq();
  expect(parser, TT_LBRACE);
  while (!is_eof(parser) && !is_ttype(parser, TT_RBRACE)) {
    node_add_child(seq, parse_statement(parser));
  }
  expect(parser, TT_RBRACE);
  return seq;
}

static node_t *parse_toplevel_statement(parser_t *parser) {
  location_t location;

  switch (lexer_ttype(parser->lexer)) {
  case TT_KW_ARRAY:
    return parse_array_statement(parser);
  case TT_KW_FUNC:
    return parse_func_statement(parser);
  case TT_KW_CONST:
    return parse_const_statement(parser);
  default:
    break;
  }

  location = lexer_get_location(parser->lexer);
  fprintf(stderr, "error: unexpected '%s' (%s). Only 'array', 'func' or 'const' are allowed as toplevel statement. (line:%d,column:%d)\n",
	  lexer_text(parser->lexer),
          ttype_to_string(lexer_ttype(parser->lexer)),
          location.line,
          location.column);
  ++parser->error_count;
  lexer_next(parser->lexer);
  return node_new_invalid();
}

static node_t *parse_statement(parser_t *parser) {
  switch (lexer_ttype(parser->lexer)) {
  case TT_LBRACE:
    return parse_block(parser);
  case TT_KW_IF:
    return parse_if_statement(parser);
  case TT_KW_WHILE:
    return parse_while_statement(parser);
  case TT_KW_LOOP:
    return parse_loop_statement(parser);
  case TT_KW_FOR:
    return parse_for_statement(parser);
  case TT_KW_BREAK:
    return parse_break_statement(parser);
  case TT_KW_CONTINUE:
    return parse_continue_statement(parser);
  case TT_KW_PUTI:
    return parse_puti(parser);
  case TT_KW_PUTC:
    return parse_putc(parser);
  case TT_KW_GETI:
    return parse_geti(parser);
  case TT_KW_GETC:
    return parse_getc(parser);
  case TT_KW_RETURN:
    return parse_return_statement(parser);
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
  cond = node_new_group(parse_expr(parser), "Condition");
  expect(parser, TT_RPAREN);
  then = node_new_group(parse_statement(parser), "Then-Clause");

  if (is_ttype(parser, TT_KW_ELSE)) {
    lexer_next(parser->lexer);
    els = node_new_group(parse_statement(parser), "Else-Clause");
  }

  return node_new_if(cond, then, els);
}

static node_t *parse_while_statement(parser_t *parser) {
  node_t *cond = NULL, *body = NULL;

  expect(parser, TT_KW_WHILE);
  expect(parser, TT_LPAREN);
  cond = node_new_group(parse_expr(parser), "Condition");
  expect(parser, TT_RPAREN);
  body = node_new_group(parse_statement(parser), "Body-Clause");

  return node_new_while(cond, body);
}

static node_t *parse_loop_statement(parser_t *parser) {
  node_t *body = NULL;
  expect(parser, TT_KW_LOOP);
  body = parse_statement(parser);
  return node_new_loop_statement(body);
}

static node_t *parse_for_statement(parser_t *parser) {
  node_t *init, *cond, *next, *body;
  expect(parser, TT_KW_FOR);
  expect(parser, TT_LPAREN);
  if (!is_ttype(parser, TT_SEMICOLON)) {
    init = node_new_group(parse_expr(parser), "Init-Clause");
  }
  else {
    init = node_new_empty();
  }
  expect(parser, TT_SEMICOLON);
  if (!is_ttype(parser, TT_SEMICOLON)) {
    cond = node_new_group(parse_expr(parser), "Condition-Clause");
  }
  else {
    cond = node_new_empty();
  }
  expect(parser, TT_SEMICOLON);
  if (!is_ttype(parser, TT_RPAREN)) {
    next = node_new_group(parse_expr(parser), "Next-Clause");
  }
  else {
    next = node_new_empty();
  }
  expect(parser, TT_RPAREN);
  body = node_new_group(parse_statement(parser), "Body-Clause");
  return node_new_for_statement(init, cond, next, body);
}

static node_t *parse_break_statement(parser_t *parser) {
  expect(parser, TT_KW_BREAK);
  expect(parser, TT_SEMICOLON);
  return node_new_break();
}

static node_t *parse_continue_statement(parser_t *parser) {
  expect(parser, TT_KW_CONTINUE);
  expect(parser, TT_SEMICOLON);
  return node_new_continue();
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
  node = node_new_geti(parse_ident(parser));
  expect(parser, TT_SEMICOLON);
  return node;
}

/*
 * <<GetCStatement>> ::= 'getc' <Variable> ';'
 */
static node_t *parse_getc(parser_t *parser) {
  node_t *node;
  expect(parser, TT_KW_GETC);
  node = node_new_getc(parse_ident(parser));
  expect(parser, TT_SEMICOLON);
  return node;
}

/*
 * <<ArrayDeclStatement>> ::= 'array' <Ident> '[' <Integer> ']' ';'
 */
static node_t *parse_array_statement(parser_t *parser) {
  node_t *ident, *capacity;

  expect(parser, TT_KW_ARRAY);
  ident = parse_ident(parser);
  expect(parser, TT_LBRACKET);
  capacity = parse_integer(parser);
  expect(parser, TT_RBRACKET);
  expect(parser, TT_SEMICOLON);

  return node_new_array_decl(ident, capacity);
}

/*
 * <<ReturnStatement>> ::= 'return' <<Expr>> ';'
 */
static node_t *parse_return_statement(parser_t *parser) {
  node_t *expr;
  expect(parser, TT_KW_RETURN);
  expr = parse_expr(parser);
  expect(parser, TT_SEMICOLON);
  return node_new_return(expr);
}

static node_t *parse_halt_statement(parser_t *parser) {
  expect(parser, TT_KW_HALT);
  expect(parser, TT_SEMICOLON);
  return node_new_halt();
}

/*
 * <<FuncParam>> ::= [ <Ident> { ',' <Ident> } ]
 */
static node_t *parse_func_param(parser_t *parser) {
  node_t *param = node_new_func_param();

  if (is_ttype(parser, TT_SYMBOL)) {
    node_add_child(param, parse_ident(parser));

    while (is_ttype(parser, TT_COMMA)) {
      expect(parser, TT_COMMA);
      node_add_child(param, parse_ident(parser));
    }
  }
  return param;
}

/*
 * <<FuncStatement>> ::= 'func' '(' <<FuncParam>> ')' <<BlockStatement>>
 */
static node_t *parse_func_statement(parser_t *parser) {
  node_t *ident, *param, *body;

  expect(parser, TT_KW_FUNC);
  ident = parse_ident(parser);
  expect(parser, TT_LPAREN);
  param = parse_func_param(parser);
  expect(parser, TT_RPAREN);
  body = parse_block(parser);

  if (!node_is_all_paths_ended_with_return(body)) {
    fprintf(stderr, "error: function '%s' has code path(s) not returning a value.\n", node_get_name(ident));
    ++parser->error_count;
  }

  return node_new_func(ident, param, body);
}

/*
 * <<ConstStatement>> ::= 'const' <Ident> '=' <Integer> ';'
 */
static node_t *parse_const_statement(parser_t *parser) {
  node_t *ident, *value;

  expect(parser, TT_KW_CONST);
  ident = parse_ident(parser);
  expect(parser, TT_EQ);
  value = parse_integer(parser);
  expect(parser, TT_SEMICOLON);

  return node_new_const_statement(ident, value);
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
  location_t location;

  switch (lexer_ttype(parser->lexer)) {
  case TT_INTEGER:
  case TT_CHAR:
    return parse_integer(parser);
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
    node = parse_ident(parser);
    if (is_ttype(parser, TT_LBRACKET)) {
      node = node_new_array(node, parse_array_indexer(parser));
    }
    else if (is_ttype(parser, TT_LPAREN)) {
      node = node_new_func_call(node, parse_func_call_arg(parser));
    }
    else {
      node = node_new_variable(node);
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
  fprintf(stderr, "error: unexpected '%s' (%s). (line:%d,column:%d)\n", lexer_text(parser->lexer), ttype_to_string(lexer_ttype(parser->lexer)), location.line, location.column);
  lexer_next(parser->lexer);
  ++parser->error_count;
  return node_new_invalid();
}

static node_t *parse_array_indexer(parser_t *parser) {
  node_t *indexer = NULL;
  expect(parser, TT_LBRACKET);
  indexer = parse_expr(parser);
  expect(parser, TT_RBRACKET);
  return indexer;
}

static node_t *parse_func_call_arg(parser_t *parser) {
  node_t *node = node_new_func_call_arg();

  expect(parser, TT_LPAREN);
  if (!is_eof(parser) && !is_ttype(parser, TT_RPAREN)) {
    node_add_child(node, parse_expr(parser));
    while (is_ttype(parser, TT_COMMA)) {
      expect(parser, TT_COMMA);
      node_add_child(node, parse_expr(parser));
    }
  }
  expect(parser, TT_RPAREN);
  return node;
}

static node_t *parse_ident(parser_t *parser) {
  if (is_ttype(parser, TT_SYMBOL)) {
    node_t *node = node_new_ident(lexer_text(parser->lexer));
    lexer_next(parser->lexer);
    return node;
  }
  return node_new_invalid();
}

static node_t *parse_integer(parser_t *parser) {
  if (is_ttype(parser, TT_INTEGER) || is_ttype(parser, TT_CHAR)) {
    node_t *node = node_new_integer(lexer_int_value(parser->lexer));
    lexer_next(parser->lexer);
    return node;
  }
  return node_new_invalid();
}
