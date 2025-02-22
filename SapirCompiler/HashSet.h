#ifndef HASHSET_H
#define HASHSET_H

#include <stdbool.h>

#define MAX_LOAD_FACTOR 0.7
#define TOMBSTONE ((void*)-1)

// Tomebstone HashSet  // doesnt allocates space
typedef struct HashSet { 
    void** table;
    int capacity;
    int size;
    int tombstones;
} HashSet;

// Create a new hashset
HashSet* hashset_create(int initial_size);

// Insert an element into the hashset
bool hashset_insert(HashSet* set, char* element);

// Remove an element from the hashset
bool hashset_remove(HashSet* set, char* element);

// Check if an element exists in the hashset
bool hashset_contains(HashSet* set, char* element);

// Print the contents of the hashset (for debugging purposes)
void hashset_print(HashSet* set);

// Free the hashset
void hashset_free(HashSet* set);

#endif
