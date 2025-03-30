#ifndef PARSER_H
#define PARSER_H

#include "Queue.h"
#include "SyntaxTree.h"

SyntaxTree* syntax_tree;
SyntaxTree* commit_parser(Queue* tokens);

#endif