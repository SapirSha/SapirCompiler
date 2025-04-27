#include "ParserTableGenerator.h"
#include "HashSet.h"
#include "HashMap.h"
HashMap* follow;

// initialize follow structure
void init_follow() {
    follow = createHashMap(nonterminalsList->size, string_hash, string_equals);
    // for every nonterminal create a hashset that represents the possible terminals that can be after it
    for (int i = 0; i < nonterminalsList->size; i++) {
        hashmap_insert(follow, *(char**)arraylist_get(nonterminalsList, i), hashset_create(terminalsList->size, string_hash, string_equals));
    }

    hashset_insert(hashmap_get(follow, "START'"), "$");
}

bool add_rules_content_to_nonterminals_follow(char* nonterminal, Rule* rule) {
    char* next_token = get_nth_token(rule->ruleContent, 0);
    if (next_token == NULL) return false;
    if (!isNonterminal(next_token)) {
        bool changed = hashset_insert(hashmap_get(follow, nonterminal), next_token);
        if (!changed) free(next_token);
        return changed;
    }
    else {
        bool changed = false;
        for (int r2 = 0; r2 < rules->size; r2++)
        {
            Rule* a_rule = arraylist_get(rules, r2);
            if (strcmp(a_rule->nonterminal, next_token) == 0) {
                changed |= add_rules_content_to_nonterminals_follow(nonterminal, a_rule);
            }
        }
        free(next_token);
        return changed;
    }
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
                        // if the symbol after the nonterminal is a terminal its a follow
                        if (!isNonterminal(after_symbol)) {
                            changed |= hashset_insert(hashmap_get(follow, current_symbol), after_symbol);
                        }
                        else {

                            // go through all the rules
                            for (int r = 0; r < rules->size; r++) {
                                Rule* candidate = (Rule*)rules->array[r];

                                // find the contents of the nonterminal after_symbol
                                if (strcmp(candidate->nonterminal, after_symbol) == 0) {
                                    char* sample = strdup(candidate->ruleContent);
                                    char* firstSym = get_nth_token(sample, 0);
                                    free(sample);
                                    // if the first symbol in the content is terminal, add as follow
                                    if (firstSym && !isNonterminal(firstSym)) {
                                        bool f = hashset_insert(hashmap_get(follow, current_symbol), firstSym);
                                        if (!f) free(firstSym);
                                        changed |= f;
                                        break;
                                    }
                                    // if the first symbol is nonterminal, add its follows to current
                                    else if (firstSym && isNonterminal(firstSym)) {
                                        for (int r2 = 0; r2 < rules->size; r2++)
                                        {
                                            Rule* a_rule = arraylist_get(rules, r2);
                                            if (strcmp(a_rule->nonterminal, firstSym) == 0) {
                                                changed |= add_rules_content_to_nonterminals_follow(current_symbol, a_rule);
                                            }
                                        }
                                        free(firstSym);
                                    }
                                    else free(firstSym);
                                }
                            }
                        }
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