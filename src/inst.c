#include "inst.h"
#include "utils/memory.h"

inst_t *inst_new(opcode_t opcode, int operand) {
  inst_t *inst = (inst_t *)AK_MEM_MALLOC(sizeof(inst_t));
  inst->opcode = opcode;
  inst->operand = operand;
  return inst;
}

void inst_release(inst_t **pinst) {
  AK_MEM_FREE(*pinst);
  *pinst = NULL;
}

int inst_stack_size(inst_t *inst) {
  switch (inst->opcode) {
  case OP_PUSH:
  case OP_COPY:
  case OP_DUP:
    return 1;

  case OP_SLIDE:
    return inst->operand;

  case OP_POP:
  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_DIV:
  case OP_MOD:
  case OP_STORE:
  case OP_LOAD:
  case OP_PUTC:
  case OP_PUTI:
  case OP_GETC:
  case OP_GETI:
  case OP_JZ:
  case OP_JNEG:
    return -1;

  case OP_SWAP:
  case OP_LABEL:
  case OP_CALL:
  case OP_JMP:
  case OP_RET:
  case OP_HALT:
    return 0;
  }
}
