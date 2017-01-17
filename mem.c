#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "ioput.h"

void *malloc_assert(size_t size){
    void *mem = malloc(size);
    mem_assert(mem);
    return mem;
}
/* Do all stuff you usually want to do when allocating a string */
char *str_malloc(size_t size){
    char *mem = malloc(sizeof(char) * (size + 1));
    mem_assert(mem);
    mem[size] = '\0';
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
/* Append character to string */
void str_append(char **str, char c){
    char *tmp = str_malloc(strlen(*str) + 1);
    strcpy(tmp, *str);
    tmp[strlen(*str)] = c;
    free(*str);
    *str = tmp;
}
