#include "ErrorHandler.h"
#include <stdio.h>
#include <string.h>

#define MAX_ERROR_MSG_LENGTH 256
static char ErrorMsg[MAX_ERROR_MSG_LENGTH];

void out_put_error(char* msg) {
	printf("%s", msg);
}
void handle_unkown_character_error(char character, int row, int col) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Lexical Error At line %d, column %d: Unknown Character %c\n"
		,row, col, character);

	out_put_error(ErrorMsg);
	current_error_state = LEXICAL_ERROR;
}

static const char* STATES_TO_STRING[NUM_STATES] = {
	[KEYWORD] = "Keyword",
	[IDENTIFIER] = "Identifier",
	[STRING_LITERAL] = "String Literal",
	[NUMBER] = "Number",
	[OPERATOR] = "Operator",
	[SEPARATOR] = "Separator",
	[START] = "Start",
	[COMMENT] = "Comment",
	[ERROR] = "Error",
};
char* state_to_string(LEXER_STATE state) {
	return STATES_TO_STRING[state];
}

void handle_lexical_error(LEXER_STATE previous_state, char next_input, int row, int col) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Lexical Error At line %d, column %d: %c cannot appear after %s\n"
		, row, col, next_input, state_to_string(previous_state));

	out_put_error(ErrorMsg);
	current_error_state = LEXICAL_ERROR;
}

void handle_lexical_above_max_token_length(char* expected_token, int row, int col) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Lexical Error At line %d, column %d: Maximum Token length is %d, "
		, row, col, MAX_TOKEN_LENGTH
	);
	out_put_error(ErrorMsg);

	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"but gotten token %s with length %d\n"
		,expected_token, strlen(expected_token)
	);
	out_put_error(ErrorMsg);

	current_error_state = LEXICAL_ERROR;
}

void handle_parser_error(int state_accured, Token* latest_token, Token* next_token, ArrayList* allowed_tokens, ArrayList* allowed_statements){
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Invalid grammer, after token '%s' in line %d column %d\n"
		, latest_token->lexeme, latest_token->row, latest_token->col
	);

	out_put_error(ErrorMsg);
	out_put_error("The following tokens are allowed: ");
	char* str_pointer;
	for (int i = 0; i < allowed_tokens->size; i++) {
		str_pointer = *(char**)allowed_tokens->array[i];
		out_put_error(str_pointer);
		if (i + 1 < allowed_tokens->size) 
			out_put_error(",");
	}
	out_put_error("\nOr the following statements: ");
	for (int i = 0; i < allowed_statements->size; i++) {
		str_pointer = *(char**)allowed_statements->array[i];
		out_put_error(str_pointer);
		if (i + 1 < allowed_statements->size)
			out_put_error(",");
	}

	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"\nBut instead Got Token '%s' in line %d column %d.\n"
		, next_token->lexeme, next_token->row, next_token->col
	);
	out_put_error(ErrorMsg);
	
	current_error_state = PARSER_ERROR;
}


char* DATA_TYPE_TO_STRING[] = {
	[NONE] = "NONE",
	[INT] = "INT",
	[STRING] = "STRING",
	[BOOL] = "BOOL",
	[UNKNOWN] = "UNKNOWN",
};

void handle_sementic_error(Token left, char* tree_string, Data_Type left_type, Data_Type right_type) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Sementic Error at line %d, column %d: %s. %s is incompatible with %s\n"
		, left.row, left.col, tree_string, DATA_TYPE_TO_STRING[left_type], DATA_TYPE_TO_STRING[right_type]
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}


void handle_sementic_error_identifier_already_defined(Token id) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Sementic Error at line %d, column %d: identifier %s already defined.\n"
		, id.row, id.col, id.lexeme
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}

void handle_sementic_error_identifier_not_defined(Token id) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Sementic Error at line %d, column %d: identifier %s is not defined.\n"
		, id.row, id.col, id.lexeme
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}
void handle_sementic_error_condition_must_be_bool(Token id, char* tree_string, Data_Type type) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Condition %s at line %d column %d must be a bool! instead it was %s\n"
		, tree_string, id.row, id.col, DATA_TYPE_TO_STRING[type]
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}

void handle_sementic_function_doesnt_exist(Token func_call) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Function call in line %d col %d refrences non-existant function %s\n"
		, func_call.row, func_call.col, func_call.lexeme
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}


void handle_sementic_function_call_arguments_miscount(Token func_call, int call_amount, int func_amount) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Function call in line %d col %d refrences a function that has %d parameters, while the call has %d arguments\n"
		, func_call.row, func_call.col, func_amount, call_amount
	);
	out_put_error(ErrorMsg);


	current_error_state = SEMENTIC_ERROR;
}


void handle_sementic_function_call_arguments_mismatch(Token func_call, int argument_index, Data_Type arg, Data_Type param) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Function call in line %d col %d has type %s as argument %d while the function has type %s as argument %d.\n"
		, func_call.row, func_call.col, DATA_TYPE_TO_STRING[arg], argument_index, DATA_TYPE_TO_STRING[param], argument_index
	);
	out_put_error(ErrorMsg);


	current_error_state = SEMENTIC_ERROR;
}

void handle_sementic_invalid_print_type(Token content, Data_Type type) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Print instruction at line %d col %d cannot be %s (print works only for int and static strings)\n"
		, content.row, content.col, DATA_TYPE_TO_STRING[type]
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}

void handle_sementic_get_type(Token content, Data_Type type) {
	snprintf(ErrorMsg, MAX_ERROR_MSG_LENGTH,
		"Get instruction at line %d col %d cannot be %s (input works only for int)\n"
		, content.row, content.col, DATA_TYPE_TO_STRING[type]
	);
	out_put_error(ErrorMsg);

	current_error_state = SEMENTIC_ERROR;
}


