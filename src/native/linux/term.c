#include "term.h"

/* Linux specific includes */
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STDOUT_FILENO 1 

/* Terminal related functions here */

char relmove_seq[] = "\x1b[nA"; 

void 
init_term_struct(struct term_s* term) 
{
    clear_term(term);      
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    term->cols = (uint16_t)(w.ws_col);
    term->rows = (uint16_t)(w.ws_row); 
    term->cursor_c = 0;
    term->cursor_r = 0;
    term->saved_cursor_c = 0;
    term->saved_cursor_r = 0;
}

void
destroy_term_struct(struct term_s* term) 
{
    move_cursor_to_pos(term, term->cursor_r + 2, 0);
    vifree(term);
}

void 
do_escape_seq(char dir, uint16_t n) 
{
    char* buf = vicalloc(0xf, 1);
    sprintf(buf, "\x1b[%d%c", n, (int) dir); 
    write(STDOUT_FILENO, buf, strlen(buf)); 
    vifree(buf); 
}

void 
move_cursor_up(struct term_s* term, uint16_t n)
{
    do_escape_seq('A', n); 
    term->cursor_r -= 1; 
}

void 
move_cursor_down(struct term_s* term, uint16_t n)
{
    do_escape_seq('B', n); 
    term->cursor_r += 1; 
}

void 
move_cursor_left(struct term_s* term, uint16_t n)
{
    do_escape_seq('D', n); 
    term->cursor_c -= 1; 
}


void 
move_cursor_right(struct term_s* term, uint16_t n)
{
    do_escape_seq('C', n); 
    term->cursor_c += 1; 
}


void
move_cursor_to_pos(struct term_s* term, uint16_t r, uint16_t c)
{
    char* buf = vicalloc(0xf, 1);
    sprintf(buf, "\x1b[%d;%dH", (int) (r + 1), (int) (c + 1)); 
    write(STDOUT_FILENO, buf, strlen(buf)); 
    vifree(buf); 
    term->cursor_c = c;
    term->cursor_r = r;
}

void 
move_to_cursor(struct vi_s* vi) 
{
    move_cursor_to_pos(vi->term, 
            vi->fb->buffer_r - vi->fb->scroll_r, vi->fb->buffer_c);
}

void
clear_term(struct term_s* term)
{
    char* buf = "\x1b[2J";
    write(STDOUT_FILENO, buf, strlen(buf)); 
    move_cursor_to_pos(term, 0, 0);
}

void
clear_term_line(struct term_s* term) 
{
    char* buf = "\x1b[2K";
    write(STDOUT_FILENO, buf, strlen(buf)); 
    move_cursor_to_pos(term, term->cursor_r, 0);
}

void 
write_char(char c) 
{
    write(STDOUT_FILENO, &c, 1);
}

void 
write_string(char* str) 
{
    while (*str != '\0') write_char(*str++);
}
