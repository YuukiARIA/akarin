#include "label.h"
#include "utils/array.h"
#include "utils/memory.h"

struct label_t {
  int      id;
  label_t *parent;
};

struct ltable_t {
  array_t *labels;
};

ltable_t *ltable_new(void) {
  ltable_t *ltable = (ltable_t *)AK_MEM_MALLOC(sizeof(ltable_t));
  ltable->labels = array_new(64);
  return ltable;
}

void ltable_release(ltable_t **pltable) {
  ltable_t *ltable = *pltable;

  for (int i = 0; i < array_count(ltable->labels); ++i) {
    label_t *label = (label_t *)array_get(ltable->labels, i);
    AK_MEM_FREE(label);
  }
  array_release(&ltable->labels);

  AK_MEM_FREE(ltable);
  *pltable = NULL;
}

label_t *ltable_alloc(ltable_t *ltable) {
  int id = array_count(ltable->labels);
  label_t *label = (label_t *)AK_MEM_MALLOC(sizeof(label_t));
  label->id = id;
  label->parent = NULL;
  array_append(ltable->labels, label);
  return label;
}

label_t *ltable_get(ltable_t *ltable, int id) {
  return (label_t *)array_get(ltable->labels, id);
}

int ltable_count(ltable_t *ltable) {
  return array_count(ltable->labels);
}

static label_t *get_root(label_t *label) {
  label_t *l = label;
  while (l->parent) {
    l = l->parent;
  }
  return l;
}

void ltable_unify(ltable_t *ltable, int label_id1, int label_id2) {
  array_t *labels = ltable->labels;
  label_t *l1 = array_get(labels, label_id1);
  label_t *l2 = array_get(labels, label_id2);
  l1 = get_root(l1);
  l2 = get_root(l2);
  l2->parent = l1;
}

int label_get_id(label_t *label) {
  return label->id;
}

int label_get_unified_id(label_t *label) {
  return get_root(label)->id;
}
