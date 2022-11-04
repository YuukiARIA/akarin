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
  printf("    -d              Dump syntax tree.\n");
}

static emitter_t *create_emitter(emit_mode_t emit_mode) {
  switch (emit_mode) {
  case EMIT_SYMBOLIC:
    return emitter_ws_new('S', 'T', 'L');
  case EMIT_PSEUDO_CODE:
    return emitter_pseudo_new(8);
  default:
    return emitter_ws_new(' ', '\t', '\n');
  }
}

static void generate_code(node_t *node, emit_mode_t emit_mode) {
  emitter_t *emitter = create_emitter(emit_mode);
  codegen_t *codegen = codegen_new(node, emitter);
  codegen_generate(codegen);
  codegen_release(&codegen);
  emitter_release(&emitter);
}

int main(int argc, char *argv[]) {
  FILE *input = stdin;
  int needs_close = 0;
  int dump_tree = 0;
  parser_t *parser;
  node_t *node;
  emit_mode_t emit_mode = EMIT_WHITESPACE;
  int i;
  int error_count;

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
    else if (strcmp(argv[i], "-d") == 0) {
      dump_tree = 1;
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

  parser = parser_new(input);
  node = parser_parse(parser);

  if (needs_close) {
    fclose(input);
    input = NULL;
  }

  error_count = parser_get_total_error_count(parser);
  parser_release(&parser);

  if (error_count == 0) {
    if (dump_tree) {
      node_dump_tree(node);
    }
    else {
      generate_code(node, emit_mode);
    }
  }
  else {
    fprintf(stderr, "%d errors found.\n", error_count);
  }

  node_release(&node);
  return error_count == 0 ? 0 : 1;
}
