#ifndef TERM_H
#define TERM_H

#include "vi.h"

struct term_s {
    uint16_t rows;          // rows of current terminal 
    uint16_t cols;          // cols of current terminal 
    uint16_t cursor_c;      // cursor column of current tmerinal 
    uint16_t cursor_r;      // cursor row of current terminal 
    uint16_t saved_cursor_c; 
    uint16_t saved_cursor_r;
};

/*
 * Queries the OS and initializes the termainl struct with current 
 * values. Also clears the temrinal and moves cursor to 0,0.
 */ 
void init_term_struct(struct term_s* term); 

/* 
 * Finds the rows/cols of current terminal and populates thenm
 * in the given struct.
 */
void update_term_size(struct term_s* term); 

/*
 * Functions for moving the cursor in the terminal. Updates cursor
 * in the given struct.
 */
void move_cursor_up(struct term_s* term, uint16_t n);
void move_cursor_down(struct term_s* term, uint16_t n);
void move_cursor_left(struct term_s* term, uint16_t n);
void move_cursor_right(struct term_s* term, uint16_t n);
void move_cursor_to_pos(struct term_s* term, uint16_t r, uint16_t c); 
struct vi_s;; 
void move_to_cursor(struct vi_s* vi);

/*
 * Writes a char at current position
 */
void write_char(char c); 
void write_string(char* str); 

/*
 * Clear the terminal screen and move cursor 0,0
 */
void clear_term(struct term_s* term); 

/*
 * Clear from cursor to beginning of the line
 */
void clear_term_line(struct term_s* term);

void destroy_term_struct(struct term_s* term); 


#endif
