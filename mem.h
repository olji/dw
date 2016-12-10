#ifndef MEM_H
#define MEM_H

#include <stdio.h>

void *malloc_assert(size_t size);
void *calloc_assert(size_t size, size_t amonut);
void mem_assert(void* pointer);

#endif
