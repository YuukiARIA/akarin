#include "emitter.h"
#include "inst.h"
#include "utils/memory.h"
#include "utils/array.h"

void emitter_release(emitter_t **pemitter) {
  AK_MEM_FREE(*pemitter);
  *pemitter = NULL;
}

void emitter_emit_code(emitter_t *emitter, array_t *instructions) {
  for (int i = 0; i < array_count(instructions); ++i) {
    inst_t *inst = (inst_t *)array_get(instructions, i);
    emitter->emit(emitter, inst);
  }
  emitter->end(emitter);
}
