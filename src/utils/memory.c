#include <stdio.h>
#include <stdlib.h>

typedef struct mem_t mem_t;
struct mem_t {
  void       *ptr;
  int         size;
  const char *file;
  int         line;
  const char *func;
  mem_t      *next;
};

static mem_t *g_mems = NULL;

static void allocate(void *ptr, int size, const char *file, int line, const char *func) {
  mem_t *add;

  if (!ptr) {
    return;
  }

  add = (mem_t *)malloc(sizeof(mem_t));
  add->ptr = ptr;
  add->file = file;
  add->line = line;
  add->func = func;
  add->size = size;
  add->next = NULL;

  if (!g_mems) {
    g_mems = add;
  }
  else {
    mem_t *mem = g_mems;
    while (mem->next) {
      mem = mem->next;
    }
    mem->next = add;
  }
}

static void release(void *ptr, const char *file, int line, const char *func) {
  mem_t *mem;
  mem_t *prev;

  if (!g_mems || !ptr) {
    return;
  }

  mem = g_mems;

  if (mem->ptr == ptr) {
    g_mems = mem->next;
    free(mem);
    return;
  }

  prev = mem;
  mem = mem->next;

  while (mem) {
    if (mem->ptr == ptr) {
      prev->next = mem->next;
      free(mem);
      break;
    }
    prev = mem;
    mem = mem->next;
  }
}

void *akarin_malloc(size_t size, const char *file, int line, const char *func) {
  void *ptr = malloc(size);
  allocate(ptr, size, file, line, func);
  return ptr;
}

void *akarin_calloc(size_t n, size_t size, const char *file, int line, const char *func) {
  void *ptr = calloc(n, size);
  allocate(ptr, n * size, file, line, func);
  return ptr;
}

void *akarin_realloc(void *ptr, size_t size, const char *file, int line, const char *func) {
  void *newptr = realloc(ptr, size);
  release(ptr, file, line, func);
  allocate(newptr, size, file, line, func);
  return newptr;
}

void akarin_free(void *ptr, const char *file, int line, const char *func) {
  free(ptr);
  release(ptr, file, line, func);
}

void akarin_memory_print(void) {
  mem_t *mem = g_mems;
  if (mem) {
    fprintf(stderr, "\x1B[31;1mDetected Memory Leaks\n");
    while (mem) {
      fprintf(stderr, "%p (%d): %s:%d %s\n", mem->ptr, mem->size, mem->file, mem->line, mem->func);
      mem = mem->next;
    }
    fprintf(stderr, "\x1B[0m");
  }
}
