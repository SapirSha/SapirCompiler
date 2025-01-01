#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>

#define MAX_LOAD_FACTOR 0.7
#define GROWTH_FACTOR 2

// Define a structure for the key-value pair
typedef struct KeyValuePair {
    char* key;
    void* value;
    struct KeyValuePair* next;
} KeyValuePair;

// Define a structure for the hash table
typedef struct HashMap {
    KeyValuePair** table;
    int size;
    int count;
} HashMap;

// Create a new hash map
HashMap* createHashMap(int size);

// Insert a key-value pair into the hash map
void hashmap_insert(HashMap* hashMap, char* key, void* value);

// Get a value by key from the hash map
void* hashmap_get(HashMap* hashMap, char* key);
bool hashmap_exists(HashMap* hashMap, char* key);

// Print the hash map
void hashmap_print(HashMap* hashMap);

// Free the hash map
void freeHashMap(HashMap* hashMap);


#endif