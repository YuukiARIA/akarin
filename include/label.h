#pragma once

typedef struct label_t label_t;
typedef struct ltable_t ltable_t;

ltable_t *ltable_new(void);
void      ltable_release(ltable_t **pltable);
label_t  *ltable_alloc(ltable_t *ltable);
label_t  *ltable_get(ltable_t *ltable, int id);
int       ltable_count(ltable_t *ltable);
void      ltable_unify(ltable_t *ltable, int label_id1, int label_id2);

int label_get_id(label_t *label);
int label_get_unified_id(label_t *label);
