#include "SyntaxTree.h"
#include <stdlib.h>
#include <stdio.h>
#include "Queue.h"
#include "ErrorHandler.h"

#pragma warning(disable:4996)

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

}


char* ast_to_string(SyntaxTree* tree) {
    if (!tree) return strdup("");
    if (tree->type == TERMINAL_TYPE)
        return strdup(tree->info.terminal_info.token.lexeme);

    int cap = 128, len = 0;
    char* buf = malloc(cap);
    if (!buf) handle_out_of_memory_error();

    int n = tree->info.nonterminal_info.num_of_children;
    for (int i = 0; i < n; i++) {
        char* child = ast_to_string(tree->info.nonterminal_info.children[i]);
        int need = len + strlen(child) + (i < n - 1 ? 1 : 0) + 1;
        if (need > cap) {
            while (cap < need) cap *= 2;
            buf = realloc(buf, cap);
            if (!buf) handle_out_of_memory_error();
        }
        memcpy(buf + len, child, strlen(child));
        len += strlen(child);
        if (i < n - 1) buf[len++] = ' ';
        free(child);
    }
    buf[len] = '\0';
    return buf;
}
