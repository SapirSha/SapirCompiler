#include <stdlib.h>
#include <string.h>
#include "HashMap.h"

HashMap* createHashMap(unsigned int default_capacity, unsigned int(*hash)(void* key), int (*equals)(void* key1, void* key2)) {
    if (!hash || !equals) return NULL;

    HashMap* map = malloc(sizeof(HashMap));
    if (!map) return NULL;

    map->capacity = (default_capacity < MINIMUM_HASHMAP_CAPACITY) ? MINIMUM_HASHMAP_CAPACITY : default_capacity;
    map->size = 0;
    map->hash = hash;
    map->equals = equals;
    map->buckets = calloc(map->capacity, sizeof(HashMapNode*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    return map;
}

static void expand(HashMap* map) {
    unsigned int new_capacity = map->capacity * GROWTH_FACTOR;
    HashMapNode** new_buckets = calloc(new_capacity, sizeof(HashMapNode*));

    for (unsigned int i = 0; i < map->capacity; i++) {
        HashMapNode* node = map->buckets[i];
        while (node) {
            HashMapNode* next = node->next;
            unsigned int new_index = map->hash(node->key) % new_capacity;
            node->next = new_buckets[new_index];
            new_buckets[new_index] = node;
            node = next;
        }
    }
    free(map->buckets);
    map->buckets = new_buckets;
    map->capacity = new_capacity;
}

void hashmap_insert(HashMap* map, void* key, void* value) {
    unsigned int index = map->hash(key) % map->capacity;
    HashMapNode* node = map->buckets[index];
    while (node) {
        if (map->equals(node->key, key)) {
            node->value = value;
            return;
        }
        node = node->next;
    }

    if ((map->size + 1) > map->capacity * MAX_LOAD_FACTOR) {
        expand(map);
        index = map->hash(key) % map->capacity;
    }

    HashMapNode* new_node = malloc(sizeof(HashMapNode));
    if (!new_node) return;
    new_node->key = key;
    new_node->value = value;
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    map->size++;
}

void* hashmap_get(HashMap* map, void* key) {
    if (!map || !key) return NULL;

    unsigned int index = map->hash(key) % map->capacity;
    HashMapNode* node = map->buckets[index];
    while (node) {
        if (map->equals(node->key, key)) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void freeHashMap(HashMap* map) {
    if (!map) return;

    for (unsigned int i = 0; i < map->capacity; i++) {
        HashMapNode* node = map->buckets[i];
        while (node) {
            HashMapNode* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

unsigned int string_hash(void* key) {
    char* str = (char*)key;
    unsigned int hash = 317;
    int c;
    while ((c = *str++))
        hash = (hash * 7 + hash * c);
    return hash;
}

int string_equals(void* key1, void* key2) {
    return strcmp((char*)key1, (char*)key2) == 0;
}
