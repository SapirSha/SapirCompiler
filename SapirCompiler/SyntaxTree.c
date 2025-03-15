#include "SyntaxTree.h"
#include <stdlib.h>
#include <stdio.h>
#include "Queue.h"

typedef struct {
    SyntaxTree* node;
    int rank;
} RankedNode;

void print_tree_with_ranks(SyntaxTree* root) {
    if (root == NULL) return;

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
