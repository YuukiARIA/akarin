#include "inst.h"
#include "utils/memory.h"

static inst_t *inst_new(opcode_t opcode) {
  inst_t *inst = (inst_t *)AK_MEM_MALLOC(sizeof(inst_t));
  inst->opcode = opcode;
  return inst;
}

static inst_t *inst_new_with_value(opcode_t opcode, int value) {
  inst_t *inst = inst_new(opcode);
  inst->value = value;
  return inst;
}

static inst_t *inst_new_with_label(opcode_t opcode, label_t *label) {
  inst_t *inst = inst_new(opcode);
  inst->label = label;
  return inst;
}

inst_t *inst_new_push(int value) {
  return inst_new_with_value(OP_PUSH, value);
}

inst_t *inst_new_copy(int value) {
  return inst_new_with_value(OP_COPY, value);
}

inst_t *inst_new_slide(int value) {
  return inst_new_with_value(OP_SLIDE, value);
}

inst_t *inst_new_dup(void) {
  return inst_new(OP_DUP);
}

inst_t *inst_new_pop(void) {
  return inst_new(OP_POP);
}

inst_t *inst_new_swap(void) {
  return inst_new(OP_SWAP);
}

inst_t *inst_new_add(void) {
  return inst_new(OP_ADD);
}

inst_t *inst_new_sub(void) {
  return inst_new(OP_SUB);
}

inst_t *inst_new_mul(void) {
  return inst_new(OP_MUL);
}

inst_t *inst_new_div(void) {
  return inst_new(OP_DIV);
}

inst_t *inst_new_mod(void) {
  return inst_new(OP_MOD);
}

inst_t *inst_new_store(void) {
  return inst_new(OP_STORE);
}

inst_t *inst_new_load(void) {
  return inst_new(OP_LOAD);
}

inst_t *inst_new_putc(void) {
  return inst_new(OP_PUTC);
}

inst_t *inst_new_puti(void) {
  return inst_new(OP_PUTI);
}

inst_t *inst_new_getc(void) {
  return inst_new(OP_GETC);
}

inst_t *inst_new_geti(void) {
  return inst_new(OP_GETI);
}

inst_t *inst_new_label(label_t *label) {
  return inst_new_with_label(OP_LABEL, label);
}

inst_t *inst_new_call(label_t *label) {
  return inst_new_with_label(OP_CALL, label);
}

inst_t *inst_new_jmp(label_t *label) {
  return inst_new_with_label(OP_JMP, label);
}

inst_t *inst_new_jz(label_t *label) {
  return inst_new_with_label(OP_JZ, label);
}

inst_t *inst_new_jneg(label_t *label) {
  return inst_new_with_label(OP_JNEG, label);
}

inst_t *inst_new_ret(void) {
  return inst_new(OP_RET);
}

inst_t *inst_new_halt(void) {
  return inst_new(OP_HALT);
}

void inst_release(inst_t **pinst) {
  AK_MEM_FREE(*pinst);
  *pinst = NULL;
}
