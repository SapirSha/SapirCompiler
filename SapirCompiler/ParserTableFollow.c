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

static bool add_nonterminals_rules_first_symbols_as_follows(char* to_follow_nonterminal, char* followed_by_nonterminal) {
    Rule* current;
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
    if (isNonterminal(followed_by_symbol))
        changed = add_nonterminals_rules_first_symbols_as_follows(to_follow_nonterminal, symbol_dup);
    else
        changed = add_terminal_as_follow(to_follow_nonterminal, symbol_dup);
    if (!changed) free(symbol_dup);
    return changed;
}

static bool add_rules_content_to_nonterminals_follow(char* nonterminal, Rule* rule) {
    char* first_symbol = get_first_symbol(rule->ruleContent);
    bool changed = add_symbol_as_follow(nonterminal, first_symbol);
    free(first_symbol);
    return changed;
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
                        add_symbol_as_follow(current_symbol, after_symbol);
                    }
                    else { // end of rule's content
                        // add to last symbol's follows, the current rule's follows
                        HashSet* LastSymbolSet = hashmap_get(follow, current_symbol);
                        HashSet* currentFollow = hashmap_get(follow, currentrule_nonterminal);
                        changed |= hashset_union(LastSymbolSet, currentFollow);
                    }
                }
            }
            if (!changed) free(contentCopy);
            arraylist_free(tokens);
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