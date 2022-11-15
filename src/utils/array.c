#include <stdlib.h>
#include "utils/array.h"
#include "utils/memory.h"

struct array_t {
  int    count;
  int    capacity;
  void **data;
};

static void ensure_capacity(array_t *array);

array_t *array_new(int initial_capacity) {
  array_t *array = (array_t *)AK_MEM_MALLOC(sizeof(array_t));
  array->count = 0;
  array->capacity = initial_capacity;
  array->data = (void **)AK_MEM_CALLOC(array->capacity, sizeof(void *));
  return array;
}

void array_release(array_t **parray) {
  AK_MEM_FREE((*parray)->data);
  AK_MEM_FREE(*parray);
  *parray = NULL;
}

void array_append(array_t *array, void *item) {
  ensure_capacity(array);
  array->data[array->count++] = item;
}

int array_count(array_t *array) {
  return array->count;
}

void *array_get(array_t *array, int index) {
  return array->data[index];
}

array_t *array_concat(array_t *array1, array_t *array2) {
  array_t *array = array_new(array1->count + array2->count);
  for (int i = 0; i < array1->count; ++i) {
    array_append(array, array1->data[i]);
  }
  for (int i = 0; i < array2->count; ++i) {
    array_append(array, array2->data[i]);
  }
  return array;
}

static void ensure_capacity(array_t *array) {
  if (array->count == array->capacity) {
    array->capacity *= 2;
    array->data = (void **)AK_MEM_REALLOC(array->data, sizeof(void *) * array->capacity);
  }
}
