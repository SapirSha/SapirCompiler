#include "ParserTableGenerator.h"

ActionCell** actionTable;
int** gotoTable;


/* Create the action and goto tables*/
void init_tables() {
    actionTable = malloc(states->size * sizeof(ActionCell*));
    if (!actionTable) {
        handle_out_of_memory_error();
        return;
    }

    ActionCell* actionContent = malloc(states->size * terminalsList->size * sizeof(ActionCell));
    if (!actionContent) {
        handle_out_of_memory_error();
        return;
    }

    for (int i = 0; i < states->size; i++)
        actionTable[i] = actionContent + i * terminalsList->size;

    gotoTable = malloc(states->size * sizeof(int*));
    if (!gotoTable) {
        handle_out_of_memory_error();
        return;
    }

    int* gotoContent = malloc(states->size * nonterminalsList->size * sizeof(int));
    if (!gotoContent) {
        handle_out_of_memory_error();
        return;
    }
    for (int i = 0; i < states->size; i++)
        gotoTable[i] = gotoContent + i * nonterminalsList->size;
}

/* fill the tables */
void build_parsing_tables() {
    // initialized all the values as -1 and error
    for (int i = 0; i < states->size; i++) {
        for (int j = 0; j < terminalsList->size; j++) {
            actionTable[i][j] = (ActionCell){ .type = ERROR_ACTION };
        }
        for (int j = 0; j < nonterminalsList->size; j++) {
            gotoTable[i][j] = -1;
        }
    }

    // go through all the states
    for (int i = 0; i < states->size; i++) {
        Parser_State* s = arraylist_get(states, i);

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
            Parser_State* g = goto_state(s, sym);

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
                    actionTable[i][col] = (ActionCell){ .type = SHIFT_ACTION, .value = j };
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
        Parser_State* s = arraylist_get(states, i);

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
                        actionTable[i][dollarIdx] = (ActionCell){ .type = ACCEPT_ACTION };
                    }
                }
                else {
                    // go through all the terminals
                    for (int k = 0; k < terminalsList->size; k++) {
                        char* term = *(char**)arraylist_get(terminalsList, k);

                        // if current item can be followed by the terminal, reduce by the rule
                        if (hashset_contains(hashmap_get(follow, item->rule->nonterminal), term)) {
                            if (actionTable[i][k].type == ERROR_ACTION)
                                actionTable[i][k] = (ActionCell){ .type = REDUCE_ACTION, .value = item->rule->ruleID };
                        }
                    }
                }
            }
        }
    }

}

void free_parser_table() {
    free(*actionTable);
    free(actionTable);

    free(*gotoTable);
    free(gotoTable);
    parser_tables_initialized = false;
}
