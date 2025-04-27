#include "SyntaxTree.h"
#include <stdlib.h>
#include <stdio.h>
#include "Queue.h"
#include "ErrorHandler.h"

#pragma warning(disable:4996)


SyntaxTree* create_terminal_node(Token token) {
	SyntaxTree* node = malloc(sizeof(SyntaxTree));
	if (node == NULL) {
		handle_out_of_memory_error();
		return NULL;
	}
	node->type = TERMINAL_TYPE;
	node->info.terminal_info.token = token;
	return node;
}

SyntaxTree* create_nonterminal_node(char* nonterminal, int num_of_children) {
	SyntaxTree* node = malloc(sizeof(SyntaxTree));
	if (node == NULL) {
		handle_out_of_memory_error();
		return NULL;
	}
	node->type = NONTERMINAL_TYPE;
	node->info.nonterminal_info.nonterminal = nonterminal;
	node->info.nonterminal_info.num_of_children = num_of_children;
	node->info.nonterminal_info.children = calloc(num_of_children, sizeof(SyntaxTree*));
	if (node->info.nonterminal_info.children == NULL) {
		handle_out_of_memory_error();
		free(node);
		return NULL;
	}
    return node;
}
void free_nonterminal_tree(SyntaxTree* nonterminal_tree) {
    int number_of_children = nonterminal_tree->info.nonterminal_info.num_of_children;
    SyntaxTree** children = nonterminal_tree->info.nonterminal_info.children;
    for (int i = 0; i < number_of_children; i++) {
        if (children[i]) free_Syntax_tree(children[i]);
    }
    free(children);
    free(nonterminal_tree);
}

void free_terminal_tree(SyntaxTree* terminal_tree) {
    free(terminal_tree);
}


void free_Syntax_tree(SyntaxTree* tree) {
    if (tree == NULL) return;
    if (tree->type == NONTERMINAL_TYPE)
        free_nonterminal_tree(tree);
    else if (tree->type == TERMINAL_TYPE)
        free_terminal_tree(tree);
}



typedef struct {
    SyntaxTree* node;
    int rank;
} RankedNode;


void print_tree_with_ranks(SyntaxTree* root) {
    if (root == NULL) return;
    printf("\nRANKS:\n");

    Queue* q = queue_init(sizeof(RankedNode));

    RankedNode rootRank = { root, 0 };
    queue_enqueue(q, &rootRank);

    while (q->size != 0) {
        RankedNode current;
        RankedNode* temp = queue_dequeue(q);
        if (temp == NULL || temp->node == NULL) return;
        current = *temp;
        if (current.node->type == NONTERMINAL_TYPE) {
            printf("Rank %d NONTERMINAL: %s\n", current.rank, current.node->info.nonterminal_info.nonterminal);

            for (int i = 0; i < current.node->info.nonterminal_info.num_of_children; i++) {
                RankedNode child = { current.node->info.nonterminal_info.children[i], current.rank + 1 };
                queue_enqueue(q, &child);
            }
        }
        else {
            printf("Rank %d TERMINAL: %s\n", current.rank, current.node->info.terminal_info.token.lexeme);
        }
    }
    printf("\n");

}


void print_tree_postorder(SyntaxTree* tree) {
    if (tree == NULL) return;

    if (tree->type == NONTERMINAL_TYPE) {
        for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++)
            print_tree_postorder(tree->info.nonterminal_info.children[i]);
        printf("NONTERMINAL: %s\n", tree->info.nonterminal_info.nonterminal);
    }
    else {
        printf("TERMINAL: %s\n", tree->info.terminal_info.token.lexeme);
    }
}



void print_tree_preorder(SyntaxTree* tree) {

    if (tree == NULL) return;
    if (tree->type == NONTERMINAL_TYPE) {
        printf("NONTERMINAL: %s\n", tree->info.nonterminal_info.nonterminal);
        for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++)
            print_tree_postorder(tree->info.nonterminal_info.children[i]);
    }
    else {
        printf("TERMINAL: %s\n", tree->info.terminal_info.token.lexeme);
    }
    printf("\n");

}