#include <stdio.h>
#include <stdlib.h>
#include "emitter.h"

typedef struct {
  emitter_t base;
  char      space;
  char      tab;
  char      newline;
} emitter_ws_t;

static void ws_push(emitter_t *self, int value);
static void ws_copy(emitter_t *self, int n);
static void ws_slide(emitter_t *self, int n);
static void ws_dup(emitter_t *self);
static void ws_pop(emitter_t *self);
static void ws_swap(emitter_t *self);
static void ws_add(emitter_t *self);
static void ws_sub(emitter_t *self);
static void ws_mul(emitter_t *self);
static void ws_div(emitter_t *self);
static void ws_mod(emitter_t *self);
static void ws_store(emitter_t *self);
static void ws_load(emitter_t *self);
static void ws_putc(emitter_t *self);
static void ws_puti(emitter_t *self);
static void ws_getc(emitter_t *self);
static void ws_geti(emitter_t *self);
static void ws_label(emitter_t *self, int label_id);
static void ws_call(emitter_t *self, int label_id);
static void ws_jmp(emitter_t *self, int label_id);
static void ws_jz(emitter_t *self, int label_id);
static void ws_jneg(emitter_t *self, int label_id);
static void ws_ret(emitter_t *self);
static void ws_halt(emitter_t *self);
static void ws_end(emitter_t *self);

static void encode_integer(emitter_t *self, int n);
static void encode_uint(emitter_t *self, unsigned int n);
static void encode_uint_rec(emitter_t *self, unsigned int n);
static void emit_chars(emitter_t *self, const char *s);
static void emit_char(emitter_t *self, char c);

emitter_t *emitter_ws_new(char space, char tab, char newline) {
  emitter_ws_t *emitter = (emitter_ws_t *)malloc(sizeof(emitter_ws_t));

  EMITTER_OVERRIDE(emitter->base, ws);

  emitter->space = space;
  emitter->tab = tab;
  emitter->newline = newline;

  return (emitter_t *)emitter;
}

static void ws_push(emitter_t *self, int value) {
  emit_chars(self, "SS");
  encode_integer(self, value);
}

static void ws_copy(emitter_t *self, int n) {
  emit_chars(self, "STS");
  encode_integer(self, n);
}

static void ws_slide(emitter_t *self, int n) {
  emit_chars(self, "STL");
  encode_integer(self, n);
}

static void ws_dup(emitter_t *self) {
  emit_chars(self, "SLS");
}

static void ws_pop(emitter_t *self) {
  emit_chars(self, "SLL");
}

static void ws_swap(emitter_t *self) {
  emit_chars(self, "SLT");
}

static void ws_add(emitter_t *self) {
  emit_chars(self, "TSSS");
}

static void ws_sub(emitter_t *self) {
  emit_chars(self, "TSST");
}

static void ws_mul(emitter_t *self) {
  emit_chars(self, "TSSL");
}

static void ws_div(emitter_t *self) {
  emit_chars(self, "TSTS");
}

static void ws_mod(emitter_t *self) {
  emit_chars(self, "TSTT");
}

static void ws_store(emitter_t *self) {
  emit_chars(self, "TTS");
}

static void ws_load(emitter_t *self) {
  emit_chars(self, "TTT");
}

static void ws_putc(emitter_t *self) {
  emit_chars(self, "TLSS");
}

static void ws_puti(emitter_t *self) {
  emit_chars(self, "TLST");
}

static void ws_getc(emitter_t *self) {
  emit_chars(self, "TLTS");
}

static void ws_geti(emitter_t *self) {
  emit_chars(self, "TLTT");
}

static void ws_label(emitter_t *self, int label_id) {
  emit_chars(self, "LSS");
  encode_uint(self, (unsigned int)label_id);
}

static void ws_call(emitter_t *self, int label_id) {
  emit_chars(self, "LST");
  encode_uint(self, (unsigned int)label_id);
}

static void ws_jmp(emitter_t *self, int label_id) {
  emit_chars(self, "LSL");
  encode_uint(self, (unsigned int)label_id);
}

static void ws_jz(emitter_t *self, int label_id) {
  emit_chars(self, "LTS");
  encode_uint(self, (unsigned int)label_id);
}

static void ws_jneg(emitter_t *self, int label_id) {
  emit_chars(self, "LTT");
  encode_uint(self, (unsigned int)label_id);
}

static void ws_ret(emitter_t *self) {
  emit_chars(self, "LTL");
}

static void ws_halt(emitter_t *self) {
  emit_chars(self, "LLL");
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
