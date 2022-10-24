#include "operator.h"

const char* unary_op_to_string(unary_op_t uop) {
  switch (uop) {
  case UOP_POSITIVE: return "POSITIVE";
  case UOP_NEGATIVE: return "NEGATIVE";
  case UOP_NOT:      return "NOT";
  default:           return "INVALID";
  }
}

const char* binary_op_to_string(binary_op_t bop) {
  switch (bop) {
  case BOP_EQ:  return "EQ";
  case BOP_NEQ: return "NEQ";
  case BOP_LT:  return "LT";
  case BOP_LE:  return "LE";
  case BOP_GT:  return "GT";
  case BOP_GE:  return "GE";
  case BOP_ADD: return "ADD";
  case BOP_SUB: return "SUB";
  case BOP_MUL: return "MUL";
  case BOP_DIV: return "DIV";
  case BOP_MOD: return "MOD";
  case BOP_AND: return "AND";
  case BOP_OR:  return "OR";
  default:      return "INVALID";
  }
}
