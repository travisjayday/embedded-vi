#ifndef FB_H
#define FB_H

#include "vi.h"

#define INIT_GAP_SIZE 4

#define NONE_DELETED     0  // no line in buffere was deleted
#define DELETED_UPWARD   1  // line was deleted and cursor will go up
#define DELETED_STAY     2  // line was deleted and cursor will stay 

struct vi_line {
    /*
     * Note: size > data_n. data gets reallocated as line grows. 
     */
    uint32_t        sog;    // start of the gap  
    uint32_t        eog;    // end of the gap  

    uint32_t        size;   // the size allocated for data
    char*           data;   // the char array for this line (including gap 
    uint32_t        data_n; // the number of used characters in this line

    struct vi_line* prevl;  // pointer to previous line in fb or NULL
    struct vi_line* nextl;  // pointer to next line in fb or NULL
};

struct fb_s {
    /*
     * Inserts a line into the buffer at row. row is zero-index. 
     * insert(0) --> insert line at beginning
     * insert(1) --> insert line one after beginning etc
     * If child == NULL, a new line is allocated. Otherwise the given
     * line is inserted. 
     */
    void (*insert_line_after)(struct fb_s* fb, 
            struct vi_line* parent, struct vi_line* child); 

    /*
     * Inserts a char into a viline struct indexed by row. 
     */
    void (*insert_char)(struct fb_s* fb, struct vi_line* line, char c); 

    /*
     * Backspaces a char. Returns the deleted char  if the framebuffer 
     * changed. 0 otherwise. 
     */
    char (*backspace_char)(struct fb_s* fb, struct vi_line* line); 

    /*
     * Functions to move the cursor in a line. Needed for gap buffer.
     * These functions change buffer_c.
     */
    void (*cursor_left)(struct fb_s*); 
    void (*cursor_right)(struct fb_s*); 
    void (*cursor_down)(struct fb_s*); 
    void (*cursor_up)(struct fb_s*); 

    /*
     * Aligns the gap of the currentline to the cursor for inserts.
     * Takes O(n) time but after that, inserts, deletes, and moves will be
     * fast.
     */ 
    void (*move_gap_to_cursor)(struct fb_s*); 

    /*
     * Given a linear row index, return the corresponding vi_line struct. 
     */
    struct vi_line* (*seek_line)(struct fb_s*, uint32_t row);

    /*
     * Cuts the current line out of the buffer. Does nothing if currentline
     * is the first and only line. Deletes line properly if deleting tail. 
     * 
     * Returns: 
     *  NONE_DELETED if no line deleted (on first line) 
     *  DELETED_STAY if deleting line within buffer
     *  DELETED_UPWARD if deleting tail and cursor needs to move upward
     */
    uint8_t (*cutl)(struct fb_s*);

    /*
     * Allocate an empty vi_line. 
     */
    struct vi_line* (*alloc_emptyl)();

    /* 
     * Given a lower line, combine it with the line before and delete it. 
     * Essentially remove the newline character.
     */
    void (*merge_lines_up)(struct fb_s* fb, struct vi_line* lower_line);
   


    /* 
     * The main textbuffer. A pointer to a list of line
     * pointers. 
     *       
     * vi_line** buffer
     *       |
     *       +----- vi_line* (len 2) --- `..`
     *       |
     *       +----- vi_line* (len 5) --- `.....`
     *       |
     *       +----- vi_line* (len 3) --- `...`
     *       |
     *       .
     */      
    // note buffer_size > buffer_n and buffer_size is usually 2**n 
    uint32_t         buffer_n;      // number of vi_line* objects in list 
    uint32_t         buffer_r;      // current row we are in buffer
    uint32_t         buffer_c;      // current column we are in buffer
    uint32_t         buffer_scroll; // scroll offset from vi_line zero 
    uint32_t         scroll_r; 

    struct vi_line*  headl;     // pointer to first line in fb
    struct vi_line*  scroll;    // pointer to the first line visible on screen 
    struct vi_line*  currentl;  // pointer to currently editing line in fb
    struct vi_line*  taill;     // pointer to last line in fb
};

// initialize a framebuffer struct with a single empty line 
void init_fb_struct(struct fb_s* fb); 
void destroy_fb_struct(struct fb_s* fb);



#endif
