#include "fb.h"

void 
free_line(struct vi_line* line) 
{
    vifree(line->data);
    vifree(line);
}

struct vi_line* 
alloc_emptyl() 
{
    // allocate an initial line of size 0 
    struct vi_line* line = vimalloc(1 * sizeof(struct vi_line));

    // populate first line as empty line 
    line->sog = 0; 
    line->eog = INIT_GAP_SIZE - 1;
    line->data = vimalloc(INIT_GAP_SIZE + 1);
    for (int i = 0; i < INIT_GAP_SIZE + 1; i++) line->data[i] = '\0';
    line->size = INIT_GAP_SIZE + 1;
    line->data[0] = '\0'; 
    line->data_n = 0;
    line->prevl = NULL;
    line->nextl = NULL; 

    return line;
}

struct vi_line* 
seek_line(struct fb_s* fb, uint32_t row) 
{
    struct vi_line* line = fb->headl; 
    for (uint32_t i = 0; i < row; i++) {
        line = line->nextl;  
        if (line == NULL) return NULL;
    }
    return line;
}

void 
merge_lines_up(struct fb_s* fb, struct vi_line* lower_line) 
{
    if (lower_line->data_n != 0) {
        // a newline was added in the middle of the line, so need to
        // revert the newline
        fb->currentl = lower_line->prevl;
        fb->buffer_c = fb->currentl->data_n;
        fb->move_gap_to_cursor(fb);         // align gap for insertion
        fb->currentl = lower_line;          // seek added line
        for (uint8_t i = 0; i < lower_line->size - 1; i++) {
            if (i >= lower_line->sog && i <= lower_line->eog) continue;
            fb->insert_char(fb, lower_line->prevl, *(lower_line->data + i));
        }
    }
    uint32_t t = lower_line->prevl->data_n - lower_line->data_n;
    fb->cutl(fb);                                   // delete added line
    fb->buffer_c = t;
}

void 
move_gap_to_cursor(struct fb_s* fb)
{
    struct vi_line* line = fb->currentl;

    // remove the gap 
    if (fb->buffer_c > line->sog) {
        printlog("sog > bufferc");
        // gap is on the left of c
        vimemmove(line->data + line->sog, 
                line->data + line->eog + 1, 
                fb->buffer_c - line->sog);  
    }  
    else if (fb->buffer_c < line->sog) {
        printlog("sog < bufferc");
        // gap is on the right of c  
        vimemmove(line->data + line->eog - (line->sog - fb->buffer_c) + 1, 
                line->data + fb->buffer_c, 
                line->sog - fb->buffer_c); 
    }
    else if (fb->buffer_c == line->sog) {
        // gap is already aligned
        return;
    }

    // update gap offsets 
    uint32_t gapsize = line->eog - line->sog; 
    line->sog = fb->buffer_c; 
    line->eog = line->sog + gapsize; 
}

void
cursor_left(struct fb_s* fb)
{
    struct vi_line* line = fb->currentl;
    fb->buffer_c--;
    line->data[line->eog] = line->data[fb->buffer_c];
    line->eog--;
    line->sog--; 
}

void
cursor_right(struct fb_s* fb)
{
    struct vi_line* line = fb->currentl;
    line->data[fb->buffer_c] = line->data[line->eog + 1];
    fb->buffer_c++;
    line->eog++;
    line->sog++; 
}

void
cursor_down(struct fb_s* fb)
{
    fb->buffer_r++;
    fb->currentl = fb->currentl->nextl;
    uint16_t next_line_w = fb->currentl->data_n;

    // clap buffer_c to next line width; 
    if (fb->buffer_c >= next_line_w) fb->buffer_c = next_line_w;

    // move gap to cursor
    if (fb->buffer_c != fb->currentl->sog) move_gap_to_cursor(fb);
}

void
cursor_up(struct fb_s* fb)
{
    fb->buffer_r--;
    fb->currentl = fb->currentl->prevl;
    uint16_t prev_line_w = fb->currentl->data_n;

    // clap buffer_c to next prev width;
    if (fb->buffer_c >= prev_line_w) fb->buffer_c = prev_line_w;

    // move gap to cursor
    if (fb->buffer_c != fb->currentl->sog) move_gap_to_cursor(fb);
}

void
insert_char(struct fb_s* fb, struct vi_line* line, char c)
{
    if (line->sog + 1 == line->eog) {
        // need to grow the gap because gapsize = 2 
        line->data = virealloc(line->data, line->size + INIT_GAP_SIZE - 1); 
        if (line->size - (line->eog + 1) != 0) {
            vimemmove(line->data + line->eog + INIT_GAP_SIZE, 
                    line->data + line->eog + 1, 
                    line->size - line->eog - 1);
        }
        line->size += INIT_GAP_SIZE - 1;
        line->eog += INIT_GAP_SIZE - 1;
    }

    // we have enough gapspace 
    line->data[fb->buffer_c] = c;
    fb->buffer_c++;
    line->sog++;
    line->data_n++;
}


char
backspace_char(struct fb_s* fb, struct vi_line* line)
{
    if (fb->buffer_c != 0) {
        char c = line->data[fb->buffer_c - 1];
        fb->buffer_c--;
        line->sog--;
        line->data_n--;
        return c; 
    }
    else {
        return -1;  
    }
}
 

void
insert_line_after(struct fb_s* fb, struct vi_line* parent, struct vi_line* child)
{
    struct vi_line* line = child;
    if (line == NULL) {
        line = alloc_emptyl();
        uint32_t diff = fb->currentl->data_n - fb->buffer_c;
        if (diff > 0) {
            // newline pressed somewhere in middle of current line
            line->size = diff + INIT_GAP_SIZE + 1;
            virealloc(line->data, line->size); 
            vimemcpy(line->data + INIT_GAP_SIZE, 
                    fb->currentl->data + fb->currentl->eog + 1, 
                    diff + 1);
            line->sog = 0; 
            line->eog = INIT_GAP_SIZE - 1;
            line->data[line->size - 1] = '\0';
            line->data_n = diff; 

            fb->currentl->eog = fb->currentl->size - 2;     // blank out rest of line
            fb->currentl->data_n -= diff;
        }
    }
    if (parent == NULL) {
        printlog("Inserting into beginnging");
        // inserting line at beginning of buffer
        fb->scroll      = line;
        line->nextl     = fb->headl;
        fb->headl->prevl = line;
        if (fb->headl == fb->taill) fb->taill = line;
        fb->headl       = line;
        line->prevl     = NULL;
    }
    else if (parent->nextl == NULL) {
        // inserting a line at the end of buffer. `parent` is the current tali. 
        parent->nextl   = line; 
        line->prevl     = parent;  
        fb->taill       = line;  
    }
    else { // inserting a line in the middle of buffer. 
        struct vi_line* nextl = parent->nextl;
        parent->nextl   = line;      
        line->nextl     = nextl;
        line->prevl     = parent; 
        nextl->prevl    = line;
    }
    fb->currentl = line;
    fb->buffer_n++;
    fb->buffer_r++;
    fb->buffer_c = 0; 
}

uint8_t
cutl(struct fb_s* fb)
{
    struct vi_line* line = fb->currentl;
    struct vi_line* prev = line->prevl;
    uint8_t ret = DELETED_STAY;

    if (prev == NULL) {
        // no previous line
        if (line->nextl == NULL) {
            // on first and only line, do nothing
            return NONE_DELETED;
        }
        else {
            // no previous line but next line exists
            fb->headl = line->nextl;
            fb->scroll = line->nextl;
            fb->currentl = line->nextl;
            line->nextl->prevl = NULL;
            ret = DELETED_STAY;
        }
    }
    else {
        // previous line exists
        prev->nextl = line->nextl; 
        if (line->nextl != NULL) {
            // we are deleted in middle of buffer
            line->nextl->prevl = prev;
            fb->currentl = line->nextl;
            ret = DELETED_STAY;
        }
        else {
            // we are deleting at end of buffer
            fb->taill = prev;
            vi->fb->cursor_up(vi->fb);
            ret = DELETED_UPWARD;
        }
    }

    if (line == fb->scroll) {
        fb->scroll_r--;
        fb->scroll = fb->scroll->prevl;
        cursor_up(fb);
    }

    return ret;
}

void
init_fb_struct(struct fb_s* fb) 
{
    // store function pointers 
    fb->insert_line_after = &insert_line_after; 
    fb->insert_char       = &insert_char; 
    fb->cursor_left       = &cursor_left;
    fb->cursor_right      = &cursor_right;
    fb->cursor_down       = &cursor_down;
    fb->cursor_up         = &cursor_up;
    fb->backspace_char    = &backspace_char;
    fb->move_gap_to_cursor= &move_gap_to_cursor;
    fb->seek_line         = &seek_line;
    fb->cutl              = &cutl;
    fb->alloc_emptyl      = &alloc_emptyl;
    fb->merge_lines_up    = &merge_lines_up;

    // allocate empty vi_line
    struct vi_line* first_line = alloc_emptyl(); 

    // init remaining fb 
    fb->headl             = first_line;
    fb->scroll            = first_line; 
    fb->taill             = first_line; 
    fb->currentl          = first_line; 
    fb->buffer_n          = 1; 
    fb->buffer_c          = 0; 
    fb->buffer_r          = 0; 
    fb->buffer_scroll     = 0; 
}

void 
destroy_fb_struct(struct fb_s* fb)
{
    struct vi_line* line = fb->headl;
    while (line != NULL) {
        struct vi_line* nextl = line->nextl; 
        vifree(line->data);
        vifree(line);
        line = nextl;
    }
    vifree(fb);
}


