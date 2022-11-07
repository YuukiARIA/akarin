#include <stdlib.h>
#include <string.h>
#include "vartable.h"
#include "utils/memory.h"

struct varentry_t {
  vartable_t *vartable;
  int         id;
  bool        is_local;
  char       *name;
};

struct vartable_t {
  vartable_t  *parent;
  int          count;
  int          capacity;
  varentry_t **vars;
};

vartable_t *vartable_new(vartable_t *parent) {
  vartable_t *vartable = (vartable_t *)AK_MEM_MALLOC(sizeof(vartable_t));
  vartable->parent = parent;
  vartable->count = 0;
  vartable->capacity = 64;
  vartable->vars = (varentry_t **)AK_MEM_CALLOC(vartable->capacity, sizeof(varentry_t *));
  return vartable;
}

void vartable_release(vartable_t **pvartable) {
  vartable_t *vt = *pvartable;

  for (int i = 0; i < vt->count; ++i) {
    varentry_t *e = vt->vars[i];
    AK_MEM_FREE(e->name);
    AK_MEM_FREE(e);
  }

  AK_MEM_FREE(vt->vars);
  AK_MEM_FREE(vt);

  *pvartable = NULL;
}

static varentry_t *lookup(vartable_t *vartable, const char *name) {
  for (int i = 0; i < vartable->count; ++i) {
    varentry_t *entry = vartable->vars[i];
    if (strcmp(entry->name, name) == 0) {
      return entry;
    }
  }
  return NULL;
}

varentry_t *vartable_add_var(vartable_t *vartable, const char *name) {
  int id;
  varentry_t *entry = lookup(vartable, name);

  if (!entry) {
    if (vartable->count == vartable->capacity) {
      vartable->capacity *= 2;
      vartable->vars = (varentry_t **)AK_MEM_REALLOC(vartable->vars, vartable->capacity);
    }

    id = vartable->count++;
    entry = (varentry_t *)AK_MEM_MALLOC(sizeof(varentry_t));
    entry->vartable = vartable;
    entry->id = id;
    entry->is_local = vartable->parent != NULL;
    entry->name = (char *)AK_MEM_CALLOC(strlen(name) + 1, sizeof(char));
    strcpy(entry->name, name);
    vartable->vars[id] = entry;
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

  return vartable_add_var(vartable, name);
}

int varentry_get_id(varentry_t *e) {
  return e->id;
}

bool varentry_is_local(varentry_t *e) {
  return e->is_local;
}

const char *varentry_get_name(varentry_t *e) {
  return e->name;
}
