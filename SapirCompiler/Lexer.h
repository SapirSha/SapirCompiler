#ifndef LEXER_H
#define LEXER_H

#include "Queue.h"
#include "LexerFsm.h"

#define MAX_TOKEN_LENGTH 48

Queue* tokenize(const char* input);

#endif