#include "LinkedList.h"
#include "string.h"
#include "stdlib.h"
#include <stdio.h>

/*

typedef struct LinkedListNode {
	struct LinkedListNode* next;
	void* value;
} LinkedListNode;


typedef struct LinkedList {
	LinkedListNode* head;
	unsigned int object_byte_size;
	unsigned int length;
	int (*compare)(void*, void*);
} LinkedList;


*/

LinkedList* linkedlist_init(unsigned int object_byte_size, int (*compare)(void*, void*)) {
	LinkedList* lst = (LinkedList*) malloc(sizeof(LinkedList));
	lst->compare = compare;
	lst->object_byte_size = object_byte_size;
	lst->length = 0;
	lst->head = NULL;
	return lst;
}

unsigned int linkedlist_count(LinkedList* lst) {
	return lst->length;
}

bool linkedlist_contains(LinkedList* lst, void* value) {
	LinkedListNode* pointer = lst->head;
	while (pointer != NULL && memcmp(value, pointer->value, lst->object_byte_size) != 0)
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
	lst->length++;
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
	if (index >= lst->length) return NULL;
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
	lst->length++;
}
void* linkedlist_pop(LinkedList* lst) {
	LinkedListNode* temp = lst->head;
	lst->head = lst->head->next;
	lst->length--;
	return temp->value;
}
void* linkedlist_peek(LinkedList* lst) {
	return lst->head;
}
// Stack

void linkedlist_print(LinkedList* lst, void (*print)(void*)) {
	printf("LIST(%d items): ", lst->length);
	LinkedListNode* pointer = lst->head;
	while (pointer != NULL) {
		print(pointer->value);
		pointer = pointer->next;
		if (pointer != NULL) printf(", ");
	}
	printf("\n");
}


unsigned int linkedlist_free(LinkedList*);