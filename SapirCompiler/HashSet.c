#include "HashSet.h"
#include <stdio.h>

static void hashset_expand(HashSet* set) {
    unsigned int new_capacity = set->capacity * GROWTH_FACTOR;
    HashSetNode** new_buckets = calloc(new_capacity, sizeof(HashSetNode*));

    for (unsigned int i = 0; i < set->capacity; i++) {
        HashSetNode* node = set->buckets[i];
        while (node) {
            HashSetNode* next = node->next;
            unsigned int new_index = set->hash(node->key) % new_capacity;
            node->next = new_buckets[new_index];
            new_buckets[new_index] = node;
            node = next;
        }
    }
    free(set->buckets);
    set->buckets = new_buckets;
    set->capacity = new_capacity;
}

HashSet* hashset_create(unsigned int default_capacity, unsigned int (*hash)(void* key), int (*equals)(void* key1, void* key2)) {
    HashSet* set = malloc(sizeof(HashSet));
    if (!set)
        return NULL;

    set->capacity = (default_capacity < MINIMUM_HASHSET_CAPACITY) ? MINIMUM_HASHSET_CAPACITY : default_capacity;
    set->size = 0;
    set->hash = hash;
    set->equals = equals;
    set->buckets = calloc(set->capacity, sizeof(HashSetNode*));
    if (!set->buckets) {
        free(set);
        return NULL;
    }

    return set;
}

bool hashset_contains(HashSet* set, void* key) {
    unsigned int index = set->hash(key) % set->capacity;
    HashSetNode* node = set->buckets[index];
    while (node) {
        if (set->equals(node->key, key))
            return true;
        node = node->next;
    }
    return false;
}

bool hashset_insert(HashSet* set, void* key) {
    if (hashset_contains(set, key))
        return false;

    if ((set->size + 1) > set->capacity * MAX_LOAD_FACTOR)
        hashset_expand(set);

    unsigned int index = set->hash(key) % set->capacity;
    HashSetNode* new_node = malloc(sizeof(HashSetNode));
    if (!new_node)
        return false;

    new_node->key = key;
    new_node->next = set->buckets[index];
    set->buckets[index] = new_node;
    set->size++;
    return true;
}

bool hashset_remove(HashSet* set, void* key) {
    unsigned int index = set->hash(key) % set->capacity;
    HashSetNode* node = set->buckets[index];
    HashSetNode* prev = NULL;

    while (node) {
        if (set->equals(node->key, key)) {
            if (prev)
                prev->next = node->next;
            else
                set->buckets[index] = node->next;
            free(node);
            set->size--;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}

void hashset_free(HashSet* set) {
    if (!set)
        return;

    for (unsigned int i = 0; i < set->capacity; i++) {
        HashSetNode* node = set->buckets[i];
        while (node) {
            HashSetNode* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(set->buckets);
    free(set);
}

bool hashset_add_hashset(HashSet* set1, HashSet* set2) {
    bool changed = false;
    for (unsigned int i = 0; i < set2->capacity; i++) {
        HashSetNode* node = set2->buckets[i];
        while (node) {
            if (!hashset_contains(set1, node->key)) {
                changed = true;
                hashset_insert(set1, node->key);
            }
            node = node->next;

        }
    }
    return changed;
}


void hashset_print(HashSet* set, void print(void*)) {
    printf("HASHSET: ");
    for (unsigned int i = 0; i < set->capacity; i++) {
        HashSetNode* node = set->buckets[i];
        while (node) {
            print(node->key);
            node = node->next;
        }
    }
    printf("\n");
}