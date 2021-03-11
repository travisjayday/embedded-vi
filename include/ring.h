#ifndef RING_H
#define RING_H

#include "vi.h"

/*
 * A circular stack to which infinitely many items can be pushed but
 * finitely many can be popped. Operated under fixed size memory for
 * zero heap allocations. 
 *
 * To initialize the buffer:
 *  Call init_ring, specifying the number of items and the size of an item.
 *
 * To push an item: 
 *  Get a void* reference to the next block of memory using `next()`
 *  Populate the void* by casting to whichever struct you've chosen.
 *  Commit the changes py calling `commit()`
 *  Pop a struct from the stack using `pop()`
 */
struct ring_s {
    uint32_t head;          // head of the stack (semi-equivalent to stack pointer)
    uint32_t tail;          // tail of the stack (semi-equivalent to base pointer)
    void* buffer;           // pointer to the raw memory of the buffer
    uint32_t item_size;     // size of an item in bytes
    uint32_t item_count;    // number of items this buffer stores
    
    /*
     * Returns the most recent item pushed to the stack. If no items
     * remain, returns NULL.
     */
    void* (*pop)(struct ring_s*); 

    /*
     * Returns a pointer to a chunk of memory which can be populated
     * with the next item. You must call push after to commit the 
     * changes to the ring!
     */
    void* (*next)(struct ring_s* ring); 

    /*
     * Commit the changes to the ring. 
     */
    void (*commit)(struct ring_s*); 

    /*
     * Called when a new item is requested with next() but the new item
     * would overwrite the oldest item in the circular buffer. 
     * This functions should clean up / deallocate the oldest item before
     * overwriting it with new data. Set to NULL for no deallocation.
     */
    void (*deallocator)(void* item);

    /*
     * Deallocates the entire ring and cleans up. 
     */
    void (*destroy)(struct ring_s*);
};

/*
 *  Returns an allocated and initialized ring_s struct pointer. 
 */
struct ring_s* init_ring(uint32_t item_count, uint32_t item_size);

#endif
