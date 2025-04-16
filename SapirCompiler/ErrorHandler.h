#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
#include "Lexer.h"

typedef enum {
	NO_ERROR,
	LEXICAL_ERROR,
	STRUCTURE_ERROR,
	SEMENTIC_ERROR
} ErrorType;


ErrorType current_error_state;
void out_put_error(char* msg);
void handle_unkown_character_error(char character, int row, int col);
void handle_lexical_error(LEXER_STATE previous_state, char next_input, int row, int col);


#endif