#ifndef PARSER_TABLE_GENERATOR_H
#define PARSER_TABLE_GENERATOR_H

#pragma warning(disable:4996)

#include "Tokens.h"
#include "ArrayList.h"
#include "stdbool.h"
#include "HashMap.h"

#define DEFAULT_NUMBER_OF_STATES 100
#define DEFAULT_NUMBER_OF_RULES 100
#define DEFAULT_AMOUNT_OF_LR_ITEMS 32
#define DEFAULT_AMOUNT_OF_NONTERMINALS 100
#define DEFAULT_AMOUNT_OF_TERMINALS 100

typedef struct {
    char* nonterminal;
    char* ruleContent;
    int ruleTerminalCount;
    int nonterminal_position;
    int ruleID;
} Rule;

typedef struct {
    Rule* rule;
    int dot;
} LRItem;

typedef struct {
    ArrayList* items;
} Parser_State;

typedef enum {
    ERROR_ACTION,
    SHIFT_ACTION,
    REDUCE_ACTION,
    ACCEPT_ACTION,
    GOTO_ACTION,
} ACTION_TYPE;

typedef struct {
    ACTION_TYPE type;
    int value;
} ActionCell;

extern ActionCell** actionTable;
extern int** gotoTable;

extern ArrayList* nonterminalsList;
extern ArrayList* terminalsList;

extern int associationArray[NUM_OF_TOKENS];
extern ArrayList* rules;

extern ArrayList* states;
extern HashMap* follow;

int parser_tables_initialized;

int create_parser_tables();

bool isNonterminal(const char* symbol);

void free_parser_table();

void free_non_and_terminals();
void free_rules();
void free_follow();

void add_rule(const char* nonterminal, const char* content);
void add_rules();
int count_symbols(const char* ruleContent);
char* get_next_symbol(LRItem* item);
char* get_nth_token(char* s, int n);
void build_states(const char* startNonterminal);
Parser_State* goto_state(Parser_State* s, char* symbol);
void collect_symbols();
void free_states();
void set_nonterminals_position();
int find_row_of_nonterminal_in_table(const char* nonterminal);
int find_column_of_terminal_in_table(const char* terminal);
int find_state(Parser_State* s);
void** create_matrix(int rows, int cols, int object_size);
int get_terminal_index(const char* sym);
int get_nonterminal_index(const char* sym);
char* actiontypetostring(int action);
#endif