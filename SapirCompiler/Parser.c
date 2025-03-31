#include "Parser.h"
#include "LinkedList.h"
#include "ParserTableGenerator.h"
#include <stdlib.h>
#include "Queue.h"
#include "ArrayList.h"
#include <string.h>
#include <ctype.h>
#include "SyntaxTree.h"
#include <stdbool.h>
#include <stdio.h>

#pragma warning(disable:4996)


static const int ZERO = 0;

char* actiontypetostring2(int action) {
    switch (action)
    {
    case ACCEPT: return "A";
    case REDUCE: return "R";
    case SHIFT:  return "S";
    case ERROR:  return "E";

    default:
        return NULL;
    }
}

void print_actioncell(ActionCell* cell) {
    printf("%s%d", actiontypetostring2(cell->type), cell->value);
}

void printINT2(int* in) {
    printf("%d", *in);
}

void printTOKEN2(Token* token) {
    printf("Y:%d L:%s", token->type, token->lexeme);
}

static void printAST(SyntaxTree **t) {
    SyntaxTree* tree = *t;
    if (tree == NULL) return;
    if (tree->type == NONTERMINAL_TYPE) {
        printf("%s:%d", tree->info.nonterminal_info.nonterminal, tree->info.nonterminal_info.num_of_children);
    }
    else exit(-7);
    
}

static void reverse_type(SyntaxTree** arr, int n, NodeType type) {
    bool found2 = true;
    for (int i = 0, j = n - 1; i < j; found2 = true) {
        SyntaxTree* low = arr[i];
        SyntaxTree* high = arr[j];
        if (low->type != type) {
            i++;
            found2 = false;
        }
        if (high->type != type) {
            j--;
            found2 = false;
        }
        if (!found2) continue;

        arr[i++] = high;
        arr[j--] = low;
    }
}

void check_for_single_children(SyntaxTree** arr, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i]->type == NONTERMINAL_TYPE 
            && arr[i]->info.nonterminal_info.num_of_children == 1) {
            SyntaxTree* temp = arr[i]->info.nonterminal_info.children[0];
            free(arr[i]);
            arr[i] = temp;
        }
    }
}

static SyntaxTree* new_nonterminal_node(Rule* reduce_rule) {
    SyntaxTree* node = malloc(sizeof(SyntaxTree));
    node->type = NONTERMINAL_TYPE;
    node->info.nonterminal_info = (struct NonterminalType){
        .nonterminal = reduce_rule->nonterminal,
        .num_of_children = reduce_rule->ruleTerminalCount,
        .children = calloc(reduce_rule->ruleTerminalCount, sizeof(SyntaxTree*)) };
    return node;
}

static SyntaxTree* new_terminal_node(Token token) {
    SyntaxTree* node = malloc(sizeof(SyntaxTree));
    node->type = TERMINAL_TYPE;
    node->info.terminal_info.token = token;
    return node;
}

#define GET_CURRENT_STATE *(int*)linkedlist_peek(States)
#define GET_NEXT_ACTION_COL associationArray[((Token*)queue_peek(tokens))->type]
#define GET_NEXT_GOTO_COL(rule) rule->nonterminal_position

#define GET_NEXT_ACTION \
    actionTable[GET_CURRENT_STATE][GET_NEXT_ACTION_COL]

#define GET_NEXT_GOTO(rule) \
    gotoTable[GET_CURRENT_STATE][GET_NEXT_GOTO_COL(rule)]


SyntaxTree* commit_parser(Queue* tokens) {
	LinkedList* States = linkedlist_init(sizeof(unsigned int));
    LinkedList* prev_tokens = linkedlist_init(sizeof(Token));
    LinkedList* prev_nodes = linkedlist_init(sizeof(SyntaxTree*));

    linkedlist_push(States, &ZERO);
    
    ActionCell current_action;

    bool loop = true;
    while (loop) {
        current_action = GET_NEXT_ACTION;
        linkedlist_print(States, printINT2);

        switch (current_action.type) {

        case ERROR:
            printf("ERROR");
            loop = false;
            exit(-1);
            break;

        case ACCEPT:
            printf("ACCEPT");
            loop = false;
            break;

        case SHIFT:
            linkedlist_push(States, &current_action.value);
            linkedlist_push(prev_tokens, queue_dequeue(tokens));
            break;

        case REDUCE:
            Rule* reduce_rule = arraylist_get(rules, current_action.value);
            SyntaxTree* new_node = new_nonterminal_node(reduce_rule);

            char* content = strdup(reduce_rule->ruleContent);
            char* token = strtok(content, " ");

            for (int i = 0; i < reduce_rule->ruleTerminalCount; i++) {
                linkedlist_pop(States);

                if (!isNonterminal(token)) {
                    Token* temp = linkedlist_pop(prev_tokens);
                    new_node->info.nonterminal_info.children[i] = new_terminal_node(*temp);
                }
                else {
                    new_node->info.nonterminal_info.children[i] = *(SyntaxTree**)linkedlist_pop(prev_nodes);
                }

                token = strtok(NULL, " ");
            }

            free(content);


            reverse_type(new_node->info.nonterminal_info.children,
                new_node->info.nonterminal_info.num_of_children,
                NONTERMINAL_TYPE);

            reverse_type(new_node->info.nonterminal_info.children,
                new_node->info.nonterminal_info.num_of_children,
                TERMINAL_TYPE);

            check_for_single_children(new_node->info.nonterminal_info.children, new_node->info.nonterminal_info.num_of_children);


            linkedlist_push(prev_nodes, &new_node);
            linkedlist_push(States, &GET_NEXT_GOTO(reduce_rule));
            break;
        default:
            printf("INVALID ACTION!");
            exit(-1);
        }
    }
    SyntaxTree* tree = NULL;
    if (linkedlist_peek(prev_nodes))
        tree = *(SyntaxTree**)linkedlist_peek(prev_nodes);

    printf("\n");
    linkedlist_print(prev_tokens, printTOKEN2);
    linkedlist_print(prev_nodes, printAST);
    if (tree)
        print_tree_with_ranks(tree);
    printf("\n");
    if (tree)
        print_tree_postorder(tree);
    printf("\n");
    printf("\nENDED IN: ");
    print_actioncell(&current_action);
    printf("\nEND PARSER\n\n");

    return tree;
}