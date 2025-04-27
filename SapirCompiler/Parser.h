#ifndef PARSER_H
#define PARSER_H
#include "Queue.h"
#include "SyntaxTree.h"

SyntaxTree* commit_parser(Queue* tokens);
int* new_int_with_allocation(int integer);

#endif