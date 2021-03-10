#include "mem.h"
#include <stdlib.h>
#include <string.h>

/* Memory specific functions here */

void* 
vimalloc(size_t size)
{
    return malloc((size_t) size); 
}

void*
virealloc(void* ptr, size_t size) 
{
    return realloc(ptr, (size_t) size); 
}

void* 
vicalloc(size_t nmemb, size_t size) 
{
    return calloc(nmemb, size); 
}

void vifree(void *ptr) 
{
    free(ptr); 
}

void* 
vimemmove(void* dest, const void* src, size_t n) 
{
    return memmove(dest, src, n);
}

void* 
vimemset(void* str, int c, size_t n) 
{
    return memset(str, c, n);
}

void* 
vimemcpy(void* dest, const void* src, size_t n) 
{
    return memcpy(dest, src, n);
}
