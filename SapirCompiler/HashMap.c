#include "HashMap.h"

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Hash function to map a string key to an index
static unsigned int hash(char* key, int tableSize) {
    unsigned int hashValue = 31;
    while (*key) {
        hashValue = ((hashValue + *key) * *key) % tableSize;
        key++; 
    }
    return hashValue % tableSize;
}

// Create a new hash map
HashMap* createHashMap(int size) {
    HashMap* hashMap = malloc(sizeof(HashMap));
    hashMap->size = size;
    hashMap->count = 0;
    hashMap->table = malloc(sizeof(KeyValuePair*) * size);
    for (int i = 0; i < size; i++) {
        hashMap->table[i] = NULL;
    }
    return hashMap;
}

// Insert a key-value pair into the hash map
void hashmap_insert(HashMap* hashMap, char* key, int value) {
    // Resize if the load factor exceeds the threshold
    if ((float)(hashMap->count + 1) / hashMap->size > MAX_LOAD_FACTOR) {
        printf("Resizing the hash table...\n");
        // Double the table size
        int newSize = (int)(hashMap->size * GROWTH_FACTOR);
        KeyValuePair** newTable = malloc(sizeof(KeyValuePair*) * newSize);
        for (int i = 0; i < newSize; i++) {
            newTable[i] = NULL;
        }

        // Rehash all existing keys and place them in the new table
        for (int i = 0; i < hashMap->size; i++) {
            KeyValuePair* current = hashMap->table[i];
            while (current != NULL) {
                unsigned int index = hash(current->key, newSize);
                KeyValuePair* next = current->next;

                current->next = newTable[index];
                newTable[index] = current;

                current = next;
            }
        }

        // Free the old table and update to the new table
        free(hashMap->table);
        hashMap->table = newTable;
        hashMap->size = newSize;
    }

    unsigned int index = hash(key, hashMap->size);
    KeyValuePair* newPair = malloc(sizeof(KeyValuePair));
    newPair->key = _strdup(key);
    newPair->value = value;
    newPair->next = NULL;

    KeyValuePair* p = hashMap->table[index];
    bool found = false;
    while (p != NULL) {
        if (strcmp(p->key, key) == 0){
            p->value = value;
            found = true;
        }
        p = p->next;
    }

    if (!found) {
        // Insert at the beginning of the list (head)
        newPair->next = hashMap->table[index];
        hashMap->table[index] = newPair;

        hashMap->count++;
    }
}

// Get a value by key from the hash map
int hashmap_get(HashMap* hashMap, char* key) {
    unsigned int index = hash(key, hashMap->size);
    KeyValuePair* current = hashMap->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;  // Return 0 if key is not found
}

int hashmap_exists(HashMap* hashMap, char* key) {
    unsigned int index = hash(key, hashMap->size);
    KeyValuePair* current = hashMap->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return 1; // Found
        }
        current = current->next;
    }
    return 0;  // Return 0 if key is not found
}

// Print the hash map
void hashmap_printHashMap(HashMap* hashMap) {
    for (int i = 0; i < hashMap->size; i++) {
        KeyValuePair* current = hashMap->table[i];
        if (current != NULL) {
            printf("Bucket %d:\n", i);
            while (current != NULL) {
                printf("  Key: %s, Value: %d\n", current->key, current->value);
                current = current->next;
            }
        }
    }
}

// Free the hash map
void freeHashMap(HashMap* hashMap) {
    for (int i = 0; i < hashMap->size; i++) {
        KeyValuePair* current = hashMap->table[i];
        while (current != NULL) {
            KeyValuePair* temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
    free(hashMap->table);
    free(hashMap);
}
