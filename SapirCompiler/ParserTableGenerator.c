#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#pragma warning(disable:4996)

#define MAX_ITEMS       100
#define MAX_STATES      100
#define MAX_RULES       100
#define MAX_TOKEN_LEN   32

typedef struct {
    char nonterminal[MAX_TOKEN_LEN];
    char ruleContent[256];
} Rule;

typedef struct {
    Rule* rule;
    int dot;
} LRItem;

typedef struct {
    LRItem items[MAX_ITEMS];
    int numItems;
} State;

Rule rules[MAX_RULES];
int numRules = 0;

State* states[MAX_STATES];
int numStates = 0;

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
                for (int j = 0; j < numRules; j++) {
                    if (strcmp(rules[j].nonterminal, symbol) == 0) {
                        LRItem newItem;
                        newItem.rule = &rules[j];
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
    Rule augmented;
    strcpy(augmented.nonterminal, "S'");
    {
        int len = strlen(start_nonterminal) + 3;
        char* buffer = malloc(len);
        snprintf(buffer, len, "%s $", start_nonterminal);
        strcpy(augmented.ruleContent, buffer);
        free(buffer);
    }
    rules[numRules++] = augmented;

    State* I0 = malloc(sizeof(State));
    I0->numItems = 0;
    LRItem startItem;
    startItem.rule = &rules[numRules - 1];
    startItem.dot = 0;
    I0->items[I0->numItems++] = startItem;
    closure(I0);
    states[numStates++] = I0;

    bool addedNew;
    do {
        addedNew = false;
        for (int i = 0; i < numStates; i++) {
            State* s = states[i];
            char symbols[100][MAX_TOKEN_LEN];
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
                        strcpy(symbols[symbolCount], sym);
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

int main() {
    numRules = 0;

    // Defined grammar:
    // S -> a c B
    // B -> c B
    // B -> d C
    // B -> f
    // C -> e

    // UPPER CASE FOR NONTERMINALS
    // LOWERCASE FOR TERMINALS
    // SPACE SEPARATES SYMBOLS

    Rule r1;
    strcpy(r1.nonterminal, "S");
    strcpy(r1.ruleContent, "a c B");
    rules[numRules++] = r1;

    Rule r2;
    strcpy(r2.nonterminal, "B");
    strcpy(r2.ruleContent, "c B");
    rules[numRules++] = r2;

    Rule r3;
    strcpy(r3.nonterminal, "B");
    strcpy(r3.ruleContent, "d C");
    rules[numRules++] = r3;

    Rule r4;
    strcpy(r4.nonterminal, "B");
    strcpy(r4.ruleContent, "f");
    rules[numRules++] = r4;

    Rule r5;
    strcpy(r5.nonterminal, "C");
    strcpy(r5.ruleContent, "e");
    rules[numRules++] = r5;

    build_states("S");

    for (int i = 0; i < numStates; i++) {
        print_state(states[i], i);
        printf("\n");
    }

    return 0;
}
