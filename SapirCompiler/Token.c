#include "Tokens.h"
#include "ErrorHandler.h"
#include <stdlib.h>

#pragma warning(disable:4996)

Token* create_new_token(int type, const char* lexeme, int row, int col) {
	Token* token = malloc(sizeof(Token));
	if (!token) {
		handle_out_of_memory_error();
		return NULL;
	}

	token->type = type;
	token->lexeme = strdup(lexeme);
	token->row = row;
	token->col = col;
	return token;
}