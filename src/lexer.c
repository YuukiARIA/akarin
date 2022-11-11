#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils/memory.h"

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
  { "if",       TT_KW_IF       },
  { "else",     TT_KW_ELSE     },
  { "while",    TT_KW_WHILE    },
  { "loop",     TT_KW_LOOP     },
  { "break",    TT_KW_BREAK    },
  { "continue", TT_KW_CONTINUE },
  { "puti",     TT_KW_PUTI     },
  { "putc",     TT_KW_PUTC     },
  { "geti",     TT_KW_GETI     },
  { "getc",     TT_KW_GETC     },
  { "array",    TT_KW_ARRAY    },
  { "halt",     TT_KW_HALT     },
  { "func",     TT_KW_FUNC     },
  { "return",   TT_KW_RETURN   },
  { "const",    TT_KW_CONST    },
};
static const int g_keyword_count = sizeof(g_keywords) / sizeof(struct keyword_t);

static const char g_esc_chars[256] = {
  ['a'] = '\a',
  ['b'] = '\b',
  ['e'] = '\x1B',
  ['r'] = '\r',
  ['n'] = '\n',
  ['t'] = '\t',
  ['\\'] = '\\',
  ['\''] = '\''
};

static int  peek(lexer_t *lexer);
static void succ(lexer_t *lexer);

static void clear_buf(lexer_t *lexer) {
  memset(lexer->text, 0, TEXT_BUF_SIZE);
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

static void set_token(lexer_t *lexer, ttype_t ttype, const char *text) {
  lexer->ttype = ttype;
  strcpy(lexer->text, text);
}

lexer_t *lexer_new(FILE *input) {
  lexer_t *lexer = (lexer_t *)AK_MEM_MALLOC(sizeof(lexer_t));
  lexer->input = input;
  lexer->location.column = 1;
  lexer->location.line = 1;
  lexer->cur = getc(input);
  lexer->error_count = 0;
  return lexer;
}

void lexer_release(lexer_t **plexer) {
  AK_MEM_FREE(*plexer);
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

static int hex_char_to_int(char x) {
  if (isdigit(x)) {
    return x - '0';
  }
  switch (toupper(x)) {
  case 'A': return 10;
  case 'B': return 11;
  case 'C': return 12;
  case 'D': return 13;
  case 'E': return 14;
  case 'F': return 15;
  }
  return 0;
}

static int lex_escaped_char(lexer_t *lexer) {
  int c = peek(lexer);
  if (c == 'x') {
    succ(lexer);
    if (isxdigit(peek(lexer))) {
      c = hex_char_to_int(peek(lexer));
      succ(lexer);
    }
    if (isxdigit(peek(lexer))) {
      c = 16 * c + hex_char_to_int(peek(lexer));
      succ(lexer);
    }
  }
  else if (g_esc_chars[c]) {
    c = g_esc_chars[c];
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

static bool is_symbol_head(int c) {
  return c == '_' || isalpha(c);
}

static bool is_symbol_part(int c) {
  return c == '_' || isalnum(c);
}

void lexer_lex_symbol(lexer_t *lexer) {
  int i;

  clear_buf(lexer);
  while (is_symbol_part(peek(lexer))) {
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

static bool lex_op(lexer_t *lexer) {
  switch (peek(lexer)) {
  case ',':
    succ(lexer);
    set_token(lexer, TT_COMMA, ",");
    return true;
  case ';':
    succ(lexer);
    set_token(lexer, TT_SEMICOLON, ";");
    return true;
  case '=':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      set_token(lexer, TT_EQEQ, "==");
      return true;
    }
    set_token(lexer, TT_EQ, "=");
    return true;
  case '!':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      set_token(lexer, TT_EXCLAEQ, "!=");
      return true;
    }
    set_token(lexer, TT_EXCLA, "!");
    return true;
  case '&':
    succ(lexer);
    set_token(lexer, TT_AMP, "&");
    return true;
  case '|':
    succ(lexer);
    set_token(lexer, TT_BAR, "|");
    return true;
  case '+':
    succ(lexer);
    set_token(lexer, TT_PLUS, "+");
    return true;
  case '-':
    succ(lexer);
    set_token(lexer, TT_MINUS, "-");
    return true;
  case '*':
    succ(lexer);
    set_token(lexer, TT_ASTERISK, "*");
    return true;
  case '/':
    succ(lexer);
    set_token(lexer, TT_SLASH, "/");
    return true;
  case '%':
    succ(lexer);
    set_token(lexer, TT_PERCENT, "%");
    return true;
  case '<':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      set_token(lexer, TT_LE, "<=");
      return true;
    }
    set_token(lexer, TT_LT, "<");
    return true;
  case '>':
    succ(lexer);
    if (peek(lexer) == '=') {
      succ(lexer);
      set_token(lexer, TT_GE, ">=");
      return true;
    }
    set_token(lexer, TT_GT, ">");
    return true;
  case '(':
    succ(lexer);
    set_token(lexer, TT_LPAREN, "(");
    return true;
  case ')':
    succ(lexer);
    set_token(lexer, TT_RPAREN, ")");
    return true;
  case '{':
    succ(lexer);
    set_token(lexer, TT_LBRACE, "{");
    return true;
  case '}':
    succ(lexer);
    set_token(lexer, TT_RBRACE, "}");
    return true;
  case '[':
    succ(lexer);
    set_token(lexer, TT_LBRACKET, "[");
    return true;
  case ']':
    succ(lexer);
    set_token(lexer, TT_RBRACKET, "]");
    return true;
  }

  return false;
}

void lexer_next(lexer_t *lexer) {
  int c;

  lexer->ivalue = 0;

  lexer_skip_ws(lexer);

  c = peek(lexer);

  if (c == EOF) {
    set_token(lexer, TT_EOF, "<EOF>");
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
  else if (is_symbol_head(c)) {
    lexer_lex_symbol(lexer);
    return;
  }
  else if (lex_op(lexer)) {
    return;
  }

  fprintf(stderr, "error: unrecognizable character '%c' (line:%d,column:%d)\n", c, lexer->location.line, lexer->location.column);
  set_token(lexer, TT_UNKNOWN, "<UNKNOWN>");
  succ(lexer);
  ++lexer->error_count;
}
