#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "opcode.h"
#include "emitter.h"
#include "label.h"
#include "utils/memory.h"

typedef struct {
  emitter_t   base;
  const char *space;
  const char *tab;
  const char *newline;
  bool        strict;
} emitter_ws_t;

static void ws_emit(emitter_t *self, inst_t *inst);
static void ws_end(emitter_t *self);

static void encode_integer(emitter_t *self, int n);
static void encode_uint(emitter_t *self, unsigned int n);
static void encode_uint_rec(emitter_t *self, unsigned int n);
static void emit_chars(emitter_t *self, const char *s);
static void emit_char(emitter_t *self, char c);

emitter_t *emitter_ws_new(const char *space, const char *tab, const char *newline, bool strict) {
  emitter_ws_t *emitter = (emitter_ws_t *)AK_MEM_MALLOC(sizeof(emitter_ws_t));
  emitter->base.emit = ws_emit;
  emitter->base.end = ws_end;
  emitter->space = space;
  emitter->tab = tab;
  emitter->newline = newline;
  emitter->strict = strict;
  return (emitter_t *)emitter;
}

static void ws_emit(emitter_t *self, inst_t *inst) {
  if (inst->opcode != OP_NOP) {
    emit_chars(self, opcode_to_ws(inst->opcode));
  }

  switch (inst->opcode) {
  case OP_PUSH:
  case OP_COPY:
  case OP_SLIDE:
    encode_integer(self, inst->value);
    break;
  case OP_LABEL:
  case OP_CALL:
  case OP_JMP:
  case OP_JZ:
  case OP_JNEG:
    encode_uint(self, (unsigned int)label_get_unified_id(inst->label));
    break;
  default:
    break;
  }
}

static void ws_end(emitter_t *self) {
  emitter_ws_t *emitter = (emitter_ws_t *)self;

  /* if set to non-pure whitespace format, print newline on the end */
  if (!emitter->strict) {
    putchar('\n');
  }
}

static void encode_integer(emitter_t *self, int n) {
  emit_char(self, n >= 0 ? 'S' : 'T');
  encode_uint(self, (unsigned int)abs(n));
}

static void encode_uint(emitter_t *self, unsigned int n) {
  if (n == 0) {
    emit_char(self, 'S');
  }
  else {
    encode_uint_rec(self, n);
  }
  emit_char(self, 'L');
}

static void encode_uint_rec(emitter_t *self, unsigned int n) {
  if (n != 0) {
    encode_uint_rec(self, n >> 1);
    emit_char(self, (n & 1) ? 'T' : 'S');
  }
}

static void emit_chars(emitter_t *self, const char *s) {
  const char *p = s;
  while (*p) {
    emit_char(self, *p++);
  }
}

static void emit_char(emitter_t *self, char c) {
  emitter_ws_t *emitter = (emitter_ws_t *)self;
  switch (c) {
  case 'S':
    printf("%s", emitter->space);
    break;
  case 'T':
    printf("%s", emitter->tab);
    break;
  case 'L':
    printf("%s", emitter->newline);
    break;
  default:
    putchar(c);
    break;
  }
}
