#include <stddef.h>
#include <stdint.h>

/* temp includes */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vi.h"
#include "history.h"

#define save_cursor()                               \
    vi->term->saved_cursor_r = vi->term->cursor_r;  \
    vi->term->saved_cursor_c = vi->term->cursor_c;

#define restore_cursor()                            \
    vi->term->cursor_r = vi->term->saved_cursor_r;  \
    vi->term->cursor_c = vi->term->saved_cursor_c;  \
    move_to_cursor(vi);

#define save_cursor_local()                         \
    uint16_t __cursor_r = vi->term->cursor_r;       \
    uint16_t __cursor_c = vi->term->cursor_c;

#define restore_cursor_local()                      \
    vi->term->cursor_r = __cursor_r;                \
    vi->term->cursor_c = __cursor_c;                \
    move_to_cursor(vi);

#define exit_to_command_mode()                      \
    vi->mode = VI_MODE_COMMAND;                     \
    vi->term->cursor_r = vi->term->saved_cursor_r;  \
    vi->term->cursor_c = vi->term->saved_cursor_c;  \
    move_to_cursor(vi);


struct vi_s* vi;

/* 
 * Clears the current status message and replaces is with a new one
 */
void write_status();                        // left side status 
void write_end_status(char* status);        // right side status

/*
 * Adds a character to the current vi_line in the line buffer. 
 * Reallocates the line if more memory is needed. 
 */
void insert_char_in_line(char c);
 
/*
 * Adds an empty line to the current buffer. Reallocates the buffer
 * if more memory is needed. Allocates the nex empty vi_line object. 
 */
void add_empty_line();

/*
 * Keypress handlers. 
 */
void keypress_handler(uint16_t key);        // main handler who calls others
void keypress_command_handler(char c);      // in command mode
void keypress_insert_command(char c);       // in insert command mode 

/*
 * Stops vi. Stops logging, unregsiters handlers. Restores terminal.
 * Deallocates memory. Exits program. 
 */
void exit_vi();

/*
 * Redraw areas in terminal 
 */
void redraw_buffer(uint8_t mode);// redraw entire terminal space
void redraw_line();             // redraw current line 


void
init_vi()
{
    vi              = (struct vi_s*)    vimalloc(sizeof(struct vi_s));
    vi->term        = (struct term_s*)  vimalloc(sizeof(struct term_s));
    vi->input       = (struct input_s*) vimalloc(sizeof(struct input_s));
    vi->fb          = (struct fb_s*)    vimalloc(sizeof(struct fb_s));
    vi->his         = (struct his_s*)   vimalloc(sizeof(struct his_s));
    vi->cmdbuf      = (char*)           vicalloc(1, VI_CMDBUF_SIZE * sizeof(char));
    vi->cmdbuf_i    = 0; 
    vi->mode        = VI_MODE_COMMAND;
    vi->keep_status = 0;

    init_logging();                                     // open the logfile
    init_fb_struct(vi->fb);                             // allocate fb and register funcs 
    init_term_struct(vi->term);                         // populate terminal variables
    init_input_struct(vi->input, &keypress_handler);    // register input callback
    init_history_struct(vi->his);                       // allocate history rings

    vi->statusbuf   = (char*) vimalloc(vi->term->cols * sizeof(char));
    vimemset(vi->statusbuf, ' ', vi->term->cols);

#ifdef TESTS_ENABLED
    do_tests();  
    exit_vi();
    return; 
#endif

    redraw_buffer(REDRAW_FULL);                    // draw all the ~
    clear_term_line(vi->term);                          // remove first ~ line

    printflog("Workign with a %dx%d terminal\n", vi->term->rows, vi->term->cols);
    printlog("Polling input...");
    while (1) {
        //move_cursor_down(vi->term, 1); 
        poll_char(vi->input); 
    }
}

int main() { init_vi(); }
 
 
/* 
 * Clears the current status message and replaces is with a new one
 */
void 
write_status() 
{
    save_cursor_local();
    move_cursor_to_pos(vi->term, vi->term->rows - 1, 0); 
    clear_term_line(vi->term);
    write_string(vi->statusbuf);
    restore_cursor_local();
}

void 
write_end_status(char* status) 
{
    save_cursor_local();
    move_cursor_to_pos(vi->term, 
           vi->term->rows - 1, vi->term->cols - 1 - strlen(status)); 
    clear_term_line(vi->term);
    move_cursor_to_pos(vi->term, 
           vi->term->rows - 1, vi->term->cols - 1 - strlen(status)); 
    write_string(status);
    restore_cursor_local();
}

void
keypress_command_handler(char c) 
{
    uint32_t oc = vi->fb->buffer_c;
    uint32_t or = vi->fb->buffer_r;

    switch(c) { 
        case ':':
            // enter insert command mode 
            vi->mode = VI_MODE_INSERT_COMMAND;
            vi->cmdbuf_i = 0; 
            vi->term->saved_cursor_r = vi->term->cursor_r; 
            vi->term->saved_cursor_c = vi->term->cursor_c; 
            move_cursor_to_pos(vi->term, vi->term->rows - 1, 0); 
            clear_term_line(vi->term);
            write_char(':');
            break;
        case 'o': 
            // insert a new line after current line in buffer
            vi->fb->insert_line_after(vi->fb, vi->fb->currentl, 
                    vi->fb->alloc_emptyl(vi->fb));
            
            // if we reached end of buffer, scroll one row down
            if (vi->term->cursor_r + 1 > vi->term->rows - 2) {
                vi->fb->scroll = vi->fb->scroll->nextl;
                vi->fb->scroll_r++;
            }
            else {
                // update cursor based on fb->buffer_r/c
                move_to_cursor(vi);
            }

            // redraw for newline to take effect
            redraw_buffer(REDRAW_PARTIAL);

            vi->his->add_undoable_at_loc(VICMD_ADDL, 
                    0, vi->fb->currentl, or, oc);

            // fall through
        case 'i': 
            vi->mode = VI_MODE_INSERT;
            break;
        case 'h': 
            if (vi->fb->buffer_c != 0) {
                vi->fb->cursor_left(vi->fb);
                move_to_cursor(vi);
            }
            break;
        case 'l': 
            if (vi->fb->buffer_c + 1 <= vi->fb->currentl->data_n) {
                vi->fb->cursor_right(vi->fb);
                move_to_cursor(vi);
            }
            break;
        case 'j': 
            if (vi->fb->currentl->nextl != NULL) {
                if (vi->term->cursor_r + 1 > vi->term->rows - 2) {
                    vi->fb->scroll = vi->fb->scroll->nextl;
                    vi->fb->scroll_r++;
                    redraw_buffer(REDRAW_PARTIAL);
                }
                vi->fb->cursor_down(vi->fb);
                move_to_cursor(vi);
            }
            break;
        case 'k': 
            if (vi->fb->currentl->prevl != NULL) {
                if (vi->fb->currentl == vi->fb->scroll) {
                    vi->fb->scroll = vi->fb->scroll->prevl; 
                    vi->fb->scroll_r--;
                    redraw_buffer(REDRAW_PARTIAL);
                }
                vi->fb->cursor_up(vi->fb);
                move_to_cursor(vi);
            }
            break; 
        case 'u': 
            vi->his->undo(vi->his, vi->fb);
            redraw_buffer(REDRAW_FULL);
            break;
        case 'd': 
            vi->mode = VI_MODE_DEL_CMD;
            break;
        case '\n':
            // don't allow new line if in cmomand mode
            if (vi->fb->currentl->nextl != NULL) {
                vi->fb->cursor_down(vi->fb);
            }
            break;
        default: 
            break;
    }
}

void
keypress_insert_handler(char c) 
{
    char t; 
    uint32_t oc = vi->fb->buffer_c;
    uint32_t or = vi->fb->buffer_r;

    switch (c) {
        case '\n':
            // insert a new line after current line in buffer
            t = vi->fb->currentl->eog;
            vi->fb->insert_line_after(vi->fb, vi->fb->currentl, NULL);
            
            // if we reached end of buffer, scroll one row down
            if (vi->term->cursor_r + 1 > vi->term->rows - 2) {
                vi->fb->scroll = vi->fb->scroll->nextl;
                vi->fb->scroll_r++;
            }
            else {
                // update cursor based on fb->buffer_r/c
                move_to_cursor(vi);
            }
            vi->his->add_undoable_at_loc(VICMD_ADDL, 
                    t, vi->fb->currentl, or, oc);

            // redraw for newline to take effect
            redraw_buffer(REDRAW_FULL);
            break;
        case '\x7f': // delete character
            t = vi->fb->backspace_char(vi->fb, vi->fb->currentl);
            if (t != -1) {
                redraw_buffer(REDRAW_PARTIAL);
                move_to_cursor(vi);
                vi->his->add_undoable(VICMD_BACKSPACE_C, t, NULL);
            }
            else {
                if (vi->fb->currentl != vi->fb->headl) {
                    vi->fb->merge_lines_up(vi->fb, (struct vi_line*) vi->fb->currentl);
                    vi->fb->move_gap_to_cursor(vi->fb);
                    redraw_buffer(REDRAW_FULL);
                    vi->his->add_undoable_at_loc(VICMD_DELL, 
                        0, vi->fb->currentl, vi->fb->buffer_r, vi->fb->buffer_c);
                }
            }
            break;
        case '\x1b': // escape character
            // exit command insert mode to command mode
            vi->mode = VI_MODE_COMMAND;
            break;
        case '\t':
            for (uint8_t i = 0; i < VI_TABWIDTH; i++) {
                vi->his->add_undoable(VICMD_INSERT_C, c, NULL);
                vi->term->cursor_c++;
                vi->fb->insert_char(vi->fb, vi->fb->currentl, ' ');
                write_char(' '); 
            }
            redraw_buffer(REDRAW_PARTIAL);
            break;
        default: 
            vi->his->add_undoable(VICMD_INSERT_C, c, NULL);
            vi->term->cursor_c++;
            vi->fb->insert_char(vi->fb, vi->fb->currentl, c);
            write_char(c);
            redraw_buffer(REDRAW_PARTIAL);
    }
}

void 
keypress_insert_command(char c) 
{
    switch (c) {
        case '\n':
            // execute given command 
            vi->cmdbuf[vi->cmdbuf_i] = '\0';
            printlog("Executing");
            printlog(vi->cmdbuf);
            if      (strcmp(vi->cmdbuf, "q") == 0) exit_vi();
            else if (strcmp(vi->cmdbuf, "redraw") == 0) {
                redraw_buffer(REDRAW_FULL);
                exit_to_command_mode();
            }
            else {
                vistrcpy(vi->statusbuf, "Unknown command"); 
                vi->keep_status = 1;
                exit_to_command_mode();
            }
            break;
        case '\x1b': 
            // exit command insert mode to command mode
            exit_to_command_mode();
            break;
        default: 
            vi->cmdbuf[vi->cmdbuf_i++] = c;
            write_char(c);
    }
}

void
keypress_delmode_handler(char c) 
{
    uint8_t ret;
    struct vi_line* current = vi->fb->currentl;
    struct vi_line* his;
    switch (c) {
        case 'd': 
            ret = vi->fb->cutl(vi->fb);
            if (ret == NONE_DELETED) {
                // no more lines could be deleted 
                his = vimalloc(sizeof(struct vi_line));
                memcpy(his, current, sizeof(struct vi_line));
                
                vi->fb->currentl->sog = 0;
                vi->fb->currentl->eog = vi->fb->currentl->size - 1;
                vi->fb->currentl->data_n = 0;

                vistrcpy(vi->statusbuf, "-- No lines in buffer --"); 
                vi->keep_status = 1;
                vi->his->add_undoable(VICMD_DD, 0, his); 
                vi->fb->buffer_c = 0; 
            }
            else if (ret == DELETED_UPWARD) {
                his = current;
                vi->his->add_undoable(VICMD_DD, 0, his); 
            }
            else {
                his = current;
                vi->his->add_undoable(VICMD_DD, 0, his); 
            }
            move_to_cursor(vi);
            redraw_buffer(REDRAW_FULL);
            break;
    }
    vi->mode = VI_MODE_COMMAND;
}

void 
keypress_handler(uint16_t key) 
{
    char c = (char) key; 
    if (c == '1') exit_vi();
    if (c == '2') exit(2);

    switch (vi->mode) {
        case VI_MODE_COMMAND:        keypress_command_handler(c); break;
        case VI_MODE_INSERT:         keypress_insert_handler(c);  break;
        case VI_MODE_INSERT_COMMAND: keypress_insert_command(c);  break;
        case VI_MODE_DEL_CMD:        keypress_delmode_handler(c); break;
    }

    // write debug message on screen 
    if (vi->keep_status == 0) {
        switch (vi->mode) {
            case VI_MODE_INSERT: 
                vistrcpy(vi->statusbuf, "-- INSERT --"); 
                vi->statusbuf[12] = ' ';
                break;
            case VI_MODE_COMMAND: 
                vimemset(vi->statusbuf, ' ', 20); 
                break;
            case VI_MODE_INSERT_COMMAND:
                // don't write any status if typing command
                return;
        }
    }
    else {
        vi->keep_status = 0;
    }

    struct vi_line* line = vi->fb->currentl;
    for (uint8_t i = 0; i < line->size - 1; i++) {
        if (i >= line->sog && i <= line->eog) *(vi->statusbuf + 10 + i) = '_';
        else *(vi->statusbuf + 10 + i) = line->data[i];
    } 
    visprintf(vi->statusbuf + vi->term->cols - 18, "%d,%d,(%d,%d)", vi->fb->buffer_r, vi->fb->buffer_c, vi->term->cursor_r, vi->term->cursor_c);
    write_status();
}

void
exit_vi()
{
    destroy_history_struct(vi->his);     
    stop_logging();
    destroy_input_struct(vi->input); 
    destroy_term_struct(vi->term); 
    destroy_fb_struct(vi->fb);
    vifree(vi->cmdbuf);
    vifree(vi->statusbuf);
    vifree(vi); 
    exit_prog(0);
}

void
redraw_line() 
{
    save_cursor_local();
    clear_term_line(vi->term);
    move_cursor_to_pos(vi->term, vi->term->cursor_r, 0);
    write_string(vi->fb->currentl->data);
    restore_cursor_local();
}

void
redraw_buffer(uint8_t mode) 
{
    save_cursor_local();
    struct vi_line* line = vi->fb->scroll;
    for (uint16_t i = 0; i < vi->term->rows - 1; i++) {
        move_cursor_to_pos(vi->term, i, 0);
        if (line != NULL) {
            clear_term_line(vi->term);
            if (line->data_n != 0) 
                for (uint32_t i = 0; i < line->size - 1; i++) 
                    if (i < line->sog || i > line->eog)
                        write_char(line->data[i]);
            line = line->nextl;
        }
        else {
            if (mode == REDRAW_FULL)
                clear_term_line(vi->term);
            write_char('~');
        }
    }
    restore_cursor_local();
}


