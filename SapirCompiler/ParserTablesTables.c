#include "ParserTableGenerator.h"
#include "HashSet.h"
#include "ErrorHandler.h"

#define DEFUALT_AMOUNT_OF_TRANSITIONS 10

ActionCell** actionTable;
int** gotoTable;

static void init_tables_cells() {
    // initialized all the values as -1 and error
    for (int i = 0; i < states->size; i++) {
        for (int j = 0; j < terminalsList->size; j++) {
            actionTable[i][j] = (ActionCell){ .type = ERROR_ACTION };
        }
        for (int j = 0; j < nonterminal_list->size; j++) {
            gotoTable[i][j] = -1;
        }
    }
}

/* Create the action and goto tables */
static void init_tables() {
    actionTable = (ActionCell**)create_matrix(states->size, terminalsList->size, sizeof(ActionCell));
    gotoTable = (int**)create_matrix(states->size, nonterminal_list->size, sizeof(int));

    init_tables_cells();
}

static void insert_unique_possible_item_transition(Stack* transitions, HashSet* already_in, LRItem* transition_due_to) {
    char* transition_from_item = get_nth_token(transition_due_to->rule->ruleContent, transition_due_to->dot);
    if (transition_from_item != NULL) {
        if (hashset_insert(already_in, transition_from_item))
            stack_push(transitions, transition_from_item);
        else
            free(transition_from_item);
    }
}

static Stack* possible_transitions_in_state(Parser_State* state) {
    Stack* transitions = stack_init();
    HashSet* already_in = hashset_create(DEFUALT_AMOUNT_OF_TRANSITIONS, string_hash, string_equals);
    for (int i = 0; i < state->items->size; i++) {
        LRItem* cur_item = state->items->array[i];
        insert_unique_possible_item_transition(transitions, already_in, cur_item);
    }
    hashset_free(already_in);
    return transitions;
}

static inline Parser_State* transition_state_would_look_like(Parser_State* from_state, char* transition_symbol) {
    Parser_State* state = goto_state(from_state, transition_symbol);
    return state;
}

static inline bool is_real_state(Parser_State* state) {
    return find_state(state) != -1;
}

static void set_transition_from_state_to_state(Parser_State* from_state, Parser_State* to_state, char* transition) {
    int from_state_index = find_state(from_state);
    int to_state_index = find_state(to_state);
    if (isNonterminal(transition)) {
        int col = get_nonterminal_index(transition);
        if (col != -1)
            gotoTable[from_state_index][col] = to_state_index;
    }
    else {
        int col = get_terminal_index(transition);
        if (col != -1)
            actionTable[from_state_index][col] = (ActionCell){ .type = SHIFT_ACTION, .value = to_state_index };
    }
}

static void insert_state_transitions_into_table(Parser_State* from_state) {
    Stack* possible_transitions = possible_transitions_in_state(from_state);
    while (possible_transitions->size > 0) {
        char* transition_symbol = stack_pop(possible_transitions);
        Parser_State* to_state = transition_state_would_look_like(from_state, transition_symbol);
        if (is_real_state(to_state)) 
            set_transition_from_state_to_state(from_state, to_state, transition_symbol);
        free(transition_symbol);
        free(to_state);
    }
    stack_free(possible_transitions, free);
}

static inline bool is_accept_item(LRItem* item) {
    return (strcmp(item->rule->nonterminal, "START'") == 0);
}

static void add_accept_to_state(int state_id) {
    int dollarIdx = get_terminal_index("$");
    if (dollarIdx != -1) {
        actionTable[state_id][dollarIdx] = (ActionCell){ .type = ACCEPT_ACTION };
    }
}

static void add_reduce_to_all_follow_terminals(int state_id, LRItem* item) {
    // go through all the terminals
    for (int k = 0; k < terminalsList->size; k++) {
        char* term = *(char**)arraylist_get(terminalsList, k);

        // if current item can be followed by the terminal, reduce by the rule
        if (hashset_contains(hashmap_get(follow, item->rule->nonterminal), term)) {
            if (actionTable[state_id][k].type == ERROR_ACTION)
                actionTable[state_id][k] = (ActionCell){ .type = REDUCE_ACTION, .value = item->rule->ruleID };
        }
    }
}

static void make_reduce_from_state(int state_id, LRItem* item) {
    // for the start state add accept if at end
    if (is_accept_item(item))
        add_accept_to_state(state_id);
    else
        add_reduce_to_all_follow_terminals(state_id, item);
}

static bool at_end_of_content(LRItem* item) {
    int tokenCount = count_symbols(item->rule->ruleContent);
    return item->dot == tokenCount;
}

/* fill the tables */
void build_parsing_tables() {
    init_tables();

    // go through all the states
    for (int i = 0; i < states->size; i++) {
        Parser_State* state = arraylist_get(states, i);
        insert_state_transitions_into_table(state);
    }

    // go through all the states
    for (int i = 0; i < states->size; i++) {
        Parser_State* s = arraylist_get(states, i);
        // go through each states items
        for (int j = 0; j < s->items->size; j++) {
            LRItem* item = arraylist_get(s->items, j);
            if (at_end_of_content(item))
                make_reduce_from_state(i, item);
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
