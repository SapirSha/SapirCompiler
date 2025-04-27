#include "Lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "LexerFsm.h"
#include "Queue.h"
#include "ErrorHandler.h"
#include "Tokens.h"
#include "Boolean.h"

#pragma warning(disable:4996)

#define MAX_TOKEN_LENGTH 64
#define NEW_LINE '\n'

static void add_token(Queue* tokens, enum State state, const char* lexeme, int row, int col) {
	Token_Types type = convert_state_to_token_type(state);
	Token token = {
		.type = type,
		.lexeme = strdup(lexeme),
		.row = row,
		.col = col
	};

	queue_enqueue(tokens, &token);
}

static inline void handle_error(char* code_pointer, int row, int col) {
	handle_unknown_character_error(*code_pointer, row, col);
}

static inline void handle_new_line(int* current_line, int* latest_line_start, int current_index_in_text) {
	(*current_line)++;
	(*latest_line_start) = current_index_in_text + 1;
}

static int handle_above_maximum_token_length(char* got_until_now, int current_length,
	char* code_pointer, int token_start_line, int token_start_column) {
	int remaining_length = strcspn(code_pointer, "\t\n\r \0");
	int gotten_token_length = current_length + remaining_length;
	char* gotten_token = malloc(gotten_token_length + 1);
	if (gotten_token) {
		strncpy(gotten_token, got_until_now, current_length);
		strncpy(gotten_token + current_length, code_pointer, gotten_token_length - current_length);

		handle_lexical_above_max_token_length(gotten_token, token_start_line, token_start_column);
		free(gotten_token);
		
		return remaining_length - 1;
	}
	else {
		handle_out_of_memory_error();
		return remaining_length;
	}
}

static inline void insert_eof_token(Queue* q, int row, int col) {
	Token eof = {
		.type = TOKEN_EOF,
		.lexeme = strdup("End Of Program"),
		.row = row,
		.col = col
	};
	queue_enqueue(q, &eof);
	queue_enqueue(q, &eof);
}

#define CURRENT_POSITION(current_line, current_line_start_pos, code_pointer, code)\
                current_line, pos - code - current_line_start_pos + 1

#define CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, code_pointer, code)\
				current_line, pos - code - current_token_start_pos + 1

Queue* tokenize(const char* code) {
	enum State state = START_STATE;
	enum State prev_state = START_STATE;
	Queue* tokens = queue_init(sizeof(Token));
	char* pos = code;

	char current_token[MAX_TOKEN_LENGTH + 1];
	int current_token_length = 0;
	
	int current_line = 1;
	int current_line_start_pos = 0;
	int current_token_start_pos = 0;

	while (*pos != '\0') {
		if (*pos == NEW_LINE) handle_new_line(&current_line, &current_line_start_pos, pos - code);
		state = get_next_state(state, *pos);

		if (state == START_STATE) {
			if (!is_ignored_state(prev_state)) {
				current_token[current_token_length] = '\0';
				add_token(tokens, prev_state, current_token, 
					CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, pos, code));
			}
			current_token_length = 0;
			current_token_start_pos = pos - code + 1;
			if (prev_state == START_STATE) pos++;
		} 
		else if (state == ERROR_STATE) {
			handle_error(pos, CURRENT_POSITION(current_line, current_line_start_pos, pos, code));
			current_token_length = 0;
			state = START_STATE;
			pos++;
		}
		else {
			if (current_token_length > MAX_TOKEN_LENGTH) {
				pos += handle_above_maximum_token_length(current_token, current_token_length,
					pos, CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, pos, code) - 1);
				current_token_length = 0;
				state = START_STATE;
			}
			else {
				current_token[current_token_length] = *pos;
				current_token_length++;
				pos++;
			}
		}
		prev_state = state;
	}

	if (!is_ignored_state(state)) {
		current_token[current_token_length] = '\0';
		add_token(tokens, state, current_token, 
			CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, pos, code));
	}

	insert_eof_token(tokens, CURRENT_POSITION(current_line, current_line_start_pos, pos, code));
	queue_print(tokens, printToken);

	return tokens;
}
