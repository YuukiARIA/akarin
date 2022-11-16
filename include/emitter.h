#pragma once

#include "inst.h"
#include "utils/array.h"

typedef struct emitter_t emitter_t;

struct emitter_t {
  void (*emit)(emitter_t *self, inst_t *inst);
  void (*end)(emitter_t *self);
};

void emitter_release(emitter_t **pemitter);
void emitter_emit_code(emitter_t *emitter, array_t *instructions);
