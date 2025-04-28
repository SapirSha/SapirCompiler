#include "ParserTableGenerator.h"
#include <stdio.h>
#include "HashMap.h"
#include "HashSet.h"
#include <ctype.h>
#include "ErrorHandler.h"

char* actiontypetostring(int action) {
    static const char* action_type_string_lookup[5] = {
        [ERROR_ACTION] = "E",
        [SHIFT_ACTION] = "S",
        [REDUCE_ACTION] = "R",
        [ACCEPT_ACTION] = "A",
        [GOTO_ACTION] = "G",
    };

    return action_type_string_lookup[action];
}

/*
A function that determines if a symbol is nonterminal:
 *  A symbol is a nonterminal if his first letter is uppercase
*/
bool isNonterminal(const char* symbol) {
    return (symbol && isupper(symbol[0]));
}

/*
A function that counts the number of symbols in the rule
 * symbols are separated by spaces (' ')
*/
int count_symbols(const char* ruleContent) {
    char* content_dup = strdup(ruleContent);
    char* symbol = strtok(content_dup, " ");
    int count = 0;
    while (symbol != NULL) {
        count++;
        symbol = strtok(NULL, " ");
    }
    free(content_dup);
    return count;
}

/*
A function that returns the token in position n in the input
 * Position is the number of symbols passed, and not character position in the string
*/
char* get_nth_token(char* content, int n) {
    char* content_dup = strdup(content);
    char* symbol = strtok(content_dup, " ");
    while(n-- > 0)
        symbol = strtok(NULL, " ");
    char* result = symbol? strdup(symbol) : NULL;
    free(content_dup);
    return result;
}

char* get_first_symbol(char* content) {
    return get_nth_token(content, 0);
}

/*
 A function that returns the symbol after the dot in an LRItem
*/
char* get_next_symbol(LRItem* item) {
    return get_nth_token(item->rule->ruleContent, item->dot);
}


void print_string_arraylist(char** sym) {
    printf("%s ", *sym);
}

void print_string(char* sym) {
    printf("%s ", sym);
}

void print_follows() {
    printf("FOLLOWS:\n");
    for (int i = 0; i < nonterminal_list->size; i++) {
        char* str = *(char**)arraylist_get(nonterminal_list, i);
        printf("%s:\t\t\t", str);
        hashset_print(hashmap_get(follow, str), print_string);
    }
}

void print_rules() {
    for (int i = 0; i < rules->size; i++) {
        Rule* r = (Rule*)rules->array[i];
        printf("Rule %d: %s -> %s (length: %d) - pos %d\n", r->ruleID, r->nonterminal, r->ruleContent, r->ruleTerminalCount, r->nonterminal_position);
    }
}

int find_row_of_nonterminal_in_table(const char* nonterminal) {
    int i = get_nonterminal_index(nonterminal);
    if (i != -1) return i;
    else {
        handle_other_errors("\t--- UNKNOWN NONTERMINAL - INVALID BNF");
        exit(-1);
    }
}

int find_column_of_terminal_in_table(const char* terminal) {
    int i = get_terminal_index(terminal);
    if (i != -1) return i;
    else {
        handle_other_errors("\t--- UNKNOWN TERMINAL - INVALID BNF");
        exit(-1);
    }
}

void set_nonterminals_position() {
    for (int i = 0; i < rules->size; i++) {
        Rule* rule = arraylist_get(rules, i);
        rule->nonterminal_position = find_row_of_nonterminal_in_table(rule->nonterminal);
    }
}

void print_parsing_tables() {
    printf("ACTION TABLE:\n\t");
    for (int j = 0; j < terminalsList->size; j++) {
        printf("%.5s \t", *(char**)arraylist_get(terminalsList, j));
    }
    printf("\n");
    for (int i = 0; i < states->size; i++) {
        printf("%d\t", i);
        for (int j = 0; j < terminalsList->size; j++) {
            char buffer[20];
            sprintf(buffer, "%s%d", actiontypetostring(actionTable[i][j].type), actionTable[i][j].value);
            printf("%s\t", buffer);
        }
        printf("\n");
    }
    printf("\nGOTO TABLE:\n\t");
    for (int j = 0; j < nonterminal_list->size; j++) {
        printf("%.5s\t", *(char**)arraylist_get(nonterminal_list, j));
    }
    printf("\n");
    for (int i = 0; i < states->size; i++) {
        printf("%d\t", i);
        for (int j = 0; j < nonterminal_list->size; j++) {
            if (gotoTable[i][j] == -1)
                printf("err\t");
            else
                printf("%d\t", gotoTable[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_state(Parser_State* s) {
    printf("State:\n");
    for (int i = 0; i < s->items->size; i++) {
        LRItem* item = arraylist_get(s->items, i);
        printf("  %s -> ", item->rule->nonterminal);

        char* contentCopy = strdup(item->rule->ruleContent);
        char* token = strtok(contentCopy, " ");
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
        free(contentCopy);
    }
}

void** create_matrix(int rows, int cols, int object_size) {
    void** row_pointers = malloc(sizeof(void*) * rows);
    //char for single byte
    char* matrix = malloc(object_size * rows * cols);
    if (!row_pointers || !matrix) {
        handle_out_of_memory_error();
        return NULL;
    }
    for (int row = 0; row < rows; row++)
        row_pointers[row] = matrix + object_size * row * cols;
    return row_pointers;
}

inline bool is_nonterminals_rule(char* nonterminal, Rule* rule) {
    return strcmp(rule->nonterminal, nonterminal) == 0;
}

Stack* get_all_nonterminals_rule(char* nonterminal) {
    Stack* result = stack_init();
    for (int i = 0; i < rules->size; i++) {
        Rule* current = rules->array[i];
        if (is_nonterminals_rule(nonterminal, current))
            stack_push(result, current);
    }
    return result;
}
