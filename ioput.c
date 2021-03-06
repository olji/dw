#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "ioput.h"
#include "config.h"

extern struct dw_config CONFIG;

/* Most are simple wrappers using va_list */
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
    if (CONFIG.script_friendly == 0){
        va_list args;
        printf(NOTE_OUT);
        printf("NOTE: ");
        printf(NORMAL_OUT);
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}
void debug(char *format, ...){
#if DEBUG
    va_list args;
    printf(DEBUG_OUT);
    printf("DEBUG: ");
    printf(NORMAL_OUT);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

/* Output format, read input and only accept characters found in answers */
char ask(char *answers, char *format, ...){
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(" [");
    for (int i = 0; i < strlen(answers); ++i){
        if (i > 0){
            printf("/");
        }
        printf("%c", answers[i]);
    }
    printf("] ");
    char ans = 0;
    do{
        if (ans != 0){
            printf("Invalid answer\n");
        }
        scanf("%s", &ans);
        /* Get rid of newline */
        fgetc(stdin);
        ans = tolower(ans);
    } while (strchr(answers, ans) == NULL);
    return ans;
}
