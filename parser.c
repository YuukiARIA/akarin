#include <stdlib.h>
#include "parser.h"
#include "lexer.h"

struct parser_t {
  lexer_t *lexer;
};

typedef enum {
  UOP_INVALID,
  UOP_POSITIVE,
  UOP_NEGATIVE,
  UOP_NOT,
} unary_op_t;

typedef enum {
  BOP_INVALID,
  BOP_ADD,
  BOP_SUB,
  BOP_MUL,
  BOP_DIV,
  BOP_MOD,
  BOP_EQ,
  BOP_NEQ,
  BOP_AND,
  BOP_OR,
  BOP_LT,
  BOP_LE,
  BOP_GT,
  BOP_GE,
  BOP_ASSIGN,
} binary_op_t;

static binary_op_t ttype_to_binary_op(ttype_t ttype);
static const char* binary_op_to_string(binary_op_t bop);
static void parse_expr(parser_t *parser);
static void parse_assign(parser_t *parser);
static void parse_addsub(parser_t *parser);
static void parse_muldiv(parser_t *parser);
static void parse_atomic(parser_t *parser);

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

void parser_parse(parser_t *parser) {
  lexer_succ(parser->lexer);
  lexer_next(parser->lexer);
  parse_expr(parser);
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

static const char* binary_op_to_string(binary_op_t bop) {
  switch (bop) {
  case BOP_EQ:  return "EQ";
  case BOP_NEQ: return "NEQ";
  case BOP_LT:  return "LT";
  case BOP_LE:  return "LE";
  case BOP_GT:  return "GT";
  case BOP_GE:  return "GE";
  case BOP_ADD: return "ADD";
  case BOP_SUB: return "SUB";
  case BOP_MUL: return "MUL";
  case BOP_DIV: return "DIV";
  case BOP_MOD: return "MOD";
  case BOP_AND: return "AND";
  case BOP_OR:  return "OR";
  default:      return "INVALID";
  }
}

static void parse_expr(parser_t *parser) {
  parse_assign(parser);
}

static void parse_assign(parser_t *parser) {
  parse_addsub(parser);
  if (lexer_ttype(parser->lexer) == TT_EQ) {
    lexer_next(parser->lexer);
    parse_assign(parser);
    puts("ASSIGN");
  }
}

static void parse_addsub(parser_t *parser) {
  parse_muldiv(parser);
  while (lexer_ttype(parser->lexer) == TT_PLUS || lexer_ttype(parser->lexer) == TT_MINUS) {
    binary_op_t bop = ttype_to_binary_op(lexer_ttype(parser->lexer));
    lexer_next(parser->lexer);
    parse_muldiv(parser);
    printf("%s\n", binary_op_to_string(bop));
  }
}

static void parse_muldiv(parser_t *parser) {
  parse_atomic(parser);
  while (lexer_ttype(parser->lexer) == TT_ASTERISK || lexer_ttype(parser->lexer) == TT_SLASH || lexer_ttype(parser->lexer) == TT_PERCENT) {
    binary_op_t bop = ttype_to_binary_op(lexer_ttype(parser->lexer));
    lexer_next(parser->lexer);
    parse_atomic(parser);
    printf("%s\n", binary_op_to_string(bop));
  }
}

static void parse_atomic(parser_t *parser) {
  switch (lexer_ttype(parser->lexer)) {
  case TT_INTEGER:
    {
      int ival = lexer_int_value(parser->lexer);
      lexer_next(parser->lexer);
      printf("PUSH %d\n", ival);
    }
    return;
  case TT_SYMBOL:
    {
      printf("LOAD %s\n", lexer_text(parser->lexer));
      lexer_next(parser->lexer);
    }
    return;
  case TT_LPAREN:
    lexer_next(parser->lexer);
    parse_expr(parser);
    lexer_next(parser->lexer);
    return;
  default:
    break;
  }
  printf("unexpected: ttype=%d\n", lexer_ttype(parser->lexer));
}
