#include "Tokens.h"

#include <stdlib.h>
#include <stdio.h>

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
    new_token->lexeme = lexeme;
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
        [TOKEN_IDENTIFIER] = "IDENTIFIER",
        [TOKEN_NUMBER] = "NUMBER",
        [TOKEN_OPERATOR] = "OPERATOR",
        [TOKEN_KEYWORD] = "KEYWORD",
        [TOKEN_STRING] = "String Literal",
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
