#include <stdlib.h>
#include "emitter.h"
#include "inst.h"
#include "utils/memory.h"
#include "utils/array.h"

static void emit(emitter_t *emitter, inst_t *inst);

void emitter_release(emitter_t **pemitter) {
  AK_MEM_FREE(*pemitter);
  *pemitter = NULL;
}

void emitter_emit_code(emitter_t *emitter, array_t *instructions) {
  for (int i = 0; i < array_count(instructions); ++i) {
    inst_t *inst = (inst_t *)array_get(instructions, i);
    emit(emitter, inst);
  }
  emitter->end(emitter);
}

static void emit(emitter_t *emitter, inst_t *inst) {
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
