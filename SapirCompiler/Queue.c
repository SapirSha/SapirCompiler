#include "Queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ErrorHandler.h"

Queue* queue_init(unsigned int object_size) {
	Queue* queue = malloc(sizeof(Queue));
	if (!queue) {
		handle_out_of_memory_error();
		return NULL;
	}

	queue->head = NULL;
	queue->tail = NULL;
	queue->object_size = object_size;
	queue->size = 0;

	return queue;
}

static QueueNode* init_queue_node(void* value, unsigned int object_size) {
	QueueNode* node = malloc(sizeof(QueueNode));
	if (!node) {
		handle_out_of_memory_error();
		return NULL;
	}

	node->next = NULL;
	node->value = malloc(object_size);
	if (!node->value) {
		handle_out_of_memory_error();
		return NULL;
	}

	if (!node->value) handle_out_of_memory_error();

	memcpy(node->value, value, object_size);
	return node;
}

void queue_enqueue(Queue* queue, void* value) {
	QueueNode* node = init_queue_node(value, queue->object_size);

	if (queue->tail == NULL || queue->size == 0) {
		queue->head = queue->tail = node;
	}
	else {
		queue->tail->next = node;
		queue->tail = node;
	}

	queue->size++;
}

void* queue_dequeue(Queue* queue) {
	if (queue->head == NULL || queue->size == 0) return NULL;
	QueueNode* node = queue->head;
	queue->head = node->next;
	void* value = node->value;
	free(node);
	queue->size--;
	return  value;
}

void queue_print(Queue* queue, void print(void*)) {
	QueueNode* node = queue->head;
	printf("Queue(length %d): ", queue->size);
	while (node != NULL) {
		print(node->value);
		node = node->next;
		if (node != NULL) printf(", ");
	}
	printf("\n");
}

void* queue_peek(Queue* queue) {
	return queue->head ? queue->head->value : NULL;
}

void* queue_peek_last(Queue* queue) {
	return queue->tail ? queue->tail->value : NULL;
}