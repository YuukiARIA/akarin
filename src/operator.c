#include "operator.h"

const char *unary_op_to_string(unary_op_t uop) {
  switch (uop) {
  case UOP_POSITIVE: return "POSITIVE";
  case UOP_NEGATIVE: return "NEGATIVE";
  case UOP_NOT:      return "NOT";
  default:           return "INVALID";
  }
}

const char *binary_op_to_string(binary_op_t bop) {
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

binary_op_t ttype_to_binary_op(ttype_t ttype) {
  switch (ttype) {
  case TT_EQEQ:     return BOP_EQ;
  case TT_EXCLAEQ:  return BOP_NEQ;
  case TT_LT:       return BOP_LT;
  case TT_LE:       return BOP_LE;
  case TT_GT:       return BOP_GT;
  case TT_GE:       return BOP_GE;
  case TT_PLUS:     return BOP_ADD;
  case TT_MINUS:    return BOP_SUB;
  case TT_ASTERISK: return BOP_MUL;
  case TT_SLASH:    return BOP_DIV;
  case TT_PERCENT:  return BOP_MOD;
  case TT_AMP:      return BOP_AND;
  case TT_BAR:      return BOP_OR;
  default:          return BOP_INVALID;
  }
}
