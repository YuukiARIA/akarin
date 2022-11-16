#include "opcode.h"

static const struct {
  const char *ws;
  const char *str;
} g_data[] = {
  { ""    , "NOP"   },
  { "SS"  , "PUSH"  },
  { "STS" , "COPY"  },
  { "STL" , "SLIDE" },
  { "SLS" , "DUP"   },
  { "SLL" , "POP"   },
  { "SLT" , "SWAP"  },
  { "TSSS", "ADD"   },
  { "TSST", "SUB"   },
  { "TSSL", "MUL"   },
  { "TSTS", "DIV"   },
  { "TSTT", "MOD"   },
  { "TTS" , "STORE" },
  { "TTT" , "LOAD"  },
  { "TLSS", "PUTC"  },
  { "TLST", "PUTI"  },
  { "TLTS", "GETC"  },
  { "TLTT", "GETI"  },
  { "LSS" , "LABEL" },
  { "LST" , "CALL"  },
  { "LSL" , "JMP"   },
  { "LTS" , "JZ"    },
  { "LTT" , "JNEG"  },
  { "LTL" , "RET"   },
  { "LLL" , "HALT"  }
};

const char *opcode_to_ws(opcode_t opcode) {
  return g_data[opcode].ws;
}

const char *opcode_to_str(opcode_t opcode) {
  return g_data[opcode].str;
}
