#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include "Tokens.h"
typedef struct SyntaxTree;


typedef enum {
	NONTERMINAL_TYPE,
	TERMINAL_TYPE,
} NodeType;

struct TerminalType {
	Token token;
};

struct NonterminalType {
	char* nonterminal;
	struct SyntaxTree** children;
	int num_of_children;
};

typedef struct SyntaxTree {
	NodeType type;
	union {
		struct TerminalType terminal_info;
		struct NonterminalType nonterminal_info;
	} info;
} SyntaxTree;


void print_tree_with_ranks(SyntaxTree* tree);
void print_tree_postorder(SyntaxTree* tree);


#endif