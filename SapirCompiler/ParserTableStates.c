#include "ParserTableGenerator.h"
#include <stdio.h>
#include <string.h>
#include "ErrorHandler.h"

ArrayList* states;

static inline bool lr_item_equals(LRItem* item1, LRItem* item2) {
    return item1->rule->ruleID == item2->rule->ruleID && item1->dot == item2->dot;
}

static int compare_lr_items(LRItem* item1, LRItem* item2) {
    return lr_item_equals(item1, item2) ? 0 : 1;
}

static inline bool lr_arraylist_contains_lr_items(ArrayList* lr_items_list, LRItem* item) {
    return arraylist_contains(lr_items_list, item, compare_lr_items);
}

static bool lr_arraylist_equals(ArrayList* lr_items_list1, ArrayList* lr_items_list2) {
    if (!lr_items_list1 || !lr_items_list2) return false;
    if (lr_items_list1->size != lr_items_list2->size) return false;
    int size = lr_items_list1->size;
    for (int i = 0; i < size; i++) {
        LRItem* item_from_list1 = lr_items_list1->array[i];
        if (!lr_arraylist_contains_lr_items(lr_items_list2, item_from_list1))
            return false;
    }
    return true;
}

bool state_equals(Parser_State* s1, Parser_State* s2) {
    if (!s1 || !s2) return false;
    return lr_arraylist_equals(s1->items, s2->items);
}

int find_state(Parser_State* s) {
    for (int i = 0; i < states->size; i++) {
        if (state_equals(arraylist_get(states, i), s))
            return i;
    }
    return -1;
}

static inline bool state_exists(Parser_State* state) {
    return find_state(state) != -1;
}

static inline bool is_state_empty(Parser_State* state) {
    return state->items->size == 0;
}

/*
 A function that checks if a state has a certain LR Item inside it
 * LRItems items with the same rules can be different if they have different dot position
*/
static bool state_contains_item(Parser_State* s, LRItem* item) {
    if (lr_arraylist_contains_lr_items(s->items, item))
        return true;
    return false;
}

static void add_nonterminal_rules_to_state(char* nonterminal, Parser_State* state) {
    Stack* nonterminal_rules = get_all_nonterminals_rule(nonterminal);
    while (nonterminal_rules->size != 0) {
        Rule* rule = stack_pop(nonterminal_rules);
        LRItem new_state_item = (LRItem){ .rule = rule , .dot = 0 };
        if (!state_contains_item(state, &new_state_item))
            arraylist_add(state->items, &new_state_item);
    }
    stack_free(nonterminal_rules, NULL);
}

/*
 A function that commits closure for a certain state:
 * If a state can get a nonterminal, it can also get the terminals that its made of.
   In this function, we add to the current state all the rules of the nonterminals that can be got,
   with dot at zero (start of a new rule)
*/
static void closure(Parser_State* state) {
    // go through every lr item in the state
    for (int item_index = 0; item_index < state->items->size; item_index++) {
        LRItem* item = arraylist_get(state->items, item_index);
        char* next_symbol = get_next_symbol(item);
        // if the lr item indicates that a nonterminal can be gotten
        if (isNonterminal(next_symbol)) {
            // search for all the rules where the nonterminal is the left item, and add them to the state
            add_nonterminal_rules_to_state(next_symbol, state);
        }
        free(next_symbol);
    }
}

static Parser_State* parser_state_init() {
    Parser_State* newState = malloc(sizeof(Parser_State));
    if (!newState) {
        handle_out_of_memory_error();
        return NULL;
    }
    newState->items = arraylist_init(sizeof(LRItem), DEFAULT_AMOUNT_OF_LR_ITEMS);
    return newState;
}

static bool LRItem_has_transition_through_symbol(LRItem* item, char* symbol) {
    char* after_dot = get_next_symbol(item);
    if (after_dot) {
        if (strcmp(after_dot, symbol) == 0) {
            free(after_dot);
            return true;
        }
        free(after_dot);
    }
    return false;
}

static void add_transition_rules_from_previous_state(Parser_State* prev, Parser_State* new_state, char* transition_symbol) {
    for (int item_index = 0; item_index < prev->items->size; item_index++) {
        LRItem current = *(LRItem*)prev->items->array[item_index];
        if (LRItem_has_transition_through_symbol(&current, transition_symbol)) {
            current.dot++; // eat the symbol
            arraylist_add(new_state->items, &current);
        }
    }
}

/*
 This is the function that creates the new states:
 * This function create a state for an allowed symbol in the state,
 * It gets all the possibilities in the previous state to the new state and not one
*/
Parser_State* goto_state(Parser_State* prev_state, char* symbol) {
    Parser_State* newState = parser_state_init();
    // find all the rules in the previous state that have 'symbol' as an allowed possibility, and add them
    add_transition_rules_from_previous_state(prev_state, newState, symbol);
    closure(newState);
    return newState;
}

static Rule* add_starting_rule(const char* start_nonterminal) {
    int len = strlen(start_nonterminal) + 4;
    char* buffer = malloc(len);
    if (!buffer) handle_out_of_memory_error();
    snprintf(buffer, len, "%s $", start_nonterminal);
    // add an initial rule for the tables to start at, and add startnonterminal as its content
    add_rule("START'", buffer);
    free(buffer);
    return rules->array[rules->size - 1];
}

static Parser_State* init_starting_state(Rule* starting_rule) {
    Parser_State* State0 = malloc(sizeof(Parser_State));
    if (!State0) {
        handle_out_of_memory_error();
        return NULL;
    }
    State0->items = arraylist_init(sizeof(LRItem), DEFAULT_AMOUNT_OF_LR_ITEMS);
    LRItem startItem = { .rule = starting_rule, .dot = 0 };
    arraylist_add(State0->items, &startItem);
    return State0;
}

static int compare_string_pointers(char** pstr1, char** pstr2) {
    if (!pstr1 || !pstr2 || !*pstr1 || !*pstr2) return pstr1 == pstr2;
    return strcmp(*pstr1, *pstr2);
}

static void add_symbol_if_doesnt_exist(ArrayList* symbol_list, char* symbol) {
    if (!arraylist_contains(symbol_list, &symbol, compare_string_pointers)) {
        char* temp = strdup(symbol);
        arraylist_add(symbol_list, &temp);
    }
}

static ArrayList* get_possible_transition_symbols(Parser_State* prev_state) {
    ArrayList* symbolList = arraylist_init(sizeof(char*), 25);
    for (int j = 0; j < prev_state->items->size; j++) {
        char* sym = get_next_symbol(arraylist_get(prev_state->items, j));
        if (sym != NULL) {
            add_symbol_if_doesnt_exist(symbolList, sym);
            free(sym);
        }
    }
    return symbolList;
}

static void free_symbol_list(ArrayList* symbol_list) {
    for (int i = 0; i < symbol_list->size; i++)
        free(*(char**)symbol_list->array[i]);
    arraylist_free(symbol_list);
}

static void create_a_new_state_for_single_transition(Parser_State* from_state, char* transition_symbol) {
    Parser_State* new_state = goto_state(from_state, transition_symbol);
    if (!is_state_empty(new_state)) {
        if (!state_exists(new_state))
            arraylist_add(states, new_state);
        else
            free(new_state);
    }
    else
        free(new_state);
}

static void create_a_new_states_for_transitions(Parser_State* prev_state, ArrayList* symbol_list) {
    for (int k = 0; k < symbol_list->size; k++) {
        char* symbol = *(char**)arraylist_get(symbol_list, k);
        create_a_new_state_for_single_transition(prev_state, symbol);
    }
}

static void create_transition_states_for_possible_transitions() {
    // go through all the states (states size increases)
    for (int i = 0; i < states->size; i++) {
        Parser_State* current_state = arraylist_get(states, i);
        // list of possible symbols to get in the state
        ArrayList* symbol_list = get_possible_transition_symbols(current_state);

        // go through all the possible symbols that can be got in the current state
        create_a_new_states_for_transitions(current_state, symbol_list);

        free_symbol_list(symbol_list);
    }
}

/*
 Builds the states
*/
void build_states(const char* start_nonterminal) {
    states = arraylist_init(sizeof(Parser_State), DEFAULT_NUMBER_OF_STATES);
    Rule* starting_rule = add_starting_rule(start_nonterminal);
    Parser_State* state0 = init_starting_state(starting_rule);

    closure(state0);
    arraylist_add(states, state0);
    free(state0);

    create_transition_states_for_possible_transitions();
}

void free_states() {
    for (int i = 0; i < states->size; i++) {
        Parser_State* temp = states->array[i];
        arraylist_free(temp->items);
    }
    free(states);
}
