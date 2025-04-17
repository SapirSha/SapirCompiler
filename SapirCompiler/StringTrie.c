#include "StringTrie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ErrorHandler.h"

#ifdef _WIN32
#define strdup _strdup
#endif

#define CHAR_TO_STRING(c) ((char[]){(c), '\0'})

StringTrie* stringin_init() {
    StringTrie* s = (StringTrie*)malloc(sizeof(StringTrie));
    if (!s) handle_out_of_memory_error();

    if (!s) return NULL;

    s->to_clear = strdup("");
    if (!s->to_clear) {
        free(s);
        return NULL;
    }

    s->paths = NULL;
    s->is_end = TOKEN_UNKNOWN;
    return s;
}

void stringin_insert_string(StringTrie* s, const char* str, Token_Types token_type) {
    if (!s || !str) return;

    char* pos = s->to_clear;
    const char* spos = str;

    while (*pos && *spos && *pos == *spos) {
        pos++;
        spos++;
    }

    if (*spos == '\0' && *pos == '\0') {
        s->is_end = token_type;
    }
    else if (*spos == '\0') {
        StringTrie* new_child = stringin_init();

        new_child->to_clear = strdup(pos + 1);
        new_child->is_end = s->is_end;
        new_child->paths = s->paths;

        s->is_end = token_type;
        char* temp = s->to_clear;
        s->to_clear = strdup(str);

        s->paths = createHashMap(STRINGIN_INITIAL_HASHMAP, string_hash, string_equals);
        hashmap_insert(s->paths, strdup(CHAR_TO_STRING(*pos)), new_child);
        free(temp);

    }
    else if (*pos == '\0') {
        if (*s->to_clear == '\0' && !s->paths && s->is_end == TOKEN_UNKNOWN) {
            free(s->to_clear);
			s->to_clear = strdup(str);
            s->is_end = token_type;
			return;
        }
        if (!s->paths) {
            s->paths = createHashMap(STRINGIN_INITIAL_HASHMAP, string_hash, string_equals);
        }

        StringTrie* next = hashmap_get(s->paths, CHAR_TO_STRING(*spos));
        if (!next) {
            next = stringin_init();
            if (!next) {
                fprintf(stderr, "Failed to allocate memory for next node.\n");
                exit(EXIT_FAILURE);
            }

            next->to_clear = strdup(spos + 1);
            next->is_end = token_type;
            hashmap_insert(s->paths, strdup(CHAR_TO_STRING(*spos)), next);
        }
        else {
            stringin_insert_string(next, spos + 1, token_type);
        }
    }
    else {
        StringTrie* new_child = stringin_init();
        if (!new_child) {
            fprintf(stderr, "Failed to allocate memory for new child.\n");
            exit(EXIT_FAILURE);
        }

        new_child->to_clear = strdup(pos + 1);
        new_child->is_end = s->is_end;
        new_child->paths = s->paths;

        s->is_end = TOKEN_UNKNOWN;

		char* buffer = (char*)malloc(spos - str + 1);
        if (!buffer) handle_out_of_memory_error();

        for (int i = 0; i < spos - str; i++)
            buffer[i] = str[i];
		buffer[spos - str] = '\0';

        s->to_clear = strdup(buffer);
        free(buffer);

        s->paths = createHashMap(STRINGIN_INITIAL_HASHMAP, string_hash, string_equals);
        if (!s->paths) {
            fprintf(stderr, "Failed to create hashmap.\n");
            exit(EXIT_FAILURE);
        }

        hashmap_insert(s->paths, strdup(CHAR_TO_STRING(*pos)), new_child);

        StringTrie* next = stringin_init();
        if (!next) {
            fprintf(stderr, "Failed to allocate memory for next node.\n");
            exit(EXIT_FAILURE);
        }

        next->to_clear = strdup(spos + 1);
        next->is_end = token_type;
        hashmap_insert(s->paths, strdup(CHAR_TO_STRING(*spos)), next);
    }
}

Token_Types stringin_search_string(StringTrie* root, const char* str) {
    if (!root || !str) return TOKEN_UNKNOWN;

    const char* pos = root->to_clear;
    while (*pos && *str && *pos == *str) {
        pos++;
        str++;
    }
    if (*pos == '\0' && *str == '\0') {
        return root->is_end;
    }
    else if (*pos == '\0' && *str != '\0') {
        if (!root->paths) return TOKEN_UNKNOWN;

        StringTrie* child = hashmap_get(root->paths, CHAR_TO_STRING(*str));
        return child ? stringin_search_string(child, str + 1) : TOKEN_UNKNOWN;
    }
    return TOKEN_UNKNOWN;
}
int stringin_next(StringTrie** pos, char** remaining_clearance, char next_letter) {
    if (**remaining_clearance == '\0' && next_letter == '\0') {
        return (*pos)->is_end != TOKEN_UNKNOWN ? FOUND : NOT_FOUND;
    }


    if (**remaining_clearance != '\0' && next_letter == '\0') {
        return NOT_FOUND;
    }

    if (**remaining_clearance == '\0' && next_letter != '\0') {
		if ((*pos)->paths == NULL) return NOT_FOUND;
        StringTrie* child = hashmap_get((*pos)->paths, CHAR_TO_STRING(next_letter));
        if (!child) return NOT_FOUND;
        *pos = child;
        *remaining_clearance = child->to_clear;
        return YET_FOUND;
    }

    if (**remaining_clearance != next_letter) {
        return NOT_FOUND;
    }

    (*remaining_clearance)++;
    return YET_FOUND;
}


void stringin_free(StringTrie* root) {
    if (!root) return;

    if (root->to_clear) free(root->to_clear);

    if (root->paths) {
        for (int i = 0; i < root->paths->size; i++) {
            HashMapNode* current = root->paths->buckets[i];
            while (current) {
                StringTrie* child = (StringTrie*)current->value;
                stringin_free(child);
                current = current->next;
            }
        }
        freeHashMap(root->paths);
    }

    free(root);
}




void stringin_print(StringTrie* root) {
    if (!root) return;

    if (root->is_end) {
        printf("%s\n", *root->to_clear == '\0' ? "Empty Clearance" : root->to_clear);
    }

    if (root->paths) {
        for (int i = 0; i < root->paths->size; i++) {
            HashMapNode* current = root->paths->buckets[i];
            while (current) {
                printf("%s\n", current->key);
                stringin_print((StringTrie*)current->value);
                current = current->next;
            }
        }
    }
}
