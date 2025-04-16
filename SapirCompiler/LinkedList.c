#include "LinkedList.h"
#include "string.h"
#include "stdlib.h"
#include <stdio.h>

LinkedList* linkedlist_init(unsigned int object_byte_size) {
	LinkedList* lst = (LinkedList*) malloc(sizeof(LinkedList));
	lst->object_byte_size = object_byte_size;
	lst->size = 0;
	lst->head = NULL;
	return lst;
}

unsigned int linkedlist_count(LinkedList* lst) {
	return lst->size;
}

bool linkedlist_contains(LinkedList* lst, void* value, unsigned int compare_function(void*, void*)) {
	LinkedListNode* pointer = lst->head;
	while (pointer != NULL && compare_function(value, pointer->value) != 0)
		pointer = pointer->next;

	if (pointer != NULL) return true;
	else return false;
}

LinkedListNode* linkedlistnode_init(LinkedList* lst, void* value) {
	LinkedListNode* node = malloc(sizeof(LinkedListNode));
	node->next = NULL;
	node->value = malloc(sizeof(lst->object_byte_size));
	memcpy(node->value, value, lst->object_byte_size);
	return node;
}

// LinkedList
void linkedlist_add(LinkedList* lst, void* value) {
	if (lst->head == NULL) {
		lst->head = linkedlistnode_init(lst, value);
	}
	else {
		LinkedListNode* pointer = lst->head;
		while (pointer->next != NULL) pointer = pointer->next;
		pointer->next = linkedlistnode_init(lst, value);
	}
	lst->size++;
}

bool linkedlist_remove(LinkedList* lst, void* value) {
	LinkedListNode* pointer = lst->head;
	if (pointer == NULL) return false;
	if (memcmp(value, pointer->value, lst->object_byte_size) == 0)
	{
		lst->head = pointer->next;
		free(pointer);
		return true;
	}

	while (pointer->next != NULL && memcmp(value, pointer->next->value, lst->object_byte_size) != 0)
		pointer = pointer->next;

	if (pointer->next == NULL) return false;
	
	LinkedListNode* temp = pointer->next;
	pointer->next = temp->next;
	free(temp);

	return true;
}
void* linkedlist_get(LinkedList* lst, unsigned int index) {
	if (index >= lst->size) return NULL;
	LinkedListNode* pointer = lst->head;
	while (index-- > 0) pointer = pointer->next;
	return pointer->value;
}

// LinkedList

// Stack
void linkedlist_push(LinkedList* lst, void* value) {
	LinkedListNode* temp = lst->head;
	lst->head = linkedlistnode_init(lst, value);
	lst->head->next = temp;
	lst->size++;
}
void* linkedlist_pop(LinkedList* lst) {
	if (lst->head == NULL) return NULL;
	LinkedListNode* temp = lst->head;
	lst->head = lst->head->next;
	lst->size--;
	void* value = temp->value;
	free(temp);
	return value;
}
void* linkedlist_peek(LinkedList* lst) {
	return lst->head ? lst->head->value : NULL;
}
// Stack

void linkedlist_print(LinkedList* lst, void (*print)(void*)) {
	printf("LIST(%d items): ", lst->size);
	LinkedListNode* pointer = lst->head;
	while (pointer != NULL) {
		print(pointer->value);
		pointer = pointer->next;
		if (pointer != NULL) printf(", ");
	}
	printf("\n");
}


void linkedlist_free(LinkedList* list, void free_function(void*)){
	LinkedListNode* temp;
	while (list->size) {
		temp = linkedlist_pop(list);
		free_function(temp->value);
	}
	free(list);
	list = NULL;
}