#include <stdio.h>
#include <stdarg.h>
#include "output.h"

void error(char *format, ...){
    va_list args;
    fprintf(stderr, ERROR_OUT);
    fprintf(stderr, "ERR: ");
    fprintf(stderr, NORMAL_OUT);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
void note(char *format, ...){
    va_list args;
    printf(NOTE_OUT);
    printf("NOTE: ");
    printf(NORMAL_OUT);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
