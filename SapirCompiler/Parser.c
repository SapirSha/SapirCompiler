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

static void reverse_nonterminals(SyntaxTree** arr, int n) {
    bool found2 = true;
    for (int i = 0, j = n - 1; i < j; found2 = true) {
        SyntaxTree* low = arr[i];
        SyntaxTree* high = arr[j];
        if (low->type != NONTERMINAL_TYPE) {
            i++;
            found2 = false;
        }
        if (high->type != NONTERMINAL_TYPE) {
            j--;
            found2 = false;
        }
        if (!found2) continue;
        
        arr[i++] = high;
        arr[j--] = low;
    }
}

static void reverse_terminals(SyntaxTree** arr, int n) {
    bool found2 = true;
    for (int i = 0, j = n - 1; i < j; found2 = true) {
        SyntaxTree* low = arr[i];
        SyntaxTree* high = arr[j];
        if (low->type != TERMINAL_TYPE) {
            i++;
            found2 = false;
        }
        if (high->type != TERMINAL_TYPE) {
            j--;
            found2 = false;
        }
        if (!found2) continue;

        arr[i++] = high;
        arr[j--] = low;
    }
}

#define NextAction \
    actionTable[*(int*)linkedlist_peek(States)][associationArray[((Token*)queue_peek(tokens))->type]]

void commit_parser(Queue* tokens) {
	LinkedList* States = linkedlist_init(sizeof(unsigned int));
    LinkedList* prev_tokens = linkedlist_init(sizeof(Token));
    LinkedList* prev_nodes = linkedlist_init(sizeof(SyntaxTree*));

    queue_print(tokens, printTOKEN2);

    linkedlist_push(States, &ZERO);
    
    ActionCell current = NextAction;
    linkedlist_push(prev_tokens, queue_dequeue(tokens));

    linkedlist_push(States, &current.value);
    
    print_actioncell(&current);
    printf(" < --- Doing This\n ");

    current = NextAction;
    while (current.type != ERROR && current.type != ACCEPT) {
        linkedlist_print(States, printINT2);

        
        printf(" < --- Doing This\n ");

        if (current.type == REDUCE) {
            Rule* reduce_rule = arraylist_get(rules, current.value);
            printf("REDUCE RULE %d\n", current.value);
            printf("REDUCE COUNT: %d\n", reduce_rule->ruleTerminalCount);

            SyntaxTree* temp = malloc(sizeof(SyntaxTree));
            temp->type = NONTERMINAL_TYPE;
            temp->info.nonterminal_info = (struct NonterminalType){ 
                .nonterminal = reduce_rule->nonterminal, 
                .num_of_children = reduce_rule->ruleTerminalCount, 
                .children = calloc( reduce_rule->ruleTerminalCount, sizeof(SyntaxTree*))};


            printf("RULE CONTENT: \t\t");

            char* content = strdup(reduce_rule->ruleContent);
            char* token = strtok(content, " ");

            

            for (int i = 0; i < reduce_rule->ruleTerminalCount; i++) {
                *(int*)linkedlist_pop(States);
                printf("(%s->", token);
                if (!isupper(token[0])) {
                    Token* t = linkedlist_pop(prev_tokens);
                    printTOKEN2(t);
                    temp->info.nonterminal_info.children[i] =
                        malloc(sizeof(SyntaxTree));
                    *(temp->info.nonterminal_info.children[i]) = 
                        (SyntaxTree){.type = TERMINAL_TYPE, .info.terminal_info.token = *t};
                }
                else {
                    temp->info.nonterminal_info.children[i] = *(SyntaxTree**)linkedlist_pop(prev_nodes);
                }
                printf(")\t");


                token = strtok(NULL, " ");

            }
            printf("\n");


            reverse_nonterminals(temp->info.nonterminal_info.children, temp->info.nonterminal_info.num_of_children);
            reverse_terminals(temp->info.nonterminal_info.children, temp->info.nonterminal_info.num_of_children);


            linkedlist_push(prev_nodes, &temp);

            int pos = *(int*)linkedlist_peek(States);
            printf("GOTO %d: %d %d\n", gotoTable[pos][reduce_rule->nonterminal_position], pos, reduce_rule->nonterminal_position);
            linkedlist_push(States, &gotoTable[pos][reduce_rule->nonterminal_position]);
        }
        else {
            linkedlist_push(States, &current.value);
            linkedlist_push(prev_tokens, queue_dequeue(tokens));
        }
            printf("NEXT TOKEN: %d-%s\n", ((Token*)queue_peek(tokens))->type, ((Token*)queue_peek(tokens))->lexeme);
            printf("Row: %d Col: %d\n", *(unsigned int*)linkedlist_peek(States), associationArray[((Token*)queue_peek(tokens))->type]);
            current = NextAction;
            print_actioncell(&current);
    }

    printf("\n");
    linkedlist_print(prev_tokens, printTOKEN2);
    linkedlist_print(prev_nodes, printAST);
    print_tree_with_ranks(*(SyntaxTree**)linkedlist_pop(prev_nodes));

    printf("\nENDED IN: ");
    print_actioncell(&current);
    printf("\nEND PARSER");
}