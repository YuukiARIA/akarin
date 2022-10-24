#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[]) {
  FILE *input = stdin;
  parser_t *parser = parser_new(input);
  node_t *node = parser_parse(parser);
  node_release(&node);
  parser_release(&parser);
  return 0;
}
