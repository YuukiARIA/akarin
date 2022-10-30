#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codegen.h"
#include "emitter_ws.h"
#include "emitter_pseudo.h"

typedef enum {
  EMIT_WHITESPACE,
  EMIT_SYMBOLIC,
  EMIT_PSEUDO_CODE,
} emit_mode_t;

static void show_help(void) {
  printf("\e[1mAkarin\e[0m - A Whitespace Transpiler\n\n");
  printf("Usage: akarin [options] [input file]\n\n");
  printf("Read from standard input if no input file was given.\n\n");
  printf("Options:\n");
  printf("    -h              Show this help.\n");
  printf("    -s              Transpile into symbolic (S, T, L) code instead of whitespace.\n");
  printf("    -p              Transpile into pseudo mnemonic code instead of whitespace.\n");
}

int main(int argc, char *argv[]) {
  FILE *input = stdin;
  int needs_close = 0;
  parser_t *parser;
  node_t *node;
  codegen_t *codegen;
  emitter_t *emitter;
  emit_mode_t emit_mode = EMIT_WHITESPACE;
  int i;

  /* process command line args */
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      show_help();
      return 0;
    }
    else if (strcmp(argv[i], "-s") == 0) {
      emit_mode = EMIT_SYMBOLIC;
    }
    else if (strcmp(argv[i], "-p") == 0) {
      emit_mode = EMIT_PSEUDO_CODE;
    }
    else {
      input = fopen(argv[i], "r");
      needs_close = 1;
    }
  }

  if (!input) {
    fprintf(stderr, "error: could not open file - %s\n", argv[1]);
    return 1;
  }

  switch (emit_mode) {
  case EMIT_SYMBOLIC:
    emitter = emitter_ws_new('S', 'T', 'L');
    break;
  case EMIT_PSEUDO_CODE:
    emitter = emitter_pseudo_new(8);
    break;
  default:
    emitter = emitter_ws_new(' ', '\t', '\n');
    break;
  }

  parser = parser_new(input);
  node = parser_parse(parser);

  if (needs_close) {
    fclose(input);
    input = NULL;
  }

  parser_release(&parser);

  codegen = codegen_new(node, emitter);
  codegen_generate(codegen);

  codegen_release(&codegen);
  emitter_release(&emitter);
  node_release(&node);
  return 0;
}
