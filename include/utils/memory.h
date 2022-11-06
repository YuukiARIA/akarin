#pragma once

#include <stdlib.h>

#ifdef DEBUG

void *akarin_malloc(size_t size, const char *file, int line, const char *func);
void *akarin_calloc(size_t n, size_t size, const char *file, int line, const char *func);
void *akarin_realloc(void *ptr, size_t size, const char *file, int line, const char *func);
void  akarin_free(void *ptr, const char *file, int line, const char *func);
void  akarin_memory_print(void);

#define AK_MEM_MALLOC(SIZE)       ( akarin_malloc  ( (SIZE),        __FILE__, __LINE__, __func__ ) )
#define AK_MEM_CALLOC(N, SIZE)    ( akarin_calloc  ( (N), (SIZE),   __FILE__, __LINE__, __func__ ) )
#define AK_MEM_REALLOC(PTR, SIZE) ( akarin_realloc ( (PTR), (SIZE), __FILE__, __LINE__, __func__ ) )
#define AK_MEM_FREE(PTR)          ( akarin_free    ( (PTR),         __FILE__, __LINE__, __func__ ) )
#define AK_MEM_CHECK              ( akarin_memory_print() )

#else

#define AK_MEM_MALLOC             malloc
#define AK_MEM_CALLOC             calloc
#define AK_MEM_REALLOC            realloc
#define AK_MEM_FREE               free
#define AK_MEM_CHECK

#endif // DEBUG
