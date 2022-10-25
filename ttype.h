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

  TT_INTEGER,
  TT_SYMBOL,

  TT_KW_PUTI,
  TT_KW_PUTC,

  TT_EOF = -1,
} ttype_t;
