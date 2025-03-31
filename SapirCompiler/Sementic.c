#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SyntaxTree.h"
#include "Tokens.h"
#include "SymbolTable.h"
#include "Sementic.h"

#define NONTERMINAL_COUNT_DEFUALT 100

#define CURRENT_FUNCTION_SYMBOL "function"

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
	else if ((left == FLOAT && right == INT) || (right == FLOAT && left == INT)) {
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
			return NONE;
		}
	}
	else {
		return NONE;
	}
}

static Data_Type program(SyntaxTree* tree) {
	for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
		accept(tree->info.nonterminal_info.children[i]);
	}
	return NONE;
}

static Data_Type statements(SyntaxTree* tree) {
	for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++) {
		accept(tree->info.nonterminal_info.children[i]);
	}
	return NONE;

}

static Data_Type decl_with_asign(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	Data_Type left = get_type(tree->info.nonterminal_info.children[0]);

	info->data_type = left;
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;

	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}

	Data_Type right = accept(tree->info.nonterminal_info.children[3]);


	if (!compatible(left, right)) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	return NONE;
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
	return NONE;
}

static Data_Type decl(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	Data_Type left = get_type(tree->info.nonterminal_info.children[0]);
	info->data_type = left;
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}
	return NONE;
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
	symbol_table_add_scope(symbol_table);
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	accept(tree->info.nonterminal_info.children[2]);

	symbol_table_remove_scope(symbol_table);
	return NONE;
}
static Data_Type if_else_statement(SyntaxTree* tree) {

	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[2]);
	symbol_table_remove_scope(symbol_table);

	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);

	return NONE;
}
static Data_Type while_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
	accept(tree->info.nonterminal_info.children[2]);
	symbol_table_remove_scope(symbol_table);
	return NONE;
}

static Data_Type do_while_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[1]);

	Data_Type condition = accept(tree->info.nonterminal_info.children[3]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}
	symbol_table_remove_scope(symbol_table);
	return NONE;
}

static Data_Type if_block(SyntaxTree* tree) {
	accept(tree->info.nonterminal_info.children[1]);
	return NONE;
}
static Data_Type while_block(SyntaxTree* tree) {
	accept(tree->info.nonterminal_info.children[1]);
	return NONE;
}

static Data_Type for_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	Data_Type variable = accept(tree->info.nonterminal_info.children[1]);
	Data_Type condition = accept(tree->info.nonterminal_info.children[3]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);
	return NONE;
}

static Data_Type for_change_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);

	Data_Type variable = accept(tree->info.nonterminal_info.children[1]);
	Data_Type condition = accept(tree->info.nonterminal_info.children[3]);
	if (condition != BOOL) {
		handle_error("ASSIGNMENT TYPE MISMATCH");
	}

	accept(tree->info.nonterminal_info.children[4]);

	accept(tree->info.nonterminal_info.children[6]);

	symbol_table_remove_scope(symbol_table);
	return NONE;
}

static Data_Type parameter_list(SyntaxTree* tree) {
	accept(tree->info.nonterminal_info.children[0]);
	accept(tree->info.nonterminal_info.children[2]);
	return NONE;
}

static Data_Type parameter(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	info->data_type = get_type(tree->info.nonterminal_info.children[0]);
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}



	FunctionInfo* cur_info = ((FunctionInfo*)symbol_table_lookup_symbol(symbol_table, CURRENT_FUNCTION_SYMBOL)->info);
	int cur_number_of_params = cur_info->num_of_params;
	cur_info->num_of_params++;
	cur_info->params = realloc(cur_info->params, cur_info->num_of_params * sizeof(VariableInfo));
	cur_info->params[cur_number_of_params].data_type = info->data_type;
	cur_info->params[cur_number_of_params].identifier_name = info->identifier_name;
	return NONE;
}

static Data_Type function_decl(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	info->data_type = get_type(tree->info.nonterminal_info.children[5]);
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}

	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	*helper_info = *info;
	helper_info->identifier_name = CURRENT_FUNCTION_SYMBOL;
	helper_info->info = malloc(sizeof(FunctionInfo));
	((FunctionInfo*)helper_info->info)->num_of_params = 0;
	((FunctionInfo*)helper_info->info)->params = NULL;
	symbol_table_add_symbol(symbol_table, helper_info);

	accept(tree->info.nonterminal_info.children[3]);
	info->info = helper_info->info;

	accept(tree->info.nonterminal_info.children[6]);
	symbol_table_remove_scope(symbol_table);

	free(helper_info);

	return NONE;
}

static Data_Type function_decl_returns_nothing(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	info->data_type = NONE;
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}

	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	*helper_info = *info;
	helper_info->identifier_name = CURRENT_FUNCTION_SYMBOL;
	helper_info->info = malloc(sizeof(FunctionInfo));
	((FunctionInfo*)helper_info->info)->num_of_params = 0;
	((FunctionInfo*)helper_info->info)->params = NULL;
	symbol_table_add_symbol(symbol_table, helper_info);

	accept(tree->info.nonterminal_info.children[3]);
	info->info = helper_info->info;

	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);

	free(helper_info);

	return NONE;
}


static Data_Type function_decl_gets_nothing(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	info->data_type = get_type(tree->info.nonterminal_info.children[3]);
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}

	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	*helper_info = *info;
	helper_info->identifier_name = CURRENT_FUNCTION_SYMBOL;
	helper_info->info = malloc(sizeof(FunctionInfo));
	((FunctionInfo*)helper_info->info)->num_of_params = 0;
	((FunctionInfo*)helper_info->info)->params = NULL;
	symbol_table_add_symbol(symbol_table, helper_info);

	info->info = helper_info->info;

	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);

	free(helper_info);

	return NONE;
}

static Data_Type function_decl_gets_returns_nothing(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	info->data_type = get_type(tree->info.nonterminal_info.children[3]);
	info->identifier_name = tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_error("IDENTIFER ALREADY DEFINED");
	}

	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	*helper_info = *info;
	helper_info->identifier_name = CURRENT_FUNCTION_SYMBOL;
	helper_info->info = malloc(sizeof(FunctionInfo));
	((FunctionInfo*)helper_info->info)->num_of_params = 0;
	((FunctionInfo*)helper_info->info)->params = NULL;
	symbol_table_add_symbol(symbol_table, helper_info);

	info->info = helper_info->info;

	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);

	free(helper_info);

	return NONE;
}

static Data_Type return_statement(SyntaxTree* tree) {
	Data_Type info = accept(tree->info.nonterminal_info.children[1]);
	Data_Type supposed = symbol_table_lookup_symbol(symbol_table, CURRENT_FUNCTION_SYMBOL)->data_type;
	if (!compatible(info, supposed)) {
		handle_error("RETURN TYPE MISMATCH");
	}
	return NONE;
}
static Data_Type return_none_statement(SyntaxTree* tree) {
	Data_Type supposed = symbol_table_lookup_symbol(symbol_table, CURRENT_FUNCTION_SYMBOL)->data_type;
	if (supposed != NONE) {
		handle_error("RETURN TYPE MISMATCH");
	}

	return NONE;
}

static Data_Type function_call(SyntaxTree* tree) {
	IdentifiersInfo* funcSymbol = symbol_table_lookup_symbol(symbol_table,
		tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme);
	if (funcSymbol == NULL) {
		handle_error("FUNCTION NOT DEFINED");
	}

	if (funcSymbol->identifier_type != FUNCTION) {
		handle_error("IDENTIFIER IS NOT A FUNCTION");
	}

	FunctionInfo* funcInfo = (FunctionInfo*)funcSymbol->info;

	SyntaxTree* argListNode = tree->info.nonterminal_info.children[3];
	int argCount = 0;
	Data_Type argTypes[100];

	LinkedList* argList = linkedlist_init(sizeof(Data_Type));
	Data_Type type;
	while (argListNode->type == NONTERMINAL_TYPE) {
		type = accept(argListNode->info.nonterminal_info.children[2]);
		linkedlist_push(argList, &type);
		argListNode = argListNode->info.nonterminal_info.children[0];
	}
	type = accept(argListNode);
	linkedlist_push(argList, &type);

	while (linkedlist_count(argList) > 0) {
		argTypes[argCount++] = *(Data_Type*)linkedlist_pop(argList);
	}
	if (argCount != funcInfo->num_of_params) {
		handle_error("ARGUMENT COUNT MISMATCH");
	}

	for (int i = 0; i < argCount; i++) {
		if (!compatible(argTypes[i], funcInfo->params[i].data_type)) {
			handle_error("ARGUMENT TYPE MISMATCH");
		}
	}

	return funcSymbol->data_type;
}

static Data_Type function_call_with_nothing(SyntaxTree* tree) {
	IdentifiersInfo* funcSymbol = symbol_table_lookup_symbol(symbol_table,
		tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme);
	if (funcSymbol == NULL) {
		handle_error("FUNCTION NOT DEFINED");
	}

	if (funcSymbol->identifier_type != FUNCTION) {
		handle_error("IDENTIFIER IS NOT A FUNCTION");
	}

	FunctionInfo* funcInfo = (FunctionInfo*)funcSymbol->info;

	if (funcInfo->num_of_params != 0) {
		handle_error("CALL TYPE MISMATCH");
	}

	return funcSymbol->data_type;
}

static Data_Type arg_list(SyntaxTree* tree) {
	return accept(tree->info.nonterminal_info.children[2]);
}


static Data_Type for_block(SyntaxTree* tree) {
	accept(tree->info.nonterminal_info.children[1]);
	return NONE;
}

static Data_Type func_block(SyntaxTree* tree) {
	accept(tree->info.nonterminal_info.children[1]);
	return NONE;
}


static Data_Type block(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[1]);
	symbol_table_remove_scope(symbol_table);
	return NONE;
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
	hashmap_insert(visitor, "FOR_BLOCK", &for_block);
	hashmap_insert(visitor, "FOR_STATEMENT", &for_statement);
	hashmap_insert(visitor, "FOR_CHANGE_STATEMENT", &for_change_statement);
	hashmap_insert(visitor, "FUNCTION_DECLARATION_STATEMENT", &function_decl);
	hashmap_insert(visitor, "FUNCTION_BLOCK", &func_block);
	hashmap_insert(visitor, "PARAMETER_LIST", &parameter_list);
	hashmap_insert(visitor, "PARAMETER", &parameter);
	hashmap_insert(visitor, "FUNCTION_DECLARATION_NO_RETURN_STATEMENT", &function_decl_returns_nothing);
	hashmap_insert(visitor, "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT", &function_decl_gets_nothing);
	hashmap_insert(visitor, "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT", &function_decl_gets_returns_nothing);
	hashmap_insert(visitor, "RETURN_STATEMENT", &return_statement);
	hashmap_insert(visitor, "FUNCTION_CALL_WITH_NOTHING_STATEMENT", &function_call_with_nothing);
	hashmap_insert(visitor, "FUNCTION_CALL_STATEMENT", &function_call);
	hashmap_insert(visitor, "ARGUMENT_LIST", &arg_list);
	hashmap_insert(visitor, "IF_BLOCK", &if_block);
	hashmap_insert(visitor, "WHILE_BLOCK", &while_block);


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
