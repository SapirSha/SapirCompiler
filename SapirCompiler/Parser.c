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
#include "ErrorHandler.h"
#include "HashMap.h"
#include "HashSet.h"

#pragma warning(disable:4996)


static const int ZERO = 0;

char* actiontypetostring2(int action) {
    switch (action)
    {
    case ACCEPT: return "A";
    case REDUCE: return "R";
    case SHIFT:  return "S";
    case ERROR_ACTION:  return "E";

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

void printTOKEN2(Token** token) {
    printf("Y:%d L:%s", (*token)->type, (*token)->lexeme);
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
    if (!node) handle_out_of_memory_error();

    SyntaxTree* children = calloc(reduce_rule->ruleTerminalCount, sizeof(SyntaxTree*));
    if (!children) handle_out_of_memory_error();

    node->type = NONTERMINAL_TYPE;
    node->info.nonterminal_info = (struct NonterminalType){
        .nonterminal = reduce_rule->nonterminal,
        .num_of_children = reduce_rule->ruleTerminalCount,
        .children =  children};
    return node;
}

static SyntaxTree* new_terminal_node(Token token) {
    SyntaxTree* node = malloc(sizeof(SyntaxTree));
    if (!node) handle_out_of_memory_error();

    node->type = TERMINAL_TYPE;
    node->info.terminal_info.token = token;
    return node;
}

void error_action(int state_accured, Token* latest_token, Token* next_token) {
    int terminals_length = terminalsList->size;
    int nonterminals_length = nonterminalsList->size;

    ArrayList* allowed_tokens = arraylist_init(sizeof(char*), 10);
    ArrayList* allowed_statements = arraylist_init(sizeof(char*), 10);

    for (int i = 0; i < terminals_length; i++) {
        if (actionTable[state_accured][i].type != ERROR_ACTION)
            arraylist_add(allowed_tokens, terminalsList->array[i]);
    }

    for (int i = 0; i < nonterminals_length; i++) {
        if (gotoTable[state_accured][i] != -1)
            arraylist_add(allowed_statements, nonterminalsList->array[i]);
    }

    handle_parser_error(state_accured, latest_token, next_token, allowed_tokens, allowed_statements);
}

#define GET_CURRENT_STATE *(int*)linkedlist_peek(States)
#define GET_NEXT_ACTION_COL associationArray[((Token*)queue_peek(tokens))->type]
#define GET_NEXT_GOTO_COL(rule) rule->nonterminal_position

#define GET_NEXT_ACTION \
    actionTable[GET_CURRENT_STATE][GET_NEXT_ACTION_COL]

#define GET_NEXT_GOTO(rule) \
    gotoTable[GET_CURRENT_STATE][GET_NEXT_GOTO_COL(rule)]

bool panic_tokens[] = { TOKEN_EOF, TOKEN_RBRACES };

#define NUM_OF_PANIC_STATES 4
int panic_states[NUM_OF_PANIC_STATES] = { 0 ,1, 2, 3 };

static bool is_panic_token(Token* tk) {
    return panic_tokens[tk->type];
}

static bool is_panic_state(int state_num) {
    for (int i = 0; i < NUM_OF_PANIC_STATES; i++) {
        if (panic_states[i] == state_num) return true;
    }
    return false;
}

void free_syntax_tree(SyntaxTree* tree) {
    if (!tree) return;
    if (tree->type == TERMINAL_TYPE)
    {
        free(tree->info.terminal_info.token.lexeme);
        free(tree);
    }
    else {
        for (int i = 0; i < tree->info.nonterminal_info.num_of_children; i++)
            free_syntax_tree(tree->info.nonterminal_info.children[i]);
        free(tree);
    }
}

SyntaxTree* commit_parser(Queue* tokens) {
	LinkedList* States = linkedlist_init(sizeof(unsigned int));
    LinkedList* prev_tokens = linkedlist_init(sizeof(Token*));
    LinkedList* prev_nodes = linkedlist_init(sizeof(SyntaxTree*));
    char* latest_nonterminal = NULL;

    linkedlist_push(States, &ZERO);
    
    ActionCell current_action;

    bool loop = true;
    while (loop) {
        current_action = GET_NEXT_ACTION;

        switch (current_action.type) {

        case ERROR_ACTION:
            loop = false;

            Token* temp;
            if (prev_tokens->size == 0)
                temp = &(Token) { .row = 0, .col = 0, .lexeme = "START" };
            else
                temp = *(Token**)linkedlist_peek(prev_tokens);

            error_action(
                *(int*)linkedlist_peek(States),
                temp,
                ((Token*)queue_peek(tokens)));

            break;
        case ACCEPT:
            printf("ACCEPT");
            loop = false;
            break;

        case SHIFT:
            linkedlist_push(States, &current_action.value);
            Token* tk = queue_dequeue(tokens);
            linkedlist_push(prev_tokens, &tk);
            break;

        case REDUCE:
            printf("");
            Rule* reduce_rule = arraylist_get(rules, current_action.value);
            SyntaxTree* new_node = new_nonterminal_node(reduce_rule);

            char* content = strdup(reduce_rule->ruleContent);
            char* token = strtok(content, " ");

            for (int i = 0; i < reduce_rule->ruleTerminalCount; i++) {
                free(linkedlist_pop(States));
                if (current_error_state == NO_ERROR) {
                    if (!isNonterminal(token)) {
                        Token* temp = *(Token**)linkedlist_pop(prev_tokens);
                        new_node->info.nonterminal_info.children[i] = new_terminal_node(*temp);
                        free(temp);
                    }
                    else {
                        new_node->info.nonterminal_info.children[i] = *(SyntaxTree**)linkedlist_pop(prev_nodes);
                    }
                }
                token = strtok(NULL, " ");
            }

            latest_nonterminal = new_node->info.nonterminal_info.nonterminal;
            free(content);

            if (current_error_state == NO_ERROR) {
                reverse_type(new_node->info.nonterminal_info.children,
                    new_node->info.nonterminal_info.num_of_children,
                    NONTERMINAL_TYPE);

                reverse_type(new_node->info.nonterminal_info.children,
                    new_node->info.nonterminal_info.num_of_children,
                    TERMINAL_TYPE);

                check_for_single_children(new_node->info.nonterminal_info.children, new_node->info.nonterminal_info.num_of_children);

                linkedlist_push(prev_nodes, &new_node);
                linkedlist_push(States, &GET_NEXT_GOTO(reduce_rule));
            }
            break;
        default:
            exit(-1);
        }
    }

    if (current_error_state != NO_ERROR) {
        while (prev_nodes->size != 0) {
            free_syntax_tree(*(SyntaxTree**)linkedlist_peek(prev_nodes));
            free(linkedlist_pop(prev_nodes));
        }
    }

    SyntaxTree* tree = NULL;
    if (linkedlist_peek(prev_nodes))
        tree = *(SyntaxTree**)linkedlist_peek(prev_nodes);

    linkedlist_free(States, &free);
    linkedlist_free(prev_tokens, &free);

    free_parser_table();
    free_non_and_terminals();
    free_rules();
    free_follow();

    return tree;
}