#pragma once

#include "node.h"
#include "utils/array.h"

typedef struct codegen_t codegen_t;

codegen_t *codegen_new(node_t *root);
void       codegen_release(codegen_t **pcodegen);
void       codegen_generate(codegen_t *codegen);
int        codegen_get_error_count(codegen_t *codegen);
array_t   *codegen_get_instructions(codegen_t *codegen);
