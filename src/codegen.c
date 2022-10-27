#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "node.h"
#include "operator.h"

struct codegen_t {
  node_t *root;
  int     label_count;
  int     var_count;
  char    vars[256][64];
  int     cur_label_tail;
};

static void gen(codegen_t *codegen, node_t *node);
static void gen_if_statement(codegen_t *codegen, node_t *node);
static void gen_while_statement(codegen_t *codegen, node_t *node);
static void gen_break_statement(codegen_t *codegen, node_t *node);
static void gen_geti_statement(codegen_t *codegen, node_t *node);
static void gen_getc_statement(codegen_t *codegen, node_t *node);
static void gen_unary(codegen_t *codegen, node_t *node);
static void gen_assign(codegen_t *codegen, node_t *node);
static void gen_arith(codegen_t *codegen, node_t *node);
static void gen_push(int value);
static void gen_pop(void);
static void gen_swap(void);
static void gen_add(void);
static void gen_sub(void);
static void gen_mul(void);
static void gen_div(void);
static void gen_mod(void);
static void gen_store(void);
static void gen_load(void);
static void gen_putc(void);
static void gen_puti(void);
static void gen_getc(void);
static void gen_geti(void);
static void gen_jmp(int label_id);
static void gen_jz(int label_id);
static void gen_jneg(int label_id);
static void gen_halt(void);
static void encode_integer(int n);
static void encode_uint(unsigned int n);
static void encode_uint_rec(unsigned int n);
static int  alloc_label_id(codegen_t *codegen);
static void gen_label(int label_id);
static int  get_var_index(codegen_t *codegen, const char *name);
static int  allocate(codegen_t *codegen, const char *name, int size);

codegen_t *codegen_new(node_t *root) {
  codegen_t *codegen = (codegen_t *)malloc(sizeof(codegen_t));
  codegen->root = root;
  codegen->label_count = 0;
  codegen->cur_label_tail = -1;
  return codegen;
}

void codegen_release(codegen_t **pcodegen) {
  free(*pcodegen);
  *pcodegen = NULL;
}

void codegen_generate(codegen_t *codegen) {
  gen(codegen, codegen->root);
  gen_halt();
  putchar('\n');
}

static void gen(codegen_t *codegen, node_t *node) {
  switch (node_get_ntype(node)) {
  case NT_SEQ:
    gen(codegen, node_get_l(node));
    gen(codegen, node_get_r(node));
    break;
  case NT_IF:
    gen_if_statement(codegen, node);
    break;
  case NT_WHILE:
    gen_while_statement(codegen, node);
    break;
  case NT_BREAK:
    gen_break_statement(codegen, node);
    break;
  case NT_PUTI:
    gen(codegen, node_get_l(node));
    gen_puti();
    break;
  case NT_PUTC:
    gen(codegen, node_get_l(node));
    gen_putc();
    break;
  case NT_UNARY:
    gen_unary(codegen, node);
    break;
  case NT_BINARY:
    if (node_get_bop(node) == BOP_ASSIGN) {
      gen_assign(codegen, node);
    }
    else {
      gen_arith(codegen, node);
    }
    break;
  case NT_INTEGER:
    gen_push(node_get_value(node));
    break;
  case NT_VARIABLE:
    {
      int var_index = get_var_index(codegen, node_get_name(node));
      gen_push(var_index);
      gen_load();
    }
    break;
  case NT_ARRAY:
    {
      int var_index = get_var_index(codegen, node_get_name(node_get_l(node)));
      gen_push(var_index);
      gen(codegen, node_get_r(node));
      gen_add();
      gen_load();
    }
    break;
  case NT_GETI:
    gen_geti_statement(codegen, node);
    break;
  case NT_GETC:
    gen_getc_statement(codegen, node);
    break;
  case NT_ARRAY_DECL:
    {
      int size = node_get_value(node);
      allocate(codegen, node_get_name(node_get_l(node)), size);
    }
    break;
  case NT_HALT:
    gen_halt();
    break;
  default:
    break;
  }
}

static void gen_if_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node);
  node_t *then = node_get_l(node);
  node_t *els = node_get_r(node);
  int l1 = alloc_label_id(codegen);
  int l2 = alloc_label_id(codegen);

  gen(codegen, cond);
  gen_jz(l1);
  gen(codegen, then);
  gen_jmp(l2);
  gen_label(l1);
  if (els) {
    gen(codegen, els);
  }
  gen_label(l2);
}

static void gen_while_statement(codegen_t *codegen, node_t *node) {
  node_t *cond = node_get_cond(node);
  node_t *body = node_get_l(node);
  int label_head = alloc_label_id(codegen);
  int label_tail = alloc_label_id(codegen);

  gen_label(label_head);
  gen(codegen, cond);
  gen_jz(label_tail);

  codegen->cur_label_tail = label_tail;
  gen(codegen, body);
  codegen->cur_label_tail = -1;

  gen_jmp(label_head);
  gen_label(label_tail);
}

static void gen_break_statement(codegen_t *codegen, node_t *node) {
  int label = codegen->cur_label_tail;
  if (label == -1) {
    fputs("illegal break statement.", stderr);
    return;
  }

  gen_jmp(label);
}

static void gen_geti_statement(codegen_t *codegen, node_t *node) {
  node_t *var = node_get_l(node);
  int var_index = get_var_index(codegen, node_get_name(var));
  gen_push(var_index);
  gen_geti();
}

static void gen_getc_statement(codegen_t *codegen, node_t *node) {
  node_t *var = node_get_l(node);
  int var_index = get_var_index(codegen, node_get_name(var));
  gen_push(var_index);
  gen_getc();
}

static void gen_unary(codegen_t *codegen, node_t *node) {
  switch (node_get_uop(node)) {
  case UOP_NEGATIVE: /* implement -x as 0 - x. */
    gen_push(0);
    gen(codegen, node_get_l(node));
    gen_sub();
    break;
  case UOP_NOT:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen(codegen, node_get_l(node));
      gen_jz(l1);
      gen_push(0);
      gen_jmp(l2);
      gen_label(l1);
      gen_push(1);
      gen_label(l2);
    }
    break;
  default:
    break;
  }
}

static void gen_assign(codegen_t *codegen, node_t *node) {
  node_t *lhs = node_get_l(node);
  node_t *expr = node_get_r(node);

  switch (node_get_ntype(lhs)) {
  case NT_VARIABLE:
    {
      int var_index = get_var_index(codegen, node_get_name(lhs));
      gen_push(var_index);
    }
    break;
  case NT_ARRAY:
    {
      int var_index = get_var_index(codegen, node_get_name(node_get_l(lhs)));
      gen_push(var_index);
      gen(codegen, node_get_r(lhs));
      gen_add();
    }
    break;
  default:
    fprintf(stderr, "error: invalid left hand value. (ntype=%d)\n", node_get_ntype(lhs));
    return;
  }

  gen(codegen, expr);
  gen_store();
}

static void gen_arith(codegen_t *codegen, node_t *node) {
  gen(codegen, node_get_l(node));
  gen(codegen, node_get_r(node));

  switch (node_get_bop(node)) {
  case BOP_ADD:
    gen_add();
    break;
  case BOP_SUB:
    gen_sub();
    break;
  case BOP_MUL:
    gen_mul();
    break;
  case BOP_DIV:
    gen_div();
    break;
  case BOP_MOD:
    gen_mod();
    break;
  case BOP_OR:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      int l3 = alloc_label_id(codegen);
      gen_jz(l1);
      gen_pop();
      gen_push(1);
      gen_jmp(l3);
      gen_label(l1);
      gen_jz(l2);
      gen_push(1);
      gen_jmp(l3);
      gen_label(l2);
      gen_push(0);
      gen_label(l3);
    }
    break;
  case BOP_AND:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      int l3 = alloc_label_id(codegen);
      int l4 = alloc_label_id(codegen);
      gen_jz(l1);
      gen_jz(l2);
      gen_jmp(l3);
      gen_label(l1);
      gen_pop();
      gen_label(l2);
      gen_push(0);
      gen_jmp(l4);
      gen_label(l3);
      gen_push(1);
      gen_label(l4);
    }
    break;
  case BOP_EQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen_sub();       /* SUB */
      gen_jz(l1);      /* JZ L1 */
      gen_push(0);     /* PUSH 0 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(1);     /* PUSH 1 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_NEQ:
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen_sub();       /* SUB */
      gen_jz(l1);      /* JZ L1 */
      gen_push(1);     /* PUSH 1 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(0);     /* PUSH 0 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_LT: /* x < y --> x - y < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen_sub();       /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(0);     /* PUSH 0 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(1);     /* PUSH 1 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_LE: /* x <= y --> !(y - x < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen_swap();      /* SWAP */
      gen_sub();       /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(1);     /* PUSH 1 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(0);     /* PUSH 0 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_GT: /* x > y --> y - x < 0 */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen_swap();      /* SWAP */
      gen_sub();       /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(0);     /* PUSH 0 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(1);     /* PUSH 1 */
      gen_label(l2);   /* :L2 */
    }
    break;
  case BOP_GE: /* x >= y --> !(x - y < 0) */
    {
      int l1 = alloc_label_id(codegen);
      int l2 = alloc_label_id(codegen);
      gen_sub();       /* SUB */
      gen_jneg(l1);    /* JNEG L1 */
      gen_push(1);     /* PUSH 1 */
      gen_jmp(l2);     /* JMP L2 */
      gen_label(l1);   /* :L1 */
      gen_push(0);     /* PUSH 0 */
      gen_label(l2);   /* :L2 */
    }
    break;
  default:
    break;
  }
}

static void gen_push(int value) {
  printf("SS");
  encode_integer(value);
}

static void gen_pop(void) {
  printf("SLL");
}

static void gen_swap(void) {
  printf("SLT");
}

static void gen_add(void) {
  printf("TSSS");
}

static void gen_sub(void) {
  printf("TSST");
}

static void gen_mul(void) {
  printf("TSSL");
}

static void gen_div(void) {
  printf("TSTS");
}

static void gen_mod(void) {
  printf("TSTT");
}

static void gen_store(void) {
  printf("TTS");
}

static void gen_load(void) {
  printf("TTT");
}

static void gen_putc(void) {
  printf("TLSS");
}

static void gen_puti(void) {
  printf("TLST");
}

static void gen_getc(void) {
  printf("TLTS");
}

static void gen_geti(void) {
  printf("TLTT");
}

static void gen_jmp(int label_id) {
  printf("LSL");
  encode_uint((unsigned int)label_id);
}

static void gen_jz(int label_id) {
  printf("LTS");
  encode_uint((unsigned int)label_id);
}

static void gen_jneg(int label_id) {
  printf("LTT");
  encode_uint((unsigned int)label_id);
}

static void gen_halt(void) {
  printf("LLL");
}

static void encode_integer(int n) {
  putchar(n >= 0 ? 'S' : 'T');
  encode_uint((unsigned int)abs(n));
}

static void encode_uint(unsigned int n) {
  if (n == 0) {
    putchar('S');
  }
  else {
    encode_uint_rec(n);
  }
  putchar('L');
}

static void encode_uint_rec(unsigned int n) {
  if (n != 0) {
    encode_uint_rec(n >> 1);
    putchar((n & 1) ? 'T' : 'S');
  }
}

static int alloc_label_id(codegen_t *codegen) {
  return codegen->label_count++;
}

static void gen_label(int label_id) {
  printf("LSS");
  encode_uint((unsigned int)label_id);
}

static int get_var_index(codegen_t *codegen, const char *name) {
  int i;
  for (i = 0; i < codegen->var_count; ++i) {
    if (strcmp(codegen->vars[i], name) == 0) {
      return i;
    }
  }

  /* register name */
  return allocate(codegen, name, 1);
}

static int allocate(codegen_t *codegen, const char *name, int size) {
  int i = codegen->var_count;
  strcpy(codegen->vars[i], name);
  codegen->var_count += size;
  return i;
}
