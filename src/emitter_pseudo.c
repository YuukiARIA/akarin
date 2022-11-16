#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "emitter.h"
#include "label.h"
#include "utils/memory.h"

typedef struct {
  emitter_t base;
  int       indent;
} emitter_pseudo_t;

static void pseudo_emit(emitter_t *self, inst_t *inst);
static void pseudo_end(emitter_t *self);

static void indent_puts(emitter_t *self, const char *str);
static void indent_printf(emitter_t *self, const char *fmt, ...);
static void indent(emitter_t *self);

emitter_t *emitter_pseudo_new(int indent) {
  emitter_pseudo_t *emitter = (emitter_pseudo_t *)AK_MEM_MALLOC(sizeof(emitter_pseudo_t));
  emitter->base.emit = pseudo_emit;
  emitter->base.end = pseudo_end;
  emitter->indent = indent;
  return (emitter_t *)emitter;
}

static void pseudo_emit(emitter_t *self, inst_t *inst) {
  switch (inst->opcode) {
  case OP_PUSH:
    indent_printf(self, "PUSH %d\n", inst->value);
    break;
  case OP_COPY:
    indent_printf(self, "COPY %d\n", inst->value);
    break;
  case OP_SLIDE:
    indent_printf(self, "SLIDE %d\n", inst->value);
    break;
  case OP_DUP:
    indent_puts(self, "DUP");
    break;
  case OP_POP:
    indent_puts(self, "POP");
    break;
  case OP_SWAP:
    indent_puts(self, "SWAP");
    break;
  case OP_ADD:
    indent_puts(self, "ADD");
    break;
  case OP_SUB:
    indent_puts(self, "SUB");
    break;
  case OP_MUL:
    indent_puts(self, "MUL");
    break;
  case OP_DIV:
    indent_puts(self, "DIV");
    break;
  case OP_MOD:
    indent_puts(self, "MOD");
    break;
  case OP_STORE:
    indent_puts(self, "STORE");
    break;
  case OP_LOAD:
    indent_puts(self, "LOAD");
    break;
  case OP_PUTC:
    indent_puts(self, "PUTC");
    break;
  case OP_PUTI:
    indent_puts(self, "PUTI");
    break;
  case OP_GETC:
    indent_puts(self, "GETC");
    break;
  case OP_GETI:
    indent_puts(self, "GETI");
    break;
  case OP_LABEL:
    printf("L%d:\n", label_get_unified_id(inst->label));
    break;
  case OP_CALL:
    indent_printf(self, "CALL L%d\n", label_get_unified_id(inst->label));
    break;
  case OP_JMP:
    indent_printf(self, "JMP L%d\n", label_get_unified_id(inst->label));
    break;
  case OP_JZ:
    indent_printf(self, "JZ L%d\n", label_get_unified_id(inst->label));
    break;
  case OP_JNEG:
    indent_printf(self, "JNEG L%d\n", label_get_unified_id(inst->label));
    break;
  case OP_RET:
    indent_puts(self, "RET");
    break;
  case OP_HALT:
    indent_puts(self, "HALT");
    break;
  }
}

static void pseudo_end(emitter_t *self) {
}

static void indent_puts(emitter_t *self, const char *str) {
  indent(self);
  puts(str);
}

static void indent_printf(emitter_t *self, const char *fmt, ...) {
  va_list args;

  indent(self);

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

static void indent(emitter_t *self) {
  int n = ((emitter_pseudo_t *)self)->indent;
  while (n-- > 0) {
    putchar(' ');
  }
}
