#include "StringIn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_TO_STRING(c) ((char[]){(c), '\0'})

StringIn* stringin_init() {
    StringIn* node = (StringIn*)malloc(sizeof(StringIn));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->isEndOfString = false;
    node->root = createHashMap(DEFAULT_HASHMAP_SIZE);
    if (!node->root) {
        fprintf(stderr, "Memory allocation failed\n");
        free(node);
        exit(EXIT_FAILURE);
    }
    return node;
}

void stringin_insert_string(StringIn* root, const char* str) {
    StringIn* current = root;
    while (*str) {
        char* ch = CHAR_TO_STRING(*str);

        StringIn* child = hashmap_get(current->root, ch);
        if (!child) {
            child = stringin_init();
            hashmap_insert(current->root, ch, child);
        }
        current = child;
        str++;
    }
    current->isEndOfString = true;
}

bool stringin_search_string(StringIn* root, const char* str) {
    StringIn* current = root;
    while (*str) {
        StringIn* child = hashmap_get(current->root, CHAR_TO_STRING(*str));
        if (!child) return false;
        current = child;
        str++;
    }
    return current->isEndOfString;
}

int stringin_next_key(StringIn** pos, const char* str) {
    if (*str == '\0') return (*pos)->isEndOfString ? FOUND : NOT_FOUND;

    StringIn* child = hashmap_get((*pos)->root, CHAR_TO_STRING(*str));
    if (!child) return NOT_FOUND;

    *pos = child;
    return YET_FOUND;
}

void stringin_free(StringIn* root) {
    if (!root) return;

    for (int i = 0; i < root->root->size; i++) {
        KeyValuePair* current = root->root->table[i];
        while (current) {
            StringIn* child = (StringIn*)current->value;
            stringin_free(child);
            current = current->next;
        }
    }
    freeHashMap(root->root);
    free(root);
}
