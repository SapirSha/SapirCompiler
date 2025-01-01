#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_STRING,
    TOKEN_SEPARATOR,
} Token_Types;


typedef struct Tokens{
    char* lexeme;
    Token_Types type;
    struct Tokens* next;
} Tokens;

typedef struct TokensQueue {
    Tokens* head;
    Tokens* tail;
} TokensQueue;


// Initialize the queue
TokensQueue* tokens_init();

// Enqueue a new token
void tokens_enqueue(TokensQueue* queue, char* lexeme, int type);

// Dequeue a token (removes and returns the front token)
Tokens* tokens_dequeue(TokensQueue* queue);

// Print all tokens in the queue
void tokens_print(TokensQueue* queue);

// Free the memory used by the queue
void tokens_free(TokensQueue* queue);

#endif