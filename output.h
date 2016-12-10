#ifndef OUTPUT_H
#define OUTPUT_H

#define ERROR_OUT "\033[1;31m"
#define NOTE_OUT "\033[1;36m"
#define NORMAL_OUT "\033[1;0m"

void error(char *format, ...);
void note(char *format, ...);
#endif
