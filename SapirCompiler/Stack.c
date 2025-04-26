#include "Stack.h"
#include <stdlib.h>
#include "ErrorHandler.h"
#include <stdio.h>

Stack* stack_init() {
	Stack* stack = malloc(sizeof(Stack));
	if (stack == NULL) {
		handle_out_of_memory_error();
		return NULL;
	}
	stack->head = NULL;
	stack->size = 0;
	return stack;
}

static StackNode* create_node(void* value) {
	StackNode* node = malloc(sizeof(StackNode));
	if (node == NULL) {
		handle_out_of_memory_error();
		return NULL;
	}
	node->value = value;
	node->next = NULL;
	return node;
}

void stack_push(Stack* stack, void* value) {
	StackNode* node = create_node(value);
	node->next = stack->head;
	stack->head = node;
	stack->size++;
}
void* stack_pop(Stack* stack) {
	if (stack->size == 0) return NULL;
	
	StackNode* node = stack->head;
	stack->head = node->next;
	void* value = node->value;
	free(node);
	stack->size--;
	return value;
}

void* stack_peek(Stack* stack) {
	if (stack->size == 0) return NULL;
	return stack->head->value;
}

void stack_free(Stack* stack, void free_function(void*)) {
	StackNode* current = stack->head;
	while (current != NULL) {
		StackNode* next = current->next;
		free_function(current->value);
		free(current);
		current = next;
	}
	free(stack);
}
void stack_print(Stack* stack, void (*print_function)(void*)) {
	StackNode* current = stack->head;
	while (current != NULL) {
		print_function(current->value);
		current = current->next;
		if (current != NULL) printf(", ");
	}
	printf("\n");
}