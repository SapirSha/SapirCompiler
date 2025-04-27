#include "ParserTableGenerator.h"
#include <stdio.h>
#include "HashMap.h"
#include "HashSet.h"
#include <ctype.h>

char* actiontypetostring(int action) {
    switch (action)
    {
    case ACCEPT_ACTION: return "A";
    case REDUCE_ACTION: return "R";
    case SHIFT_ACTION:  return "S";
    case ERROR_ACTION:  return "E";

    default:
        return NULL;
    }
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
    int count = 0;
    bool inToken = false;
    while (*ruleContent != '\0') {
        if (!isspace((char)*ruleContent) && !inToken) {
            count++;
            inToken = true;
        }
        else if (isspace((char)*ruleContent)) {
            inToken = false;
        }
        ruleContent++;
    }
    return count;
}

/*
A function that returns the token in position n in the input
 * Position is the number of symbols passed, and not character position in the string
*/
char* get_nth_token(char* s, int n) {
    int currentToken = 0;
    char* p = s;
    while (*p) {
        while (*p && isspace((char)*p))
            p++;
        if (*p == '\0') break;
        if (currentToken == n) {
            char* start = p;
            while (*p != '\0' && !isspace((char)*p))
                p++;
            int len = (p - start) / sizeof(char);
            char* token = malloc(len + 1);
            if (!token) {
                handle_out_of_memory_error();
                return NULL;
            }
            strncpy(token, start, len);
            token[len] = '\0';
            return token;
        }
        else {
            while (*p && !isspace((char)*p))
                p++;
            currentToken++;
        }
    }
    return NULL;
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
    for (int i = 0; i < nonterminalsList->size; i++) {
        char* str = *(char**)arraylist_get(nonterminalsList, i);
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
    int i;
    for (i = 0; i < nonterminalsList->size && strcmp(*(char**)arraylist_get(nonterminalsList, i), nonterminal) != 0; i++);
    if (i < nonterminalsList->size)
        return i;
    else {
        handle_other_errors("\n\t---UNKNOWN NONTERMINAL\n");
        exit(1);
    }
}

int find_column_of_terminal_in_table(const char* terminal) {
    int i;
    for (i = 0; i < terminalsList->size && strcmp(*(char**)arraylist_get(terminalsList, i), terminal) != 0; i++);
    if (i < terminalsList->size)
        return i;
    else {
        printf("\n\t---UNKNOWN TERMINAL %s\n", terminal);
        handle_other_errors("\n\t---UNKNOWN TERMINAL \n");
        exit(1);
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
    for (int j = 0; j < nonterminalsList->size; j++) {
        printf("%.5s\t", *(char**)arraylist_get(nonterminalsList, j));
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
