#ifndef LEXER_H
#define LEXER_H

#include "Queue.h"

typedef enum {
    START = 0,
    IDENTIFIER,
    NUMBER,
    OPERATOR,
    STRING_LITERAL,
    COMMENT,
    SEPARATOR,
    KEYWORD,
    ERROR,
    NUM_STATES,
} LEXER_STATE;

Queue* tokenize(const char* input);

#endif