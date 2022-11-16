#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "opcode.h"
#include "emitter.h"
#include "label.h"
#include "utils/memory.h"

typedef struct {
  emitter_t base;
  int       indent;
} emitter_pseudo_t;

static void pseudo_emit(emitter_t *self, inst_t *inst);
static void pseudo_end(emitter_t *self);

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
  if (inst->opcode != OP_LABEL) {
    indent_printf(self, "%s", opcode_to_str(inst->opcode));
  }

  switch (inst->opcode) {
  case OP_PUSH:
  case OP_COPY:
  case OP_SLIDE:
    printf(" %d", inst->value);
    break;
  case OP_LABEL:
    printf("L%d:", label_get_unified_id(inst->label));
    break;
  case OP_CALL:
  case OP_JMP:
  case OP_JZ:
  case OP_JNEG:
    printf(" L%d", label_get_unified_id(inst->label));
    break;
  default:
    break;
  }

  putchar('\n');
}

static void pseudo_end(emitter_t *self) {
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
