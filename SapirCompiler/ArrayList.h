#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#define GROWTH_FACTOR 2

// Define a structure for the ArrayList // allocates space
typedef struct ArrayList { 
	void** array;
	unsigned int object_size;
	int size;
	int capacity;
} ArrayList;

// Initialize an empty ArrayList
ArrayList* arraylist_init(int object_size, int initial_capacity);

// Add an element to the end of the ArrayList
void arraylist_add(ArrayList*, void*);

// Get the element at the specified index
void* arraylist_get(ArrayList*, int);

// Reset the ArrayList to an empty state
void arraylist_reset(ArrayList*);

// Check if the ArrayList is empty
int arraylist_is_empty(ArrayList*);

// Free the memory allocated for the ArrayList
void arraylist_free(ArrayList*);

// Print ArrayList
void arraylist_print(ArrayList*, void printfunc(void*));

#endif