#include "Tokens.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Initialize the queue
TokensQueue* tokens_init() {
    TokensQueue* queue = malloc(sizeof(TokensQueue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

// Enqueue a new token
void tokens_enqueue(TokensQueue* queue, char* lexeme, int type) {
    Tokens* new_token = malloc(sizeof(Tokens));
    new_token->lexeme = _strdup(lexeme);
    new_token->type = type;
    new_token->next = NULL;

    if (queue->tail == NULL) {
        // Queue is empty
        queue->head = new_token;
        queue->tail = new_token;
    }
    else {
        queue->tail->next = new_token;
        queue->tail = new_token;
    }
}

// Dequeue a token (removes and returns the front token)
Tokens* tokens_dequeue(TokensQueue* queue) {
    if (queue->head == NULL) {
        // Queue is empty
        return NULL;
    }

    Tokens* front_token = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL) {
        // Queue is now empty
        queue->tail = NULL;
    }

    return front_token;
}

// Print all tokens in the queue
void tokens_print(TokensQueue* queue) {
    char* translate_tokens[] = {
	[TOKEN_UNKNOWN] = "UNKNOWN",
    [TOKEN_IDENTIFIER] = "IDENTIFIER",
    [TOKEN_NUMBER] = "NUMBER",
    [TOKEN_FLOAT_NUMBER] = "FLOAT NUMBER",
    [TOKEN_OPERATOR] = "OPERATOR",
    [TOKEN_KEYWORD] = "KEYWORD",
    [TOKEN_STRING_LITERAL] = "String Literal",
    [TOKEN_SEPARATOR] = "SEPARATOR",

    [TOKEN_OPERATOR_PLUS] = "+",
    [TOKEN_OPERATOR_MINUS] = "-",
    [TOKEN_OPERATOR_MULTIPLY] = "*",
    [TOKEN_OPERATOR_DIVIDE] = "/",
    [TOKEN_OPERATOR_ASSIGN] = "=",
    [TOKEN_OPERATOR_GREATER] = ">",
    [TOKEN_OPERATOR_LESS] = "<",
    [TOKEN_OPERATOR_MODULO] = "%",
    [TOKEN_OPERATOR_AND] = "&",
    [TOKEN_OPERATOR_OR] = "|",
    [TOKEN_OPERATOR_XOR] = "^",
    [TOKEN_OPERATOR_NOT] = "!",
    [TOKEN_OPERATOR_INCREMENT] = "++",
    [TOKEN_OPERATOR_DECREMENT] = "--",
    [TOKEN_OPERATOR_ADD_ASSIGN] = "+=",
    [TOKEN_OPERATOR_SUB_ASSIGN] = "-=",
    [TOKEN_OPERATOR_MUL_ASSIGN] = "*=",
    [TOKEN_OPERATOR_DIV_ASSIGN] = "/=",
    [TOKEN_OPERATOR_MOD_ASSIGN] = "%=",
    [TOKEN_OPERATOR_AND_ASSIGN] = "&=",
    [TOKEN_OPERATOR_OR_ASSIGN] = "|=",
    [TOKEN_OPERATOR_XOR_ASSIGN] = "^=",
    [TOKEN_OPERATOR_LEFT_SHIFT] = "<<",
    [TOKEN_OPERATOR_RIGHT_SHIFT] = ">>",
    [TOKEN_OPERATOR_EQUAL] = "==",
    [TOKEN_OPERATOR_NOT_EQUAL] = "!=",
    [TOKEN_OPERATOR_GREATER_EQUAL] = ">=",
    [TOKEN_OPERATOR_LESS_EQUAL] = "<=",
    [TOKEN_OPERATOR_ALSO] = "&&",
    [TOKEN_OPERATOR_EITHER] = "||",

    [TOKEN_IF] = "IF",
    [TOKEN_WHILE] = "WHILE",
    [TOKEN_RETURN] = "RETURN",
    [TOKEN_FOR] = "FOR",
    [TOKEN_ELSE] = "ELSE",
    [TOKEN_INT] = "INT",
    [TOKEN_STRING] = "STRING",
    [TOKEN_CHAR] = "CHAR",
    [TOKEN_FLOAT] = "FLOAT",
    [TOKEN_DOUBLE] = "DOUBLE",
    [TOKEN_VOID] = "VOID",
    [TOKEN_BOOL] = "BOOL",
    [TOKEN_TRUE] = "TRUE",
    [TOKEN_FALSE] = "FALSE",

    [TOKEN_LPAREN] = "(",
    [TOKEN_RPAREN] = ")",
    [TOKEN_LBRACES] = "{",
    [TOKEN_RBRACES] = "}",
    [TOKEN_LBRACKETS] = "[",
    [TOKEN_RBRACKETS] = "]",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_COMMA] = ",",


    [TOKEN_EOF] = "EOF"
    };



    printf("TOKENS:\n");
    Tokens* current = queue->head;
    while (current != NULL) {
        printf("Type: %-10s\tLexeme: %s\n", translate_tokens[current->type], current->lexeme);
        current = current->next;
    }
}

// Free the memory used by the queue
void tokens_free(TokensQueue* queue) {
    Tokens* current = queue->head;
    while (current != NULL) {
        Tokens* next = current->next;
        free(current);
        current = next;
    }
    free(queue);
}
