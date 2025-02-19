#ifndef PARSERTABLEGENERATOR_H
#define PARSERTABLEGENERATOR_H

#include "Tokens.h"

#define MAX_ITEMS       100
#define MAX_STATES      100
#define MAX_RULES       100
#define MAX_TOKEN_LEN   32
#define MAX_SYMBOLS     50


typedef struct {
    char nonterminal[MAX_TOKEN_LEN];
    char ruleContent[256];
} Rule;

typedef struct {
    Rule* rule;
    int dot;
} LRItem;

typedef struct {
    LRItem items[MAX_ITEMS];
    int numItems;
} State;

char* actionTable[MAX_STATES][MAX_SYMBOLS];

int gotoTable[MAX_STATES][MAX_SYMBOLS];

int assosiation_array[NUM_OF_TOKENS];




int create_parser_tables();

#endif