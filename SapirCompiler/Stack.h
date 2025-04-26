#ifndef STACK_H
#define STACK_H

typedef struct StackNode {
	struct StackNode* next;
	void* value;
} StackNode;

typedef struct Stack {
	StackNode* head;
	unsigned int size;
} Stack;

Stack* stack_init();
void stack_push(Stack*, void*);
void* stack_pop(Stack*);
void* stack_peek(Stack*);
void stack_free(Stack*, void free_function(void*));
void stack_print(Stack*, void (*print_function)(void*));


#endif