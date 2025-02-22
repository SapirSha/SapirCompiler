#ifndef PARSERTABLEGENERATOR_H
#define PARSERTABLEGENERATOR_H

#include "Tokens.h"
#include "ArrayList.h"

#define MAX_STATES      100
#define MAX_SYMBOLS     50

#define DEFAULT_NUMBER_OF_STATES 100
#define DEFAULT_NUMBER_OF_RULES 100
#define DEFUALT_AMOUNT_OF_LR_ITEMS 32


typedef struct {
    char* nonterminal;
    char* ruleContent;
    int rule_terminal_count;
    int rule_id;
} Rule;

typedef struct {
    Rule* rule;
    int dot;
} LRItem;

typedef struct {
    ArrayList* items;
} State;

char* actionTable[MAX_STATES][MAX_SYMBOLS];

int gotoTable[MAX_STATES][MAX_SYMBOLS];

int assosiation_array[NUM_OF_TOKENS];

ArrayList* rules;

int create_parser_tables();

#endif
