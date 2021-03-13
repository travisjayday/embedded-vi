#ifndef HASHT_H
#define HASHT_H

#include <stddef.h>
#include <stdint.h>

#include "vi.h"

#define HASH_ENTRY_TABL 1
struct hash_entry_table {
    uintptr_t key; 
};

#define HASH_ENTRY_DICT 2
struct hash_entry_dict {
    uintptr_t key; 
    uintptr_t value; 
};

struct hasht_s {
    /*
     *  Places an item into the hashtable. 
     *  Returns a pointerto the void* item inside the
     *  buckets buffer. 
     */
    void* (*put)(struct hasht_s*, void* key, void* value); 

    /*
     * Checks whether an item is in the hashtable. 
     * Returns a pointer to the void* item inside the 
     * buuckets buffer or NULL if not found. 
     */
    void* (*get)(struct hasht_s*, void* key); 

    /*
     * Removes an item from the hashtable. 
     * Returns:
     *  A positive number if line was removed. 
     *  Zero if line was not found
     */
    uint8_t (*remove)(struct hasht_s*, void* key); 

    uint8_t type;     // Either HASH_ENTRY_TABL or HASH_ENTRY_DICT
    void* buckets;          // pointer to buckets memory (either 
                            // hash_entry_dic or hash_entry_table)
    uint32_t buckets_n;     // number of allocated buckets 
    uint32_t buckets_used;  // number of buckets in use
};

/*
 * Create a hasht_s that stores items of size item_size. 
 * Allocates an initial bucket capacity void*'ers. 
 */
struct hasht_s* init_hasht(uint32_t capacity, uint8_t type);
void destroy_hasht(struct hasht_s* hasht);

#endif
