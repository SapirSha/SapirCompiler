#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinkedList.h"

#define strdup _strdup

typedef struct NonTerminalRule {
	char* nonterminal;
	char* rule_content;
} NonTerminalRule;


int add_rule(LinkedList* lst, char* nonterminal, char* rule_content) {
	NonTerminalRule* rule = (NonTerminalRule*)malloc(sizeof(NonTerminalRule));
	rule->nonterminal = strdup(nonterminal);
	rule->rule_content = strdup(rule_content);
	linkedlist_add(lst, rule);
}

int compareNonTerminalRule(NonTerminalRule* r1, NonTerminalRule* r2) {
	if (strcmp(r1->nonterminal, r2->nonterminal) != 0)
		return strcmp(r1->nonterminal, r2->nonterminal);
	else {
		return strcmp(r1->rule_content, r2->rule_content);
	}
}

LinkedList* initiate_rules(char* nonterminal, char* rule_content) {
	LinkedList* rules = linkedlist_init(sizeof(NonTerminalRule), compareNonTerminalRule);
	add_rule(rules, nonterminal, rule_content);
	return rules;
}

void print_rule(NonTerminalRule* r1) {
	printf("\n%s -> %s", r1->nonterminal, r1->rule_content);
}

int main() {
	LinkedList* rules = initiate_rules("Z", "S$");
	add_rule(rules, "S", "aB");
	add_rule(rules, "B", "cB");
	add_rule(rules, "B", "d");
	linkedlist_print(rules, print_rule);



	return 0;
}