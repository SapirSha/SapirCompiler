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

void hashmap_insert(HashMap* hashMap, char* key, void* value) {
    unsigned int index = hash(key, hashMap->size);
    KeyValuePair* current = hashMap->table[index];

    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    KeyValuePair* newPair = malloc(sizeof(KeyValuePair));
    if (!newPair) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newPair->key = _strdup(key);
    newPair->value = value;
    newPair->next = hashMap->table[index];
    hashMap->table[index] = newPair;
    hashMap->count++;
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
