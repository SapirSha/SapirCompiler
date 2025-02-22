#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdbool.h>
// allocates space
typedef struct LinkedListNode {
	struct LinkedListNode* next;
	void* value;
} LinkedListNode;


typedef struct LinkedList {
	LinkedListNode* head;
	unsigned int object_byte_size;
	unsigned int length;
} LinkedList;

LinkedList* linkedlist_init(unsigned int object_byte_size);
unsigned int linkedlist_count(LinkedList*);
bool linkedlist_contains(LinkedList*, void*);

// LinkedList
void linkedlist_add(LinkedList*, void*);
bool linkedlist_remove(LinkedList*, void*);
void* linkedlist_get(LinkedList*, unsigned int);
// LinkedList

// Stack
void linkedlist_push(LinkedList*, void*);
void* linkedlist_pop(LinkedList*);
void* linkedlist_peek(LinkedList*);
// Stack

void linkedlist_print(LinkedList*, void (*print)(void*));
unsigned int linkedlist_free(LinkedList*);


#endif