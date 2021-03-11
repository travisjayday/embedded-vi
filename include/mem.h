#ifndef MEM_H
#define MEM_H

#include "vi.h"

void* vimalloc(size_t size);
void* virealloc(void* ptr, size_t size); 
void* vicalloc(size_t nmemb, size_t size); 
void  vifree(void* ptr); 
void* vimemmove(void* dest, const void* src, size_t n);
void* vimemset(void* str, int c, size_t n);
void* vimemcpy(void* dest, const void* src, size_t n);


#endif
