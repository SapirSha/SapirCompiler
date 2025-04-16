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