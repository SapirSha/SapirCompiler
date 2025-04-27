#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LOAD_FACTOR 0.7
#define MINIMUM_HASHMAP_CAPACITY 5
#define GROWTH_FACTOR 2

//DOESNT allocates space

typedef struct HashMapNode {
    void* key;
    void* value;
    struct HashMapNode* next;
} HashMapNode;

typedef struct HashMap {
    int capacity;
    int size;
    unsigned int (*hash)(void* key);
    int (*equals)(void* key1, void* key2);
    HashMapNode** buckets;
} HashMap;

HashMap* createHashMap(unsigned int default_capacity, unsigned int(*hash)(void* key), int (*equals)(void* key1, void* key2));

void hashmap_insert(HashMap* map, void* key, void* value);

void* hashmap_get(HashMap* map, void* key);

void freeHashMap(HashMap* map);


unsigned int string_hash(void* key);
int string_equals(void* key1, void* key2);

unsigned int int_hash(int* key);
int int_equals(int* key1, int* key2);



#endif
