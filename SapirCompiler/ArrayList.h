#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#define GROWTH_FACTOR 2

#include <stdbool.h>

//allocates space
typedef struct ArrayList { 
	void** array;
	unsigned int object_size;
	int size;
	int capacity;
} ArrayList;

ArrayList* arraylist_init(int object_size, int initial_capacity);

void arraylist_add(ArrayList*, void*);

void* arraylist_get(ArrayList*, int);

void* arraylist_set(ArrayList*, void*, int);

void arraylist_reset(ArrayList*);

int arraylist_is_empty(ArrayList*);

void arraylist_free(ArrayList*);

bool arraylist_equals(ArrayList* a1, ArrayList* a2);

void arraylist_print(ArrayList*, void printfunc(void*));

#endif