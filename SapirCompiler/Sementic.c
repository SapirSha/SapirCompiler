#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SyntaxTree.h"
#include "Tokens.h"
#include "SymbolTable.h"
#include "Sementic.h"
#include "ErrorHandler.h"

#define NONTERMINAL_COUNT_DEFUALT 100

#pragma warning(disable:4996)
int string_ids = 0;

#pragma warning(disable:4996)
HashMap* ir_visitor = NULL;

Data_Type accept(SyntaxTree* tree);

static bool compatible(Data_Type left, Data_Type right) {
	return left == right || left == UNKNOWN || right == UNKNOWN;
}

static Data_Type TOKEN_TO_DATA_TYPE[NUM_OF_TOKENS] = {
	[TOKEN_INT] = INT, [TOKEN_NUMBER] = INT,
	[TOKEN_STRING_LITERAL] = STRING,
	[TOKEN_BOOL] = BOOL, 
	[TOKEN_FALSE] = BOOL, [TOKEN_TRUE] = BOOL,
};

Data_Type get_type(SyntaxTree* tree) {
	if (tree->type != TERMINAL_TYPE) return NONE;

	Token_Types type = tree->info.terminal_info.token.type;
	if (type == TOKEN_IDENTIFIER) {
		IdentifiersInfo* info = symbol_table_lookup_symbol(symbol_table, &tree->info.terminal_info.token.lexeme);
		if (info == NULL) {
			handle_sementic_error_identifier_not_defined(tree->info.terminal_info.token);
			return UNKNOWN;
		}
		else {
			return info->data_type;
		}
	}
	else {
		if (type >= NUM_OF_TOKENS || type < 0) return NONE;
		Data_Type result = TOKEN_TO_DATA_TYPE[type];
		return result;
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

static Data_Type handle_incompatability(SyntaxTree* tree, Data_Type left, Data_Type right) {
	if (!compatible(left, right)) {
		SyntaxTree* pos = tree;
		while (pos->type == NONTERMINAL_TYPE)
			pos = pos->info.nonterminal_info.children[0];

		Token id = pos->info.terminal_info.token;
		handle_sementic_error(id, left, right);
		return UNKNOWN;
	}

	return left;
}

static Data_Type condition_must_be_bool(SyntaxTree* tree, Data_Type type) {
	if (type != BOOL && type != UNKNOWN) {
		SyntaxTree* pos = tree;
		while (pos->type == NONTERMINAL_TYPE)
			pos = pos->info.nonterminal_info.children[0];

		Token id = pos->info.terminal_info.token;
		handle_sementic_error_condition_must_be_bool(id, type);
		return UNKNOWN;
	}

	return BOOL;
}

static char* create_new_variable(Token id, Data_Type type) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	if (!info) {
		handle_out_of_memory_error();
		return NULL;
	}

	info->data_type = type;
	info->identifier_name = id.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;

	bool added = symbol_table_add_symbol(symbol_table, info);

	if (!added) {
		handle_sementic_error_identifier_already_defined(id);
		free(info);
		return NULL;
	}

	return info->identifier_new_name;
}


static Data_Type decl_with_asign(SyntaxTree* tree) {
	Data_Type left = get_type(tree->info.nonterminal_info.children[0]);
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;

	char* new_name = create_new_variable(id, left);
	if (new_name == NULL) return NONE;
	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = new_name;


	Data_Type right = accept(tree->info.nonterminal_info.children[3]);
	if (!compatible(left, right))
		return handle_incompatability(tree, left, right);


	return NONE;
}

static Data_Type expression(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	return handle_incompatability(tree, left, right);
}

static Data_Type term(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	return handle_incompatability(tree, left, right);
}

static Data_Type factor(SyntaxTree* tree) {
	return accept(tree->info.nonterminal_info.children[1]);
}

static Data_Type assign(SyntaxTree* tree) {
	Token id = tree->info.nonterminal_info.children[0]->info.terminal_info.token;
	IdentifiersInfo* info = symbol_table_lookup_symbol(symbol_table, 
		&tree->info.nonterminal_info.children[0]->info.terminal_info.token.lexeme);

	if (info) {
		Data_Type left = info->data_type;
		Data_Type right = accept(tree->info.nonterminal_info.children[2]);
		return handle_incompatability(tree, left, right);
	}

	handle_sementic_error_identifier_not_defined(id);
	return NONE;
}

static Data_Type decl(SyntaxTree* tree) {
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	Data_Type left = get_type(tree->info.nonterminal_info.children[0]);

	char* new_name = create_new_variable(id, left);

	if (new_name == NULL) return NONE;

	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = new_name;
	return left;
}



static Data_Type condition(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);

	return handle_incompatability(tree, left, right);
}

static Data_Type condition_list(SyntaxTree* tree) {
	Data_Type left = accept(tree->info.nonterminal_info.children[0]);
	Data_Type right = accept(tree->info.nonterminal_info.children[2]);
	condition_must_be_bool(tree, left);
	condition_must_be_bool(tree, right);
	return BOOL;
}



static Data_Type if_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);

	accept(tree->info.nonterminal_info.children[2]);

	symbol_table_remove_scope(symbol_table);
	return NONE;
}

static Data_Type if_else_statement(SyntaxTree* tree) {
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);

	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[2]);
	symbol_table_remove_scope(symbol_table);

	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);

	return NONE;
}


static Data_Type while_statement(SyntaxTree* tree) {
	Data_Type condition = accept(tree->info.nonterminal_info.children[1]);

	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[2]);
	symbol_table_remove_scope(symbol_table);
	return NONE;
}


static Data_Type do_while_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	accept(tree->info.nonterminal_info.children[1]);

	Data_Type condition = accept(tree->info.nonterminal_info.children[3]);

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

	accept(tree->info.nonterminal_info.children[4]);
	symbol_table_remove_scope(symbol_table);
	return NONE;
}

static Data_Type for_change_statement(SyntaxTree* tree) {
	symbol_table_add_scope(symbol_table);
	Data_Type variable = accept(tree->info.nonterminal_info.children[1]);
	Data_Type condition = accept(tree->info.nonterminal_info.children[3]);

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
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	if (!info) {
		handle_out_of_memory_error();
		return NONE;
	}

	info->data_type = get_type(tree->info.nonterminal_info.children[0]);
	info->identifier_name = id.lexeme;
	info->identifier_type = VARIABLE;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);

	if (!added) {
		handle_sementic_error_identifier_already_defined(id);
		return NONE;
	}

	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = info->identifier_new_name;


	char* supposed_name = strdup(CURRENT_FUNCTION_SYMBOL);
	FunctionInfo* cur_info = ((FunctionInfo*)symbol_table_lookup_symbol(symbol_table, &supposed_name)->info);
	int cur_number_of_params = cur_info->num_of_params;
	cur_info->num_of_params++;
	VariableInfo* temp = cur_info->params;
	cur_info->params = realloc(cur_info->params, cur_info->num_of_params * sizeof(VariableInfo));
	if (!cur_info->params) {
		free(temp);
		handle_out_of_memory_error();
		return NONE;
	}

	cur_info->params[cur_number_of_params].data_type = info->data_type;
	cur_info->params[cur_number_of_params].identifier_name = info->identifier_name;
	return NONE;
}


static Data_Type function_decl(SyntaxTree* tree) {
	IdentifiersInfo* info = malloc(sizeof(IdentifiersInfo));
	if (!info) {
		handle_out_of_memory_error();
		return NONE;
	}

	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	info->data_type = get_type(tree->info.nonterminal_info.children[5]);
	info->identifier_name = id.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_sementic_error_identifier_already_defined(id);
		free(info);
		return NONE;
	}

	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = info->identifier_new_name;

	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	if (!helper_info) {
		handle_out_of_memory_error();
		return NONE;
	}


	*helper_info = *info;
	helper_info->identifier_name = strdup(CURRENT_FUNCTION_SYMBOL);
	helper_info->info = malloc(sizeof(FunctionInfo));
	if (!helper_info->info) {
		handle_out_of_memory_error();
		return NONE;
	}

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
	if (!info) {
		handle_out_of_memory_error();
		return NONE;
	}

	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	info->data_type = NONE;
	info->identifier_name = id.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_sementic_error_identifier_already_defined(id);
		free(info);
		return NONE;
	}
	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = info->identifier_new_name;


	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	if (!helper_info) {
		handle_out_of_memory_error();
		return NONE;
	}

	*helper_info = *info;
	helper_info->identifier_name = strdup(CURRENT_FUNCTION_SYMBOL);
	if (!helper_info->identifier_name) handle_out_of_memory_error();

	helper_info->info = malloc(sizeof(FunctionInfo));
	if (!helper_info->info) {
		handle_out_of_memory_error();
		return NONE;
	}

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
	if (!info) {
		handle_out_of_memory_error();
		return NONE;
	}

	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	info->data_type = get_type(tree->info.nonterminal_info.children[3]);
	info->identifier_name = id.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_sementic_error_identifier_already_defined(id);
		free(info);
		return NONE;
	}
	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = info->identifier_new_name;

	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	if (!helper_info) {
		handle_out_of_memory_error();
		return NONE;
	}

	*helper_info = *info;
	helper_info->identifier_name = strdup(CURRENT_FUNCTION_SYMBOL);
	helper_info->info = malloc(sizeof(FunctionInfo));
	if (!helper_info->info) {
		handle_out_of_memory_error();
		return NONE;
	}

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
	if (!info) {
		handle_out_of_memory_error();
		return NONE;
	}
 
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	info->data_type = NONE;
	info->identifier_name = id.lexeme;
	info->identifier_type = FUNCTION;
	info->info = NULL;
	bool added = symbol_table_add_symbol(symbol_table, info);
	if (!added) {
		handle_sementic_error_identifier_already_defined(id);
		free(info);
		return NONE;
	}
	tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme = info->identifier_new_name;


	symbol_table_add_scope(symbol_table);
	IdentifiersInfo* helper_info = malloc(sizeof(IdentifiersInfo));
	if (!helper_info) {
		handle_out_of_memory_error();
		return NONE;
	}

	*helper_info = *info;
	helper_info->identifier_name = strdup(CURRENT_FUNCTION_SYMBOL);
	helper_info->info = malloc(sizeof(FunctionInfo));
	if (!helper_info->info) {
		handle_out_of_memory_error();
		return NONE;
	}

	((FunctionInfo*)helper_info->info)->num_of_params = 0;
	((FunctionInfo*)helper_info->info)->params = NULL;
	symbol_table_add_symbol(symbol_table, helper_info);

	info->info = helper_info->info;

	accept(tree->info.nonterminal_info.children[2]);
	symbol_table_remove_scope(symbol_table);

	free(helper_info);

	return NONE;
}

static Data_Type return_statement(SyntaxTree* tree) {
	Data_Type info = accept(tree->info.nonterminal_info.children[1]);
	char* supposed_name = strdup(CURRENT_FUNCTION_SYMBOL);
	Data_Type supposed = symbol_table_lookup_symbol(symbol_table, &supposed_name)->data_type;
	// fix return outside function
	return handle_incompatability(tree, info, supposed);
}
static Data_Type return_none_statement(SyntaxTree* tree) {
	char* supposed_name = strdup(CURRENT_FUNCTION_SYMBOL);
	Data_Type supposed = symbol_table_lookup_symbol(symbol_table, &supposed_name)->data_type;
	
	return handle_incompatability(tree, NONE, supposed);
}

static Data_Type function_call(SyntaxTree* tree) {
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	IdentifiersInfo* funcSymbol = symbol_table_lookup_symbol(symbol_table,
		&tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme);

	if (funcSymbol == NULL) {
		handle_sementic_function_doesnt_exist(id);
		return NONE;
	}
	else if (funcSymbol->identifier_type != FUNCTION) {
		handle_sementic_function_doesnt_exist(id);
		return NONE;
	}

	FunctionInfo* funcInfo = (FunctionInfo*)funcSymbol->info;

	SyntaxTree* argListNode = tree->info.nonterminal_info.children[3];
	int argCount = 0;
	Data_Type argTypes[100];


	LinkedList* argList = linkedlist_init(sizeof(Data_Type));
	Data_Type type;
	while (argListNode->type == NONTERMINAL_TYPE 
		&& strcmp(argListNode->info.nonterminal_info.nonterminal, "ARGUMENT_LIST") == 0) {
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
		handle_sementic_function_call_arguments_miscount(id, argCount, funcInfo->num_of_params);
		return UNKNOWN;
	}

	bool argument_type_mismatch = false;
	for (int i = 0; i < argCount; i++) {
		if (!compatible(argTypes[i], funcInfo->params[i].data_type)) {
			handle_sementic_function_call_arguments_mismatch(id, i + 1, argTypes[i], funcInfo->params[i].data_type);
			bool argument_type_mismatch = true;
		}
		if (argument_type_mismatch) return UNKNOWN;
	}

	return funcSymbol->data_type;
}

static Data_Type function_call_with_nothing(SyntaxTree* tree) {
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	IdentifiersInfo* funcSymbol = symbol_table_lookup_symbol(symbol_table,
		&tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme);

	if (funcSymbol == NULL) {
		handle_sementic_function_doesnt_exist(id);
		return NONE;
	}

	if (funcSymbol->identifier_type != FUNCTION) {
		handle_sementic_function_doesnt_exist(id);
		return NONE;
	}

	FunctionInfo* funcInfo = (FunctionInfo*)funcSymbol->info;

	if (funcInfo->num_of_params != 0) {
		handle_sementic_function_call_arguments_miscount(id, 0, funcInfo->num_of_params);
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



static char* PRINT_INT_EXPRESSION_CHANGER = "PRINT_INT_EXPRESSION";
Data_Type print_sem(SyntaxTree* tree) {
	Data_Type type_of_var = accept(tree->info.nonterminal_info.children[1]);
	if (type_of_var == INT) {
		tree->info.nonterminal_info.nonterminal = PRINT_INT_EXPRESSION_CHANGER;
	}
	else if (type_of_var == STRING) {
		StringInfo* info = malloc(sizeof(StringInfo));
		if (!info) {
			handle_out_of_memory_error();
			return NONE;
		}

		info->id = string_ids++;
		info->tk = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
		info->size = strlen(info->tk.lexeme);
		info->local = 0;

		if (hashmap_get(symbol_table->GlobalStrings, info->tk.lexeme) != NULL) {
			free(info);
		}
		else {
			hashmap_insert(symbol_table->GlobalStrings, info->tk.lexeme, info);
		}
	}
	else {
		if (type_of_var != UNKNOWN) {
			SyntaxTree* pos = tree->info.nonterminal_info.children[1];
			while (pos->type != TERMINAL_TYPE)
				pos = tree->info.nonterminal_info.children[0];
			handle_sementic_invalid_print_type(pos->info.terminal_info.token, type_of_var);
		}
	}
	return NONE;
}

Data_Type print_sem_int(SyntaxTree* tree) {
	Data_Type type_of_var = accept(tree->info.nonterminal_info.children[1]);

	if (type_of_var != INT && type_of_var != UNKNOWN) {
		SyntaxTree* pos = tree->info.nonterminal_info.children[1];
		while (pos->type != TERMINAL_TYPE)
			pos = tree->info.nonterminal_info.children[0];
		handle_sementic_invalid_print_type(pos->info.terminal_info.token, type_of_var);
	}

	return NONE;
}

Data_Type get_decl_sementic(SyntaxTree* tree) {
	Data_Type type = decl(tree->info.nonterminal_info.children[1]);
	if (type != INT && type != UNKNOWN) {
		SyntaxTree* pos = tree->info.nonterminal_info.children[1];
		while (pos->type != TERMINAL_TYPE)
			pos = tree->info.nonterminal_info.children[0];
		handle_sementic_get_type(pos->info.terminal_info.token, type);
	}

	return NONE;
}

Data_Type get_sementic(SyntaxTree* tree) {
	Token id = tree->info.nonterminal_info.children[1]->info.terminal_info.token;
	IdentifiersInfo* info = symbol_table_lookup_symbol(symbol_table, 
		&tree->info.nonterminal_info.children[1]->info.terminal_info.token.lexeme);
	if (info == NULL) {
		handle_sementic_error_identifier_not_defined(id);
	}
	else {
		Data_Type type = info->data_type;
		if (type != INT && type != UNKNOWN) {
			SyntaxTree* pos = tree->info.nonterminal_info.children[1];
			while (pos->type != TERMINAL_TYPE)
				pos = tree->info.nonterminal_info.children[0];
			handle_sementic_get_type(pos->info.terminal_info.token, type);
		}
	}
	return NONE;
}

Data_Type accept(SyntaxTree* tree) {
	Data_Type(*pointer)(SyntaxTree*) = hashmap_get(ir_visitor, tree->info.nonterminal_info.nonterminal);
	if (pointer != NULL) return pointer(tree);

	if (tree->type == TERMINAL_TYPE) return get_type(tree);

	return NONE;
}

void init_visitor() {
	hashmap_insert(ir_visitor, "PROGRAM", &program);
	hashmap_insert(ir_visitor, "STATEMENTS", &statements);
	hashmap_insert(ir_visitor, "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", &decl_with_asign);
	hashmap_insert(ir_visitor, "EXPRESSION", &expression);
	hashmap_insert(ir_visitor, "TERM", &term);
	hashmap_insert(ir_visitor, "FACTOR", &factor);
	hashmap_insert(ir_visitor, "VARIABLE_ASSIGNMENT_STATEMENT", &assign);
	hashmap_insert(ir_visitor, "VARIABLE_DECLARATION_STATEMENT", &decl);
	hashmap_insert(ir_visitor, "CONDITION", &condition);
	hashmap_insert(ir_visitor, "CONDITION_LIST", &condition_list);
	hashmap_insert(ir_visitor, "IF_STATEMENT", &if_statement);
	hashmap_insert(ir_visitor, "BLOCK", &block);
	hashmap_insert(ir_visitor, "IF_ELSE_STATEMENT", &if_else_statement);
	hashmap_insert(ir_visitor, "WHILE_STATEMENT", &while_statement);
	hashmap_insert(ir_visitor, "DO_WHILE_STATEMENT", &do_while_statement);
	hashmap_insert(ir_visitor, "FOR_BLOCK", &for_block);
	hashmap_insert(ir_visitor, "FOR_STATEMENT", &for_statement);
	hashmap_insert(ir_visitor, "FOR_CHANGE_STATEMENT", &for_change_statement);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_STATEMENT", &function_decl);
	hashmap_insert(ir_visitor, "FUNCTION_BLOCK", &func_block);
	hashmap_insert(ir_visitor, "PARAMETER_LIST", &parameter_list);
	hashmap_insert(ir_visitor, "PARAMETER", &parameter);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_NO_RETURN_STATEMENT", &function_decl_returns_nothing);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT", &function_decl_gets_nothing);
	hashmap_insert(ir_visitor, "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT", &function_decl_gets_returns_nothing);
	hashmap_insert(ir_visitor, "RETURN_STATEMENT", &return_statement);
	hashmap_insert(ir_visitor, "break", &return_none_statement);
	hashmap_insert(ir_visitor, "FUNCTION_CALL_WITH_NOTHING_STATEMENT", &function_call_with_nothing);
	hashmap_insert(ir_visitor, "FUNCTION_CALL_STATEMENT", &function_call);
	hashmap_insert(ir_visitor, "ARGUMENT_LIST", &arg_list);
	hashmap_insert(ir_visitor, "IF_BLOCK", &if_block);
	hashmap_insert(ir_visitor, "WHILE_BLOCK", &while_block);
	hashmap_insert(ir_visitor, "GET_DECLARE_STATEMENT", &get_decl_sementic);
	hashmap_insert(ir_visitor, "GET_STATEMENT", &get_sementic);
	hashmap_insert(ir_visitor, "FUNCTION_STATEMENTS", &statements);
	
	hashmap_insert(ir_visitor, "PRINT_STATEMENT", &print_sem);
	hashmap_insert(ir_visitor, "PRINT_INT_EXPRESSION", &print_sem_int);



}



int sementic_analysis(SyntaxTree* tree) {
	ir_visitor = createHashMap(NONTERMINAL_COUNT_DEFUALT, string_hash, string_equals);
	init_visitor();


	symbol_table = symbol_table_init();

	accept(tree);


    return 0;
}
