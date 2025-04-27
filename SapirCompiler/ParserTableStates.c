#include "ParserTableGenerator.h"
ArrayList* states;


bool state_equals(Parser_State* s1, Parser_State* s2) {
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

int find_state(Parser_State* s) {
    for (int i = 0; i < states->size; i++) {
        if (state_equals(arraylist_get(states, i), s))
            return i;
    }
    return -1;
}

/*
 A function that checks if a state has a certain LR Item inside it
 * LRItems items with the same rules can be different if they have different dot position
*/
static bool state_contains_item(Parser_State* s, LRItem* item) {
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
static void closure(Parser_State* s) {
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
Parser_State* goto_state(Parser_State* s, const char* symbol) {
    Parser_State* newState = malloc(sizeof(Parser_State));
    if (!newState) {
        handle_out_of_memory_error();
        return NULL;
    }
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

/*
 Builds the states
*/
void build_states(const char* startNonterminal) {
    states = arraylist_init(sizeof(Parser_State), DEFAULT_NUMBER_OF_STATES);

    int len = strlen(startNonterminal) + 4;
    char* buffer = malloc(len);
    if (!buffer) handle_out_of_memory_error();
    snprintf(buffer, len, "%s $", startNonterminal);
    // add an initial rule for the tables to start at, and add startnonterminal as its content
    add_rule("START'", buffer);
    free(buffer);
    Parser_State* State0 = malloc(sizeof(Parser_State));
    if (!State0) handle_out_of_memory_error();
    State0->items = arraylist_init(sizeof(LRItem), DEFAULT_AMOUNT_OF_LR_ITEMS);
    LRItem startItem = { .rule = ((Rule*)rules->array[rules->size - 1]), .dot = 0 };
    arraylist_add(State0->items, &startItem);
    closure(State0);
    arraylist_add(states, State0);
    free(State0);
    // go through all the states (states size increases)
    for (int i = 0; i < states->size; i++) {
        Parser_State* s = arraylist_get(states, i);
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
            Parser_State* g = goto_state(s, symbol);
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
        for (int i = 0; i < symbolList->size; i++)
            free(*(char**)symbolList->array[i]);
        arraylist_free(symbolList);
    }
}



void free_states() {
    for (int i = 0; i < states->size; i++) {
        Parser_State* temp = states->array[i];
        arraylist_free(temp->items);
    }
    free(states);
}
