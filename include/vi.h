#ifndef VI_H
#define VI_H

#include <stddef.h>
#include <stdint.h>
#include "vilibc.h"
#include "mem.h"
#include "input.h"
#include "fb.h"
#include "term.h"
#include "fs.h"
#include "env.h"
#include "ring.h"

//#define TESTS_ENABLED

#ifdef TESTS_ENABLED
#include "tests.h"
#endif

#define VI_HISTORY_SIZE 16       // number of undo / redo items 
#define VI_TABWIDTH             4   // tab width

#define VI_MODE_COMMAND         0   // standard command mode
#define VI_MODE_INSERT_COMMAND  1   // typing command at bottom
#define VI_MODE_INSERT          2   // insert mode
#define VI_MODE_DEL_CMD         3   // after presssing 'd' in command
#define VI_CMDBUF_SIZE 128

#define REDRAW_FULL             0
#define REDRAW_PARTIAL          1

struct vi_s {
    struct term_s*  term; 
    struct input_s* input; 
    struct fb_s*    fb;
    struct his_s*   his; 

    uint8_t         mode;

    /* 
     * Buffer to store the command being typed in
     * VI_MODE_INSERT_COMMAND mode 
     */ 
    char*           cmdbuf; 
    uint16_t        cmdbuf_i;

    /*
     * Buffer to store the status line at the 
     * bottom of the window. Size is term->rows
     */
    char*           statusbuf; 

    /*
     * Flag to not overdraw the currrent status
     */ 
    uint8_t         keep_status; 
};


extern struct vi_s* vi;

// allocates vi, sets up terminal, etc.
void init_vi();

// redraws the framebuffer from scratch
void redraw_buffer(uint8_t redraw_mode);
 
// destroys vi and closes program
void exit_vi();

#endif
