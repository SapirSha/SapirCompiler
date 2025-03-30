#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SyntaxTree.h"
#include "Tokens.h"
#include "SymbolTable.h"
#include "Sementic.h"

#define NONTERMINAL_COUNT_DEFUALT 100


#pragma warning(disable:4996)
SymbolTable* symbol_table = NULL;
HashMap* visitor = NULL;


static void handle_error(char* msg) {
	printf("%s\n", msg);
	exit(-1);
}

static bool compatible(Data_Type left, Data_Type right) {
	if (left == right) {
		return true;
	}
	else if (left == FLOAT && right == INT || right == FLOAT && left == INT) {
		return true;
	}
	else {
		return false;
	}
}


Data_Type get_type(SyntaxTree* tree) {
	if (tree->type == TERMINAL_TYPE) {
		switch (tree->info.terminal_info.token.type) {
		case TOKEN_INT:
		case TOKEN_NUMBER:
			return INT;
		case TOKEN_STRING:
		case TOKEN_STRING_LITERAL:
			return STRING;
		case TOKEN_BOOL:
		case TOKEN_TRUE:
		case TOKEN_FALSE:
			return BOOL;
		case TOKEN_FLOAT:
		case TOKEN_FLOAT_NUMBER:
			return FLOAT;
		case TOKEN_IDENTIFIER:
			IdentifiersInfo* info =  symbol_table_lookup_symbol(symbol_table, tree->info.terminal_info.token.lexeme);
			if (info == NULL) {
				handle_error("IDENTIFER NOT DEFINED");
			}
			else {
				return info->data_type;
			}
		default:
			return -1;
		}
	}
	else {
		return -1;
	}
}

static Data_Type program(SyntaxTree* tree) {
	for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
		accept(tree->info.nonterminal_info.children[i]);
	}
}

static Data_Type statements(SyntaxTree* tree) {
	for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
		accept(tree->info.nonterminal_info.children[i]);
	}
}

static Data_Type decl_with_asign(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	Data_Type left = get_type(tree->info.nonterminal_info.children[0]);

	info->data_type = left;
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;

	symbol_table_add_symbol(symbol_table, info);

	Data_Type right = accept(tree->info.nonterminal_info.children[3]);


	if (!compatible(left, right)) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

}

static Data_Type expression(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	if (!compatible(left, right)) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
	return left;
}

static Data_Type term(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	if (!compatible(left, right)) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
	return left;
}

static Data_Type factor(SyntaxTree* tree) {
	// ( EXPRESSION )
	return accept(tree->info.nonterminal_info.children[1]);
}

static Data_Type assign(SyntaxTree* tree) {
	IdentifiersInfo* info = symbol_table_lookup_symbol(symbol_table, tree->info.nonterminal_info.children[0]->info.terminal_info.token.lexeme);
	if (info == NULL) {
		handle_error("IDENTIFER NOT DEFINED");
	}
	else {
		Data_Type left = info->data_type;
		Data_Type right = accept(tree->info.nonterminal_info.children[2]);
		if (!compatible(left, right)) {
			handle_error("ASSIGNMENT TYPE MISMATCH");
		}
	}
}

static Data_Type decl(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	Data_Type left = get_type(tree->info.nonterminal_info.children[0]);
	info->data_type = left;
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;
	symbol_table_add_symbol(symbol_table, info);
}



static Data_Type condition(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	if (!compatible(left, right)) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	return BOOL;
}

static Data_Type condition_list(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	if (left != BOOL || right != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	return BOOL;
}

static Data_Type if_statement(SyntaxTree* tree) {
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	accept(tree->info.nonterminal_info.children[2]);
}
static Data_Type if_else_statement(SyntaxTree* tree) {
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	accept(tree->info.nonterminal_info.children[2]);
	accept(tree->info.nonterminal_info.children[4]);
}
static Data_Type while_statement(SyntaxTree* tree) {
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
	accept(tree->info.nonterminal_info.children[2]);
}

static Data_Type do_while_statement(SyntaxTree* tree) {
	accept(tree->info.nonterminal_info.children[1]);

	Data_Type condition = accept(tree->info.nonterminal_info.children[3]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
}



static Data_Type block(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[1]);
	symbol_table_remove_scope(symbol_table);
}


Data_Type accept(SyntaxTree* tree) {
	if (tree->type == TERMINAL_TYPE) {
		return get_type(tree);
	}
	else {
		Data_Type(*pointer)(SyntaxTree*) = hashmap_get(visitor, tree->info.nonterminal_info.nonterminal);
		if (pointer == NULL) {
			printf("%s\n\n", tree->info.nonterminal_info.nonterminal);
			handle_error("NO FUNCTION FOR NONTERMINAL");
		}
		else {
			return pointer(tree);
		}
	}
}

void init_visitor() {
	hashmap_insert(visitor, "PROGRAM", &program);
	hashmap_insert(visitor, "STATEMENTS", &statements);
	hashmap_insert(visitor, "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", &decl_with_asign);
	hashmap_insert(visitor, "EXPRESSION", &expression);
	hashmap_insert(visitor, "TERM", &term);
	hashmap_insert(visitor, "FACTOR", &factor);
	hashmap_insert(visitor, "VARIABLE_ASSIGNMENT_STATEMENT", &assign);
	hashmap_insert(visitor, "VARIABLE_DECLARATION_STATEMENT", &decl);
	hashmap_insert(visitor, "CONDITION", &condition);
	hashmap_insert(visitor, "CONDITION_LIST", &condition_list);
	hashmap_insert(visitor, "IF_STATEMENT", &if_statement);
	hashmap_insert(visitor, "BLOCK", &block);
	hashmap_insert(visitor, "IF_ELSE_STATEMENT", &if_else_statement);
	hashmap_insert(visitor, "WHILE_STATEMENT", &while_statement);
	hashmap_insert(visitor, "DO_WHILE_STATEMENT", &do_while_statement);

}



int sementic_analysis(SyntaxTree* tree) {
	printf("\n\n\n SEMENTICS: \n");
	visitor = createHashMap(NONTERMINAL_COUNT_DEFUALT, string_hash, string_equals);
	init_visitor();

	symbol_table = symbol_table_init();

	accept(tree);


	printf("SEMENTICS END: \n");

    return 0;
}
