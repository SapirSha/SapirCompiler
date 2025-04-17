#include "HashSet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ErrorHandler.h"

#define GROWTH_FACTOR 2
#define MAX_LOAD_FACTOR 0.75
#define MINIMUM_HASHSET_CAPACITY 16

static void hashset_expand(HashSet* set) {
    unsigned int new_capacity = set->capacity * GROWTH_FACTOR;
    HashSetNode** new_buckets = calloc(new_capacity, sizeof(HashSetNode*));
    if (!new_buckets) handle_out_of_memory_error();

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
    if (!set) handle_out_of_memory_error();

    set->capacity = (default_capacity < MINIMUM_HASHSET_CAPACITY) ? MINIMUM_HASHSET_CAPACITY : default_capacity;
    set->size = 0;
    set->hash = hash;
    set->equals = equals;
    set->buckets = calloc(set->capacity, sizeof(HashSetNode*));
    if (!set->buckets) {
        free(set);
        handle_out_of_memory_error();
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
    if (!new_node) handle_out_of_memory_error();

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

bool hashset_union(HashSet* set1, HashSet* set2) {
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


void hashset_clear(HashSet* set) {
    for (unsigned int i = 0; i < set->capacity; i++) {
        HashSetNode* node = set->buckets[i];
        while (node) {
            HashSetNode* temp = node;
            node = node->next;
            free(temp);
        }
        set->buckets[i] = NULL;
    }
    set->size = 0;
}

HashSet* hashset_copy(HashSet* set) {
    HashSet* copy = hashset_create(set->capacity, set->hash, set->equals);
    if (!copy)
        return NULL;
    for (unsigned int i = 0; i < set->capacity; i++) {
        HashSetNode* node = set->buckets[i];
        while (node) {
            hashset_insert(copy, node->key);
            node = node->next;
        }
    }
    return copy;
}

bool hashset_equals(HashSet* set1, HashSet* set2) {
    if (set1->size != set2->size)
        return false;

    for (unsigned int i = 0; i < set1->capacity; i++) {
        HashSetNode* node = set1->buckets[i];
        while (node) {
            if (!hashset_contains(set2, node->key))
                return false;
            node = node->next;
        }
    }
    return true;
}

void hashset_remove_all(HashSet* set, HashSet* to_remove) {
    for (unsigned int i = 0; i < to_remove->capacity; i++) {
        HashSetNode* node = to_remove->buckets[i];
        while (node) {
            hashset_remove(set, node->key);
            node = node->next;
        }
    }
}

