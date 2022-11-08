#include <stdlib.h>
#include "emitter.h"
#include "inst.h"
#include "utils/memory.h"

void emitter_release(emitter_t **pemitter) {
  AK_MEM_FREE(*pemitter);
  *pemitter = NULL;
}

void emit(emitter_t *emitter, inst_t *inst) {
  switch (inst->opcode) {
  case OP_PUSH:
    emitter->push(emitter, inst->operand);
    break;
  case OP_COPY:
    emitter->copy(emitter, inst->operand);
    break;
  case OP_SLIDE:
    emitter->slide(emitter, inst->operand);
    break;
  case OP_DUP:
    emitter->dup(emitter);
    break;
  case OP_POP:
    emitter->pop(emitter);
    break;
  case OP_SWAP:
    emitter->swap(emitter);
    break;
  case OP_ADD:
    emitter->add(emitter);
    break;
  case OP_SUB:
    emitter->sub(emitter);
    break;
  case OP_MUL:
    emitter->mul(emitter);
    break;
  case OP_DIV:
    emitter->div(emitter);
    break;
  case OP_MOD:
    emitter->mod(emitter);
    break;
  case OP_STORE:
    emitter->store(emitter);
    break;
  case OP_LOAD:
    emitter->load(emitter);
    break;
  case OP_PUTC:
    emitter->putc(emitter);
    break;
  case OP_PUTI:
    emitter->puti(emitter);
    break;
  case OP_GETC:
    emitter->getc(emitter);
    break;
  case OP_GETI:
    emitter->geti(emitter);
    break;
  case OP_LABEL:
    emitter->label(emitter, inst->operand);
    break;
  case OP_CALL:
    emitter->call(emitter, inst->operand);
    break;
  case OP_JMP:
    emitter->jmp(emitter, inst->operand);
    break;
  case OP_JZ:
    emitter->jz(emitter, inst->operand);
    break;
  case OP_JNEG:
    emitter->jneg(emitter, inst->operand);
    break;
  case OP_RET:
    emitter->ret(emitter);
    break;
  case OP_HALT:
    emitter->halt(emitter);
    break;
  }
}

void emit_push(emitter_t *emitter, int value) {
  emitter->push(emitter, value);
}

void emit_copy(emitter_t *emitter, int n) {
  emitter->copy(emitter, n);
}

void emit_slide(emitter_t *emitter, int n) {
  emitter->slide(emitter, n);
}

void emit_dup(emitter_t *emitter) {
  emitter->dup(emitter);
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

void emit_call(emitter_t *emitter, int label) {
  emitter->call(emitter, label);
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

void emit_ret(emitter_t *emitter) {
  emitter->ret(emitter);
}

void emit_halt(emitter_t *emitter) {
  emitter->halt(emitter);
}

void emit_end(emitter_t *emitter) {
  emitter->end(emitter);
}
