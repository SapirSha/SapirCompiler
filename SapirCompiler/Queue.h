#ifndef QUEUE_H
#define QUEUE_H


typedef struct QueueNode {
	void* value;
	struct QueueNode* next;
}QueueNode;

typedef struct {
	QueueNode* head;
	QueueNode* tail;
	unsigned int object_size;
	unsigned int size;
}Queue;


Queue* queue_init(unsigned int object_size);
void queue_enqueue(Queue* queue, void* value);
void* queue_dequeue(Queue* queue);
void queue_print(Queue* queue, void print(void*));
void* queue_peek(Queue* queue);
void queue_free(Queue* queue);


#endif