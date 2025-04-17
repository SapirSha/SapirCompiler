#include "SymbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ErrorHandler.h"

#pragma warning(disable:4996)

int names_id = 1;

int type_to_size(Data_Type type){
	switch (type)
	{
	case INT:
		return 2;
	case STRING:
		return 2;
	case BOOL:
		return 1;
	default:
		break;
	}
}

int align_size(int size) {
	return size >= 2 ? 2 : 1;
}


SymbolInfo* create_new_symbol_info(char* name, Data_Type type, char* og_name) {
	SymbolInfo* info = malloc(sizeof(SymbolInfo));
	if(!info) handle_out_of_memory_error();

	info->name = name;
	info->type = type;
	info->size = type_to_size(type);
	info->alignment = align_size(info->size);
	info->origin_name = strdup(og_name);

	return info;
}

// ADD SCOPE
Scope* scope_init() {
	Scope* scope = malloc(sizeof(Scope));
	if (!scope) handle_out_of_memory_error();
	
	scope->identifiers = createHashMap(DEFUALT_IDENTIFIERS_PER_SCOPE, string_hash, string_equals);
	return scope;
}

void symbol_table_add_scope(SymbolTable* table) {
	linkedlist_push(table->scopes, scope_init());
}

// INIT
SymbolTable* symbol_table_init() {
	SymbolTable* st = malloc(sizeof(SymbolTable));
	if (!st) handle_out_of_memory_error();

	st->scopes = linkedlist_init(sizeof(Scope));
	st->SymbolMap = createHashMap(100, string_hash, string_equals);
	st->TemporaryVarMap = createHashMap(100, int_hash, int_equals);
	st->temporary_vars_offset = 0;
	st->GlobalStrings = createHashMap(100, string_hash, string_equals);
	symbol_table_add_scope(st);
	return st;
}

// REMOVE SCOPE
void symbol_table_remove_scope(SymbolTable* table) {
	Scope* scope = (Scope*)linkedlist_pop(table->scopes);
	freeHashMap(scope->identifiers);
	free(scope);
}

// ADD SYMBOL
bool symbol_table_add_symbol(SymbolTable* table, IdentifiersInfo* info) {
	Scope* scope = (Scope*)linkedlist_peek(table->scopes);
	if (hashmap_get(scope->identifiers, info->identifier_name) != NULL)
		return false;

	hashmap_insert(scope->identifiers, info->identifier_name, info);
	
	static char code[8];
	snprintf(code, 8, "%d", names_id++);

	int size = strlen(info->identifier_name) + strlen(code) + 1;
	info->identifier_new_name = malloc(size);
	if (!info->identifier_new_name) handle_out_of_memory_error();

	snprintf(info->identifier_new_name, size, "%s%s", code, info->identifier_name);

	hashmap_insert(table->SymbolMap, info->identifier_new_name,
		create_new_symbol_info(info->identifier_new_name, info->data_type, info->identifier_name));

	return true;
}

// LOOKUP SYMBOL
IdentifiersInfo* symbol_table_lookup_symbol(SymbolTable* table, const char** name) {
	LinkedListNode* pointer = table->scopes->head;
	while (pointer != NULL) {
		if (hashmap_get(((Scope*)pointer->value)->identifiers, *name) != NULL) {
			IdentifiersInfo* info = hashmap_get(((Scope*)pointer->value)->identifiers, *name);
			*name = info->identifier_new_name;
			
			return info;
		}

		pointer = pointer->next;
	}
	return NULL;
}


void print_symbolinfo(SymbolInfo* info) {
	printf("Variable: name %s", info->name);
	printf("- size %d ", info->size);
	printf("- offset %d ", info->offset);
	printf("- align %d ", info->alignment);
	printf("- local %d \n", info->local);
}

void print_symbolinfotemp(TempSymbolInfo* info) {
	printf("Temp: id %d ", info->id);
	printf("- size %d ", info->size);
	printf("- offset %d ", info->offset);
	printf("- align %d ", info->alignment);
	printf("- local %d \n", info->local);

}

void print_all_symbols(SymbolTable* table) {
	printf("START OF SYMBOLS\n");


	HashMapNode* cur;
	for (int i = 0; i < table->SymbolMap->capacity; i++) {
		cur = table->SymbolMap->buckets[i];
		while (cur) {
			print_symbolinfo(cur->value);
			cur = cur->next;
		}
	}

	for (int i = 0; i < table->TemporaryVarMap->capacity; i++) {
		cur = table->TemporaryVarMap->buckets[i];
		while (cur) {
			print_symbolinfotemp(cur->value);
			cur = cur->next;
		}
	}
	printf("END OF SYMBOLS\n");
}