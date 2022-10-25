#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define TEXT_BUF_SIZE   ( 64 )
#define TEXT_LEN_MAX    ( TEXT_BUF_SIZE - 1 )

struct lexer_t {
  FILE   *input;
  int     cur;
  int     column;
  int     line;
  int     bufpos;
  char    text[TEXT_BUF_SIZE];
  ttype_t ttype;
};

static void clear_buf(lexer_t *lexer) {
  memset(lexer->text, 0, lexer->bufpos);
  lexer->bufpos = 0;
}

static void append_char(lexer_t *lexer, int c) {
  if (lexer->bufpos < TEXT_LEN_MAX) {
    lexer->text[lexer->bufpos++] = (char)c;
  }
  else {
    fprintf(stderr, "too long text.\n");
  }
}

lexer_t *lexer_new(FILE *input) {
  lexer_t *lexer = (lexer_t *)malloc(sizeof(lexer_t));
  lexer->input = input;
  lexer->column = 0;
  lexer->line = 0;
  return lexer;
}

void lexer_release(lexer_t **plexer) {
  free(*plexer);
  *plexer = NULL;
}

int lexer_is_eof(lexer_t *lexer) {
  return lexer->cur == EOF;
}

ttype_t lexer_ttype(lexer_t *lexer) {
  return lexer->ttype;
}

int lexer_int_value(lexer_t *lexer) {
  return atoi(lexer->text);
}

float lexer_float_value(lexer_t *lexer) {
  return (float)atof(lexer->text);
}

const char *lexer_text(lexer_t *lexer) {
  return lexer->text;
}

int lexer_peek(lexer_t *lexer) {
  return lexer->cur;
}

int lexer_succ(lexer_t *lexer) {
  int c = getc(lexer->input);
  if (c == '\n') {
    ++lexer->line;
    lexer->column = 0;
  }
  lexer->cur = c;
  return c;
}

void lexer_skip_ws(lexer_t *lexer) {
  while (isspace(lexer_peek(lexer))) {
    lexer_succ(lexer);
  }
}

static void lexer_lex_integer(lexer_t *lexer) {
  clear_buf(lexer);
  while (isdigit(lexer_peek(lexer))) {
    append_char(lexer, lexer_peek(lexer));
    lexer_succ(lexer);
  }
  lexer->ttype = TT_INTEGER;
}

void lexer_lex_symbol(lexer_t *lexer) {
  clear_buf(lexer);
  while (isalpha(lexer_peek(lexer))) {
    append_char(lexer, lexer_peek(lexer));
    lexer_succ(lexer);
  }

  if (strcmp(lexer->text, "if") == 0) {
    lexer->ttype = TT_KW_IF;
  }
  else if (strcmp(lexer->text, "else") == 0) {
    lexer->ttype = TT_KW_ELSE;
  }
  else if (strcmp(lexer->text, "while") == 0) {
    lexer->ttype = TT_KW_WHILE;
  }
  else if (strcmp(lexer->text, "puti") == 0) {
    lexer->ttype = TT_KW_PUTI;
  }
  else if (strcmp(lexer->text, "putc") == 0) {
    lexer->ttype = TT_KW_PUTC;
  }
  else {
    lexer->ttype = TT_SYMBOL;
  }
}

static int lex_op(lexer_t *lexer) {
  switch (lexer_peek(lexer)) {
  case ';':
    lexer_succ(lexer);
    lexer->ttype = TT_SEMICOLON;
    return 1;
  case '=':
    lexer_succ(lexer);
    if (lexer_peek(lexer) == '=') {
      lexer_succ(lexer);
      lexer->ttype = TT_EQEQ;
      return 1;
    }
    lexer->ttype = TT_EQ;
    return 1;
  case '&':
    lexer_succ(lexer);
    lexer->ttype = TT_AMP;
    return 1;
  case '|':
    lexer_succ(lexer);
    lexer->ttype = TT_BAR;
    return 1;
  case '+':
    lexer_succ(lexer);
    lexer->ttype = TT_PLUS;
    return 1;
  case '-':
    lexer_succ(lexer);
    lexer->ttype = TT_MINUS;
    return 1;
  case '*':
    lexer_succ(lexer);
    lexer->ttype = TT_ASTERISK;
    return 1;
  case '/':
    lexer_succ(lexer);
    lexer->ttype = TT_SLASH;
    return 1;
  case '%':
    lexer_succ(lexer);
    lexer->ttype = TT_PERCENT;
    return 1;
  case '<':
    lexer_succ(lexer);
    if (lexer_peek(lexer) == '=') {
      lexer_succ(lexer);
      lexer->ttype = TT_LE;
      return 1;
    }
    lexer->ttype = TT_LT;
    return 1;
  case '>':
    lexer_succ(lexer);
    if (lexer_peek(lexer) == '=') {
      lexer_succ(lexer);
      lexer->ttype = TT_GE;
      return 1;
    }
    lexer->ttype = TT_GT;
    return 1;
  case '(':
    lexer_succ(lexer);
    lexer->ttype = TT_LPAREN;
    return 1;
  case ')':
    lexer_succ(lexer);
    lexer->ttype = TT_RPAREN;
    return 1;
  case '{':
    lexer_succ(lexer);
    lexer->ttype = TT_LBRACE;
    return 1;
  case '}':
    lexer_succ(lexer);
    lexer->ttype = TT_RBRACE;
    return 1;
  }

  return 0;
}

void lexer_next(lexer_t *lexer) {
  int c;

  lexer_skip_ws(lexer);

  c = lexer_peek(lexer);

  if (c == EOF) {
    lexer->ttype = TT_EOF;
    return;
  }

  if (isdigit(c)) {
    lexer_lex_integer(lexer);
    return;
  }
  else if (isalpha(c)) {
    lexer_lex_symbol(lexer);
    return;
  }
  else if (lex_op(lexer)) {
    return;
  }

  printf("UNKNOWN: %c\n", c);
  lexer->ttype = TT_UNKNOWN;
  lexer_succ(lexer);
}
