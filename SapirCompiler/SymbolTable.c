#include "SymbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma warning(disable:4996)

int names_id = 1;



// ADD SCOPE
Scope* scope_init() {
	Scope* scope = malloc(sizeof(Scope));
	scope->identifiers = createHashMap(DEFUALT_IDENTIFIERS_PER_SCOPE, string_hash, string_equals);
	return scope;
}

void symbol_table_add_scope(SymbolTable* table) {
	linkedlist_push(table->scopes, scope_init());
}

// INIT
SymbolTable* symbol_table_init() {
	SymbolTable* st = malloc(sizeof(SymbolTable));
	st->scopes = linkedlist_init(sizeof(Scope));
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

	char* code = malloc(8);
	snprintf(code, strlen(code), "%d", names_id++);
	info->identifier_new_name = malloc(strlen(info->identifier_name) + strlen(code));
	strcpy(info->identifier_new_name, info->identifier_name);
	info->identifier_new_name = strcat(code, info->identifier_new_name);


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