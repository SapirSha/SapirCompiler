#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
#include "Lexer.h"
#include "Tokens.h"
#include "Parser.h"
#include "ArrayList.h"

typedef enum {
	NO_ERROR,
	LEXICAL_ERROR,
	PARSER_ERROR,
	SEMENTIC_ERROR
} ErrorType;


ErrorType current_error_state;
void out_put_error(char* msg);
void handle_unkown_character_error(char character, int row, int col);
void handle_lexical_error(LEXER_STATE previous_state, char next_input, int row, int col);
void handle_parser_error(int state_accured, Token* latest_token, Token* next_token, ArrayList* allowed_tokens, ArrayList* allowed_statements);

#endif