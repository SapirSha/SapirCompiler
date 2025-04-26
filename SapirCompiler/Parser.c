#include "Parser.h"
#include "ParserTableGenerator.h"
#include "Stack.h"
#include "SyntaxTree.h"
#include <stdlib.h>
#include "Boolean.h"
#include "ErrorHandler.h"
#include <stdio.h>

typedef enum {
	TERMINAL,
	NONTERMINAL
} ValueType;

typedef struct {
	ValueType type;
	void* value;
} Value;

static int* new_int_with_allocation(int integer) {
	int* pointer = malloc(sizeof(int));
	if (pointer) {

		*pointer = integer;
		return pointer;
	}
	else {
		handle_out_of_memory_error();
		return NULL;
	}
}

static Value* create_value(ValueType type, void* value) {
	Value* val = malloc(sizeof(Value));
	if (!val) {
		handle_out_of_memory_error();
		return NULL;
	}
	val->type = type;
	val->value = value;
	return val;
}

static inline ActionCell get_next_action(int state, Token* token) {
	return actionTable[state][associationArray[token->type]];
}

static inline ActionCell get_next_goto(int* state, Rule* rule) {
	return (ActionCell) { .type = GOTO_ACTION, .value = gotoTable[*state][rule->nonterminal_position] };
}

static BOOLEAN handle_error(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	printf("ERROR ACTION");
	return False;
}

static BOOLEAN handle_shift(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	stack_push(states, new_int_with_allocation(action->value));
	Token* token = queue_dequeue(tokens);
	SyntaxTree* node = create_terminal_node(*token);
	stack_push(nodes, node);
	*action = get_next_action(action->value, queue_peek(tokens));
	return True;
}

static SyntaxTree* add_latest_nodes_to_tree(SyntaxTree* tree, Stack* nodes, Stack* states, Rule* rule, int rule_length) {
	for (int i = 0; i < rule_length; i++) {
		free(stack_pop(states));
		SyntaxTree* child = stack_pop(nodes);
		tree->info.nonterminal_info.children[rule_length - i - 1] = child;
	}
	return tree;
}

static inline void parent_is_unneccessary_to_save(Stack* states) {
	free(stack_pop(states));
}

static BOOLEAN handle_reduce(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	int rule_id = action->value;
	Rule* rule = rules->array[rule_id];
	int rule_length = rule->ruleTerminalCount;
	if (rule_length == 1)
		parent_is_unneccessary_to_save(states);
	else {
		SyntaxTree* node = create_nonterminal_node(rule->nonterminal, rule_length);
		node = add_latest_nodes_to_tree(node, nodes, states, rule, rule_length);
		stack_push(nodes, node);
	}
	*action = get_next_goto(stack_peek(states), rule);
	return True;
}

static BOOLEAN handle_accept(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	printf("ACCEPT");
	return False;
}

static BOOLEAN handle_goto(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	if (action->value == -1) {
		printf("ERROR GOTO\n");
		return False;
	}
	else {
		stack_push(states, new_int_with_allocation(action->value));
		*action = get_next_action(action->value, queue_peek(tokens));
		return True;
	}
}

static ActionCell* new_actioncell(ACTION_TYPE type, int value) {
	ActionCell* action = malloc(sizeof(ActionCell));
	if (action == NULL) {
		handle_out_of_memory_error();
		return NULL;
	}
	action->type = type;
	action->value = value;
	return action;
}


BOOLEAN(*action_functions[])(ActionCell*, Queue*, Stack*, Stack*) = {
	[ERROR_ACTION] = handle_error,
	[SHIFT_ACTION] = handle_shift,
	[REDUCE_ACTION] = handle_reduce,
	[ACCEPT_ACTION] = handle_accept,
	[GOTO_ACTION] = handle_goto,
};

void printInt(int* num) {
	printf("%d", *num);
}

SyntaxTree* commit_parser(Queue* tokens) {
	Stack* states = stack_init();
	Stack* nodes = stack_init();

	stack_push(states, calloc(1, sizeof(int)));
	ActionCell* action = new_actioncell(0, 0);

	*action = get_next_action(0, queue_peek(tokens));

	BOOLEAN loop = True;
	while (loop) {
		loop = action_functions[action->type](action, tokens, nodes, states);
	}

	SyntaxTree* root = NULL;
	if (action->type == ACCEPT_ACTION) {
		free_Syntax_tree(stack_pop(nodes));
		root = stack_pop(nodes);

		printf("\nRANKS:\n");
		print_tree_with_ranks(root);

		printf("\nPOST:\n");
		print_tree_postorder(root);
	}
	else root = NULL;

	free(action);
	return root;
}
