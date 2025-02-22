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

typedef struct StringTrie {
    char* to_clear;
    HashMap* paths;
    Token_Types is_end;
} StringTrie;

StringTrie* stringin_init(void);
void stringin_insert_string(StringTrie* s, const char* str, Token_Types token_type);
Token_Types stringin_search_string(const StringTrie* root, const char* str);
int stringin_next(StringTrie** pos, char** remaining_clearance, char next_letter);
void stringin_free(StringTrie* root);
void stringin_print(StringTrie* root);

#endif
