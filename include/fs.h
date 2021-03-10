#ifndef FS_H
#define FS_H

#include "vi.h"

/*
 * Writes the current textbuffer to file 
 */
void write_buffer(char* filepath); 

/* 
 * Reads a file from disk and loads it into current textbuffer
 */
void open_buffer(char* filepath); 

#endif
