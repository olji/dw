#ifndef MEM_H
#define MEM_H

#include <stdio.h>

void *malloc_assert(size_t size);
char *str_malloc(size_t size);
void *calloc_assert(size_t size, size_t amonut);
void mem_assert(void* pointer);

/* Assumes str is allocated and null terminated */
void str_append(char **str, char c);

#endif
