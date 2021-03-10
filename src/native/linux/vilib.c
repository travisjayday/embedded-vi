#include "vilibc.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int 
visprintf(char *str, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    int i = vsprintf(str, format, args);
    va_end(args);
    return i;
}

char* 
vistrcpy(char* dest, const char* src) 
{
    return strcpy(dest, src);    
}

size_t 
vistrlen(const char *str)
{
    return strlen(str);
}

