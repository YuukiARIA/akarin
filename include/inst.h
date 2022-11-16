#pragma once

typedef enum {
  OP_PUSH,
  OP_COPY,
  OP_SLIDE,
  OP_DUP,
  OP_POP,
  OP_SWAP,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_STORE,
  OP_LOAD,
  OP_PUTC,
  OP_PUTI,
  OP_GETC,
  OP_GETI,
  OP_LABEL,
  OP_CALL,
  OP_JMP,
  OP_JZ,
  OP_JNEG,
  OP_RET,
  OP_HALT
} opcode_t;

typedef struct {
  opcode_t opcode;
  int      operand;
} inst_t;

inst_t *inst_new_push(int value);
inst_t *inst_new_copy(int value);
inst_t *inst_new_slide(int value);
inst_t *inst_new_dup(void);
inst_t *inst_new_pop(void);
inst_t *inst_new_swap(void);
inst_t *inst_new_add(void);
inst_t *inst_new_sub(void);
inst_t *inst_new_mul(void);
inst_t *inst_new_div(void);
inst_t *inst_new_mod(void);
inst_t *inst_new_store(void);
inst_t *inst_new_load(void);
inst_t *inst_new_putc(void);
inst_t *inst_new_puti(void);
inst_t *inst_new_getc(void);
inst_t *inst_new_geti(void);
inst_t *inst_new_label(int label);
inst_t *inst_new_call(int label);
inst_t *inst_new_jmp(int label);
inst_t *inst_new_jz(int label);
inst_t *inst_new_jneg(int label);
inst_t *inst_new_ret(void);
inst_t *inst_new_halt(void);

void    inst_release(inst_t **pinst);
