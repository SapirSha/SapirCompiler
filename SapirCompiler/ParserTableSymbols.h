#ifndef PARSER_TABLE_SYMBOLS_H
#define PARSER_TABLE_SYMBOLS_H

#include <stdbool.h>
#include "HashMap.h"
#include "ArrayList.h"

enum SymbolType {
	NONTERMINAL_SYMBOL,
	TERMINAL_SYMBOL
};


typedef struct {
	ArrayList* nonterminal_list;
	ArrayList* terminals_list;
	HashMap* symbols_indexes;
} ParserSymbols;


ParserSymbols* init_symbols(int initial_size);
enum SymbolType get_symbol_type(char* symb);
void add_symbol(ParserSymbols* symbols, char* symb);
bool symbol_exists(ParserSymbols* symbols, char* symb);
void* set_index_of_terminal(ParserSymbols* symbols, char* symb, int index);
int get_index_of_symbol(ParserSymbols* symbols, char* symb);
char* get_symbol_of_index_terminal(ParserSymbols* symbols, int index);
char* get_symbol_of_index_nonterminal(ParserSymbols* symbols, int index);


#endif