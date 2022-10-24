#pragma once

#include <stdio.h>

typedef struct parser_t parser_t;

parser_t *parser_new(FILE *input);
void      parser_release(parser_t **pparser);
void      parser_parse(parser_t *parser);
