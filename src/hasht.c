#include "hasht.h"

#define EMPTY_ITEM      (uintptr_t) 0x0
#define DELETED_ITEM    (uintptr_t) 0x1

/*
 * Non-cryptographic Hash function.
 *
 * See 
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 * key is a pointer to data to hash, len is the length of the data to hash. 
 */
uint32_t 
fnv_hash_1a_32(void *key, uint32_t len) 
{
    uint8_t* p = key;
    uint32_t h = 0x811c9dc5;
    for (uint32_t i = 0; i < len; i++) h = (h ^ p[i]) * 0x01000193;
    return h;
}

uint64_t 
fnv_hash_1a_64(void *key, uint64_t len) 
{
    uint8_t* p = key;
    uint64_t h = 0xcbf29ce484222325ULL;
    for (uint64_t i = 0; i < len; i++) h = (h ^ p[i]) * 0x100000001b3ULL;
    return h;
}

/*
 * Expand the current hashtable. 
 * Resizees the hashtable buckets to be double the size of the old ones.
 */
void
upsize_table(struct hasht_s* master_table)
{
    // create a temporary table 
    struct hasht_s* tmp_table = init_hasht(
            master_table->buckets_n * 2, master_table->type); 

    // add all items in master to temp table
    if (master_table->type == HASH_ENTRY_TABL) {
        // item is a hashtable
        struct hash_entry_table* item = master_table->buckets;  
        for (uint32_t i = 0; i < master_table->buckets_n; i++) {
            uintptr_t key = item[i].key;
            if (key > DELETED_ITEM)
                tmp_table->put(tmp_table, (void*) key, NULL); 
        }
    }
    else if (master_table->type == HASH_ENTRY_DICT) {
        // item is a hashdict
        struct hash_entry_dict* entry = master_table->buckets;  
        for (uint32_t i = 0; i < master_table->buckets_n; i++) {
            uintptr_t key = entry[i].key;
            if (key > DELETED_ITEM)
                tmp_table->put(tmp_table, (void*) key, (void*) entry->value); 
        }
    }         
    else {
        // emergency exit 
        exit_vi();
    }

    // switch buckets between master and temp table
    uintptr_t* old_buckets = master_table->buckets;  
    master_table->buckets = tmp_table->buckets; 
    master_table->buckets_n = tmp_table->buckets_n; 

    // free old memory
    vifree(tmp_table); 
    vifree(old_buckets); 
}

/*
 * Hashes a void* address on 32/64 bit systems. 
 */
uint32_t
do_hash(void* item)
{
    uintptr_t key = (uintptr_t) item;
    uint32_t hash; 
    if (sizeof(uintptr_t) == sizeof(uint64_t)) 
        // 64 bit system
        hash = fnv_hash_1a_64(&key, sizeof(uintptr_t));
    else 
        // 32 bit system
        hash = fnv_hash_1a_32(&key, sizeof(uintptr_t));
    return hash;
}

void*
put_into_dict(struct hasht_s* hasht, void* key, void* value) 
{
    uint32_t index = do_hash(key) % hasht->buckets_n; 
    struct hash_entry_dict* buckets = hasht->buckets;  

    for (;;) {
        uintptr_t current_key = buckets[index].key;
        if (current_key == (uintptr_t) key) {
            // Found the item in the table dictionary. 
            // Update value. 
            buckets[index].value = (uintptr_t) value;  
            break;
        }
        else if (current_key <= DELETED_ITEM) {
            // allow insertion into deleted slot
            // Found an empty space in the bucket. This means the
            // item was not in the dictionary, so add it for first time.
            buckets[index].key = (uintptr_t) key;  
            buckets[index].value = (uintptr_t) value;  
            hasht->buckets_used++; 
            break;
        }
        else {
            // A collision has occurered. 
            // Use linear open address
            // Collision resulution to find suitable bucket
            index = (index + 1) % hasht->buckets_n; 
        }
    } 

    return buckets + index; 
}

void*
put_into_table(struct hasht_s* hasht, void* item)
{
    uint32_t index = do_hash(item) % hasht->buckets_n; 
    struct hash_entry_table* buckets = hasht->buckets;  

    // allow insertions into a deleted or empty slot
    uintptr_t current_item = 0x00;
    while ((current_item = buckets[index].key) > DELETED_ITEM) {
        // A collision has ocurred. 
        
        // item already inside table. Just abort.
        if (current_item == (uintptr_t) item) return NULL; 
        
        // Use linear open address
        // collision resolution to find suitable bucket
        index = (index + 1) % hasht->buckets_n; 
    }

    // item already inside table. Just abort.
    if (current_item == (uintptr_t) item) return NULL; 

    // insert the item
    buckets[index].key = (uintptr_t) item;
    hasht->buckets_used++; 

    return buckets + index; 
}

void*
put(struct hasht_s* hasht, void* key, void* value) 
{
    void* ret; 
    switch (hasht->type) {
        case HASH_ENTRY_TABL: 
            ret = put_into_table(hasht, key); 
            break;
        case HASH_ENTRY_DICT:
            ret = put_into_dict(hasht, key, value); 
            break;
        default:
            return NULL;
    }

    float load_factor = hasht->buckets_used / hasht->buckets_n; 
    if (load_factor > 0.70f) {
        // according to Wikipedia, if load factor > 0.7, and
        // open addressing is used, performance will drastically
        // decrease. Hence, resize table now. 
        upsize_table(hasht); 
    }

    return ret;
}

void*
get_from_dict(struct hasht_s* hasht, void* key, 
        struct hash_entry_dict** item_ptr) 
{
    uint32_t index = do_hash(key) % hasht->buckets_n; 
    struct hash_entry_dict* buckets = hasht->buckets;  

    for (;;) {
        uintptr_t current_key = buckets[index].key;
        if (current_key == (uintptr_t) key) {
            // Found the item in the table dictionary. 
            
            // return pointer to item in bucketes 
            if (item_ptr != NULL)
                *item_ptr = buckets + index; 
            
            // Return value. 
            return (void*) buckets[index].value;
        }
        else if (current_key == EMPTY_ITEM) {
            // reached an empty item. This means the key 
            // is not in the table. Return NULL.
            return NULL;
        }
        else {
            // A collision has occurered. 
            // Use linear open address
            // Collision resulution to find suitable bucket
            index = (index + 1) % hasht->buckets_n; 
        }
    } 
}

void*
get_from_table(struct hasht_s* hasht, void* item)
{
    uint32_t index = do_hash(item) % hasht->buckets_n; 
    struct hash_entry_table* buckets = hasht->buckets;  

    // allow insertions into a deleted or empty slot
    uintptr_t current_item = 0x00;
    while ((current_item = buckets[index].key) != EMPTY_ITEM) {
        // item already inside table 
        if (current_item == (uintptr_t) item) {
            return (void*) (buckets + index); 
        }
        index = (index + 1) % hasht->buckets_n; 
    }
    return NULL;
}

void* 
get(struct hasht_s* hasht, void* item)
{
    switch (hasht->type) {
        case HASH_ENTRY_TABL: 
            return get_from_table(hasht, item); 
        case HASH_ENTRY_DICT:
            return get_from_dict(hasht, item, NULL); 
        default:
            return NULL;
    }
}

uint8_t 
remove_hashed(struct hasht_s* hasht, void* item)
{
    if (hasht->type == HASH_ENTRY_TABL) { 
        void* entry = get_from_table(hasht, item); 
        if (entry == NULL) 
            // did not find the entry in the table
            return 0;
        *((uintptr_t*) entry) = DELETED_ITEM; 
    }
    else if (hasht->type == HASH_ENTRY_DICT) {
        struct hash_entry_dict* entry = NULL; 
        get_from_dict(hasht, item, &entry); 
        if (entry == NULL) 
            // did not find key in dict
            return 0; 
        entry->key = DELETED_ITEM;
        entry->value = 0x00;
    }

    return 1; 
}


struct hasht_s* 
init_hasht(uint32_t capacity, uint8_t type)
{
    if ((capacity & (capacity - 1)) != 0) 
        // Capacity must be a power of two! 
        // Return null to screw the caller huehuehue. 
        return NULL;  

    struct hasht_s* hasht = (struct hasht_s*) vimalloc(sizeof(struct hasht_s)); 
    hasht->type = type;

    // allocate the buckets depending on type 
    switch (hasht->type) {
        case HASH_ENTRY_TABL: 
            hasht->buckets = vicalloc(capacity, sizeof(struct hash_entry_table));
            break;
        case HASH_ENTRY_DICT:
            hasht->buckets = vicalloc(capacity, sizeof(struct hash_entry_dict));
            break;
        default:
            // unkown type. screw the caller.
            return NULL;
    }

    hasht->buckets_n = capacity; 
    hasht->buckets_used = 0;
    hasht->put = &put;
    hasht->get = &get;
    hasht->remove= &remove_hashed; 

    return hasht;
}

void
destroy_hasht(struct hasht_s* hasht)
{
    vifree(hasht->buckets); 
    vifree(hasht); 
}
