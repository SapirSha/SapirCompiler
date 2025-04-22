#include "Lexer.h"

#include <stdlib.h>
#include "LexerFsm.h"
#include "Queue.h"
#include "ErrorHandler.h"
#include "Tokens.h"
#include "Boolean.h"

#define MAX_TOKEN_LENGTH 48

#include <stdio.h>
void printTokem(Token* token) {
	if (token == NULL) {
		printf("NULL\n");
		return;
	}
	printf("Token Type: %d, Lexeme: %s, Row: %d, Col: %d\n", token->type, token->lexeme, token->row, token->col);
}

static void add_token(Queue* tokens, enum State state, const char* lexeme, int row, int col) {
	Token_Types type = convert_state_to_token_type(state);

	Token* token = create_new_token(type, lexeme, row, col);
	if (!token) {
		handle_out_of_memory_error();
		return;
	}

	queue_enqueue(tokens, token);
}

static void handle_error(char* code_pointer, int row, int col) {
	handle_unkown_character_error(*code_pointer, row, col);
}

Queue* tokenize(const char* code) {
	enum State current_state = START_STATE;
	enum State former_state = current_state;
	Queue* tokens = queue_init(sizeof(Token));
	char* pos = code;

	char current_token[MAX_TOKEN_LENGTH + 1];
	int current_token_length = 0;

	while (*pos) {
		current_state = get_next_state(current_state, *pos);
		if (current_state == START_STATE) {
			if (!is_ignored_state(former_state)) {
				current_token[current_token_length] = '\0';
				add_token(tokens, former_state, current_token, 0, 0);
				current_token_length = 0;
				pos--;
			}
		} 
		else if (current_state == ERROR_STATE) {
			handle_error(pos, 0, 0);
			current_token_length = 0;
			current_state = START_STATE;
		}
		else {
			current_token[current_token_length] = *pos;
			current_token_length++;
		}
		former_state = current_state;
		pos++;
	}

	if (current_token_length > 0) {
		current_token[current_token_length] = '\0';
		add_token(tokens, current_state, current_token, 0, 0);
		current_token_length = 0;
	}

	queue_print(tokens, printTokem);
	return tokens;
}
