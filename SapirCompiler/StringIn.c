#include "StringIn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* char_to_string(char c) {
	char* buffer = malloc(sizeof(char) * 2);
	buffer[0] = c;
	buffer[1] = '\0';
	return buffer;
}

StringIn* stringin_init() {
    StringIn* node = (StringIn*)malloc(sizeof(StringIn));
    node->isEndOfString = false;
    node->root = createHashMap(DEFAULT_HASHMAP_SIZE);
    return node;
}

void stringin_insertString(StringIn* root, const char* str) {
    StringIn* current = root;
    while (*str) {
        char* ch = char_to_string(*str);

        // Check if child exists, else create it
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

bool stringin_searchString(StringIn* root, const char* str) {
    StringIn* current = root;
    while (*str) {
        StringIn* child = hashmap_get(current->root, (char[]) { *str, '\0' });


        if (!child)
            return false;

        current = child;
        str++;
    }
    return current->isEndOfString;
}

int stringin_next_key(StringIn** pos, const char* str) {
	if (*str == '\0')
		return (*pos)->isEndOfString;

    char* ch = char_to_string(*str);

    StringIn* child = hashmap_get((*pos)->root, ch);

    if (!child) {
        return NOT_FOUND;
    }


    *pos = child;
    return YET_FOUND;
}


void stringin_free(StringIn* root) {
    return; // TODO
}