#pragma once

typedef enum {
  OP_NOP,
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

const char *opcode_to_ws(opcode_t opcode);
const char *opcode_to_str(opcode_t opcode);
