#pragma once

#include <stdbool.h>

typedef struct varentry_t varentry_t;
typedef struct vartable_t vartable_t;

vartable_t *vartable_new(vartable_t *parent);
void        vartable_release(vartable_t **pvartable);

vartable_t *vartable_get_parent(vartable_t *vartable);
varentry_t *vartable_add_var(vartable_t *vartable, const char *name);
varentry_t *vartable_lookup_or_add_var(vartable_t *vartable, const char *name);

int         varentry_get_id(varentry_t *e);
bool        varentry_is_local(varentry_t *e);
const char *varentry_get_name(varentry_t *e);
