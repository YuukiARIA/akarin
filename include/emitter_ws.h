#pragma once

#include <stdbool.h>
#include "emitter.h"

emitter_t *emitter_ws_new(const char *space, const char *tab, const char *newline, bool strict);
