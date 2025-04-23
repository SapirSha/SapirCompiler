#include "Lexer.h"

#include <stdlib.h>
#include "LexerFsm.h"
#include "Queue.h"
#include "ErrorHandler.h"
#include "Tokens.h"
#include "Boolean.h"

#define MAX_TOKEN_LENGTH 48
#define NEW_LINE '\n'

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
	free(token);
}

static void handle_error(char* code_pointer, int row, int col) {
	handle_unkown_character_error(*code_pointer, row, col);
}

static void handle_new_line(int* current_line, int* latest_line_start, int current_index_in_text) {
	(*current_line)++;
	(*latest_line_start) = current_index_in_text + 1;
}

#define INSER_END_OF_FILE_TOKEN(tokens_queue, current_line, current_col) {\
    Token* end_of_file = create_new_token(TOKEN_EOF, "$", current_line, current_col);\
    queue_enqueue(tokens, end_of_file);\
    queue_enqueue(tokens, end_of_file);\
    free(end_of_file); }

#define CURRENT_POSITION(current_line, current_line_start_pos, code_pointer, code)\
                current_line, pos - code - current_line_start_pos + 1

#define CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, code_pointer, code)\
				current_line, pos - code - current_token_start_pos + 1

Queue* tokenize(const char* code) {
	enum State current_state = START_STATE;
	enum State former_state = current_state;
	Queue* tokens = queue_init(sizeof(Token));
	char* pos = code;

	char current_token[MAX_TOKEN_LENGTH + 1];
	int current_token_length = 0;
	
	int current_line = 1;
	int current_line_start_pos = 0;
	int current_token_start_pos = 0;

	while (*pos) {

		current_state = get_next_state(current_state, *pos);
		if (current_state == START_STATE) {
			if (!is_ignored_state(former_state)) {
				current_token[current_token_length] = '\0';
				add_token(tokens, former_state, current_token, 
					CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, pos, code));
				pos--;
			}
			current_token_length = 0;
		} 
		else if (current_state == ERROR_STATE) {
			handle_error(pos, CURRENT_POSITION(current_line, current_line_start_pos, pos, code));
			current_token_length = 0;
			current_state = START_STATE;
		}
		else {
			if (current_token_length == 0) current_token_start_pos = pos - code + 1;
			current_token[current_token_length] = *pos;
			current_token_length++;
		}
		former_state = current_state;
		if (*pos == NEW_LINE) handle_new_line(&current_line, &current_line_start_pos, pos - code);
		pos++;
	}

	if (!is_ignored_state(current_state)) {
		current_token[current_token_length] = '\0';
		add_token(tokens, current_state, current_token, 
			CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, pos, code));
		current_token_length = 0;
	}
	INSER_END_OF_FILE_TOKEN(tokens, current_line, pos - code - current_line_start_pos + 1);
	queue_print(tokens, printTokem);

	return tokens;
}
