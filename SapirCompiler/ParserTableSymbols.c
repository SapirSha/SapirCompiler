#include "ParserTableGenerator.h"

ArrayList* nonterminalsList;
ArrayList* terminalsList;




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

void free_non_and_terminals() {
    for (int i = 0; i < terminalsList->size; i++) {
        free(*(char**)terminalsList->array[i]);
    }
    arraylist_free(terminalsList);

    for (int i = 0; i < nonterminalsList->size; i++) {
        free(*(char**)nonterminalsList->array[i]);
    }
    arraylist_free(nonterminalsList);
}


