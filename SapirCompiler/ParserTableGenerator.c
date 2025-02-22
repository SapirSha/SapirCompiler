#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ParserTableGenerator.h"
#include "Tokens.h"
#include "HashMap.h"
#include <ctype.h>
#include "HashSet.h"

#define strdup _strdup

#pragma warning(disable:4996)


ArrayList* states;

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
    for (int i = 0; i < s->items->size; i++) {
		LRItem* it = arraylist_get(s->items, i);
        if (it->rule == item->rule && it->dot == item->dot)
            return true;
    }
    return false;
}

void closure(State* s) {
    bool added;
    do {
        added = false;
        for (int i = 0; i < s->items->size; i++) {
            LRItem* item = arraylist_get(s->items, i);
            char* symbol = get_next_symbol(item);
            if (symbol != NULL && isupper(symbol[0])) {
                for (int j = 0; j < rules->size; j++) {
                    if (strcmp(((Rule*)rules->array[j])->nonterminal, symbol) == 0) {
                        LRItem newItem;
                        newItem.rule = ((Rule*)rules->array[j]);
                        newItem.dot = 0;
                        if (!state_contains_item(s, &newItem)) {
							arraylist_add(s->items, &newItem);
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
    newState->items = arraylist_init(sizeof(LRItem), DEFUALT_AMOUNT_OF_LR_ITEMS);
    for (int i = 0; i < s->items->size; i++) {
		LRItem item = *(LRItem*)arraylist_get(s->items, i);
        char* next = get_next_symbol(&item);
        if (next != NULL && strcmp(next, symbol) == 0) {
            LRItem advanced = item;
            advanced.dot++;
            if (!state_contains_item(newState, &advanced)) {
				arraylist_add(newState->items, &advanced);
            }
        }
    }
    closure(newState);
    return newState;
}

bool state_equals(State* s1, State* s2) {
    if (s1->items->size != s2->items->size) return false;
    for (int i = 0; i < s1->items->size; i++) {
        bool found = false;
        for (int j = 0; j < s2->items->size; j++) {
            if (((LRItem*)arraylist_get(s1->items, i))->rule == ((LRItem*)arraylist_get(s2->items, j))->rule &&
                ((LRItem*)arraylist_get(s1->items, i))->dot == ((LRItem*)arraylist_get(s2->items, j))->dot) {
                found = true;
                break;
            }
        }
        if (!found) return false;
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
    states = arraylist_init(sizeof(State), DEFAULT_NUMBER_OF_STATES);



    int len = strlen(start_nonterminal) + 3;
    char* buffer = malloc(len);
    sprintf(buffer, "%s $", start_nonterminal);
    add_rule("START'", buffer);
    free(buffer);


    State* I0 = malloc(sizeof(State));
	I0->items = arraylist_init(sizeof(LRItem), DEFUALT_AMOUNT_OF_LR_ITEMS);
    LRItem startItem;
    startItem.rule = ((Rule*)rules->array[rules->size - 1]);
    startItem.dot = 0;
	arraylist_add(I0->items, &startItem);
    closure(I0);
    arraylist_add(states, I0);

    bool addedNew;
    do {
        addedNew = false;
        for (int i = 0; i < states->size; i++) {
			State* s = arraylist_get(states, i);
            ArrayList* symbolList = arraylist_init(sizeof(char**), 100);
            for (int j = 0; j < s->items->size; j++) {
                char* sym = get_next_symbol(arraylist_get(s->items, j));
                
                if (sym != NULL) {
                    bool exists = false;
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
                }
            }
            for (int k = 0; k < symbolList->size; k++) {
                State* g = goto_state(s, *(char**)arraylist_get(symbolList, k));
                if (g->items->size == 0) {
                    free(g);
                    continue;
                }
                int idx = find_state(g);
                if (idx == -1) {
                    arraylist_add(states, g);
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
    for (int i = 0; i < s->items->size; i++) {
		LRItem* item = arraylist_get(s->items, i);
        printf("  %s -> ", item->rule->nonterminal);
        char* buffer;
        buffer = strdup( item->rule->ruleContent);
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

        free(buffer);
    }
}

ArrayList* nonterminalsList;
ArrayList* terminalsList;

bool symbol_exists(ArrayList* list, const char* sym) {

    for (int i = 0; i < list->size; i++) {
        if (strcmp(*(char**)arraylist_get(list, i), sym) == 0)
            return true;
    }
    return false;
}

void collect_symbols() {
	nonterminalsList = arraylist_init(sizeof(char**), DEFUALT_AMOUNT_OF_NONTERMINALS);
	terminalsList = arraylist_init(sizeof(char**), DEFUALT_AMOUNT_OF_TERMINALS);

    for (int i = 0; i < rules->size; i++) {
        if (!symbol_exists(nonterminalsList, ((Rule*)rules->array[i])->nonterminal)) {
            char* temp = strdup(((Rule*)rules->array[i])->nonterminal);
            arraylist_add(nonterminalsList, &temp);
        }
        char* buffer;
        buffer = strdup(((Rule*)rules->array[i])->ruleContent);
        char* token = strtok(buffer, " ");
        while (token != NULL) {
            if (!(isupper(token[0]) || strcmp(token, "$") == 0)) {
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
        free(buffer);
    }
    if (!symbol_exists(terminalsList, "$")) {
        char* temp = strdup("$");
        arraylist_add(terminalsList, &temp);
    }
}

HashMap* follow = NULL;

void init_follow() {
    follow = createHashMap(nonterminalsList->size);
    

    for (int i = 0; i < nonterminalsList->size; i++) {
        hashmap_insert(follow, *(char**)arraylist_get(nonterminalsList, i), hashset_create(terminalsList->size));
    }

    hashset_insert(hashmap_get(follow, "START'"), "$");
}

void compute_follow() {
    init_follow();
    bool changed;
    do {
        changed = false;
        for (int i = 0; i < rules->size; i++) {
            char* A = ((Rule*)rules->array[i])->nonterminal;
            char* buffer;
            buffer = strdup( ((Rule*)rules->array[i])->ruleContent);
			ArrayList* tokens = arraylist_init(sizeof(char**), 50);
            char* tok = strtok(buffer, " ");
            while (tok != NULL) {
                arraylist_add(tokens, &tok);
                tok = strtok(NULL, " ");
            }
            for (int j = 0; j < tokens->size; j++) {
                if (isupper((*(char**)arraylist_get(tokens, j))[0]) || strcmp(*(char**)arraylist_get(tokens, j), "START'") == 0) { // if nonterminal
                    if (j + 1 < tokens->size) { // if has another token after
                        if (!(isupper((*(char**)arraylist_get(tokens, j + 1))[0]) || strcmp(*(char**)arraylist_get(tokens, j), "START'") == 0)) { // if not nonterminal
                            hashset_insert(hashmap_get(follow, *(char**)arraylist_get(tokens, j)), *(char**)arraylist_get(tokens, j + 1));  // insert as possibility for nonterminal
                        }
                        else {
                            char* sample;
                            
                            for (int r = 0; r < rules->size; r++) { // get rule where the nonterminal is on the left side
                                if (strcmp(((Rule*)rules->array[r])->nonterminal, *(char**)arraylist_get(tokens, j + 1)) == 0) {
                                    sample = strdup(((Rule*)rules->array[r])->ruleContent);
                                    char* firstSym = get_nth_token(sample, 0);
                                    free(sample);
                                    if (firstSym && !(isupper(firstSym[0]) || strcmp(firstSym, "START'") == 0)) { // if first symbol is terminal
                                        hashset_insert(hashmap_get(follow, *(char**)arraylist_get(tokens, j)), firstSym); // insert as possibility
                                        break;
									}
									else if (firstSym && isupper(firstSym[0])) { // if first symbol is nonterminal
										HashSet* firstSymFollow = hashmap_get(follow, firstSym);
										HashSet* current_follow = hashmap_get(follow, *(char**)arraylist_get(tokens, j));
										for (int k = 0; k < firstSymFollow->capacity; k++) {
											if (firstSymFollow->table[k] && firstSymFollow->table[k] != TOMBSTONE) {
												if (!hashset_contains(current_follow, firstSymFollow->table[k])) {
													hashset_insert(current_follow, firstSymFollow->table[k]);
													changed = true;
												}
											}
										}
									}
                                }
                            }
                        }
                    }
                    else {
						// put everything in follow(A) in follow(tokens[j])
                        HashSet* current_set = hashmap_get(follow, *(char**)arraylist_get(tokens, j));
                        HashSet* could_get_set = hashmap_get(follow, A);
                        for (int k = 0; k < could_get_set->capacity; k++) {
                            if (could_get_set->table[k] && could_get_set->table[k] != TOMBSTONE) {
                                if (!hashset_contains(current_set, could_get_set->table[k])) {
                                    hashset_insert(current_set, could_get_set->table[k]);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
            arraylist_free(tokens);
            free(buffer);
        }
    } while (changed);
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

void build_parsing_tables() {
    for (int i = 0; i < states->size; i++) {
        for (int j = 0; j < terminalsList->size; j++) {
            actionTable[i][j] = strdup("error");
        }
        for (int j = 0; j < nonterminalsList->size; j++) {
            gotoTable[i][j] = -1;
        }
    }

    for (int i = 0; i < states->size; i++) {
        State* s = arraylist_get(states, i);
		ArrayList* symbols = arraylist_init(sizeof(char**), 100);

        for (int j = 0; j < s->items->size; j++) {
            char* sym = get_next_symbol(arraylist_get(s->items, j));
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
            }
        }
        for (int k = 0; k < symbols->size; k++) {
            State* g = goto_state(s, *(char**)arraylist_get(symbols, k));
            if (g->items->size == 0) {
                free(g);
                continue;
            }
            int j = find_state(g);
            if (j == -1) {
                free(g);
                continue;
            }
            if (!(isupper((*(char**)arraylist_get(symbols, k))[0]) || strcmp(*(char**)arraylist_get(symbols, k), "START'") == 0)) {
                int termIdx = getTerminalIndex(*(char**)arraylist_get(symbols, k));
                if (termIdx != -1) {
                    char* buf = malloc(sizeof(int) + 3);
                    snprintf(buf, sizeof(buf), "s%d", j);
                    free(actionTable[i][termIdx]);
                    actionTable[i][termIdx] = strdup(buf);
                    free(buf);
                }
            }
            else { 
                int ntIdx = getNonterminalIndex(*(char**)arraylist_get(symbols, k));
                if (ntIdx != -1) {
                    gotoTable[i][ntIdx] = j;
                }
            }
            free(g);
        }
    }

    for (int i = 0; i < states->size; i++) {
        State* s = arraylist_get(states, i);
        for (int j = 0; j < s->items->size; j++) {
			LRItem* item = arraylist_get(s->items, j);  
            int tokenCount = 0;
            char* buf;
            buf = strdup(item->rule->ruleContent);
            char* t = strtok(buf, " ");
            while (t != NULL) {
                tokenCount++;
                t = strtok(NULL, " ");
            }
            free(buf);

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
                    for (int k = 0; k < terminalsList->size; k++) {
                        
                        if (/*follow[Aidx][k] */ hashset_contains(hashmap_get(follow, item->rule->nonterminal), *(char**)arraylist_get(terminalsList, k))) {
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
    for (int j = 0; j < terminalsList->size; j++) {
        printf("%s\t", *(char**)arraylist_get(terminalsList, j));
    }
    printf("\n");
    for (int i = 0; i < states->size; i++) {
        printf("%d\t", i);
        for (int j = 0; j < terminalsList->size; j++) {
            printf("%s\t", actionTable[i][j]);
        }
        printf("\n");
    }
    printf("\nGOTO TABLE:\n");
    printf("State\t");
    for (int j = 0; j < nonterminalsList->size; j++) {
        printf("%s\t", *(char**)arraylist_get(nonterminalsList, j));
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







int find_column_of_terminal_in_table(char* terminal) {
    int i;
    for (i = 0; i < terminalsList->size && strcmp(*(char**) arraylist_get(terminalsList, i), terminal); i++);
    if (i < terminalsList->size) return i;
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
    
    for (int i = 0; i < states->size; i++) {
        print_state(arraylist_get(states,i), i);
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
