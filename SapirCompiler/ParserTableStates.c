#include "ParserTableGenerator.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#pragma warning(disable:4996)

/*
A function that determines if a symbol is nonterminal:
 *  A symbol is a nonterminal if his first letter is uppercase
*/
bool isNonterminal(const char* symbol) {
    return (symbol && isupper(symbol[0]));
}

/*
A function that counts the number of symbols in the rule
 * symbols are separated by spaces (' ')
*/
static int count_symbols(const char* ruleContent) {
    int count = 0;
    bool inToken = false;
    while (*ruleContent != '\0') {
        if (!isspace((char)*ruleContent) && !inToken) {
            count++;
            inToken = true;
        }
        else if (isspace((char)*ruleContent)) {
            inToken = false;
        }
        ruleContent++;
    }
    return count;
}

/*
A function that adds a rule into the rules array list, that represents the BNF
 * Nonterminals first letter is uppercase
*/
void add_rule(const char* nonterminal, const char* content) {
    Rule rule;
    rule.nonterminal = strdup(nonterminal);
    rule.ruleContent = strdup(content);
    rule.ruleTerminalCount = count_symbols(content);
    rule.ruleID = rules->size;
    arraylist_add(rules, &rule);
}

/*
A function that returns the token in position n in the input
 * Position is the number of symbols passed, and not character position in the string
*/
static char* get_nth_token(const char* s, int n) {
    int currentToken = 0;
    char* p = s;
    while (*p) {
        while (*p && isspace((char)*p))
            p++;
        if (*p == '\0') break;

        if (currentToken == n) {
            char* start = p;

            while (*p != '\0' && !isspace((char)*p))
                p++;

            int len = (p - start) / sizeof(char);
            char* token = malloc(len + 1);
            strncpy(token, start, len);
            token[len] = '\0';

            return token;
        }
        else {
            while (*p && !isspace((char)*p))
                p++;

            currentToken++;
        }
    }
    return NULL;
}

/*
 A function that returns the symbol after the dot in an LRItem
*/
static char* get_next_symbol(LRItem* item) {
    return get_nth_token(item->rule->ruleContent, item->dot);
}

/*
 A function that checks if a state has a certain LR Item inside it
 * LRItems items with the same rules can be different if they have different dot position
*/
static bool state_contains_item(State* s, LRItem* item) {
    for (int i = 0; i < s->items->size; i++) {
        LRItem* existing = arraylist_get(s->items, i);
        if (existing->rule == item->rule && existing->dot == item->dot)
            return true;
    }
    return false;
}

/*
 A function that commits closure for a certain state:
 * If a state can get a nonterminal, it can also get the terminals that its made of.
   In this function, we add to the current state all the rules of the nonterminals that can be got,
   with dot at zero (start of a new rule)
*/
static void closure(State* s) {
    bool added;
    added = false;
    // go through every lr item in the state
    for (int i = 0; i < s->items->size; i++) {
        LRItem* item = arraylist_get(s->items, i);
        char* symbol = get_next_symbol(item);

        // if the lr item indicates that a nonterminal can be gotten
        if (symbol != NULL && isNonterminal(symbol)) {
            // search for all the rules where the nonterminal is the left item, and add them to the state
            for (int j = 0; j < rules->size; j++) {
                Rule* ruleCandidate = (Rule*)rules->array[j];
                if (strcmp(ruleCandidate->nonterminal, symbol) == 0) {
                    LRItem newItem = { .rule = ruleCandidate, .dot = 0 };
                    if (!state_contains_item(s, &newItem)) { // add only if not already in state
                        arraylist_add(s->items, &newItem);
                        added = true;
                    }
                }
            }
        }
        free(symbol);
    }
}
/*
 This is the function that creates the new states:
 * This function create a state for an allowed symbol in the state,
 * It gets all the possibilities in the previous state to the new state and not one
*/
static State* goto_state(State* s, const char* symbol) {
    State* newState = malloc(sizeof(State));
    newState->items = arraylist_init(sizeof(LRItem), DEFAULT_AMOUNT_OF_LR_ITEMS);

    // find all the rules in the previous state that have 'symbol' as an allowed possibility, and add them
    for (int i = 0; i < s->items->size; i++) {
        LRItem item = *(LRItem*)arraylist_get(s->items, i);
        char* next = get_next_symbol(&item);

        if (next != NULL && strcmp(next, symbol) == 0) {
            LRItem advanced = item;
            advanced.dot++; // advance the dot

            if (!state_contains_item(newState, &advanced)) {
                arraylist_add(newState->items, &advanced);
            }
        }
        free(next);
    }

    closure(newState);
    return newState;
}

bool state_equals(State* s1, State* s2) {
    if (s1->items->size != s2->items->size)
        return false;
    for (int i = 0; i < s1->items->size; i++) {
        bool found = false;
        LRItem* item1 = arraylist_get(s1->items, i);
        for (int j = 0; j < s2->items->size; j++) {
            LRItem* item2 = arraylist_get(s2->items, j);
            if (item1->rule == item2->rule && item1->dot == item2->dot) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

int find_state(State* s) {
    for (int i = 0; i < states->size; i++) {
        if (state_equals(arraylist_get(states, i), s))
            return i;
    }
    return -1;
}

/*
 Builds the states
*/
void build_states(const char* startNonterminal) {
    states = arraylist_init(sizeof(State), DEFAULT_NUMBER_OF_STATES);

    int len = strlen(startNonterminal) + 4;
    char* buffer = malloc(len);
    snprintf(buffer, len, "%s $", startNonterminal);

    // add an initial rule for the tables to start at, and add startnonterminal as its content
    add_rule("START'", buffer);
    free(buffer);

    State* State0 = malloc(sizeof(State));
    State0->items = arraylist_init(sizeof(LRItem), DEFAULT_AMOUNT_OF_LR_ITEMS);
    LRItem startItem = { .rule = ((Rule*)rules->array[rules->size - 1]), .dot = 0 };
    arraylist_add(State0->items, &startItem);


    closure(State0);

    arraylist_add(states, State0);

    // go through all the states (states size increases)
    for (int i = 0; i < states->size; i++) {
        State* s = arraylist_get(states, i);

        // list of possible symbols to get in the state
        ArrayList* symbolList = arraylist_init(sizeof(char*), 25);
        for (int j = 0; j < s->items->size; j++) {
            char* sym = get_next_symbol(arraylist_get(s->items, j));

            if (sym != NULL) {
                bool exists = false;

                // check if symbol already exists
                for (int k = 0; k < symbolList->size; k++) {
                    if (strcmp(sym, *(char**)arraylist_get(symbolList, k)) == 0) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    char* temp = strdup(sym);
                    arraylist_add(symbolList, &temp);
                }
                free(sym);
            }
        }

        // go through all the possible symbols that can be got in the current state
        for (int k = 0; k < symbolList->size; k++) {
            char* symbol = *(char**)arraylist_get(symbolList, k);

            // commit goto
            State* g = goto_state(s, symbol);
            if (g->items->size == 0) {
                free(g);
                continue;
            }


            int idx = find_state(g);
            // if states doesnt already exist add to list of states
            if (idx == -1) {
                arraylist_add(states, g);
            }
            else {
                free(g);
            }
        }
        arraylist_free(symbolList);
    }
}


void print_state(State* s, int index) {
    printf("State %d:\n", index);
    for (int i = 0; i < s->items->size; i++) {
        LRItem* item = arraylist_get(s->items, i);
        printf("  %s -> ", item->rule->nonterminal);

        char* contentCopy = strdup(item->rule->ruleContent);
        char* token = strtok(contentCopy, " ");
        int pos = 0;
        while (token != NULL) {
            if (pos == item->dot)
                printf(". ");
            printf("%s ", token);
            pos++;
            token = strtok(NULL, " ");
        }
        if (item->dot >= pos)
            printf(". ");
        printf("\n");
        free(contentCopy);
    }
}

void print_rules() {
    for (int i = 0; i < rules->size; i++) {
        Rule* r = (Rule*)rules->array[i];
        printf("Rule %d: %s -> %s (length: %d) - pos %d\n", r->ruleID, r->nonterminal, r->ruleContent, r->ruleTerminalCount, r->nonterminal_position);
    }
}

void add_rules() {
    add_rule("PROGRAM", "STATEMENTS");
    add_rule("STATEMENTS", "STATEMENTS STATEMENT");
    add_rule("STATEMENTS", "STATEMENT");

    add_rule("STATEMENT", "VARIABLE_ASSIGNMENT_STATEMENT");
    add_rule("STATEMENT", "VARIABLE_DECLARATION_STATEMENT");
    add_rule("STATEMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT");
    add_rule("STATEMENT", "IF_STATEMENT");
    add_rule("STATEMENT", "WHILE_STATEMENT");
    add_rule("STATEMENT", "DO_WHILE_STATEMENT");
    add_rule("STATEMENT", "FOR_STATEMENT");
    add_rule("STATEMENT", "PRINT_STATEMENT");
    add_rule("STATEMENT", "GET_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_CALL_STATEMENT");
    add_rule("STATEMENT", "RETURN_STATEMENT");

    add_rule("CONDITION_LIST", "CONDITION");
    add_rule("CONDITION_LIST", "CONDITION_LIST && CONDITION");
    add_rule("CONDITION_LIST", "CONDITION_LIST || CONDITION");

    add_rule("CONDITION", "( CONDITION_LIST )");
    add_rule("CONDITION", "EXPRESSION == EXPRESSION");
    add_rule("CONDITION", "EXPRESSION != EXPRESSION");
    add_rule("CONDITION", "EXPRESSION > EXPRESSION");
    add_rule("CONDITION", "EXPRESSION >= EXPRESSION");
    add_rule("CONDITION", "EXPRESSION < EXPRESSION");
    add_rule("CONDITION", "EXPRESSION <= EXPRESSION");


    add_rule("EXPRESSION", "EXPRESSION + TERM");
    add_rule("EXPRESSION", "EXPRESSION - TERM");
    add_rule("EXPRESSION", "TERM");

    add_rule("TERM", "TERM % FACTOR");
    add_rule("TERM", "TERM * FACTOR");
    add_rule("TERM", "TERM / FACTOR");
    add_rule("TERM", "FACTOR");

    add_rule("FACTOR", "( EXPRESSION )");
    add_rule("FACTOR", "identifier");
    add_rule("FACTOR", "number");
    add_rule("FACTOR", "FUNCTION_CALL_STATEMENT");


    add_rule("VARIABLE_TYPE", "int");
    add_rule("VARIABLE_TYPE", "char");
    add_rule("VARIABLE_TYPE", "string");
    add_rule("VARIABLE_TYPE", "float");
    add_rule("VARIABLE_TYPE", "bool");


    add_rule("BLOCK", "{ STATEMENTS }");
    add_rule("BLOCK", "{ }");

    add_rule("IF_STATEMENT", "if CONDITION_LIST IF_BLOCK");
    add_rule("IF_STATEMENT", "if CONDITION_LIST IF_BLOCK else ELSE_BLOCK ");
    add_rule("IF_STATEMENT", "if CONDITION_LIST IF_BLOCK else IF_STATEMENT ");

    add_rule("IF_BLOCK", "BLOCK");
    add_rule("IF_BLOCK", "STATEMENT");
    add_rule("ELSE_BLOCK", "BLOCK");
    add_rule("ELSE_BLOCK", "STATEMENT");

    add_rule("VARIABLE_DECLARATION_STATEMENT", "VARIABLE_TYPE identifier");
    add_rule("VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", "VARIABLE_TYPE identifier = EXPRESSION");

    add_rule("VARIABLE_ASSIGNMENT_STATEMENT", "identifier = EXPRESSION");

    add_rule("WHILE_STATEMENT", "while CONDITION_LIST BLOCK");
    add_rule("WHILE_STATEMENT", "while CONDITION_LIST BLOCK change CHANGE_BLOCK");

    add_rule("DO_WHILE_STATEMENT", "do BLOCK while CONDITION_LIST");

    add_rule("FOR_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST FOR_BLOCK change CHANGE_BLOCK");
    add_rule("FOR_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST change CHANGE_BLOCK FOR_BLOCK");
    add_rule("FOR_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST FOR_BLOCK");

    add_rule("FOR_ASSIGNMENT", "VARIABLE_ASSIGNMENT_STATEMENT");
    add_rule("FOR_ASSIGNMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT");

    add_rule("FOR_BLOCK", "BLOCK");
    add_rule("FOR_BLOCK", "STATEMENT");

    add_rule("CHANGE_BLOCK", "VARIABLE_ASSIGNMENT_STATEMENT");

    add_rule("PRINT_STATEMENT", "print EXPRESSION");
    add_rule("GET_STATEMENT", "get VARIABLE_DECLARATION_STATEMENT");
    add_rule("GET_STATEMENT", "get identifier");

    add_rule("FUNCTION_DECLARATION_STATEMENT", "function identifier gets PARAMETER_LIST returns VARIABLE_TYPE FUNCTION_BLOCK");
    add_rule("FUNCTION_DECLARATION_STATEMENT", "function identifier gets PARAMETER_LIST FUNCTION_BLOCK");
    add_rule("FUNCTION_DECLARATION_STATEMENT", "function identifier returns VARIABLE_TYPE FUNCTION_BLOCK");
    add_rule("FUNCTION_DECLARATION_STATEMENT", "function identifier FUNCTION_BLOCK");


    add_rule("PARAMETER_LIST", "PARAMETER_LIST , PARAMETER");
    add_rule("PARAMETER_LIST", "PARAMETER");
    add_rule("PARAMETER", "VARIABLE_TYPE identifier");

    add_rule("FUNCTION_BLOCK", "BLOCK");
    add_rule("FUNCTION_BLOCK", "STATEMENT");

    add_rule("RETURN_STATEMENT", "return EXPRESSION");

    add_rule("FUNCTION_CALL_STATEMENT", "call identifier");
    add_rule("FUNCTION_CALL_STATEMENT", "call identifier with ARGUMENT_LIST");

    add_rule("ARGUMENT_LIST", "ARGUMENT_LIST , EXPRESSION");
    add_rule("ARGUMENT_LIST", "EXPRESSION");
}


void init_states() {
    rules = arraylist_init(sizeof(Rule), DEFAULT_NUMBER_OF_RULES);

    add_rules();


    build_states("PROGRAM");

    for (int i = 0; i < states->size; i++) {
        print_state(arraylist_get(states, i), i);
        printf("\n");
    }
}