#include "history.h"

void 
add_undoable_at_loc(uint8_t cmd, uint8_t arg, void* dptr, uint32_t row, uint32_t col) 
{
    struct cmd_s* ptr = (struct cmd_s*) vi->his->undo_ring->next(vi->his->undo_ring);
    ptr->cmd = cmd;
    ptr->arg = arg;
    ptr->dptr = (uintptr_t) dptr;
    ptr->buffer_c = col;
    ptr->buffer_r = row;
    ptr->scroll_r = vi->fb->scroll_r;
    vi->his->undo_ring->commit(vi->his->undo_ring);
}

void 
add_undoable(uint8_t cmd, uint8_t arg, void* dptr) 
{
    add_undoable_at_loc(cmd, arg, dptr, vi->fb->buffer_r, vi->fb->buffer_c);
}

uint8_t
undo(struct his_s* his, struct fb_s* fb) 
{
    struct cmd_s* cmd = (struct cmd_s*) his->undo_ring->pop(his->undo_ring);
    if (cmd == NULL) return 1;

    uint32_t r = fb->buffer_r;
    uint32_t c = fb->buffer_c;
    struct vi_line* l = fb->currentl;
     
    switch (cmd->cmd) {
        case VICMD_INSERT_C:
            // restore cursor
            fb->buffer_r = cmd->buffer_r;
            fb->buffer_c = cmd->buffer_c + 1;
            if (fb->currentl == NULL) break;

            fb->currentl = fb->seek_line(fb, fb->buffer_r);
            
            // restore scroll
            fb->scroll_r = cmd->scroll_r;
            fb->scroll = fb->seek_line(fb, cmd->scroll_r); 

            fb->move_gap_to_cursor(fb); 
            fb->backspace_char(fb, fb->currentl);
            return 0; 
        case VICMD_BACKSPACE_C:
            // restore cursor
            fb->buffer_r = cmd->buffer_r;
            fb->buffer_c = cmd->buffer_c;
            fb->currentl = fb->seek_line(fb, fb->buffer_r);
            if (fb->currentl == NULL) break;

            // restore scroll
            fb->scroll_r = cmd->scroll_r;
            fb->scroll = fb->seek_line(fb, cmd->scroll_r);

            fb->move_gap_to_cursor(fb); 
            fb->insert_char(fb, fb->currentl, cmd->arg);
            return 0; 
        case VICMD_ADDL:
            // restore cursor
            fb->buffer_c = cmd->buffer_c;
            fb->buffer_r = cmd->buffer_r;
            l = fb->seek_line(fb, fb->buffer_r)->nextl;
            fb->currentl = l;
            if (fb->currentl == NULL) break;

            // restore scroll
            fb->scroll_r = cmd->scroll_r;
            fb->scroll = fb->seek_line(fb, cmd->scroll_r); 

            fb->merge_lines_up(fb, (struct vi_line*) cmd->dptr);

            // move cursor back 
            fb->currentl = l;
            fb->buffer_c = cmd->buffer_c;
            fb->buffer_r = cmd->buffer_r;
            fb->move_gap_to_cursor(fb);
            return 0;
        case VICMD_DELL:
            // restore cursor
            fb->buffer_c = cmd->buffer_c;
            fb->buffer_r = cmd->buffer_r;
            fb->currentl = fb->seek_line(fb, fb->buffer_r);
            if (fb->currentl == NULL) break;

            // restore scroll
            fb->scroll_r = cmd->scroll_r;
            fb->scroll = fb->seek_line(fb, cmd->scroll_r); 

            if (fb->currentl == NULL) break;
            vi->fb->insert_line_after(vi->fb, (struct vi_line*) cmd->dptr, NULL);
            fb->move_gap_to_cursor(fb);
            return 0;
        case VICMD_DD:
            // restore cursor
            fb->buffer_r = cmd->buffer_r;
            fb->buffer_c = cmd->buffer_c;
            fb->currentl = fb->seek_line(fb, fb->buffer_r); 
            if (fb->currentl == NULL) break;

            // restore scroll
            fb->scroll_r = cmd->scroll_r;
            fb->scroll = fb->seek_line(fb, cmd->scroll_r); 
 
            // the line that was cut
            l = (struct vi_line*) cmd->dptr;

            if (fb->currentl == fb->headl) {
                if (l->prevl == NULL) {
                    // DD'd the very first line 
                    if (l->nextl == NULL) {
                        // it was the only line in buffer
                        fb->insert_line_after(fb, NULL, l); 
                        fb->currentl = l->nextl;
                        fb->cutl(fb);
                        fb->buffer_c = cmd->buffer_c;
                        fb->move_gap_to_cursor(fb);
                    }
                    else {
                        // there were other lines after it
                        fb->insert_line_after(fb, NULL, l); 
                        fb->buffer_r = cmd->buffer_r; 
                    }
                }
                else {
                    // Restoring at tail, so just insert it
                    fb->insert_line_after(fb, fb->currentl, l); 
                }
            }
            else if (fb->currentl == fb->taill) {
                if (l->nextl == NULL) {
                    // intiially DD'd the last line, so just insert it
                    fb->insert_line_after(fb, fb->currentl, l); 
                }
                else {
                    // initailly DD'd second to last line, so now this line
                    // will become the new tail
                    l->nextl = NULL;
                    fb->insert_line_after(fb, fb->currentl->prevl, l); 
                    fb->buffer_r--;
                }
            }
            else {
                // DD'd a random line in the body of the buffer
                fb->insert_line_after(fb, fb->currentl->prevl, l); 
                fb->buffer_r--;
            }

            fb->buffer_c = cmd->buffer_c;
            return 0;
    }

    fb->buffer_r = r;
    fb->buffer_c = c;
    fb->currentl = l; 

    return 0;
}

void
deallocate_cmd(void* cmd_ptr) 
{
    struct cmd_s* cmd = (struct cmd_s*) cmd_ptr;
    if (cmd == NULL) return; 
    struct vi_line* line;
    struct vi_line* start; 
    switch (cmd->cmd) {
        // comamnds that do not contain a DPTR
        case VICMD_INSERT_C:
        case VICMD_BACKSPACE_C:
            // no deallocation is needed as no pointers are held.
            return;
        // commands that contain a struct vi_line* DPTR
        case VICMD_DELL:
        case VICMD_DD:
        case VICMD_ADDL:
            line = (struct vi_line*) cmd->dptr; 
            start = vi->fb->headl; 
            while (start != NULL) {
                if (start == line) return; // line is still in use...
                start = start->nextl;
            }
            vifree(line->data); 
            break;
    }
}

void 
init_history_struct(struct his_s* his)
{
    his->undo_ring = init_ring(VI_HISTORY_SIZE, sizeof(struct cmd_s));
    his->undo_ring->deallocator = &deallocate_cmd;
    his->add_undoable = &add_undoable;
    his->add_undoable_at_loc = &add_undoable_at_loc;
    his->undo = &undo;
}

void 
destroy_history_struct(struct his_s* his)
{
    his->undo_ring->destroy(his->undo_ring);
    vifree(his);
}
