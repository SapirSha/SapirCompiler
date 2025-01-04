// StringIn.h
#ifndef STRINGIN_H
#define STRINGIN_H
#include "HashMap.h"
#include <stdbool.h>
#include "Tokens.h"

#define STRINGIN_INITIAL_HASHMAP 7
#define MAX_CHAR_BUF 2

#define NOT_FOUND 0
#define FOUND 1
#define YET_FOUND 2

typedef struct StringIn {
    char* to_clear;
    HashMap* paths;
    Token_Types is_end;
} StringIn;

StringIn* stringin_init(void);
void stringin_insert_string(StringIn* s, const char* str, Token_Types token_type);
Token_Types stringin_search_string(const StringIn* root, const char* str);
int stringin_next(StringIn** pos, char** remaining_clearance, char next_letter);
void stringin_free(StringIn* root);
void stringin_print(StringIn* root);

#endif
