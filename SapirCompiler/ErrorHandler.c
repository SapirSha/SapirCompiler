#include "ErrorHandler.h"
#include "stdio.h"

#define MAX_ERROR_MSG_LENGTH 250
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