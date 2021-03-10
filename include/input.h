#ifndef INPUT_H
#define INPUT_H

#include "vi.h"

struct input_s {
    /*
     * Keypress callback handler. Called whenever the user hits a key.
     */
    void (*keypress_h)(uint16_t keycode);       
};

/*
 * Initialiezs the keypress handler and registers the callback routine. 
 * Whenever a keypress is made, the passed function pointer gets invoked. 
 */ 
void init_input_struct(struct input_s* input, void (*keypress_h)(uint16_t keycode)); 

void destroy_input_struct(struct input_s* input);

void poll_char(struct input_s* input); 


#endif 
