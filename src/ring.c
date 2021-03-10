#include "ring.h"

void*
next(struct ring_s* ring)
{
    return ring->buffer + ((ring->head + 1) % ring->item_count) * ring->item_size;
}

void
push(struct ring_s* ring, void* item) 
{
    ring->head++;                           // increment head
    ring->head %= ring->item_count;         // stay within bounds

    if (ring->head == ring->tail) {         // if head reaches tail, truncate by 
        ring->tail++;                       // moving tail one up
        ring->tail %= ring->item_count;     // stay within bounds
    }

    (void) item;
    // copy item into buffer 
    /*vimemcpy(ring->buffer + ring->head * ring->item_size, 
            item, ring->item_size); */
}

void* 
pop(struct ring_s* ring)
{
    if (ring->head == ring->tail)           // cannot remove item because 
        return NULL;                        // there's nothing left to remove

    void* ret = ring->buffer + ring->head * ring->item_size;  // pointer to objec to return 

    if (ring->head == 0)                    // wrap to back 
        ring->head = ring->item_count - 1;  // be careful with unsigned ints
    else 
        ring->head--;                       // okay, just subtract

    return ret; 
}

struct ring_s* 
init_ring(uint32_t item_count, uint32_t item_size) 
{
    struct ring_s* ring = (struct ring_s*) vimalloc(sizeof(struct ring_s));
    ring->buffer = vimalloc(item_count * item_size); 
    ring->head = 0;
    ring->tail = 0; 
    ring->item_size = item_size;
    ring->item_count = item_count;
    ring->push = &push;
    ring->pop = &pop;
    ring->next = &next;
    return ring;
}


