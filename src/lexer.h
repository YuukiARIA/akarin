#pragma once

#include "ttype.h"
#include "location.h"

typedef struct lexer_t lexer_t;

lexer_t    *lexer_new(FILE *input);
void        lexer_release(lexer_t **plexer);
void        lexer_next(lexer_t *lexer);
int         lexer_is_eof(lexer_t *lexer);
ttype_t     lexer_ttype(lexer_t *lexer);
location_t  lexer_get_location(lexer_t *lexer);
int         lexer_int_value(lexer_t *lexer);
const char *lexer_text(lexer_t *lexer);
