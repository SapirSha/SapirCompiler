#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "HashMap.h"
#include "LinkedList.h"
#include "Sementic.h"


#define DEFUALT_IDENTIFIERS_PER_SCOPE 10

typedef enum {
	NONE = 0,
	INT,
	STRING,
	BOOL,
	UNKNOWN,
} Data_Type;

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
	char* name;
	char* origin_name;
	Data_Type type;
	int size;
	int alignment;

	bool local;
	int offset;
} SymbolInfo;

typedef struct {
	int id;
	int size;
	int alignment;
	int offset;
	int local;
} TempSymbolInfo;

typedef struct {
	int id;
	int size;
	Token tk;

	int local;
} StringInfo;

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
	HashMap* SymbolMap;

	HashMap* TemporaryVarMap;
	int temporary_vars_offset;

	HashMap* GlobalStrings;
}SymbolTable;

int align_size(int size);
SymbolTable* symbol_table_init();
void symbol_table_add_scope(SymbolTable* table);
void symbol_table_remove_scope(SymbolTable* table);
bool symbol_table_add_symbol(SymbolTable* table, IdentifiersInfo* info);
IdentifiersInfo* symbol_table_lookup_symbol(SymbolTable* table, char** name);
void print_all_symbols(SymbolTable* table);
SymbolTable* symbol_table;


#endif