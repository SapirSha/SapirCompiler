#ifndef LEXER_H
#define LEXER_H

#include "Queue.h"
#include "LexerFsm.h"

#define MAX_TOKEN_LENGTH 48

typedef enum {
    START = 0,
    OPERATOR,
    SEPARATOR,
    KEYWORD,
    NUM_STATES = 100,
} LEXER_STATE;

Queue* tokenize(const char* input);

#endif