#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ParserTableGenerator.h"
#include "Tokens.h"
#include "HashMap.h"
#include <ctype.h>

#define strdup _strdup

#pragma warning(disable:4996)

State* states[MAX_STATES];
int numStates = 0;

int count_terminals(char* rule_content) {
    int count = 0;
    bool in_terminal = false;

    while (*rule_content != '\0') {
        if (*rule_content != ' ' && !in_terminal) count++;
        in_terminal = *rule_content != ' ' ? true : false;
        rule_content++;
    }

    return count;
}

void add_rule(char* nonterminal, char* content) {
    Rule rule;
    rule.nonterminal = strdup(nonterminal);
    rule.ruleContent = strdup(content);
    rule.rule_terminal_count = count_terminals(content);
    rule.rule_id = rules->size;
    arraylist_add(rules, &rule);
}

char* get_nth_token(const char* s, int n) {
    static char token[256];
    int currentToken = 0;
    const char* p = s;

    while (*p) {
        while (*p && isspace((unsigned char)*p))
            p++;
        if (!*p) break;
        if (currentToken == n) {
            int i = 0;
            while (*p && !isspace((unsigned char)*p)) {
                token[i++] = *p;
                p++;
            }
            token[i] = '\0';
            return token;
        }
        else {
            while (*p && !isspace((unsigned char)*p))
                p++;
            currentToken++;
        }
    }
    return NULL;
}

char* get_next_symbol(LRItem* item) {
    return get_nth_token(item->rule->ruleContent, item->dot);
}

bool state_contains_item(State* s, LRItem* item) {
    for (int i = 0; i < s->numItems; i++) {
        LRItem* it = &s->items[i];
        if (it->rule == item->rule && it->dot == item->dot)
            return true;
    }
    return false;
}

void closure(State* s) {
    bool added;
    do {
        added = false;
        for (int i = 0; i < s->numItems; i++) {
            LRItem* item = &s->items[i];
            char* symbol = get_next_symbol(item);
            if (symbol != NULL && isupper(symbol[0])) {
                for (int j = 0; j < rules->size; j++) {
                    if (strcmp(((Rule*)rules->array[j])->nonterminal, symbol) == 0) {
                        LRItem newItem;
                        newItem.rule = ((Rule*)rules->array[j]);
                        newItem.dot = 0;
                        if (!state_contains_item(s, &newItem)) {
                            s->items[s->numItems++] = newItem;
                            added = true;
                        }
                    }
                }
            }
        }
    } while (added);
}

State* goto_state(State* s, const char* symbol) {
    State* newState = malloc(sizeof(State));
    newState->numItems = 0;
    for (int i = 0; i < s->numItems; i++) {
        LRItem item = s->items[i];
        char* next = get_next_symbol(&item);
        if (next != NULL && strcmp(next, symbol) == 0) {
            LRItem advanced = item;
            advanced.dot++;
            if (!state_contains_item(newState, &advanced)) {
                newState->items[newState->numItems++] = advanced;
            }
        }
    }
    closure(newState);
    return newState;
}

bool state_equals(State* s1, State* s2) {
    if (s1->numItems != s2->numItems) return false;
    for (int i = 0; i < s1->numItems; i++) {
        bool found = false;
        for (int j = 0; j < s2->numItems; j++) {
            if (s1->items[i].rule == s2->items[j].rule &&
                s1->items[i].dot == s2->items[j].dot) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

int find_state(State* s) {
    for (int i = 0; i < numStates; i++) {
        if (state_equals(states[i], s))
            return i;
    }
    return -1;
}


void build_states(char* start_nonterminal) {
    /*
    Rule augmented;
    strcpy(augmented.nonterminal, "START'");
    
    int len = strlen(start_nonterminal) + 3;
    char* buffer = malloc(len);
    snprintf(buffer, len, "%s $", start_nonterminal);
    augmented.ruleContent = strdup(buffer);
    free(buffer);
    
    rules[numRules++] = augmented;
    */
    int len = strlen(start_nonterminal) + 3;
    char* buffer = malloc(len);
    sprintf(buffer, "%s $", start_nonterminal);
    add_rule("START'", buffer);
    free(buffer);

    State* I0 = malloc(sizeof(State));
    I0->numItems = 0;
    LRItem startItem;
    startItem.rule = ((Rule*)rules->array[rules->size - 1]);
    startItem.dot = 0;
    I0->items[I0->numItems++] = startItem;
    closure(I0);
    states[numStates++] = I0;

    bool addedNew;
    do {
        addedNew = false;
        for (int i = 0; i < numStates; i++) {
            State* s = states[i];
            char *symbols[100];
            int symbolCount = 0;
            for (int j = 0; j < s->numItems; j++) {
                char* sym = get_next_symbol(&s->items[j]);
                if (sym != NULL) {
                    bool exists = false;
                    for (int k = 0; k < symbolCount; k++) {
                        if (strcmp(sym, symbols[k]) == 0) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        symbols[symbolCount] = strdup(sym);
                        symbolCount++;
                    }
                }
            }
            for (int k = 0; k < symbolCount; k++) {
                State* g = goto_state(s, symbols[k]);
                if (g->numItems == 0) {
                    free(g);
                    continue;
                }
                int idx = find_state(g);
                if (idx == -1) {
                    states[numStates++] = g;
                    addedNew = true;
                }
                else {
                    free(g);
                }
            }
        }
    } while (addedNew);
}

void print_state(State* s, int index) {
    printf("State %d:\n", index);
    for (int i = 0; i < s->numItems; i++) {
        LRItem* item = &s->items[i];
        printf("  %s -> ", item->rule->nonterminal);
        char buffer[256];
        strcpy(buffer, item->rule->ruleContent);
        char* token = strtok(buffer, " ");
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
    }
}

char* nonterminalsList[MAX_SYMBOLS];
int numNonterminals = 0;
char* terminalsList[MAX_SYMBOLS];
int numTerminals = 0;

bool symbol_exists(char* arr[], int count, const char* sym) {
    printf("\n\nCOUNT: %d\n", count);
    printf("CHECKING FOR: %s\n", sym);

    for (int i = 0; i < count; i++) {
        printf("ARR[0] - %s\n", arr[0]);
        printf("HERE - %s\n", arr[i]);
        if (strcmp(arr[i], sym) == 0)
            return true;
    }
    printf("RETUURND FALSE");
    return false;
}

void collect_symbols() {
    for (int i = 0; i < rules->size; i++) {
        if (!symbol_exists(nonterminalsList, numNonterminals, ((Rule*)rules->array[i])->nonterminal)) {
            nonterminalsList[numNonterminals++] = strdup(((Rule*)rules->array[i])->nonterminal);
        }
        char buffer[256];
        strcpy(buffer, ((Rule*)rules->array[i])->ruleContent);
        char* token = strtok(buffer, " ");
        while (token != NULL) {
            if (!(isupper(token[0]) || strcmp(token, "$") == 0)) {
                if (!symbol_exists(terminalsList, numTerminals, token)) {
                    terminalsList[numTerminals++] = strdup(token);
                }
            }
            else {
                if (!symbol_exists(nonterminalsList, numNonterminals, token)) {
                    nonterminalsList[numNonterminals++] = strdup(token);
                }
            }
            token = strtok(NULL, " ");
        }
    }
    if (!symbol_exists(terminalsList, numTerminals, "$")) {
        terminalsList[numTerminals++] = strdup("$");
    }
}

bool follow[MAX_SYMBOLS][MAX_SYMBOLS] = { false };

void init_follow() {
    for (int i = 0; i < numNonterminals; i++) {
        for (int j = 0; j < numTerminals; j++) {
            follow[i][j] = false;
        }
    }
    int idx = -1;
    for (int i = 0; i < numNonterminals; i++) {
        if (strcmp(nonterminalsList[i], "START'") == 0) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int j = 0; j < numTerminals; j++) {
            if (strcmp(terminalsList[j], "$") == 0) {
                follow[idx][j] = true;
                break;
            }
        }
    }
}

void compute_follow() {
    init_follow();
    bool changed;
    do {
        changed = false;
        for (int i = 0; i < rules->size; i++) {
            char* A = ((Rule*)rules->array[i])->nonterminal;
            char buffer[256];
            strcpy(buffer, ((Rule*)rules->array[i])->ruleContent);
            char* tokens[50];
            int ntokens = 0;
            char* tok = strtok(buffer, " ");
            while (tok != NULL) {
                tokens[ntokens++] = tok;
                tok = strtok(NULL, " ");
            }
            for (int j = 0; j < ntokens; j++) {
                if (isupper(tokens[j][0]) || strcmp(tokens[j], "START'") == 0) {
                    if (j + 1 < ntokens) {
                        if (!(isupper(tokens[j + 1][0]) || strcmp(tokens[j + 1], "START'") == 0)) {
                            int Aidx = -1;
                            for (int k = 0; k < numNonterminals; k++) {
                                if (strcmp(nonterminalsList[k], tokens[j]) == 0)
                                    Aidx = k;
                            }
                            int termIdx = -1;
                            for (int k = 0; k < numTerminals; k++) {
                                if (strcmp(terminalsList[k], tokens[j + 1]) == 0)
                                    termIdx = k;
                            }
                            if (Aidx != -1 && termIdx != -1 && !follow[Aidx][termIdx]) {
                                follow[Aidx][termIdx] = true;
                                changed = true;
                            }
                        }
                        else {
                            int Bidx = -1;
                            for (int k = 0; k < numNonterminals; k++) {
                                if (strcmp(nonterminalsList[k], tokens[j + 1]) == 0)
                                    Bidx = k;
                            }
                            if (Bidx != -1) {
                                char sample[256] = "";
                                for (int r = 0; r < rules->size; r++) {
                                    if (strcmp(((Rule*)rules->array[r])->nonterminal, tokens[j + 1]) == 0) {
                                        strcpy(sample, ((Rule*)rules->array[r])->ruleContent);
                                        break;
                                    }
                                }
                                char* firstSym = get_nth_token(sample, 0);
                                if (firstSym && !(isupper(firstSym[0]) || strcmp(firstSym, "START'") == 0)) {
                                    int termIdx = -1;
                                    for (int k = 0; k < numTerminals; k++) {
                                        if (strcmp(terminalsList[k], firstSym) == 0)
                                            termIdx = k;
                                    }
                                    int Aidx = -1;
                                    for (int k = 0; k < numNonterminals; k++) {
                                        if (strcmp(nonterminalsList[k], tokens[j]) == 0)
                                            Aidx = k;
                                    }
                                    if (Aidx != -1 && termIdx != -1 && !follow[Aidx][termIdx]) {
                                        follow[Aidx][termIdx] = true;
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                    else {
                        int Aidx = -1;
                        for (int k = 0; k < numNonterminals; k++) {
                            if (strcmp(nonterminalsList[k], tokens[j]) == 0)
                                Aidx = k;
                        }
                        int LHSidx = -1;
                        for (int k = 0; k < numNonterminals; k++) {
                            if (strcmp(nonterminalsList[k], A) == 0)
                                LHSidx = k;
                        }
                        if (Aidx != -1 && LHSidx != -1) {
                            for (int t = 0; t < numTerminals; t++) {
                                if (follow[LHSidx][t] && !follow[Aidx][t]) {
                                    follow[Aidx][t] = true;
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (changed);
}


int getTerminalIndex(const char* sym) {
    for (int i = 0; i < numTerminals; i++) {
        if (strcmp(terminalsList[i], sym) == 0)
            return i;
    }
    return -1;
}

int getNonterminalIndex(const char* sym) {
    for (int i = 0; i < numNonterminals; i++) {
        if (strcmp(nonterminalsList[i], sym) == 0)
            return i;
    }
    return -1;
}

void build_parsing_tables() {
    for (int i = 0; i < numStates; i++) {
        for (int j = 0; j < numTerminals; j++) {
            actionTable[i][j] = strdup("error");
        }
        for (int j = 0; j < numNonterminals; j++) {
            gotoTable[i][j] = -1;
        }
    }

    for (int i = 0; i < numStates; i++) {
        State* s = states[i];
        char* symbols[100];
        int symbolCount = 0;
        for (int j = 0; j < s->numItems; j++) {
            char* sym = get_next_symbol(&s->items[j]);
            if (sym != NULL) {
                bool exists = false;
                for (int k = 0; k < symbolCount; k++) {
                    if (strcmp(sym, symbols[k]) == 0) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    symbols[symbolCount] = strdup(sym);
                    symbolCount++;
                }
            }
        }
        for (int k = 0; k < symbolCount; k++) {
            State* g = goto_state(s, symbols[k]);
            if (g->numItems == 0) {
                free(g);
                continue;
            }
            int j = find_state(g);
            if (j == -1) {
                free(g);
                continue;
            }
            if (!(isupper(symbols[k][0]) || strcmp(symbols[k], "START'") == 0)) {
                int termIdx = getTerminalIndex(symbols[k]);
                if (termIdx != -1) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "s%d", j);
                    free(actionTable[i][termIdx]);
                    actionTable[i][termIdx] = strdup(buf);
                }
            }
            else { 
                int ntIdx = getNonterminalIndex(symbols[k]);
                if (ntIdx != -1) {
                    gotoTable[i][ntIdx] = j;
                }
            }
            free(g);
        }
    }

    for (int i = 0; i < numStates; i++) {
        State* s = states[i];
        for (int j = 0; j < s->numItems; j++) {
            LRItem* item = &s->items[j];
            int tokenCount = 0;
            char* buf;
            buf = strdup(item->rule->ruleContent);
            char* t = strtok(buf, " ");
            free(buf);
            while (t != NULL) {
                tokenCount++;
                t = strtok(NULL, " ");
            }
            if (item->dot == tokenCount) {
                if (strcmp(item->rule->nonterminal, "START'") == 0) {
                    int dollarIdx = getTerminalIndex("$");
                    if (dollarIdx != -1) {
                        free(actionTable[i][dollarIdx]);
                        actionTable[i][dollarIdx] = strdup("acc");
                    }
                }
                else {
                    int Aidx = getNonterminalIndex(item->rule->nonterminal);
                    for (int k = 0; k < numTerminals; k++) {
                        if (follow[Aidx][k]) {
                            char buf2[sizeof(int) + 3];
                            int prodNum = item->rule->rule_id;
                            snprintf(buf2, sizeof(buf2), "r%d", item->rule->rule_id);
                            free(actionTable[i][k]);
                            actionTable[i][k] = strdup(buf2);
                        }
                    }
                }
            }
        }
    }
}

void print_parsing_tables() {
    printf("ACTION TABLE:\n");
    printf("State\t");
    for (int j = 0; j < numTerminals; j++) {
        printf("%s\t", terminalsList[j]);
    }
    printf("\n");
    for (int i = 0; i < numStates; i++) {
        printf("%d\t", i);
        for (int j = 0; j < numTerminals; j++) {
            printf("%s\t", actionTable[i][j]);
        }
        printf("\n");
    }
    printf("\nGOTO TABLE:\n");
    printf("State\t");
    for (int j = 0; j < numNonterminals; j++) {
        printf("%s\t", nonterminalsList[j]);
    }
    printf("\n");
    for (int i = 0; i < numStates; i++) {
        printf("%d\t", i);
        for (int j = 0; j < numNonterminals; j++) {
            if (gotoTable[i][j] == -1)
                printf("err\t");
            else
                printf("%d\t", gotoTable[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}







int find_column_of_terminal_in_table(char* terminal) {
    int i;
    for (i = 0; i < numTerminals && strcmp(terminalsList[i], terminal); i++);
    if (i < numTerminals) return i;
    else {
        printf("UNKNOWN TERMINAL %s", terminal);
        exit(0);
    }
}

#define printassosiation(STR) printf("%s = %d\n", #STR, assosiation_array[STR]);
void create_assosiation_map() {
    assosiation_array[TOKEN_IDENTIFIER] = find_column_of_terminal_in_table("id");
    printassosiation(TOKEN_IDENTIFIER);
    assosiation_array[TOKEN_OPERATOR_PLUS] = find_column_of_terminal_in_table("+");
    printassosiation(TOKEN_OPERATOR_PLUS);
    assosiation_array[TOKEN_OPERATOR_MINUS] = find_column_of_terminal_in_table("-");
    printassosiation(TOKEN_OPERATOR_MINUS);
    assosiation_array[TOKEN_NUMBER] = find_column_of_terminal_in_table("number");
    printassosiation(TOKEN_NUMBER);
    assosiation_array[TOKEN_FLOAT] = find_column_of_terminal_in_table("number");
    printassosiation(TOKEN_FLOAT);
}


void print_rules() {
    for (int i = 0; i < rules->size; i++) {
        printf("Rule %d: %s -> %s (length: %d)\n", ((Rule*)rules->array[i])->rule_id, ((Rule*)rules->array[i])->nonterminal, ((Rule*)rules->array[i])->ruleContent, ((Rule*)rules->array[i])->rule_terminal_count);
    }
}




int create_parser_tables() {
    rules = arraylist_init(sizeof(Rule), DEFAULT_NUMBER_OF_RULES);

    // UPERCASE FOR NONTERMINAL
    // LOWERCASE FOR TERMINAL
    // SEPARATE TOKENS WITH SPACE
    
   

    add_rule("S", "E");
    add_rule("E", "E + T");
    add_rule("E", "E - T");
    add_rule("E", "T");
    add_rule("T", "T * F");
    add_rule("T", "F");
    add_rule("F", "id");
    add_rule("F", "number");

    printf("\n");


    build_states("S");
    
    for (int i = 0; i < numStates; i++) {
        print_state(states[i], i);
        printf("\n");
    }
    
    collect_symbols();
    compute_follow();

    build_parsing_tables();

    print_parsing_tables();
    
    create_assosiation_map();

    print_rules();

    
    return 0;
}
