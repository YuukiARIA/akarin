#pragma once

#include "ttype.h"

typedef enum {
  UOP_INVALID,
  UOP_POSITIVE,
  UOP_NEGATIVE,
  UOP_NOT,
} unary_op_t;

typedef enum {
  BOP_INVALID,
  BOP_ADD,
  BOP_SUB,
  BOP_MUL,
  BOP_DIV,
  BOP_MOD,
  BOP_EQ,
  BOP_NEQ,
  BOP_AND,
  BOP_OR,
  BOP_LT,
  BOP_LE,
  BOP_GT,
  BOP_GE,
} binary_op_t;

const char *unary_op_to_string(unary_op_t uop);
const char *binary_op_to_string(binary_op_t bop);
binary_op_t ttype_to_binary_op(ttype_t ttype);
