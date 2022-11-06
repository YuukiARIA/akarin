#pragma once

typedef struct emitter_t emitter_t;

struct emitter_t {
  void (*push)(emitter_t *self, int value);
  void (*copy)(emitter_t *self, int n);
  void (*slide)(emitter_t *self, int n);
  void (*dup)(emitter_t *self);
  void (*pop)(emitter_t *self);
  void (*swap)(emitter_t *self);
  void (*add)(emitter_t *self);
  void (*sub)(emitter_t *self);
  void (*mul)(emitter_t *self);
  void (*div)(emitter_t *self);
  void (*mod)(emitter_t *self);
  void (*store)(emitter_t *self);
  void (*load)(emitter_t *self);
  void (*putc)(emitter_t *self);
  void (*puti)(emitter_t *self);
  void (*getc)(emitter_t *self);
  void (*geti)(emitter_t *self);
  void (*label)(emitter_t *self, int label);
  void (*call)(emitter_t *self, int label);
  void (*jmp)(emitter_t *self, int label);
  void (*jz)(emitter_t *self, int label);
  void (*jneg)(emitter_t *self, int label);
  void (*ret)(emitter_t *self);
  void (*halt)(emitter_t *self);
  void (*end)(emitter_t *self);
};

void emitter_release(emitter_t **pemitter);

void emit_push(emitter_t *emitter, int value);
void emit_copy(emitter_t *emitter, int n);
void emit_slide(emitter_t *emitter, int n);
void emit_dup(emitter_t *emitter);
void emit_pop(emitter_t *emitter);
void emit_swap(emitter_t *emitter);
void emit_add(emitter_t *emitter);
void emit_sub(emitter_t *emitter);
void emit_mul(emitter_t *emitter);
void emit_div(emitter_t *emitter);
void emit_mod(emitter_t *emitter);
void emit_store(emitter_t *emitter);
void emit_load(emitter_t *emitter);
void emit_getc(emitter_t *emitter);
void emit_geti(emitter_t *emitter);
void emit_putc(emitter_t *emitter);
void emit_puti(emitter_t *emitter);
void emit_store(emitter_t *emitter);
void emit_load(emitter_t *emitter);
void emit_label(emitter_t *emitter, int label);
void emit_call(emitter_t *emitter, int label);
void emit_jmp(emitter_t *emitter, int label);
void emit_jz(emitter_t *emitter, int label);
void emit_jneg(emitter_t *emitter, int label);
void emit_ret(emitter_t *emittter);
void emit_halt(emitter_t *emitter);
void emit_end(emitter_t *emitter);

#define EMITTER_OVERRIDE_ONE(OBJ, PREFIX, INST) (OBJ).INST = PREFIX ## _ ## INST
#define EMITTER_OVERRIDE(OBJ, PREFIX) \
  do { \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, push ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, copy ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, slide); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, dup  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, pop  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, swap ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, add  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, sub  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, mul  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, div  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, mod  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, store); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, load ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, putc ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, puti ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, getc ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, geti ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, label); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, call ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, jmp  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, jz   ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, jneg ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, ret  ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, halt ); \
    EMITTER_OVERRIDE_ONE(OBJ, PREFIX, end  ); \
  } while (0)
