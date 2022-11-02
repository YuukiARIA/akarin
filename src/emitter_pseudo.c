#include <stdio.h>
#include <stdlib.h>
#include "emitter.h"

typedef struct {
  emitter_t base;
  int       indent;
} emitter_pseudo_t;

static void pseudo_push(emitter_t *self, int value);
static void pseudo_copy(emitter_t *self, int n);
static void pseudo_slide(emitter_t *self, int n);
static void pseudo_dup(emitter_t *self);
static void pseudo_pop(emitter_t *self);
static void pseudo_swap(emitter_t *self);
static void pseudo_add(emitter_t *self);
static void pseudo_sub(emitter_t *self);
static void pseudo_mul(emitter_t *self);
static void pseudo_div(emitter_t *self);
static void pseudo_mod(emitter_t *self);
static void pseudo_store(emitter_t *self);
static void pseudo_load(emitter_t *self);
static void pseudo_putc(emitter_t *self);
static void pseudo_puti(emitter_t *self);
static void pseudo_getc(emitter_t *self);
static void pseudo_geti(emitter_t *self);
static void pseudo_label(emitter_t *self, int label);
static void pseudo_call(emitter_t *self, int label);
static void pseudo_jmp(emitter_t *self, int label);
static void pseudo_jz(emitter_t *self, int label);
static void pseudo_jneg(emitter_t *self, int label);
static void pseudo_ret(emitter_t *self);
static void pseudo_halt(emitter_t *self);
static void pseudo_end(emitter_t *self);

static void indent_puts(emitter_t *self, const char *str);
static void indent(emitter_t *self);

emitter_t *emitter_pseudo_new(int indent) {
  emitter_pseudo_t *emitter = (emitter_pseudo_t *)malloc(sizeof(emitter_pseudo_t));

  EMITTER_OVERRIDE(emitter->base, pseudo);

  emitter->indent = indent;

  return (emitter_t *)emitter;
}

static void pseudo_push(emitter_t *self, int value) {
  indent(self);
  printf("PUSH %d\n", value);
}

static void pseudo_copy(emitter_t *self, int n) {
  indent(self);
  printf("COPY %d\n", n);
}

static void pseudo_slide(emitter_t *self, int n) {
  indent(self);
  printf("SLIDE %d\n", n);
}

static void pseudo_dup(emitter_t *self) {
  indent_puts(self, "DUP");
}

static void pseudo_pop(emitter_t *self) {
  indent_puts(self, "POP");
}

static void pseudo_swap(emitter_t *self) {
  indent_puts(self, "SWAP");
}

static void pseudo_add(emitter_t *self) {
  indent_puts(self, "ADD");
}

static void pseudo_sub(emitter_t *self) {
  indent_puts(self, "SUB");
}

static void pseudo_mul(emitter_t *self) {
  indent_puts(self, "MUL");
}

static void pseudo_div(emitter_t *self) {
  indent_puts(self, "DIV");
}

static void pseudo_mod(emitter_t *self) {
  indent_puts(self, "MOD");
}

static void pseudo_store(emitter_t *self) {
  indent_puts(self, "STORE");
}

static void pseudo_load(emitter_t *self) {
  indent_puts(self, "LOAD");
}

static void pseudo_putc(emitter_t *self) {
  indent_puts(self, "PUTC");
}

static void pseudo_puti(emitter_t *self) {
  indent_puts(self, "PUTI");
}

static void pseudo_getc(emitter_t *self) {
  indent_puts(self, "GETC");
}

static void pseudo_geti(emitter_t *self) {
  indent_puts(self, "GETI");
}

static void pseudo_label(emitter_t *self, int label) {
  printf("L%d:\n", label);
}

static void pseudo_call(emitter_t *self, int label) {
  indent(self);
  printf("CALL L%d\n", label);
}

static void pseudo_jmp(emitter_t *self, int label) {
  indent(self);
  printf("JMP L%d\n", label);
}

static void pseudo_jz(emitter_t *self, int label) {
  indent(self);
  printf("JZ L%d\n", label);
}

static void pseudo_jneg(emitter_t *self, int label) {
  indent(self);
  printf("JNEG L%d\n", label);
}

static void pseudo_ret(emitter_t *self) {
  indent_puts(self, "RET");
}

static void pseudo_halt(emitter_t *self) {
  indent_puts(self, "HALT");
}

static void pseudo_end(emitter_t *self) {
}

static void indent_puts(emitter_t *self, const char *str) {
  indent(self);
  puts(str);
}

static void indent(emitter_t *self) {
  int n = ((emitter_pseudo_t *)self)->indent;
  while (n-- > 0) {
    putchar(' ');
  }
}
