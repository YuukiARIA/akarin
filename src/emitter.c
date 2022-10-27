#include <stdlib.h>
#include "emitter.h"

void emitter_release(emitter_t **pemitter) {
  free(*pemitter);
  *pemitter = NULL;
}

void emit_push(emitter_t *emitter, int value) {
  emitter->push(emitter, value);
}

void emit_pop(emitter_t *emitter) {
  emitter->pop(emitter);
}

void emit_swap(emitter_t *emitter) {
  emitter->swap(emitter);
}

void emit_add(emitter_t *emitter) {
  emitter->add(emitter);
}

void emit_sub(emitter_t *emitter) {
  emitter->sub(emitter);
}

void emit_mul(emitter_t *emitter) {
  emitter->mul(emitter);
}

void emit_div(emitter_t *emitter) {
  emitter->div(emitter);
}

void emit_mod(emitter_t *emitter) {
  emitter->mod(emitter);
}

void emit_store(emitter_t *emitter) {
  emitter->store(emitter);
}

void emit_load(emitter_t *emitter) {
  emitter->load(emitter);
}

void emit_getc(emitter_t *emitter) {
  emitter->getc(emitter);
}

void emit_geti(emitter_t *emitter) {
  emitter->geti(emitter);
}

void emit_putc(emitter_t *emitter) {
  emitter->putc(emitter);
}

void emit_puti(emitter_t *emitter) {
  emitter->puti(emitter);
}

void emit_label(emitter_t *emitter, int label) {
  emitter->label(emitter, label);
}

void emit_jmp(emitter_t *emitter, int label) {
  emitter->jmp(emitter, label);
}

void emit_jz(emitter_t *emitter, int label) {
  emitter->jz(emitter, label);
}

void emit_jneg(emitter_t *emitter, int label) {
  emitter->jneg(emitter, label);
}

void emit_halt(emitter_t *emitter) {
  emitter->halt(emitter);
}
