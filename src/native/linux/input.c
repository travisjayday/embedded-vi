#include "input.h"

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define ttyfd 0 

/* Keyboard input related functions here */
static struct termios orig_termios; 


/* put terminal in raw mode - see termio(7I) for modes */
void 
tty_raw(void)
{
    struct termios raw;
    raw = orig_termios;  /* copy original and then modify below */
    /* input modes - clear indicated ones giving: no break, no CR to NL,
       no parity check, no strip char, no start/stop output (sic) control */
    // raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // raw.c_iflag &= ~(| INPCK | ISTRIP | IXON);
    /* output modes - clear giving: no post processing such as NL to CR+NL */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - clear giving: echoing off, canonical off (no erase with
       backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer */
    raw.c_cc[VMIN] = 5; raw.c_cc[VTIME] = 8; /* after 5 bytes or .8 seconds
                                                after first byte seen      */
    raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0; /* immediate - anything       */
    raw.c_cc[VMIN] = 2; raw.c_cc[VTIME] = 0; /* after two bytes, no timer  */
    raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 8; /* after a byte or .8 seconds */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(ttyfd,TCSAFLUSH,&raw) < 0) fatal("can't set raw mode");
}

/* reset tty - useful also for restoring the terminal when this process
   wishes to temporarily relinquish the tty
*/
int tty_reset(void)
{
    /* flush and reset */
    if (tcsetattr(ttyfd,TCSAFLUSH,&orig_termios) < 0) return -1;
    return 0;
}

void 
init_input_struct(struct input_s* input, void (*keypress_h)(uint16_t keycode))
{
    /* check that input is from a tty */
    if (!isatty(ttyfd)) fatal("not on a tty");

    /* store current tty settings in orig_termios */
    if (tcgetattr(ttyfd,&orig_termios) < 0) fatal("can't get tty settings");

    /* register the tty reset with the exit handler */
    //if (atexit(tty_atexit) != 0) fatal("atexit: can't register tty reset");
    tty_raw();
    input->keypress_h = keypress_h; 
}

void 
destroy_input_struct(struct input_s* input) 
{
    tty_reset(); 
    vifree(input);
}

void 
poll_char(struct input_s* input)
{
    // keep track of last keypress statically
    static uint16_t key = 0; 

    if (key == '\x1b') {
        char lobyte = 0; 
        read(STDIN_FILENO, &lobyte, 1); 
        key = key << 8 | lobyte;
    }
    else {
        key = 0;
        read(STDIN_FILENO, &key, 1); 
    }

    if (key != 0) {
        //printf("%x|", (uint32_t) key);
        input->keypress_h((uint16_t) key);
    }
}
