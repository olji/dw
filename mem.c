#include <stdlib.h>
#include "mem.h"
#include "output.h"

void *malloc_assert(size_t size){
    void *mem = malloc(size);
    mem_assert(mem);
    return mem;
}
void *calloc_assert(size_t amount, size_t size){
    void *mem = calloc(amount, size);
    mem_assert(mem);
    return mem;
}
void mem_assert(void *pointer){
    if (pointer == NULL){
        error("Allocation error\n");
        exit(1);
    }
}
