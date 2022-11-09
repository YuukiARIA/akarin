#include <stdlib.h>
#include <string.h>
#include "vartable.h"
#include "utils/memory.h"
#include "utils/array.h"

struct varentry_t {
  int   offset;
  bool  is_local;
  char *name;
};

struct vartable_t {
  vartable_t *parent;
  int         offset;
  array_t    *vars;
};

vartable_t *vartable_new(vartable_t *parent) {
  vartable_t *vartable = (vartable_t *)AK_MEM_MALLOC(sizeof(vartable_t));
  vartable->parent = parent;
  vartable->offset = 0;
  vartable->vars = array_new(64);
  return vartable;
}

void vartable_release(vartable_t **pvartable) {
  vartable_t *vt = *pvartable;

  for (int i = 0; i < array_count(vt->vars); ++i) {
    varentry_t *e = (varentry_t *)array_get(vt->vars, i);
    AK_MEM_FREE(e->name);
    AK_MEM_FREE(e);
  }
  array_release(&vt->vars);

  AK_MEM_FREE(vt);

  *pvartable = NULL;
}

vartable_t *vartable_get_parent(vartable_t *vartable) {
  return vartable->parent;
}

static varentry_t *lookup(vartable_t *vartable, const char *name) {
  for (int i = 0; i < array_count(vartable->vars); ++i) {
    varentry_t *entry = (varentry_t *)array_get(vartable->vars, i);
    if (strcmp(entry->name, name) == 0) {
      return entry;
    }
  }
  return NULL;
}

varentry_t *vartable_add_var(vartable_t *vartable, const char *name, int size) {
  varentry_t *entry = lookup(vartable, name);

  if (!entry) {
    entry = (varentry_t *)AK_MEM_MALLOC(sizeof(varentry_t));
    entry->offset = vartable->offset;
    entry->is_local = vartable->parent != NULL;
    entry->name = AK_MEM_STRDUP(name);

    array_append(vartable->vars, entry);
    vartable->offset += size;
  }

  return entry;
}

varentry_t *vartable_lookup_or_add_var(vartable_t *vartable, const char *name) {
  varentry_t *entry = lookup(vartable, name);

  if (entry) {
    return entry;
  }

  if (vartable->parent) {
    return vartable_lookup_or_add_var(vartable->parent, name);
  }

  return vartable_add_var(vartable, name, 1);
}

int varentry_get_offset(varentry_t *e) {
  return e->offset;
}

bool varentry_is_local(varentry_t *e) {
  return e->is_local;
}

const char *varentry_get_name(varentry_t *e) {
  return e->name;
}
