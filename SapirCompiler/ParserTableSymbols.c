#include "ParserTableGenerator.h"
#include "HashSet.h"
#include "Parser.h"

ArrayList* nonterminal_list;
ArrayList* terminalsList;
HashMap* symbols_indexes;

static void symbols_init() {
    nonterminal_list = arraylist_init(sizeof(char*), DEFAULT_AMOUNT_OF_NONTERMINALS);
    terminalsList = arraylist_init(sizeof(char*), DEFAULT_AMOUNT_OF_TERMINALS);
    symbols_indexes = createHashMap(DEFAULT_AMOUNT_OF_TERMINALS * 2, string_hash, string_equals);
}

static inline void add_to_nonterminals_list(char* symbol) {
    char* dup = strdup(symbol);
    int* index = new_int_with_allocation(nonterminal_list->size);
    arraylist_add(nonterminal_list, &dup);
    hashmap_insert(symbols_indexes, dup, index);
}

static inline void add_to_terminals_list(char* symbol) {
    char* dup = strdup(symbol);
    int* index = new_int_with_allocation(terminalsList->size);
    arraylist_add(terminalsList, &dup);
    hashmap_insert(symbols_indexes, dup, index);
}

static void add_symbol(char* symbol) {
    if (hashmap_get(symbols_indexes, symbol) == NULL) {
        if (isNonterminal(symbol)) {
            add_to_nonterminals_list(symbol);
        }
        else {
            add_to_terminals_list(symbol);
        }
    }
}

static void add_symbols_in_content(char* rule_content) {
    char* copy = strdup(rule_content);
    char* symbol = strtok(copy, " ");
    while (symbol) {
        add_symbol(symbol);
        symbol = strtok(NULL, " ");
    }
    free(copy);
}

static inline void add_rule_content_to_symbol_lists(Rule* rule) {
    add_symbol(rule->nonterminal);
    add_symbols_in_content(rule->ruleContent);
}

/*
A function that collects all the possible symbols and splits them into 2 arraylists:
* a nonterminal arraylist
* a terminal arraylist
*/
void collect_symbols() {
    symbols_init();

    for (int i = 0; i < rules->size; i++) {
        Rule* r = (Rule*)rules->array[i];
        add_rule_content_to_symbol_lists(r);
    }
}

static int inline get_index_of_symbol(char* sym) {
    int* result = hashmap_get(symbols_indexes, sym);
    if (result) return *result;
    else return -1;
}

int get_terminal_index(const char* sym) {
    if (!isNonterminal(sym))
        return get_index_of_symbol(sym);
    else return -1;
}

int get_nonterminal_index(const char* sym) {
    if (isNonterminal(sym))
        return get_index_of_symbol(sym);
    else return -1;
}

void free_non_and_terminals() {
    for (int i = 0; i < terminalsList->size; i++) {
        char* symbol = *(char**)terminalsList->array[i];
        free(hashmap_get(symbols_indexes, symbol));
        free(symbol);
    }
    arraylist_free(terminalsList);

    for (int i = 0; i < nonterminal_list->size; i++) {
        char* symbol = *(char**)nonterminal_list->array[i];
        free(hashmap_get(symbols_indexes, symbol));
        free(symbol);
    }
    arraylist_free(nonterminal_list);

    freeHashMap(symbols_indexes);
}


