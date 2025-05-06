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

#define NEW_LINE '\n'

#define CURRENT_POSITION(current_line, current_line_start_pos, code_pointer, code)\
                current_line, pos - code - current_line_start_pos + 1

#define CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, current_line_start_pos)\
				current_line, current_token_start_pos - current_line_start_pos + 1

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

static inline void handle_error(char** code_pointer, int row, int col,
	int* current_token_length, enum State* current_state) {
	// only error lexer catches is invalid character
	handle_unknown_character_error(**code_pointer, row, col);
	(*current_token_length) = 0;
	(*current_state) = START_STATE;
	(*code_pointer)++;
}

static inline void handle_new_line(int* current_line, int* latest_line_start, int current_index_in_text) {
	(*current_line)++;
	(*latest_line_start) = current_index_in_text;
}

// returns the remaining length of the token
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

static void add_to_current_token(char* current_token, int* current_token_length, char** code_pointer,
	int current_line, int current_token_start_pos, int current_line_start_pos, enum State* current_state) {
	if (*current_token_length > MAX_TOKEN_LENGTH) {
		//handle error and skip the rest of the token
		*code_pointer += handle_above_maximum_token_length(current_token, *current_token_length,
			*code_pointer, CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, current_line_start_pos) - 1);
		*current_token_length = 0;
		*current_state = START_STATE;
	}
	else {
		// add current char to token
		current_token[*current_token_length] = **code_pointer;
		(*current_token_length)++;
		(*code_pointer)++;
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

static void handle_end_of_path(Queue* tokens, enum State prev_state, char* current_token, 
	int* current_token_length, int* current_line, int* current_token_start_pos, int* current_line_start_pos,
	char** pos, char* code) {
	// ignored state is a state without a specified token
	if (!is_ignored_state(prev_state)) {
		current_token[*current_token_length] = '\0';
		add_token(tokens, prev_state, current_token,
			CURRENT_TOKEN_POSITION(*current_line, *current_token_start_pos, *current_line_start_pos));
	}
	*current_token_length = 0;
	*current_token_start_pos = *pos - code + 1;
	if (**pos == NEW_LINE) handle_new_line(current_line, current_line_start_pos, *pos - code);
	if (prev_state == START_STATE || **pos == NEW_LINE) (*pos)++; // against infinite loop
}

Queue* tokenize(char* code) {
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
		state = get_next_state(state, *pos);
		// if reached start state its the end of the path in the dfa
		if (state == START_STATE) {
			handle_end_of_path(tokens, prev_state, current_token, 
				&current_token_length, &current_line, &current_token_start_pos,
				&current_line_start_pos, &pos, code);
		} 
		else if (state == ERROR_STATE) {
			handle_error(&pos, CURRENT_POSITION(current_line, current_line_start_pos, pos, code),
				&current_token_length, &state);
		}
		else {
			// add current char to token
			add_to_current_token(current_token, &current_token_length, &pos, current_line,
				current_token_start_pos, current_line_start_pos, &state);
		}
		prev_state = state;
	}

	// add the last token
	if (!is_ignored_state(state)) {
		current_token[current_token_length] = '\0';
		add_token(tokens, state, current_token, 
			CURRENT_TOKEN_POSITION(current_line, current_token_start_pos, current_line_start_pos));
	}

	// add end tokens
	insert_eof_token(tokens, CURRENT_POSITION(current_line, current_line_start_pos, pos, code));

	return tokens;
}
