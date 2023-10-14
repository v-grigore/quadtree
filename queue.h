// GRIGORE Vlad - 313CC

#ifndef TEMA2_QUEUE_H
#define TEMA2_QUEUE_H

#include <stdbool.h>

// Type definitions for queue and queue nodes
typedef struct QueueNode {
    struct QueueNode *next, *prev;
    void *data;
} QueueNode;

typedef struct Queue {
    QueueNode *sentinel;
    QueueNode *head, *tail;
} Queue;

// Initialize queue
int initQueue(Queue **pQueue);

// Check if queue is empty
bool isQueueEmpty(Queue *queue);

// Push element in queue
int enqueue(Queue *queue, void *data);

// Pop element from queue
void dequeue(Queue *queue);

// Free memory allocated for queue
void freeQueue(Queue *queue);

#endif //TEMA2_QUEUE_H
