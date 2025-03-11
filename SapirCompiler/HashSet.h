#ifndef HASHSET_H
#define HASHSET_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOAD_FACTOR 0.7
#define MINIMUM_HASHSET_CAPACITY 5
#define GROWTH_FACTOR 2

//DOESNT allocates space

typedef struct HashSetNode {
    void* key;
    struct HashSetNode* next;
} HashSetNode;

typedef struct HashSet {
    unsigned int capacity;
    unsigned int size;
    unsigned int (*hash)(void* key);
    int (*equals)(void* key1, void* key2);
    HashSetNode** buckets;
} HashSet;

HashSet* hashset_create(unsigned int default_capacity, unsigned int (*hash)(void* key), int (*equals)(void* key1, void* key2));

bool hashset_insert(HashSet* set, void* key);

bool hashset_remove(HashSet* set, void* key);

bool hashset_contains(HashSet* set, void* key);

void hashset_free(HashSet* set);

bool hashset_add_hashset(HashSet* set1, HashSet* set2);

void hashset_print(HashSet*, void print(void*));

#endif
