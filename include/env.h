#ifndef DEBUG_H
#define DEBUG_H

#include "vi.h"
#include "stdio.h"

#define printflog(fmt, ...)                         \
    char* buf = vimalloc(512);                       \
    sprintf(buf, fmt, ##__VA_ARGS__);               \
    printlog(buf);                                  \
    vifree(buf);                                    \
    

void fatal(char* message);
void exit_prog(char code);

void init_logging();
void printlog(char* message);
void stop_logging();

#endif
