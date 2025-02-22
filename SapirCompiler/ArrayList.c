#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ArrayList.h"


ArrayList* arraylist_init(int object_size, int initial_capacity) {
    ArrayList* list = malloc(sizeof(ArrayList));
    if (!list) {
        fprintf(stderr, "Failed to allocate ArrayList structure\n");
        return NULL;
    }
    list->object_size = object_size;
    list->capacity = initial_capacity;
    list->size = 0;
    list->array = malloc(sizeof(void*) * initial_capacity);
    if (!list->array) {
        fprintf(stderr, "Failed to allocate ArrayList internal array\n");
        free(list);
        return NULL;
    }
    return list;
}

void arraylist_add(ArrayList* list, void* value) {
    if (list->size == list->capacity) {
        list->capacity *= GROWTH_FACTOR;
        void** temp = realloc(list->array, sizeof(void*) * list->capacity);
        if (!temp) {
            fprintf(stderr, "Failed to reallocate ArrayList internal array\n");
            return;
        }
        list->array = temp;
    }
    list->array[list->size] = malloc(list->object_size);
    if (!list->array[list->size]) {
        fprintf(stderr, "Failed to allocate memory for new element\n");
        return;
    }
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
	for (int i = 0; i < list->size; i++) {
        printfunc(list->array[i]);
	}
}
