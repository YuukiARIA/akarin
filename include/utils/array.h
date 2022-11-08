#pragma once

typedef struct array_t array_t;

array_t *array_new(int initial_capacity);
void     array_release(array_t **parray);
void     array_append(array_t *array, void *item);
int      array_count(array_t *array);
void    *array_get(array_t *array, int index);
array_t *array_concat(array_t *array1, array_t *array2);
