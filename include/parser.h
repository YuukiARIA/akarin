#pragma once

#include <stdio.h>
#include "node.h"

typedef struct parser_t parser_t;

parser_t *parser_new(FILE *input);
void      parser_release(parser_t **pparser);
node_t   *parser_parse(parser_t *parser);
int       parser_get_total_error_count(parser_t *parser);
