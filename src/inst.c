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
