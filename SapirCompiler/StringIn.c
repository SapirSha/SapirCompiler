#include "StringIn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#endif

/*
        char buffer[256];

        for(int i = 0; i < spos - str; i++)
            buffer[i] = str[i];
        buffer[spos - str] = '\0';

        s->to_clear = strdup(buffer);
        */

#define CHAR_TO_STRING(c) ((char[]){(c), '\0'})

StringIn* stringin_init() {
    StringIn* s = (StringIn*)malloc(sizeof(StringIn));
    if (!s) return NULL;

    s->to_clear = strdup("");
    if (!s->to_clear) {
        free(s);
        return NULL;
    }

    s->paths = NULL;
    s->is_end = false;
    return s;
}
void stringin_insert_string(StringIn* s, const char* str) {
    if (!s || !str) return;

    char* pos = s->to_clear;
    const char* spos = str;

    // Traverse the shared prefix
    while (*pos && *spos && *pos == *spos) {
        pos++;
        spos++;
    }

    if (*spos == '\0' && *pos == '\0') {
        // Exact match
        s->is_end = true;
    }
    else if (*spos == '\0') {
        // New string is a prefix of the existing string
        StringIn* new_child = stringin_init();
        if (!new_child) {
            fprintf(stderr, "Failed to allocate memory for new child.\n");
            exit(EXIT_FAILURE);
        }

        new_child->to_clear = strdup(pos + 1);
        new_child->is_end = s->is_end;
        new_child->paths = s->paths;

        s->is_end = true;
        char* temp = s->to_clear;
        s->to_clear = strdup(str);

        s->paths = createHashMap(STRINGIN_INITIAL_HASHMAP);
        hashmap_insert(s->paths, CHAR_TO_STRING(*pos), new_child);
        free(temp);

    }
    else if (*pos == '\0') {
        // Existing string is a prefix of the new string
        if (*s->to_clear == '\0' && !s->paths) {
            free(s->to_clear);
			s->to_clear = strdup(str);
            s->is_end = true;
			return;
        }
        if (!s->paths) {
            s->paths = createHashMap(STRINGIN_INITIAL_HASHMAP);
            if (!s->paths) {
                fprintf(stderr, "Failed to create hashmap.\n");
                exit(EXIT_FAILURE);
            }
        }

        StringIn* next = hashmap_get(s->paths, CHAR_TO_STRING(*spos));
        if (!next) {
            next = stringin_init();
            if (!next) {
                fprintf(stderr, "Failed to allocate memory for next node.\n");
                exit(EXIT_FAILURE);
            }

            next->to_clear = strdup(spos + 1);
            next->is_end = true;
            hashmap_insert(s->paths, CHAR_TO_STRING(*spos), next);
        }
        else {
            stringin_insert_string(next, spos + 1);
        }
    }
    else {
        // Strings diverge at this point
        StringIn* new_child = stringin_init();
        if (!new_child) {
            fprintf(stderr, "Failed to allocate memory for new child.\n");
            exit(EXIT_FAILURE);
        }

        new_child->to_clear = strdup(pos + 1);
        new_child->is_end = s->is_end;
        new_child->paths = s->paths;

        s->is_end = false;

		char* buffer = (char*)malloc(spos - str + 1);
        for (int i = 0; i < spos - str; i++)
            buffer[i] = str[i];
		buffer[spos - str] = '\0';

        s->to_clear = strdup(buffer);
        free(buffer);

        s->paths = createHashMap(STRINGIN_INITIAL_HASHMAP);
        if (!s->paths) {
            fprintf(stderr, "Failed to create hashmap.\n");
            exit(EXIT_FAILURE);
        }

        hashmap_insert(s->paths, CHAR_TO_STRING(*pos), new_child);

        StringIn* next = stringin_init();
        if (!next) {
            fprintf(stderr, "Failed to allocate memory for next node.\n");
            exit(EXIT_FAILURE);
        }

        next->to_clear = strdup(spos + 1);
        next->is_end = true;
        hashmap_insert(s->paths, CHAR_TO_STRING(*spos), next);
    }
}

bool stringin_search_string(StringIn* root, const char* str) {
    if (!root || !str) return false;

    const char* pos = root->to_clear;
    while (*pos && *str && *pos == *str) {
        pos++;
        str++;
    }
    if (*pos == '\0' && *str == '\0') {
        return root->is_end;
    }
    else if (*pos == '\0' && *str != '\0') {
        if (!root->paths) return false;

        StringIn* child = hashmap_get(root->paths, CHAR_TO_STRING(*str));
        return child ? stringin_search_string(child, str + 1) : false;
    }
    return false;
}
int stringin_next(StringIn** pos, char** remaining_clearance, char next_letter) {
    // Case 1: If we've reached the end of both the node and the string
    if (**remaining_clearance == '\0' && next_letter == '\0') {
        return (*pos)->is_end ? FOUND : NOT_FOUND;
    }

    // Case 2: If we're still in the string, but we've reached the end of the node
    if (**remaining_clearance != '\0' && next_letter == '\0') {
        return NOT_FOUND;
    }

    // Case 3: If we're at the end of the node, but still have characters left in the string
    if (**remaining_clearance == '\0' && next_letter != '\0') {
		if ((*pos)->paths == NULL) return NOT_FOUND;
        StringIn* child = hashmap_get((*pos)->paths, CHAR_TO_STRING(next_letter));
        if (!child) return NOT_FOUND;
        *pos = child;
        *remaining_clearance = child->to_clear;
        return YET_FOUND;
    }

    // Case 4: If the current character doesn't match the next letter, it's a mismatch
    if (**remaining_clearance != next_letter) {
        return NOT_FOUND;
    }

    // Case 5: If the current character matches the next letter, move forward
    (*remaining_clearance)++;
    return YET_FOUND;
}


void stringin_free(StringIn* root) {
    if (!root) return;

    if (root->to_clear) free(root->to_clear);

    if (root->paths) {
        for (int i = 0; i < root->paths->size; i++) {
            KeyValuePair* current = root->paths->table[i];
            while (current) {
                StringIn* child = (StringIn*)current->value;
                stringin_free(child);
                current = current->next;
            }
        }
        freeHashMap(root->paths);
    }

    free(root);
}




void stringin_print(StringIn* root) {
    if (!root) return;

    if (root->is_end) {
        printf("%s\n", *root->to_clear == '\0' ? "Empty Clearance" : root->to_clear);
    }

    if (root->paths) {
        for (int i = 0; i < root->paths->size; i++) {
            KeyValuePair* current = root->paths->table[i];
            while (current) {
                printf("%s\n", current->key);
                stringin_print((StringIn*)current->value);
                current = current->next;
            }
        }
    }
}
