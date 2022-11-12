#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codegen.h"
#include "node_formatter.h"
#include "inst.h"
#include "emitter_ws.h"
#include "emitter_pseudo.h"
#include "utils/memory.h"
#include "utils/array.h"

typedef enum {
  EMIT_WHITESPACE,
  EMIT_SYMBOLIC,
  EMIT_PSEUDO_CODE
} emit_mode_t;

typedef struct {
  FILE       *input;
  bool        dump_tree;
  emit_mode_t emit_mode;
} option_t;

static void show_help(void) {
  printf("\x1B[1mAkarin\x1B[0m - A Whitespace Transpiler\n\n");
  printf("Usage: akarin [options] [input file]\n\n");
  printf("Read from standard input if no input file was given.\n\n");
  printf("Options:\n");
  printf("    -h              Show this help.\n");
  printf("    -s              Transpile into symbolic (S, T, L) code instead of whitespace.\n");
  printf("    -p              Transpile into pseudo mnemonic code instead of whitespace.\n");
  printf("    -d              Dump syntax tree.\n");
}

static void process_options(int argc, char *argv[], option_t *opt) {
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      show_help();
      exit(0);
    }
    else if (strcmp(argv[i], "-s") == 0) {
      opt->emit_mode = EMIT_SYMBOLIC;
    }
    else if (strcmp(argv[i], "-p") == 0) {
      opt->emit_mode = EMIT_PSEUDO_CODE;
    }
    else if (strcmp(argv[i], "-d") == 0) {
      opt->dump_tree = true;
    }
    else {
      if (opt->input == stdin) {
	opt->input = fopen(argv[i], "r");
      }
    }
  }
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

static void emit_code(array_t *insts, emit_mode_t emit_mode) {
  emitter_t *emitter = create_emitter(emit_mode);
  emitter_emit_code(emitter, insts);
  emitter_release(&emitter);
}

static int generate_code(node_t *node, emit_mode_t emit_mode) {
  codegen_t *codegen = codegen_new(node);
  int error_count;

  codegen_generate(codegen);
  error_count = codegen_get_error_count(codegen);

  if (error_count == 0) {
    emit_code(codegen_get_instructions(codegen), emit_mode);
  }

  codegen_release(&codegen);

  return error_count;
}

static node_t *parse(FILE *input, int *error_count) {
  parser_t *parser = parser_new(input);
  node_t *node = parser_parse(parser);
  *error_count += parser_get_total_error_count(parser);
  parser_release(&parser);
  return node;
}

int main(int argc, char *argv[]) {
  option_t opt = { .input = stdin, .dump_tree = false, .emit_mode = EMIT_WHITESPACE };
  node_t *node;
  int error_count = 0;

  /* process command line args */
  process_options(argc, argv, &opt);

  if (!opt.input) {
    fprintf(stderr, "error: could not open file - %s\n", argv[1]);
    return 1;
  }

  node = parse(opt.input, &error_count);

  if (opt.input != stdin) {
    fclose(opt.input);
    opt.input = NULL;
  }

  if (error_count == 0) {
    if (opt.dump_tree) {
      node_dump_tree(node);
    }
    else {
      error_count = generate_code(node, opt.emit_mode);
    }
  }

  if (error_count > 0) {
    fprintf(stderr, "%d errors found.\n", error_count);
  }

  node_release(&node);

  AK_MEM_CHECK;
  return error_count == 0 ? 0 : 1;
}
