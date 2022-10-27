#pragma once

#include "ttype.h"

typedef struct lexer_t lexer_t;

lexer_t    *lexer_new(FILE *input);
void        lexer_release(lexer_t **plexer);
int         lexer_succ(lexer_t *lexer);
void        lexer_next(lexer_t *lexer);
int         lexer_is_eof(lexer_t *lexer);
ttype_t     lexer_ttype(lexer_t *lexer);
int         lexer_int_value(lexer_t *lexer);
const char *lexer_text(lexer_t *lexer);
