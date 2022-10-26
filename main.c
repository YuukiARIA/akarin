#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "codegen.h"

int main(int argc, char *argv[]) {
  FILE *input = stdin;
  int needs_close = 0;
  parser_t *parser;
  node_t *node;
  codegen_t *codegen;

  if (argc >= 2) {
    input = fopen(argv[1], "r");
    needs_close = 1;
  }
  if (!input) {
    fprintf(stderr, "error: could not open file - %s\n", argv[1]);
    return 1;
  }

  parser = parser_new(input);
  node = parser_parse(parser);

  if (needs_close) {
    fclose(input);
    input = NULL;
  }

  parser_release(&parser);

  codegen = codegen_new(node);
  codegen_generate(codegen);
  codegen_release(&codegen);
  node_release(&node);
  return 0;
}
