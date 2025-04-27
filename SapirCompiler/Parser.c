#include "Parser.h"
#include "ParserTableGenerator.h"
#include "Stack.h"
#include "SyntaxTree.h"
#include <stdlib.h>
#include "Boolean.h"
#include "ErrorHandler.h"
#include <stdio.h>

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

static inline ActionCell get_next_action(int state, Token* token) {
	return actionTable[state][associationArray[token->type]];
}

static inline ActionCell get_next_goto(int* state, Rule* rule) {
	return (ActionCell) { .type = GOTO_ACTION, .value = gotoTable[*state][rule->nonterminal_position] };
}

static SyntaxTree* get_rightmost_child_of_tree(SyntaxTree* tree) {
	while (tree->type == NONTERMINAL_TYPE) {
		int number_of_children = tree->info.nonterminal_info.num_of_children;
		SyntaxTree** children = tree->info.nonterminal_info.children;
		tree = children[number_of_children - 1];
	}
	return tree;
}

static Token* get_latest_shifted_token_form_nodes(Stack* nodes) {
	SyntaxTree* node = stack_peek(nodes);
	SyntaxTree* right_most_leaf = get_rightmost_child_of_tree(node);
	return &right_most_leaf->info.terminal_info.token;
}

static ArrayList* get_allowed_terminals_in_state(int state_id) {
	int terminals_length = terminalsList->size;
	ArrayList* allowed_terminals = arraylist_init(sizeof(char*), 10);

	for (int i = 0; i < terminals_length; i++) {
		if (actionTable[state_id][i].type != ERROR_ACTION)
			arraylist_add(allowed_terminals, terminalsList->array[i]);
	}

	return allowed_terminals;
}

static ArrayList* get_allowed_nonterminals_in_state(int state_id) {
	int nonterminals_length = nonterminalsList->size;
	ArrayList* allowed_statements = arraylist_init(sizeof(char*), 10);

	for (int i = 0; i < nonterminals_length; i++) {
		if (gotoTable[state_id][i] != -1)
			arraylist_add(allowed_statements, nonterminalsList->array[i]);
	}

	return allowed_statements;
}

static BOOLEAN handle_error(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	int* current_state = stack_peek(states);
	Token* latest_passed_token = get_latest_shifted_token_form_nodes(nodes);
	Token* next_token = queue_peek(tokens);
	ArrayList* allowed_terminals = get_allowed_terminals_in_state(*current_state);
	ArrayList* allowed_nonterminals = get_allowed_nonterminals_in_state(*current_state);

	handle_parser_error(current_state, latest_passed_token, next_token, allowed_terminals, allowed_nonterminals);
	return False;
}

static inline void add_next_token_as_node(Queue* tokens, Stack* nodes) {
	Token* token = queue_dequeue(tokens);
	SyntaxTree* node = create_terminal_node(*token);
	free(token);
	stack_push(nodes, node);
}

static BOOLEAN handle_shift(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	int next_state = action->value;
	stack_push(states, new_int_with_allocation(next_state));
	add_next_token_as_node(tokens, nodes);
	*action = get_next_action(action->value, queue_peek(tokens));
	return True;
}

static void add_latest_nodes_to_tree(SyntaxTree* tree, Stack* nodes, Stack* states, Rule* rule, int rule_length) {
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

static Rule* lookup_rule_for_action(ActionCell* action) {
	int rule_id = action->value;
	return rules->array[rule_id];
}

static inline void reduce_multiple_children_to_parent(Stack* nodes, Stack* states, Rule* rule) {
	int rule_length = rule->ruleTerminalCount;
	SyntaxTree* node = create_nonterminal_node(rule->nonterminal, rule_length);
	add_latest_nodes_to_tree(node, nodes, states, rule, rule_length);
	stack_push(nodes, node);
}

static BOOLEAN handle_reduce(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	Rule* rule = lookup_rule_for_action(action);
	int rule_length = rule->ruleTerminalCount;
	if (rule_length == 1) {
		parent_is_unneccessary_to_save(states);
	}
	else {
		reduce_multiple_children_to_parent(nodes, states, rule);
	}
	*action = get_next_goto(stack_peek(states), rule);
	return True;
}

static BOOLEAN handle_accept(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	return False;
}

static BOOLEAN handle_goto(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	if (action->value == -1)
		return handle_error(action, tokens, nodes, states);

	stack_push(states, new_int_with_allocation(action->value));
	*action = get_next_action(action->value, queue_peek(tokens));
	return True;
}

static ActionCell* new_actioncell() {
	ActionCell* action = malloc(sizeof(ActionCell));
	if (action == NULL) {
		handle_out_of_memory_error();
		return NULL;
	}
	return action;
}

static BOOLEAN(*action_functions[])(ActionCell*, Queue*, Stack*, Stack*) = {
	[ERROR_ACTION] = handle_error,
	[SHIFT_ACTION] = handle_shift,
	[REDUCE_ACTION] = handle_reduce,
	[ACCEPT_ACTION] = handle_accept,
	[GOTO_ACTION] = handle_goto,
};

static inline BOOLEAN call_action_type_function(ActionCell* looked_at_action, Queue* tokens, Stack* nodes, Stack* states) {
	return action_functions[looked_at_action->type](looked_at_action, tokens, nodes, states);
}

static inline SyntaxTree* try_to_extract_main_root_from_stack(ActionCell* last_action, Stack* nodes) {
	SyntaxTree* root = NULL;
	if (last_action->type == ACCEPT_ACTION) {
		free_Syntax_tree(stack_pop(nodes));
		root = stack_pop(nodes);
	}
	return root;
}

static void init_parser_variables(ActionCell** action, Queue* tokens, Stack** nodes, Stack** states) {
	*nodes = stack_init();
	*states = stack_init();
	*action = new_actioncell();

	stack_push(*states, new_int_with_allocation(0));
	**action = get_next_action(0, queue_peek(tokens));
}

static void handle_parsing_loop(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	BOOLEAN should_continue = True;
	while (should_continue) {
		should_continue = call_action_type_function(action, tokens, nodes, states);
	}
}

static void parser_free(ActionCell* action, Queue* tokens, Stack* nodes, Stack* states) {
	queue_free(tokens);
	stack_free(nodes, free_Syntax_tree);
	stack_free(states, free);
	free(action);
}

SyntaxTree* commit_parser(Queue* tokens) {
	Stack* states = NULL;
	Stack* nodes = NULL;
	ActionCell* action = NULL;
	init_parser_variables(&action, tokens, &nodes, &states);

	handle_parsing_loop(action, tokens, nodes, states);

	SyntaxTree* root = try_to_extract_main_root_from_stack(action, nodes);

	if (root) {
		print_tree_postorder(root);
		print_tree_with_ranks(root);
	}

	parser_free(action, tokens, nodes, states);

	return root;
}
