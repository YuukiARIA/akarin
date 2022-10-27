#pragma once

typedef enum {
  TT_UNKNOWN = 0,

  TT_SEMICOLON,

  TT_EQ,
  TT_EQEQ,
  TT_EXCLA,
  TT_EXCLAEQ,
  TT_LT,
  TT_LE,
  TT_GT,
  TT_GE,
  TT_AMP,
  TT_BAR,
  TT_PLUS,
  TT_MINUS,
  TT_ASTERISK,
  TT_SLASH,
  TT_PERCENT,

  TT_LPAREN,
  TT_RPAREN,
  TT_LBRACE,
  TT_RBRACE,
  TT_LBRACKET,
  TT_RBRACKET,

  TT_INTEGER,
  TT_CHAR,
  TT_SYMBOL,

  TT_KW_IF,
  TT_KW_ELSE,
  TT_KW_WHILE,
  TT_KW_BREAK,
  TT_KW_PUTI,
  TT_KW_PUTC,
  TT_KW_GETI,
  TT_KW_GETC,
  TT_KW_ARRAY,
  TT_KW_HALT,

  TT_EOF = -1,
} ttype_t;

const char *ttype_to_string(ttype_t ttype);
