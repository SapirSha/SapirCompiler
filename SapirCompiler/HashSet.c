// hashset.c
#include "hashset.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper function to calculate a hash for a string
static unsigned int hash(const char* str, int capacity) {
    unsigned int hashValue = 31;
    while (*str) {
        hashValue = ((hashValue + 31) *  *str) % capacity;
        str++;
    }
    return hashValue;
}

// Create a new HashSet
HashSet* hashset_create(int initial_size) {
    HashSet* set = (HashSet*)malloc(sizeof(HashSet));
    set->capacity = initial_size;
    set->size = 0;
    set->tombstones = 0;
    set->table = (char**)calloc(set->capacity, sizeof(char*));
    return set;
}

// Free the HashSet
void hashset_free(HashSet* set) {
    for (int i = 0; i < set->capacity; i++) {
        if (set->table[i] && set->table[i] != TOMBSTONE) {
            free(set->table[i]);
        }
    }
    free(set->table);
    free(set);
}

// Resize the HashSet
static void hashset_resize(HashSet* set) {
    int newCapacity = set->capacity * 2;
    char** newTable = (char**)calloc(newCapacity, sizeof(char*));

    for (int i = 0; i < set->capacity; i++) {
        if (set->table[i] && set->table[i] != TOMBSTONE) {
            unsigned int index = hash(set->table[i], newCapacity);
            while (newTable[index]) {
                index = (index + 1) % newCapacity;
            }
            newTable[index] = set->table[i];
        }
    }

    free(set->table);
    set->table = newTable;
    set->capacity = newCapacity;
    set->tombstones = 0;
}

// Insert a value into the HashSet
bool hashset_insert(HashSet* set, const char* value) {
    if (value == NULL) return;
    if ((float)(set->size + set->tombstones) / set->capacity > MAX_LOAD_FACTOR) {
        hashset_resize(set);
    }

    unsigned int index = hash(value, set->capacity);
    int tombstoneIndex = -1;

    while (set->table[index]) {
        if (set->table[index] == TOMBSTONE) {
            if (tombstoneIndex == -1) {
                tombstoneIndex = index;
            }
        }
        else if (strcmp(set->table[index], value) == 0) {
            return false; // Value already exists
        }
        index = (index + 1) % set->capacity;
    }

    if (tombstoneIndex != -1) {
        index = tombstoneIndex;
        set->tombstones--;
    }

    set->table[index] = _strdup(value);
    set->size++;
    return true;
}

// Check if the HashSet contains a value
bool hashset_contains(HashSet* set, const char* value) {
    unsigned int index = hash(value, set->capacity);

    while (set->table[index]) {
        if (set->table[index] != TOMBSTONE && strcmp(set->table[index], value) == 0) {
            return true;
        }
        index = (index + 1) % set->capacity;
    }

    return false;
}

// Remove a value from the HashSet
bool hashset_remove(HashSet* set, const char* value) {
    unsigned int index = hash(value, set->capacity);

    while (set->table[index]) {
        if (set->table[index] != TOMBSTONE && strcmp(set->table[index], value) == 0) {
            free(set->table[index]);
            set->table[index] = TOMBSTONE;
            set->size--;
            set->tombstones++;
            return true;
        }
        index = (index + 1) % set->capacity;
    }

    return false;
}

// Print the HashSet (for debugging)
void hashset_print(HashSet* set) {
    for (int i = 0; i < set->capacity; i++) {
        if (set->table[i] && set->table[i] != TOMBSTONE) {
            printf("Index %d: %s\n", i, set->table[i]);
        }
        else if (set->table[i] == TOMBSTONE) {
            printf("Index %d: TOMBSTONE\n", i);
        }
        else {
            printf("Index %d: NULL\n", i);
        }
    }
}
