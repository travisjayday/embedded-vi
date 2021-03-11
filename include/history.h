#ifndef HISTORY_H
#define HISTORY_H

#include "vi.h"

/*
 * Below are defines for commands that can be undone. 
 */

#define VICMD_INSERT_C      0
/* Character inserted (not newline) 
 * arg  = The character that was inserted.
 * dptr = NULL
 */
#define VICMD_BACKSPACE_C   1
/* Backspace pressed and character deleted (no line deleted).
 * arg  = The character that was backspaced.
 * dptr = NULL
 */
#define VICMD_ADDL          2
/* 'o' pressed or '\n' inserted.
 * arg  = 0
 * dptr = struct vi_line* to the added line.
 */
#define VICMD_DD            3
/* DD pressed.
 * arg  = 0
 * dptr = struct vi_line* to the deleted line.
 */
#define VICMD_DELL          4
/* Backspace pressed at beginning of line (line deleted)
 * arg = 0
 * dptr = struct vi_line* to the deleted line.
 */


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
