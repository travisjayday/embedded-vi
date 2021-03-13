#include "ring.h"
#include "env.h"

void*
next(struct ring_s* ring)
{
    // get the pointer to the next item
    uint32_t buffer_index = (ring->head + 1) % ring->item_count;
    void* next_item = ring->buffer + buffer_index * ring->item_size;
    
    if (buffer_index == ring->tail) {
        // the next item will overwrite the first item in the buffer, 
        // so deallocate it first. 
        if (ring->deallocator != NULL) ring->deallocator(next_item); 
    }
    return next_item; 
}

void
commit(struct ring_s* ring) 
{
    ring->head++;                           // increment head
    ring->head %= ring->item_count;         // stay within bounds

    if (ring->head == ring->tail) {         // if head reaches tail, truncate by 
        ring->tail++;                       // moving tail one up
        ring->tail %= ring->item_count;     // stay within bounds
    }
}

void* 
pop(struct ring_s* ring)
{
    if (ring->head == ring->tail)           // cannot remove item because 
        return NULL;                        // there's nothing left to remove

    // pointer to objec to return 
    void* ret = ring->buffer + ring->head * ring->item_size;      
    
    if (ring->head == 0)                    // wrap to back 
        ring->head = ring->item_count - 1;  // be careful with unsigned ints
    else 
        ring->head--;                       // okay, just subtract

    return ret; 
}


void
destroy(struct ring_s* ring) 
{
    void* item = ring->buffer; 
    for (uint32_t i = 0; i < ring->item_count; i++) {
        ring->deallocator(item); 
        item += ring->item_size;  
    }
    vifree(ring->buffer); 
    vifree(ring);
}

struct ring_s* 
init_ring(uint32_t item_count, uint32_t item_size) 
{
    struct ring_s* ring = (struct ring_s*) vimalloc(sizeof(struct ring_s));
    ring->buffer = vicalloc(item_count, item_size); 
    ring->head = 0;
    ring->tail = 0; 
    ring->item_size = item_size;
    ring->item_count = item_count;
    ring->commit = &commit;
    ring->pop = &pop;
    ring->next = &next;
    ring->deallocator = NULL;
    ring->destroy = &destroy;
    return ring;
}


