#ifndef PARSER_TABLE_GENERATOR_H
#define PARSER_TABLE_GENERATOR_H

#include "Tokens.h"
#include "ArrayList.h"

#define DEFAULT_NUMBER_OF_STATES 100
#define DEFAULT_NUMBER_OF_RULES 100
#define DEFAULT_AMOUNT_OF_LR_ITEMS 32
#define DEFAULT_AMOUNT_OF_NONTERMINALS 100
#define DEFAULT_AMOUNT_OF_TERMINALS 100

typedef struct {
    char* nonterminal;
    char* ruleContent;
    int ruleTerminalCount;
    int ruleID;
} Rule;

typedef struct {
    Rule* rule;
    int dot;
} LRItem;

typedef struct {
    ArrayList* items; // CHAGNE TO HASHSET?
} State;

char*** actionTable;
int** gotoTable;

int associationArray[NUM_OF_TOKENS];
ArrayList* rules;

int create_parser_tables();

#endif
