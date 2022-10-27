#pragma once

#include "node.h"

typedef struct codegen_t codegen_t;

codegen_t *codegen_new(node_t *root);
void       codegen_release(codegen_t **pcodegen);
void       codegen_generate(codegen_t *codegen);
