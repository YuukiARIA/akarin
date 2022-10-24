#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char *argv[]) {
  FILE *input = stdin;
  parser_t *parser = parser_new(input);
  parser_parse(parser);
  parser_release(&parser);
  return 0;
}
