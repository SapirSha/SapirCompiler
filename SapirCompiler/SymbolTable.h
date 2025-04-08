#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "HashMap.h"
#include "LinkedList.h"
#include "Sementic.h"


#define DEFUALT_IDENTIFIERS_PER_SCOPE 10

typedef enum {
	VARIABLE,
	FUNCTION
} IdentifierType;

typedef struct {
	char* identifier_name;
	Data_Type data_type;
} VariableInfo;

typedef struct {
	int num_of_params;
	VariableInfo* params;
} FunctionInfo;

typedef struct {
	IdentifierType identifier_type;
	char* identifier_name;
	char* identifier_new_name;
	Data_Type data_type;
	void* info;
} IdentifiersInfo;

typedef struct {
	HashMap* identifiers;
} Scope;

typedef struct {
	LinkedList* scopes;
}SymbolTable;

SymbolTable* symbol_table_init();
void symbol_table_add_scope(SymbolTable* table);
void symbol_table_remove_scope(SymbolTable* table);
bool symbol_table_add_symbol(SymbolTable* table, IdentifiersInfo* info);
IdentifiersInfo* symbol_table_lookup_symbol(SymbolTable* table, const char** name);


#endif