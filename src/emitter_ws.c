#include <stdio.h>
#include <stdlib.h>
#include "emitter.h"
#include "utils/memory.h"

typedef struct {
  emitter_t base;
  char      space;
  char      tab;
  char      newline;
} emitter_ws_t;

static void ws_emit(emitter_t *self, inst_t *inst);
static void ws_end(emitter_t *self);

static void encode_integer(emitter_t *self, int n);
static void encode_uint(emitter_t *self, unsigned int n);
static void encode_uint_rec(emitter_t *self, unsigned int n);
static void emit_chars(emitter_t *self, const char *s);
static void emit_char(emitter_t *self, char c);

emitter_t *emitter_ws_new(char space, char tab, char newline) {
  emitter_ws_t *emitter = (emitter_ws_t *)AK_MEM_MALLOC(sizeof(emitter_ws_t));
  emitter->base.emit = ws_emit;
  emitter->base.end = ws_end;
  emitter->space = space;
  emitter->tab = tab;
  emitter->newline = newline;
  return (emitter_t *)emitter;
}


static void ws_emit(emitter_t *self, inst_t *inst) {
  switch (inst->opcode) {
  case OP_PUSH:
    emit_chars(self, "SS");
    encode_integer(self, inst->operand);
    break;
  case OP_COPY:
    emit_chars(self, "STS");
    encode_integer(self, inst->operand);
    break;
  case OP_SLIDE:
    emit_chars(self, "STL");
    encode_integer(self, inst->operand);
    break;
  case OP_DUP:
    emit_chars(self, "SLS");
    break;
  case OP_POP:
    emit_chars(self, "SLL");
    break;
  case OP_SWAP:
    emit_chars(self, "SLT");
    break;
  case OP_ADD:
    emit_chars(self, "TSSS");
    break;
  case OP_SUB:
    emit_chars(self, "TSST");
    break;
  case OP_MUL:
    emit_chars(self, "TSSL");
    break;
  case OP_DIV:
    emit_chars(self, "TSTS");
    break;
  case OP_MOD:
    emit_chars(self, "TSTT");
    break;
  case OP_STORE:
    emit_chars(self, "TTS");
    break;
  case OP_LOAD:
    emit_chars(self, "TTT");
    break;
  case OP_PUTC:
    emit_chars(self, "TLSS");
    break;
  case OP_PUTI:
    emit_chars(self, "TLST");
    break;
  case OP_GETC:
    emit_chars(self, "TLTS");
    break;
  case OP_GETI:
    emit_chars(self, "TLTT");
    break;
  case OP_LABEL:
    emit_chars(self, "LSS");
    encode_uint(self, (unsigned int)inst->operand);
    break;
  case OP_CALL:
    emit_chars(self, "LST");
    encode_uint(self, (unsigned int)inst->operand);
    break;
  case OP_JMP:
    emit_chars(self, "LSL");
    encode_uint(self, (unsigned int)inst->operand);
    break;
  case OP_JZ:
    emit_chars(self, "LTS");
    encode_uint(self, (unsigned int)inst->operand);
    break;
  case OP_JNEG:
    emit_chars(self, "LTT");
    encode_uint(self, (unsigned int)inst->operand);
    break;
  case OP_RET:
    emit_chars(self, "LTL");
    break;
  case OP_HALT:
    emit_chars(self, "LLL");
    break;
  }
}

static void ws_end(emitter_t *self) {
  emitter_ws_t *emitter = (emitter_ws_t *)self;

  /* if set to non-pure whitespace format, print newline on the end */
  if (emitter->space != ' ' || emitter->tab != '\t' || emitter->newline != '\n') {
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
    putchar(emitter->space);
    break;
  case 'T':
    putchar(emitter->tab);
    break;
  case 'L':
    putchar(emitter->newline);
    break;
  default:
    putchar(c);
    break;
  }
}
