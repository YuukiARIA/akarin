#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define TEXT_BUF_SIZE   ( 64 )
#define TEXT_LEN_MAX    ( TEXT_BUF_SIZE - 1 )

struct lexer_t {
  FILE      *input;
  int        cur;
  location_t location;
  int        bufpos;
  char       text[TEXT_BUF_SIZE];
  ttype_t    ttype;
  int        ivalue;
  int        error_count;
};

struct keyword_t {
  const char *keyword;
  ttype_t     ttype;
};

static const struct keyword_t g_keywords[] = {
  { "if",     TT_KW_IF     },
  { "else",   TT_KW_ELSE   },
  { "while",  TT_KW_WHILE  },
  { "break",  TT_KW_BREAK  },
  { "puti",   TT_KW_PUTI   },
  { "putc",   TT_KW_PUTC   },
  { "geti",   TT_KW_GETI   },
  { "getc",   TT_KW_GETC   },
  { "array",  TT_KW_ARRAY  },
  { "halt",   TT_KW_HALT   },
  { "func",   TT_KW_FUNC   },
  { "return", TT_KW_RETURN },
};
static const int g_keyword_count = sizeof(g_keywords) / sizeof(struct keyword_t);

static int  peek(lexer_t *lexer);
static void succ(lexer_t *lexer);

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
  lexer->location.column = 1;
  lexer->location.line = 1;
  lexer->cur = getc(input);
  lexer->error_count = 0;
  return lexer;
}

void lexer_release(lexer_t **plexer) {
  free(*plexer);
  *plexer = NULL;
}

int lexer_is_eof(lexer_t *lexer) {
  return lexer->cur == EOF;
}

location_t lexer_get_location(lexer_t *lexer) {
  return lexer->location;
}

ttype_t lexer_ttype(lexer_t *lexer) {
  return lexer->ttype;
}

int lexer_int_value(lexer_t *lexer) {
  return lexer->ivalue;
}

const char *lexer_text(lexer_t *lexer) {
  return lexer->text;
}

int lexer_get_error_count(lexer_t *lexer) {
  return lexer->error_count;
}

static int peek(lexer_t *lexer) {
  return lexer->cur;
}

static void succ(lexer_t *lexer) {
  if (lexer->cur == EOF) {
    return;
  }
  else if (lexer->cur == '\n') {
    ++lexer->location.line;
    lexer->location.column = 1;
  }
  else {
    ++lexer->location.column;
  }
  lexer->cur = getc(lexer->input);
}

void skip_to_end_of_line(lexer_t *lexer) {
  while (!lexer_is_eof(lexer) && peek(lexer) != '\n') {
    succ(lexer);
  }
}

void lexer_skip_ws(lexer_t *lexer) {
  while (!lexer_is_eof(lexer)) {
    if (isspace(peek(lexer))) {
      succ(lexer);
    }
    else if (peek(lexer) == '#') {
      skip_to_end_of_line(lexer);
    }
    else {
      break;
    }
  }
}

static void lexer_lex_integer(lexer_t *lexer) {
  clear_buf(lexer);
  while (isdigit(peek(lexer))) {
    append_char(lexer, peek(lexer));
    succ(lexer);
  }
  lexer->ttype = TT_INTEGER;
  lexer->ivalue = atoi(lexer->text);
}

static int lex_escaped_char(lexer_t *lexer) {
  int c = peek(lexer);
  switch (c) {
  case 'r':
    c = '\r';
    break;
  case 'n':
    c = '\n';
    break;
  case 't':
    c = '\t';
    break;
  case '\\':
    c = '\\';
    break;
  case '\'':
    c = '\'';
    break;
  }
  succ(lexer);
  return c;
}

static void lex_char(lexer_t *lexer) {
  int c = 0;

  if (peek(lexer) == '\'') {
    succ(lexer);
  }

  if (peek(lexer) == '\\') {
    succ(lexer);
    c = lex_escaped_char(lexer);
  }
  else if (isprint(peek(lexer))) {
    c = peek(lexer);
    succ(lexer);
  }

  if (peek(lexer) == '\'') {
    succ(lexer);
  }

  lexer->ttype = TT_CHAR;
  lexer->ivalue = c;
}

void lexer_lex_symbol(lexer_t *lexer) {
  int i;

  clear_buf(lexer);
  while (isalpha(peek(lexer))) {
    append_char(lexer, peek(lexer));
    succ(lexer);
  }

  lexer->ttype = TT_SYMBOL;

  for (i = 0; i < g_keyword_count; ++i) {
    struct keyword_t kw = g_keywords[i];
    if (strcmp(lexer->text, kw.keyword) == 0) {
      lexer->ttype = kw.ttype;
      break;
    }
  }
}

static int lex_op(lexer_t *lexer) {
  switch (peek(lexer)) {
  case ';':
    succ(lexer);
    lexer->ttype = TT_SEMICOLON;
    return 1;
  case '=':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      lexer->ttype = TT_EQEQ;
      return 1;
    }
    lexer->ttype = TT_EQ;
    return 1;
  case '!':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      lexer->ttype = TT_EXCLAEQ;
      return 1;
    }
    lexer->ttype = TT_EXCLA;
    return 1;
  case '&':
    succ(lexer);
    lexer->ttype = TT_AMP;
    return 1;
  case '|':
    succ(lexer);
    lexer->ttype = TT_BAR;
    return 1;
  case '+':
    succ(lexer);
    lexer->ttype = TT_PLUS;
    return 1;
  case '-':
    succ(lexer);
    lexer->ttype = TT_MINUS;
    return 1;
  case '*':
    succ(lexer);
    lexer->ttype = TT_ASTERISK;
    return 1;
  case '/':
    succ(lexer);
    lexer->ttype = TT_SLASH;
    return 1;
  case '%':
    succ(lexer);
    lexer->ttype = TT_PERCENT;
    return 1;
  case '<':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      lexer->ttype = TT_LE;
      return 1;
    }
    lexer->ttype = TT_LT;
    return 1;
  case '>':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      lexer->ttype = TT_GE;
      return 1;
    }
    lexer->ttype = TT_GT;
    return 1;
  case '(':
    succ(lexer);
    lexer->ttype = TT_LPAREN;
    return 1;
  case ')':
    succ(lexer);
    lexer->ttype = TT_RPAREN;
    return 1;
  case '{':
    succ(lexer);
    lexer->ttype = TT_LBRACE;
    return 1;
  case '}':
    succ(lexer);
    lexer->ttype = TT_RBRACE;
    return 1;
  case '[':
    succ(lexer);
    lexer->ttype = TT_LBRACKET;
    return 1;
  case ']':
    succ(lexer);
    lexer->ttype = TT_RBRACKET;
    return 1;
  }

  return 0;
}

void lexer_next(lexer_t *lexer) {
  int c;

  lexer->ivalue = 0;

  lexer_skip_ws(lexer);

  c = peek(lexer);

  if (c == EOF) {
    lexer->ttype = TT_EOF;
    return;
  }

  if (c == '\'') {
    lex_char(lexer);
    return;
  }
  else if (isdigit(c)) {
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

  fprintf(stderr, "error: unrecognizable character '%c' (line:%d,column:%d)\n", c, lexer->location.line, lexer->location.column);
  lexer->ttype = TT_UNKNOWN;
  succ(lexer);
  ++lexer->error_count;
}
