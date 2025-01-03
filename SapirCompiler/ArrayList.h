#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#define GROWTH_FACTOR 2

// Define a structure for the ArrayList
typedef struct ArrayList {
	char* array;
	int size;
	int capacity;
} ArrayList;

// Initialize an empty ArrayList
ArrayList* arraylist_init(int initial_capacity);

// Add an element to the end of the ArrayList
void arraylist_add(ArrayList* list, char element);

// Get the element at the specified index
char arraylist_get(ArrayList* list, int index);

// Reset the ArrayList to an empty state
void arraylist_reset(ArrayList* list);

// Check if the ArrayList is empty
int array_list_is_empty(ArrayList* list);

// Free the memory allocated for the ArrayList
void arraylist_free(ArrayList* list);

#endif