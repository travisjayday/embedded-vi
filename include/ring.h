#ifndef RING_H
#define RING_H

#include "vi.h"

struct ring_s {
    uint32_t head; 
    uint32_t tail;
    void* buffer; 
    uint32_t item_size; 
    uint32_t item_count;
    void (*push)(struct ring_s*, void* item); 
    void* (*pop)(struct ring_s*); 
    void* (*next)(struct ring_s* ring); 
};

struct ring_s* init_ring(uint32_t item_count, uint32_t item_size);

#endif
