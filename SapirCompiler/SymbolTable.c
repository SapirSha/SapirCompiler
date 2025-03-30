#include "SymbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
void symbol_table_add_symbol(SymbolTable* table, IdentifiersInfo* info) {
	Scope* scope = (Scope*)linkedlist_peek(table->scopes);
	hashmap_insert(scope->identifiers, info->identifier_name, info);
}

// LOOKUP SYMBOL
IdentifiersInfo* symbol_table_lookup_symbol(SymbolTable* table, const char* name) {
	LinkedListNode* pointer = table->scopes->head;
	while (pointer != NULL) {
		if (hashmap_get(((Scope*)pointer->value)->identifiers, name) != NULL)
			return hashmap_get(((Scope*)pointer->value)->identifiers, name);

		pointer = pointer->next;
	}
	return NULL;
}