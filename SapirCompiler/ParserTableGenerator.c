#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "ParserTableGenerator.h"
#include "Tokens.h"
#include "HashMap.h"
#include "HashSet.h"
#include "LinkedList.h"

#pragma warning(disable:4996)

static ArrayList* states; // Convert states to hashset (make hashset dynamic)
static ArrayList* nonterminalsList; // HAS TO BE AN ARRAY LIST DONT CHANGE
static ArrayList* terminalsList; // HAS TO BE AN ARRAY LIST DONT CHANGE
// can maybe make an appeared symbols hashset?
static HashMap* follow;

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

void print_string_arraylist(char** sym) {
    printf("%s ", *sym);
}
void print_string(char* sym) {
    printf("%s ", sym);
}

static bool symbol_exists(ArrayList* list, const char* sym) {
    for (int i = 0; i < list->size; i++) {
        if (strcmp(*(char**)arraylist_get(list, i), sym) == 0)
            return true;
    }
    return false;
}

/*
A function that collects all the possible symbols and splits them into 2 arraylists:
* a nonterminal arraylist
* a terminal arraylist
*/
void collect_symbols() {
    nonterminalsList = arraylist_init(sizeof(char*), DEFAULT_AMOUNT_OF_NONTERMINALS);
    terminalsList = arraylist_init(sizeof(char*), DEFAULT_AMOUNT_OF_TERMINALS);

    // go through all the rules
    for (int i = 0; i < rules->size; i++) {
        Rule* r = (Rule*)rules->array[i];
        // if the rules nonterminal isnt in the list, add it
        if (!symbol_exists(nonterminalsList, r->nonterminal)) {
            char* temp = strdup(r->nonterminal);
            arraylist_add(nonterminalsList, &temp);
        }

        char* contentCopy = strdup(r->ruleContent);
        char* token = strtok(contentCopy, " ");

        while (token != NULL) {
            // for every symbol in the content's rule, check if its in the lists, if not add it
            if (!isNonterminal(token)) {
                if (!symbol_exists(terminalsList, token)) {
                    char* temp = strdup(token);
                    arraylist_add(terminalsList, &temp);
                }
            }
            else {
                if (!symbol_exists(nonterminalsList, token)) {
                    char* temp = strdup(token);
                    arraylist_add(nonterminalsList, &temp);
                }
            }
            token = strtok(NULL, " ");
        }
        free(contentCopy);
    }
}

// initialize follow structure
void init_follow() {
    follow = createHashMap(nonterminalsList->size, string_hash, string_equals);
    // for every nonterminal create a hashset that represents the possible terminals that can be after it
    for (int i = 0; i < nonterminalsList->size; i++) {
        hashmap_insert(follow, *(char**)arraylist_get(nonterminalsList, i), hashset_create(terminalsList->size, string_hash, string_equals));
    }

    hashset_insert(hashmap_get(follow, "START'"), "$");
}

bool add_rules_content_to_nonterminals_follow(char* nonterminal, Rule* rule) {
    char* next_token = get_nth_token(rule->ruleContent, 0);
    if (next_token == NULL) return;
    if (!isNonterminal(next_token)) {
        printf("ADDING to %s : %s\n", nonterminal, next_token);
        return hashset_insert(hashmap_get(follow, nonterminal), next_token);
    }
    else {
        bool changed = false;
        for (int r2 = 0; r2 < rules->size; r2++)
        {
            Rule* a_rule = arraylist_get(rules, r2);
            if (strcmp(a_rule->nonterminal, next_token) == 0) {
                changed |= add_rules_content_to_nonterminals_follow(nonterminal, a_rule);
            }
        }
        return changed;
    }
}


/*
A function that fills the follow structure
* A hashmap with nonterminals as keys and possible terminals that can appear after the nonterminal as content
*/
void compute_follow() {
    init_follow();

    bool changed = true;
    // if somthing changed you need to check if something new can be added
    while (changed) {
        changed = false;

        // go through all the rules
        for (int i = 0; i < rules->size; i++) {
            Rule* currentRule = (Rule*)rules->array[i];

            char* currentrule_nonterminal = currentRule->nonterminal;
            char* contentCopy = strdup(currentRule->ruleContent);

            // holds all the tokens that can be gotten in the state
            ArrayList* tokens = arraylist_init(sizeof(char*), 50);

            char* tok = strtok(contentCopy, " ");
            while (tok != NULL) {
                arraylist_add(tokens, &tok);
                tok = strtok(NULL, " ");
            }

            // for every symbol that is in the state
            for (int j = 0; j < tokens->size; j++) {
                char* current_symbol = *(char**)arraylist_get(tokens, j);

                if (isNonterminal(current_symbol)) {
                    // if not last symbol
                    if (j + 1 < tokens->size) {
                        char* after_symbol = *(char**)arraylist_get(tokens, j + 1);
                        // if the symbol after the nonterminal is a terminal its a follow
                        if (!isNonterminal(after_symbol)) {
                            changed |= hashset_insert(hashmap_get(follow, current_symbol), after_symbol);
                        } 
                        else {

                            // go through all the rules
                            for (int r = 0; r < rules->size; r++) {
                                Rule* candidate = (Rule*)rules->array[r];

                                // find the contents of the nonterminal after_symbol
                                if (strcmp(candidate->nonterminal, after_symbol) == 0) {
                                    char* sample = strdup(candidate->ruleContent);
                                    char* firstSym = get_nth_token(sample, 0);
                                    free(sample);
                                    // if the first symbol in the content is terminal, add as follow
                                    if (firstSym && !isNonterminal(firstSym)) {
                                        changed |= hashset_insert(hashmap_get(follow, current_symbol), firstSym);
                                        break;
                                    }
                                    // if the first symbol is nonterminal, add its follows to current
                                    else if (firstSym && isNonterminal(firstSym)) {
                                        for (int r2 = 0; r2 < rules->size; r2++)
                                        {
                                            Rule* a_rule = arraylist_get(rules, r2);
                                            if (strcmp(a_rule->nonterminal, firstSym) == 0) {
                                                changed |= add_rules_content_to_nonterminals_follow(current_symbol, a_rule);
                                            }
                                        }
                                    } 
                                }
                            }
                        }
                    }
                    else { // end of rule's content
                        // add to last symbol's follows, the current rule's follows
                        HashSet* LastSymbolSet = hashmap_get(follow, current_symbol);
                        HashSet* currentFollow = hashmap_get(follow, currentrule_nonterminal);
                        changed |= hashset_add_hashset(LastSymbolSet, currentFollow);
                    }
                }
            }
            arraylist_free(tokens);
        }
    }


}


int getTerminalIndex(const char* sym) {
    for (int i = 0; i < terminalsList->size; i++) {
        if (strcmp(*(char**)arraylist_get(terminalsList, i), sym) == 0)
            return i;
    }
    return -1;
}

int getNonterminalIndex(const char* sym) {
    for (int i = 0; i < nonterminalsList->size; i++) {
        if (strcmp(*(char**)arraylist_get(nonterminalsList, i), sym) == 0)
            return i;
    }
    return -1;
}

/* Create the action and goto tables*/
void init_tables() {
    actionTable = malloc(states->size * sizeof(ActionCell*));
    ActionCell* actionContent = malloc(states->size * terminalsList->size * sizeof(ActionCell));
    for (unsigned int i = 0; i < states->size; i++)
        actionTable[i] = actionContent + i * terminalsList->size;

    gotoTable = malloc(states->size * sizeof(int*));
    int* gotoContent = malloc(states->size * nonterminalsList->size * sizeof(int));
    for (unsigned int i = 0; i < states->size; i++)
        gotoTable[i] = gotoContent + i * nonterminalsList->size;
}

/* fill the tables */
void build_parsing_tables() {
    // initialized all the values as -1 and error
    for (int i = 0; i < states->size; i++) {
        for (int j = 0; j < terminalsList->size; j++) {
            actionTable[i][j] = (ActionCell){ .type = ERROR };
        }
        for (int j = 0; j < nonterminalsList->size; j++) {
            gotoTable[i][j] = -1;
        }
    }
    
    // go through all the states
    for (int i = 0; i < states->size; i++) {
        State* s = arraylist_get(states, i);

        // all the symbols a state can get
        ArrayList* symbols = arraylist_init(sizeof(char*), 50);

        // go through all the states items
        for (int j = 0; j < s->items->size; j++) {
            char* sym = get_next_symbol(arraylist_get(s->items, j));

            // add to symbols list if its not in already
            if (sym != NULL) {
                bool exists = false;
                for (int k = 0; k < symbols->size; k++) {
                    if (strcmp(sym, *(char**)arraylist_get(symbols, k)) == 0) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    char* temp = strdup(sym);
                    arraylist_add(symbols, &temp);
                }
                free(sym);
            }
        }

        // go through all the symbols that can be gotten in the state
        for (int k = 0; k < symbols->size; k++) {
            char* sym = *(char**)arraylist_get(symbols, k);

            // get the full content of the state
            State* g = goto_state(s, sym);

            if (g->items->size == 0) {
                free(g);
                continue;
            }

            int j = find_state(g);
            if (j == -1) {
                free(g);
                continue;
            }

            // if the symbol is a terminal look at action table
            if (!isNonterminal(sym)) {
                int col = getTerminalIndex(sym);

                if (col != -1) {
                   /* i: current state
                    * col: the index of the terminal
                    * j: the row where the state appears
                    */
                    actionTable[i][col] = (ActionCell){ .type = SHIFT, .value = j };
                }
            }
            else {
                // if symbol is a nonterminal look at goto table
                int col = getNonterminalIndex(sym);
                if (col != -1) {
                   /* i: current state
                    * col: the index of the nonterminal
                    * j: the row where the state appears
                    */
                    gotoTable[i][col] = j;
                }
            }
            free(g);
        }
        arraylist_free(symbols);
    }

    // go through all the states
    for (int i = 0; i < states->size; i++) {
        State* s = arraylist_get(states, i);

        // go through each states items
        for (int j = 0; j < s->items->size; j++) {
            LRItem* item = arraylist_get(s->items, j);

            // count the amount of symbols in the item
            int tokenCount = 0;
            char* copyContent = strdup(item->rule->ruleContent);
            char* t = strtok(copyContent, " ");
            while (t != NULL) {
                tokenCount++;
                t = strtok(NULL, " ");
            }
            free(copyContent);

            // if the dot is at the end (end of the rule) -> its a reduce!
            if (item->dot == tokenCount) {
                // for the start state add accept if at end
                if (strcmp(item->rule->nonterminal, "START'") == 0) {
                    int dollarIdx = getTerminalIndex("$");
                    if (dollarIdx != -1) {
                        actionTable[i][dollarIdx] = (ActionCell){ .type = ACCEPT };
                    }
                }
                else {
                    // go through all the terminals
                    for (int k = 0; k < terminalsList->size; k++) {
                        char* term = *(char**)arraylist_get(terminalsList, k);

                        // if current item can be followed by the terminal, reduce by the rule
                        if (hashset_contains(hashmap_get(follow, item->rule->nonterminal), term)) {
                            if (actionTable[i][k].type == ERROR)
                                actionTable[i][k] = (ActionCell){ .type = REDUCE, .value = item->rule->ruleID };
                        }
                    }
                }
            }
        }
    }

}

char* actiontypetostring(int action) {
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

void print_parsing_tables() {
    printf("ACTION TABLE:\n\t");
    for (int j = 0; j < terminalsList->size; j++) {
        printf("%.5s \t", *(char**)arraylist_get(terminalsList, j));
    }
    printf("\n");
    for (int i = 0; i < states->size; i++) {
        printf("%d\t", i);
        for (int j = 0; j < terminalsList->size; j++) {
            char buffer[20];
            sprintf(buffer, "%s%d", actiontypetostring(actionTable[i][j].type), actionTable[i][j].value);
            printf("%s\t", buffer);
        }
        printf("\n");
    }
    printf("\nGOTO TABLE:\n\t");
    for (int j = 0; j < nonterminalsList->size; j++) {
        printf("%.5s\t", *(char**)arraylist_get(nonterminalsList, j));
    }
    printf("\n");
    for (int i = 0; i < states->size; i++) {
        printf("%d\t", i);
        for (int j = 0; j < nonterminalsList->size; j++) {
            if (gotoTable[i][j] == -1)
                printf("err\t");
            else
                printf("%d\t", gotoTable[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int find_row_of_nonterminal_in_table(const char* nonterminal) {
    int i;
    for (i = 0; i < nonterminalsList->size && strcmp(*(char**)arraylist_get(nonterminalsList, i), nonterminal) != 0; i++);
    if (i < nonterminalsList->size)
        return i;
    else {
        printf("ERROR");
        exit(1);
    }
}

int find_column_of_terminal_in_table(const char* terminal) {
    int i;
    for (i = 0; i < terminalsList->size && strcmp(*(char**)arraylist_get(terminalsList, i), terminal) != 0; i++);
    if (i < terminalsList->size)
        return i;
    else {
        printf("ERROR");
        exit(1);
    }
}


void createAssociationMap() { // to be changed
    //associationArray[TOKEN_CONTINUE] = find_column_of_terminal_in_table("continue");
    //printf("TOKEN_CONTINUE = %d\n", associationArray[TOKEN_CONTINUE]);
    associationArray[TOKEN_LBRACES] = find_column_of_terminal_in_table("{");
    printf("TOKEN_LBRACES = %d\n", associationArray[TOKEN_LBRACES]);
    associationArray[TOKEN_RBRACES] = find_column_of_terminal_in_table("}");
    printf("TOKEN_RBRACES = %d\n", associationArray[TOKEN_RBRACES]);
    associationArray[TOKEN_OPERATOR_ALSO] = find_column_of_terminal_in_table("&&");
    printf("TOKEN_ALSO = %d\n", associationArray[TOKEN_OPERATOR_ALSO]);
    associationArray[TOKEN_OPERATOR_EITHER] = find_column_of_terminal_in_table("||");
    printf("TOKEN_EITHER = %d\n", associationArray[TOKEN_OPERATOR_EITHER]);
    associationArray[TOKEN_LPAREN] = find_column_of_terminal_in_table("(");
    printf("TOKEN_LPAREN = %d\n", associationArray[TOKEN_LPAREN]);
    associationArray[TOKEN_RPAREN] = find_column_of_terminal_in_table(")");
    printf("TOKEN_RPAREN = %d\n", associationArray[TOKEN_RPAREN]);
    associationArray[TOKEN_OPERATOR_EQUAL] = find_column_of_terminal_in_table("==");
    printf("TOKEN_EQUAL = %d\n", associationArray[TOKEN_OPERATOR_EQUAL]);
    associationArray[TOKEN_OPERATOR_NOT_EQUAL] = find_column_of_terminal_in_table("!=");
    printf("TOKEN_NOT_EQUAL = %d\n", associationArray[TOKEN_OPERATOR_NOT_EQUAL]);
    associationArray[TOKEN_OPERATOR_GREATER] = find_column_of_terminal_in_table(">");
    printf("TOKEN_REATER = %d\n", associationArray[TOKEN_OPERATOR_GREATER]);
    associationArray[TOKEN_OPERATOR_GREATER_EQUAL] = find_column_of_terminal_in_table(">=");
    printf("TOKEN_GREATER_EQUAL = %d\n", associationArray[TOKEN_OPERATOR_GREATER_EQUAL]);
    associationArray[TOKEN_OPERATOR_LESS] = find_column_of_terminal_in_table("<");
    printf("TOKEN_LESS = %d\n", associationArray[TOKEN_OPERATOR_LESS]);
    associationArray[TOKEN_OPERATOR_LESS_EQUAL] = find_column_of_terminal_in_table("<=");
    printf("TOKEN_LESS_EQUAL= %d\n", associationArray[TOKEN_OPERATOR_LESS_EQUAL]);
    associationArray[TOKEN_OPERATOR_PLUS] = find_column_of_terminal_in_table("+");
    printf("TOKEN_PLUS = %d\n", associationArray[TOKEN_OPERATOR_PLUS]);
    associationArray[TOKEN_OPERATOR_MINUS] = find_column_of_terminal_in_table("-");
    printf("TOKEN_MINUS = %d\n", associationArray[TOKEN_OPERATOR_MINUS]);
    associationArray[TOKEN_OPERATOR_MULTIPLY] = find_column_of_terminal_in_table("*");
    printf("TOKEN_ASTERISK = %d\n", associationArray[TOKEN_OPERATOR_MULTIPLY]);
    associationArray[TOKEN_OPERATOR_DIVIDE] = find_column_of_terminal_in_table("/");
    printf("TOKEN_SLASH = %d\n", associationArray[TOKEN_OPERATOR_DIVIDE]);
    associationArray[TOKEN_IF] = find_column_of_terminal_in_table("if");
    printf("TOKEN_IF = %d\n", associationArray[TOKEN_IF]);
    //associationArray[TOKEN_THEN] = find_column_of_terminal_in_table("then");
    //printf("TOKEN_THEN = %d\n", associationArray[TOKEN_THEN]);
    associationArray[TOKEN_EOF] = find_column_of_terminal_in_table("$");
    printf("TOKEN_EOF = %d\n", associationArray[TOKEN_EOF]);
    associationArray[TOKEN_IDENTIFIER] = find_column_of_terminal_in_table("identifier");
    printf("TOKEN_ID = %d\n", associationArray[TOKEN_IDENTIFIER]);
    associationArray[TOKEN_NUMBER] = find_column_of_terminal_in_table("number");
    printf("TOKEN_NUM = %d\n", associationArray[TOKEN_NUMBER]);
    associationArray[TOKEN_FLOAT_NUMBER] = find_column_of_terminal_in_table("float_number");
    printf("TOKEN_FLOAT_NUMBER = %d\n", associationArray[TOKEN_FLOAT_NUMBER]);
    associationArray[TOKEN_INT] = find_column_of_terminal_in_table("int");
    printf("TOKEN_INT = %d\n", associationArray[TOKEN_INT]);
    associationArray[TOKEN_FLOAT] = find_column_of_terminal_in_table("float");
    printf("TOKEN_FLOAT = %d\n", associationArray[TOKEN_FLOAT]);
    associationArray[TOKEN_STRING] = find_column_of_terminal_in_table("string");
    printf("TOKEN_STRING = %d\n", associationArray[TOKEN_STRING]);
    associationArray[TOKEN_CHAR] = find_column_of_terminal_in_table("char");
    printf("TOKEN_CHAR = %d\n", associationArray[TOKEN_CHAR]);
    //associationArray[TOKEN_SEMICOLON] = find_column_of_terminal_in_table(";");
    //printf("TOKEN_SEMICOLON = %d\n", associationArray[TOKEN_SEMICOLON]);
    associationArray[TOKEN_ELSE] = find_column_of_terminal_in_table("else");
    printf("TOKEN_ELSE = %d\n", associationArray[TOKEN_ELSE]);
    associationArray[TOKEN_OPERATOR_ASSIGN] = find_column_of_terminal_in_table("=");
    printf("TOKEN_ASSIGN = %d\n", associationArray[TOKEN_OPERATOR_ASSIGN]);
    associationArray[TOKEN_DO] = find_column_of_terminal_in_table("do");
    printf("TOKEN_DO = %d\n", associationArray[TOKEN_DO]);
    associationArray[TOKEN_WHILE] = find_column_of_terminal_in_table("while");
    printf("TOKEN_WHILE= %d\n", associationArray[TOKEN_WHILE]);
    associationArray[TOKEN_FOR] = find_column_of_terminal_in_table("for");
    printf("TOKEN_FOR= %d\n", associationArray[TOKEN_FOR]);
    associationArray[TOKEN_CHANGE] = find_column_of_terminal_in_table("change");
    printf("TOKEN_CHANGE= %d\n", associationArray[TOKEN_CHANGE]);
    //associationArray[TOKEN_PRINT] = find_column_of_terminal_in_table("print");
    //printf("TOKEN_PRINT= %d\n", associationArray[TOKEN_PRINT]);
    //associationArray[TOKEN_GET] = find_column_of_terminal_in_table("get");
    //printf("TOKEN_GET= %d\n", associationArray[TOKEN_GET]);
    associationArray[TOKEN_RETURNS] = find_column_of_terminal_in_table("returns");
    printf("TOKEN_RETURNS= %d\n", associationArray[TOKEN_RETURNS]);
    associationArray[TOKEN_GETS] = find_column_of_terminal_in_table("gets");
    printf("TOKEN_GETS= %d\n", associationArray[TOKEN_GETS]);
    associationArray[TOKEN_FUNCTION] = find_column_of_terminal_in_table("function");
    printf("TOKEN_FUNCTION= %d\n", associationArray[TOKEN_FUNCTION]);
    //associationArray[TOKEN_NOTHING] = find_column_of_terminal_in_table("nothing");
    //printf("TOKEN_NOTHING= %d\n", associationArray[TOKEN_NOTHING]);
    associationArray[TOKEN_COMMA] = find_column_of_terminal_in_table(",");
    printf("TOKEN_COMMA= %d\n", associationArray[TOKEN_COMMA]);
    associationArray[TOKEN_CALL] = find_column_of_terminal_in_table("call");
    printf("TOKEN_CALL= %d\n", associationArray[TOKEN_CALL]);
    associationArray[TOKEN_WITH] = find_column_of_terminal_in_table("with");
    printf("TOKEN_WITH= %d\n", associationArray[TOKEN_WITH]);
    associationArray[TOKEN_RETURN] = find_column_of_terminal_in_table("return");
    printf("TOKEN_RETURN= %d\n", associationArray[TOKEN_RETURN]);
    associationArray[TOKEN_OPERATOR_MODULO] = find_column_of_terminal_in_table("%");
    printf("TOKEN_RETURN= %d\n", associationArray[TOKEN_OPERATOR_MODULO]);
    associationArray[TOKEN_BOOL] = find_column_of_terminal_in_table("bool");
    printf("TOKEN_BOOL= %d\n", associationArray[TOKEN_BOOL]);
    associationArray[TOKEN_STRING_LITERAL] = find_column_of_terminal_in_table("string_literal");
    printf("TOKEN_BOOL= %d\n", associationArray[TOKEN_STRING_LITERAL]);
    associationArray[TOKEN_TRUE] = find_column_of_terminal_in_table("true");
    printf("TOKEN_TRUE= %d\n", associationArray[TOKEN_TRUE]);
    associationArray[TOKEN_FALSE] = find_column_of_terminal_in_table("false");
    printf("TOKEN_FALSE= %d\n", associationArray[TOKEN_FALSE]);

}
void print_follows() {
    printf("FOLLOWS:\n");
    for (int i = 0; i < nonterminalsList->size; i++) {
        char* str = *(char**)arraylist_get(nonterminalsList, i);
        printf("%s:\t\t\t", str);
        hashset_print(hashmap_get(follow, str), print_string);
    }
}

void print_rules() {
    for (int i = 0; i < rules->size; i++) {
        Rule* r = (Rule*)rules->array[i];
        printf("Rule %d: %s -> %s (length: %d) - pos %d\n", r->ruleID, r->nonterminal, r->ruleContent, r->ruleTerminalCount, r->nonterminal_position);
    }
}

void add_rules() {
    add_rule("PROGRAM", "STATEMENTS"); //
    add_rule("STATEMENTS", "STATEMENTS STATEMENT"); //
    add_rule("STATEMENTS", "STATEMENT"); //
    
    add_rule("STATEMENT", "VARIABLE_DECLARATION_STATEMENT"); //
    add_rule("STATEMENT", "VARIABLE_ASSIGNMENT_STATEMENT"); //
    add_rule("STATEMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT"); //
    add_rule("STATEMENT", "IF_STATEMENT"); //
    add_rule("STATEMENT", "IF_ELSE_STATEMENT"); //
    add_rule("STATEMENT", "WHILE_STATEMENT"); //
    add_rule("STATEMENT", "DO_WHILE_STATEMENT"); //
    add_rule("STATEMENT", "FOR_STATEMENT"); // 
    add_rule("STATEMENT", "FOR_CHANGE_STATEMENT"); //
    //add_rule("STATEMENT", "PRINT_STATEMENT");
    //add_rule("STATEMENT", "GET_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_NO_RETURN_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_CALL_STATEMENT");
    add_rule("STATEMENT", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("STATEMENT", "RETURN_STATEMENT");
    add_rule("STATEMENT", "RETURN_NONE_STATEMENT");
    add_rule("STATEMENT", "BLOCK");

    

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
    //add_rule("CONDITION", "identifier");
    add_rule("CONDITION", "FUNCTION_CALL_STATEMENT");
    add_rule("CONDITION", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");
    add_rule("CONDITION", "true");
    add_rule("CONDITION", "false");

    add_rule("EXPRESSION", "EXPRESSION + TERM"); //
    add_rule("EXPRESSION", "EXPRESSION - TERM");
    add_rule("EXPRESSION", "TERM");

    add_rule("TERM", "TERM % FACTOR"); //
    add_rule("TERM", "TERM * FACTOR");
    add_rule("TERM", "TERM / FACTOR");
    add_rule("TERM", "FACTOR");

    add_rule("FACTOR", "( EXPRESSION )"); //
    add_rule("FACTOR", "identifier");
    add_rule("FACTOR", "number");
    add_rule("FACTOR", "float_number");
    add_rule("FACTOR", "string_literal");
    add_rule("FACTOR", "CONDITION_LIST");
    add_rule("FACTOR", "FUNCTION_CALL_STATEMENT");
    add_rule("FACTOR", "FUNCTION_CALL_WITH_NOTHING_STATEMENT");

    

    add_rule("VARIABLE_TYPE", "int");
    add_rule("VARIABLE_TYPE", "char");
    add_rule("VARIABLE_TYPE", "string");
    add_rule("VARIABLE_TYPE", "float");
    add_rule("VARIABLE_TYPE", "bool");

    
    add_rule("BLOCK", "{ STATEMENTS }"); //
    add_rule("BLOCK", "{ }"); //

    add_rule("IF_STATEMENT", "if CONDITION_LIST IF_BLOCK"); //
    add_rule("IF_ELSE_STATEMENT", "if CONDITION_LIST IF_BLOCK else IF_BLOCK "); //

    add_rule("IF_BLOCK", "{ STATEMENTS }"); // ------
    add_rule("IF_BLOCK", "STATEMENT"); //

    add_rule("VARIABLE_DECLARATION_STATEMENT", "VARIABLE_TYPE identifier"); //
    add_rule("VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT", "VARIABLE_TYPE identifier = EXPRESSION"); //

    add_rule("VARIABLE_ASSIGNMENT_STATEMENT", "identifier = EXPRESSION"); //

    add_rule("WHILE_STATEMENT", "while CONDITION_LIST WHILE_BLOCK"); //

    add_rule("WHILE_BLOCK", "{ STATEMENT }"); //
    add_rule("WHILE_BLOCK", "STATEMENT"); //


    add_rule("DO_WHILE_STATEMENT", "do WHILE_BLOCK while CONDITION_LIST"); //
    add_rule("DO_WHILE_STATEMENT", "do STATEMENT while CONDITION_LIST"); //


    add_rule("FOR_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST FOR_BLOCK"); //
    add_rule("FOR_CHANGE_STATEMENT", "for FOR_ASSIGNMENT while CONDITION_LIST FOR_BLOCK change CHANGE_BLOCK"); //

    add_rule("FOR_ASSIGNMENT", "VARIABLE_ASSIGNMENT_STATEMENT"); //
    add_rule("FOR_ASSIGNMENT", "VARIABLE_DECLARATION_WITH_ASSIGNMENT_STATEMENT"); //

    add_rule("FOR_BLOCK", "{ STATEMENTS }"); //
    add_rule("FOR_BLOCK", "STATEMENT"); //

    add_rule("CHANGE_BLOCK", "VARIABLE_ASSIGNMENT_STATEMENT"); //

    //add_rule("PRINT_STATEMENT", "print EXPRESSION");
    //add_rule("GET_STATEMENT", "get VARIABLE_DECLARATION_STATEMENT");
    //add_rule("GET_STATEMENT", "get identifier");

    add_rule("FUNCTION_DECLARATION_STATEMENT", "function identifier gets PARAMETER_LIST returns VARIABLE_TYPE FUNCTION_BLOCK"); //
    add_rule("FUNCTION_DECLARATION_NO_RETURN_STATEMENT", "function identifier gets PARAMETER_LIST FUNCTION_BLOCK"); //
    add_rule("FUNCTION_DECLARATION_NO_ARGUMENTS_STATEMENT", "function identifier returns VARIABLE_TYPE FUNCTION_BLOCK");
    add_rule("FUNCTION_DECLARATION_NO_RETURN_NO_ARGUMENTS_STATEMENT", "function identifier FUNCTION_BLOCK"); //
    
    add_rule("PARAMETER_LIST", "PARAMETER_LIST , PARAMETER");
    add_rule("PARAMETER_LIST", "PARAMETER");
    add_rule("PARAMETER", "VARIABLE_TYPE identifier");

    add_rule("FUNCTION_BLOCK", "{ STATEMENTS }");
    add_rule("FUNCTION_BLOCK", "{ }");

    add_rule("RETURN_STATEMENT", "return EXPRESSION");
    //add_rule("RETURN_NONE_STATEMENT", "return");

    add_rule("FUNCTION_CALL_STATEMENT", "call identifier with ARGUMENT_LIST");
    add_rule("FUNCTION_CALL_WITH_NOTHING_STATEMENT", "call identifier");

    add_rule("ARGUMENT_LIST", "ARGUMENT_LIST , EXPRESSION");
    add_rule("ARGUMENT_LIST", "EXPRESSION");
}

void set_nonterminals_position() {
    for (int i = 0; i < rules->size; i++) {
        Rule* rule = arraylist_get(rules, i);
        rule->nonterminal_position = find_row_of_nonterminal_in_table(rule->nonterminal);
    }
}


int create_parser_tables() {
    rules = arraylist_init(sizeof(Rule), DEFAULT_NUMBER_OF_RULES);

    add_rules();

    build_states("PROGRAM");
    for (int i = 0; i < states->size; i++) {
        print_state(arraylist_get(states, i), i);
        printf("\n");
    }

    collect_symbols();

    set_nonterminals_position();

    printf("\nTerminals: ");
    arraylist_print(terminalsList, print_string_arraylist);
    printf("\nNonterminals: ");
    arraylist_print(nonterminalsList, print_string_arraylist);
    printf("\n\n");


    init_tables();

    compute_follow();

    print_follows();

    build_parsing_tables();

    print_parsing_tables();

    createAssociationMap();

    print_rules();

    return 0;
}
