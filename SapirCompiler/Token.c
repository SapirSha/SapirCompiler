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

void printToken(Token* token) {
	if (token == NULL) {
		printf("NULL\n");
		return;
	}
	printf("Token Type: %d, Lexeme: %s, Row: %d, Col: %d\n", token->type, token->lexeme, token->row, token->col);
}