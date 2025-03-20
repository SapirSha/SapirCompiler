#include "ParserTableSymbols.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#pragma warning(disable:4996)

#define MINIMUM_SIZE 10

ParserSymbols* init_symbols(int initial_size) {
	ParserSymbols* symbols = malloc(sizeof(ParserSymbols));
	symbols->nonterminal_list = arraylist_init(initial_size/2, sizeof(char*));
	symbols->terminals_list = arraylist_init(initial_size / 2, sizeof(char*));
	symbols->symbols_indexes = createHashMap(initial_size * 2, string_hash, string_equals);
	return symbols;
}

enum SymbolType get_symbol_type(char* symb) {
	return isupper(symb[0]) ? NONTERMINAL_SYMBOL : TERMINAL_SYMBOL;
}
static void add_symbol_nonterminal(ParserSymbols* symbols, char* symb) {
	int* pos = malloc(sizeof(int));
	*pos = symbols->nonterminal_list->size;
	arraylist_add(symbols->nonterminal_list, &symb);
	hashmap_insert(symbols->symbols_indexes, strdup(symb), pos);
}

static void add_symbol_terminal(ParserSymbols* symbols, char* symb) {
	int* pos = malloc(sizeof(int));
	*pos = symbols->terminals_list->size;
	arraylist_add(symbols->terminals_list, &symb);
	hashmap_insert(symbols->symbols_indexes, strdup(symb), pos);
}

void add_symbol(ParserSymbols* symbols, char* symb) {
	if (get_symbol_type(symb) == NONTERMINAL_SYMBOL)
		add_symbol_nonterminal(symbols, symb);
	else if (get_symbol_type(symb) == TERMINAL_SYMBOL)
		add_symbol_terminal(symbols, symb);
}

bool symbol_exists(ParserSymbols* symbols, char* symb) {
	return hashmap_get(symbols->symbols_indexes, symb) != NULL;
}

void* set_index_of_terminal(ParserSymbols* symbols, char* symb, int index) {
	if (get_symbol_type(symb) == NONTERMINAL_SYMBOL) {
		printf("NONTERMINAL!");
		exit(-1);
	}

	void* temp = arraylist_set(symbols->terminals_list, &symb, index);
	if (temp != NULL)
		hashmap_insert(symbols->symbols_indexes, *(char**)temp, NULL);
	int* p = malloc(sizeof(int));
	*p = index;
	hashmap_insert(symbols->symbols_indexes, strdup(symb), p);
	return temp;
}

int get_index_of_symbol(ParserSymbols* symbols, char* symb) {
	return *(int*)hashmap_get(symbols->symbols_indexes, symb);
}

char* get_symbol_of_index_terminal(ParserSymbols* symbols, int index) {
	return arraylist_get(symbols->terminals_list, index);
}

char* get_symbol_of_index_nonterminal(ParserSymbols* symbols, int index) {
	return arraylist_get(symbols->nonterminal_list, index);
}