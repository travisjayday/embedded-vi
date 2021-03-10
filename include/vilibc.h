#ifndef VILIBC_H
#define VILIBC_H

#include "vi.h"

int visprintf(char *str, const char *format, ...);
char* vistrcpy(char* dest, const char* src);
size_t vistrlen(const char *str);


#endif
