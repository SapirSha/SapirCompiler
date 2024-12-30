#include "ArrayList.h"

#include <stdlib.h>

// Initialize an empty ArrayList
ArrayList* arraylist_init(int initial_capacity) {
	ArrayList* list = malloc(sizeof(ArrayList));
	list->array = malloc(sizeof(void*) * initial_capacity);
	list->size = 0;
	list->capacity = initial_capacity;
	return list;
}

// Add an element to the end of the ArrayList
void arraylist_add(ArrayList* list, char element) {
	if (list->size == list->capacity) {
		// Resize the array if it is full
		list->capacity *= GROWTH_FACTOR;
		list->array = realloc(list->array, sizeof(char) * list->capacity);
	}

	list->array[list->size++] = element;
}

// Reset the ArrayList to an empty state
void arraylist_reset(ArrayList* list) {
	list->size = 0;
}

// Get the element at the specified index
char arraylist_get(ArrayList* list, int index) {
	if (index < 0 || index >= list->size) {
		return NULL;
	}

	return list->array[index];
}

// Check if the ArrayList is empty
int array_list_is_empty(ArrayList* list) {
	return list->size == 0;
}

// Free the memory allocated for the ArrayList
void arraylist_free(ArrayList* list) {
	free(list->array);
	free(list);
}

