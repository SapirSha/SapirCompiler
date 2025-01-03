#include "HashMap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static unsigned int hash(char* key, int tableSize) {
    unsigned long hashValue = 5381;
    while (*key) {
        hashValue = ((hashValue << 5) + hashValue) + *key;
        key++;
    }
    return hashValue % tableSize;
}

HashMap* createHashMap(int size) {
    printf("CREATING HASHMAP\n");
    HashMap* hashMap = malloc(sizeof(HashMap));
    if (!hashMap) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    hashMap->size = size;
    hashMap->count = 0;
    hashMap->table = calloc(size, sizeof(KeyValuePair*));
    if (!hashMap->table) {
        printf("Memory allocation failed\n");
        free(hashMap);
        exit(1);
    }
    return hashMap;
}

// Insert a key-value pair into the hash map
void hashmap_insert(HashMap* hashMap, char* key, void* value) {
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
        printf("COLLISION IN HASHMAP!\n");
        if (strcmp(p->key, key) == 0) {
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

void* hashmap_get(HashMap* hashMap, char* key) {
    unsigned int index = hash(key, hashMap->size);
    KeyValuePair* current = hashMap->table[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

void freeHashMap(HashMap* hashMap) {
    for (int i = 0; i < hashMap->size; i++) { 
        KeyValuePair* current = hashMap->table[i];
        while (current) {
            KeyValuePair* temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
    free(hashMap->table);
    free(hashMap);
}
