#ifndef LEXER_H
#define LEXER_H

#include "Queue.h"
#include "LexerFsm.h"

#define MAX_TOKEN_LENGTH 64

Queue* tokenize(char* input);

#endif