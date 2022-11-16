#pragma once

#include "opcode.h"
#include "label.h"

typedef struct {
  opcode_t opcode;
  union {
    int      value;
    label_t *label;
  };
} inst_t;

inst_t *inst_new_nop(void);
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
inst_t *inst_new_label(label_t *label);
inst_t *inst_new_call(label_t *label);
inst_t *inst_new_jmp(label_t *label);
inst_t *inst_new_jz(label_t *label);
inst_t *inst_new_jneg(label_t *label);
inst_t *inst_new_ret(void);
inst_t *inst_new_halt(void);

void    inst_release(inst_t **pinst);
