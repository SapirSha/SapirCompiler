#ifndef STRINGIN_H
#define STRINGIN_H

#include "HashMap.h"
#include <stdbool.h>

#define DEFAULT_HASHMAP_SIZE 5

#define FOUND 1
#define NOT_FOUND 0
#define YET_FOUND 2

typedef struct StringIn {
    HashMap* root;
    bool isEndOfString;
} StringIn;

StringIn* stringin_init();
void stringin_insertString(StringIn* root, const char* str);
bool stringin_searchString(StringIn* root, const char* str);
int stringin_next_key(StringIn** pos, const char* str);
void stringin_free(StringIn* root);

#endif