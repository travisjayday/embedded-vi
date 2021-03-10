#ifndef HISTORY_H
#define HISTORY_H

#include "vi.h"

#define VICMD_INSERT_C      0
#define VICMD_BACKSPACE_C   1
#define VICMD_ADDL          2
#define VICMD_DD            3
#define VICMD_DELL          4

/* command struct */
struct cmd_s {
    uint8_t  cmd;       // The commmand identifier VICMD_*
    uint32_t buffer_r;  // saved buffer row
    uint32_t buffer_c;  // saved buffer col
    uint32_t scroll_r;  // saved scroll offset
    uint32_t arg;       // The argument to the command
    uintptr_t dptr;
}; 

struct his_s {
    struct ring_s* undo_ring;

    /*
     * Executes undo operation
     */
    uint8_t (*undo)(struct his_s*, struct fb_s* fb);

    /* 
     * creates an cmd_s structs
     */
    void (*add_undoable)(uint8_t cmd, uint8_t arg, void* dptr); 
    void (*add_undoable_at_loc)(uint8_t cmd, uint8_t arg, 
            void* dptr, uint32_t row, uint32_t col); 
}; 

void init_history_struct(struct his_s* his);
void destroy_history_struct(struct his_s* his); 

#endif
