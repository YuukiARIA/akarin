#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "codegen.h"

int main(int argc, char *argv[]) {
  FILE *input = stdin;
  parser_t *parser = parser_new(input);
  node_t *node = parser_parse(parser);
  codegen_t *codegen = codegen_new(node);
  codegen_generate(codegen);
  codegen_release(&codegen);
  node_release(&node);
  parser_release(&parser);
  return 0;
}
