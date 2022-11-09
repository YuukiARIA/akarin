#pragma once

#include "node.h"
#include "utils/array.h"
#include "emitter.h"

typedef struct codegen_t codegen_t;

codegen_t *codegen_new(node_t *root, emitter_t *emitter);
void       codegen_release(codegen_t **pcodegen);
void       codegen_generate(codegen_t *codegen);
int        codegen_get_error_count(codegen_t *codegen);
array_t   *codegen_get_instructions(codegen_t *codegen);
