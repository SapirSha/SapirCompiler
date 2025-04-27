#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ArrayList.h"
#include "ErrorHandler.h"


ArrayList* arraylist_init(int object_size, int initial_capacity) {
    ArrayList* list = malloc(sizeof(ArrayList));
    if (!list) handle_out_of_memory_error();

    list->object_size = object_size;
    list->capacity = initial_capacity;
    list->size = 0;
    list->array = calloc(initial_capacity, sizeof(void*));

    return list;
}

void arraylist_add(ArrayList* list, void* value) {
    if (list->size == list->capacity) {
        list->capacity *= GROWTH_FACTOR;
        void** temp = realloc(list->array, sizeof(void*) * list->capacity);
        if (!temp) handle_out_of_memory_error();
        for (int i = list->size; i < list->capacity; i++)
            temp[i] = NULL;


        list->array = temp;
    }
    list->array[list->size] = malloc(list->object_size);
    if (!list->array[list->size]) handle_out_of_memory_error();
    
    memcpy(list->array[list->size], value, list->object_size);
    list->size++;
}



void arraylist_reset(ArrayList* list) {
    for (int i = 0; i < list->size; i++) {
        free(list->array[i]);
    }
    list->size = 0;
}

void* arraylist_get(ArrayList* list, int index) {
    if (index < 0 || index >= list->size) {
        return NULL;
    }
    return list->array[index];
}

void* arraylist_set(ArrayList* list, void* value, int index) {
    if (index >= list->capacity) {
        list->capacity = index * GROWTH_FACTOR;
        void** temp = realloc(list->array, sizeof(void*) * list->capacity);

        for (; list->size <= index; list->size++)
            temp[list->size] = NULL;
        for (int i = list->size; i < list->capacity; i++)
            temp[i] = NULL;

        list->array = temp;
    }
    void* temp = list->array[index];
    list->array[index] = malloc(list->object_size);
    if (list->array[index] == NULL) handle_out_of_memory_error();

    memcpy(list->array[index], value, list->object_size);
    if (index >= list->size) {
        list->size = index + 1;
    }
    else if (temp == NULL) list->size++;
    return temp;
}


int arraylist_is_empty(ArrayList* list) {
    return list->size == 0;
}

void arraylist_free(ArrayList* list) {
    for (int i = 0; i < list->size; i++) {
        free(list->array[i]);
    }
    free(list->array);
    free(list);
}

void arraylist_print(ArrayList* list, void printfunc(void*)) {
	printf("ArrayList(%d items): ", list->size);
	for (int i = 0; i < list->size; i++) 
    {
        if (list->array[i] != NULL)
            printfunc(list->array[i]);
        else printf("NULL");
		if (i < list->size - 1) printf(", ");
	}
    printf("\n");
}

bool arraylist_contains(ArrayList* list, void* value,  int cmp(void*, void*)) {
    if (!list || !value) return false;
    for (int i = 0; i < list->size; i++) {
        if (cmp(list->array[i], value) == 0)
            return true;
    }
    return false;
}

bool arraylist_equals(ArrayList* a1, ArrayList* a2) {
    if (a1->object_size != a2->object_size) return false;
    if (a1->size != a2->size) return false;
    for (int i = 0; i < a1->size; i++) {
        if (memcmp(a1->array[i], a2->array[i], a1->object_size) != 0) return false;
    }
    return true;
}