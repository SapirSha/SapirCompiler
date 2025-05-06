#include "ParserTableGenerator.h"
#include "HashSet.h"
#include "HashMap.h"
#include "Stack.h"

HashMap* follow;

static bool add_symbol_as_follow(char* to_follow_nonterminal, char* followed_by_symbol);

static bool inline add_terminal_as_follow(char* nonterminal, char* terminal) {
    return hashset_insert(hashmap_get(follow, nonterminal), terminal);
}

// initialize follow structure
static void init_follow() {
    follow = createHashMap(nonterminal_list->size, string_hash, string_equals);
    // for every nonterminal create a hashset that represents the possible terminals that can be after it
    for (int i = 0; i < nonterminal_list->size; i++) {
        char* nonterminal = *(char**)arraylist_get(nonterminal_list, i);
        hashmap_insert(follow, nonterminal, hashset_create(terminalsList->size, string_hash, string_equals));
    }

    add_terminal_as_follow("START'", "$");
}

// add the first symbol in a nonterminal rule as a follow to the nonterminal
static bool add_nonterminals_rules_first_symbols_as_follows(char* to_follow_nonterminal, char* followed_by_nonterminal) {
    bool changed = false;
    Stack* nonterminals_rules = get_all_nonterminals_rule(followed_by_nonterminal);
    while (nonterminals_rules->size != 0) {
        Rule* rule = stack_pop(nonterminals_rules);
        char* first_symbol = get_first_symbol(rule->ruleContent);
        add_symbol_as_follow(to_follow_nonterminal, first_symbol);
        free(first_symbol);
    }
    stack_free(nonterminals_rules, NULL);
    return changed;
}

static bool add_symbol_as_follow(char* to_follow_nonterminal, char* followed_by_symbol) {
    char* symbol_dup = strdup(followed_by_symbol);
    bool changed = false;
    // if the symbol as a nonterminal, all its first possible symbols are follows
    if (isNonterminal(followed_by_symbol))
        changed = add_nonterminals_rules_first_symbols_as_follows(to_follow_nonterminal, symbol_dup);
    else
        changed = add_terminal_as_follow(to_follow_nonterminal, symbol_dup);
    if (!changed) free(symbol_dup);
    return changed;
}

// adds to the last symbol nonterminal's follow
static bool add_last_symbol_nonterminals_follow(char* nonterminal, char* last_symbol) {
    if (isNonterminal(last_symbol)) {
        HashSet* nonterminal_follows = hashmap_get(follow, nonterminal);
        HashSet* last_symbol_follows = hashmap_get(follow, last_symbol);
        return hashset_union(last_symbol_follows, nonterminal_follows);
    }
    return false;
}

static Stack* split_content_into_symbols(char* content) {
    Stack* result = stack_init();
    char* content_dup = strdup(content);
    char* sym = strtok(content_dup, " "); // symbols are separated by spaces
    while (sym) {
        stack_push(result, strdup(sym));
        sym = strtok(NULL, " ");
    }
    free(content_dup);
    return result;
}

// go through the rule and add follows
static bool add_appropriate_follows_according_to_rule_contents(Rule* rule) {
    bool changed = false;
    Stack* rules_symbols = split_content_into_symbols(rule->ruleContent);

    char* last_symbol = stack_pop(rules_symbols); // last symbol is first in the stack
    changed |= add_last_symbol_nonterminals_follow(rule->nonterminal, last_symbol);
    char* current_symbol = last_symbol; // would be prev_symbol

    while (rules_symbols->size > 0) {
        char* after_symbol = current_symbol; // after symbol comes first becuase its a stack
        current_symbol = stack_pop(rules_symbols);
        if (isNonterminal(current_symbol))
            changed |= add_symbol_as_follow(current_symbol, after_symbol);
        free(after_symbol);
    }

    free(current_symbol);
    stack_free(rules_symbols, free);
    return changed;
}

/*
A function that fills the follow structure
* A hashmap with nonterminals as keys and possible terminals that can appear after the nonterminal as content
*/
void compute_follow() {
    init_follow();
    bool changed = true;
    // if somthing changed need to check if something new can be added
    while (changed) {
        changed = false;
        // go through all the rules
        for (int i = 0; i < rules->size; i++) {
            Rule* currentRule = (Rule*)rules->array[i];
            changed |= add_appropriate_follows_according_to_rule_contents(currentRule);
        }
    }
}

void free_follow() {
    for (int i = 0; i < follow->capacity; i++) {
        HashMapNode* node = follow->buckets[i];
        while (node) {
            HashMapNode* temp = node;
            node = node->next;
            hashset_free(temp->value);
        }
    }
    freeHashMap(follow);
}