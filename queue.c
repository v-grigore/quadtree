// GRIGORE Vlad - 313CC

#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

// Utility function for creating a new queue node
QueueNode *newQueueNode() {
    QueueNode *newNode = malloc(sizeof(QueueNode));

    if (!newNode) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }

    newNode->next = NULL;
    newNode->prev = NULL;

    return newNode;
}

// Initialize queue
int initQueue(Queue **pQueue) {
    *pQueue = malloc(sizeof(Queue));

    if (!*pQueue) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return -1;
    }

    (*pQueue)->sentinel = newQueueNode();

    if (!(*pQueue)->sentinel) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return -1;
    }

    (*pQueue)->sentinel->next = (*pQueue)->sentinel;
    (*pQueue)->sentinel->prev = (*pQueue)->sentinel;

    (*pQueue)->head = (*pQueue)->sentinel;
    (*pQueue)->tail = (*pQueue)->sentinel;

    return 0;
}

// Check if queue is empty
bool isQueueEmpty(Queue *queue) {
    if (queue->head == queue->sentinel)
        return true;
    return false;
}

// Push element in queue
int enqueue(Queue *queue, void *data) {
    QueueNode *newEntry = newQueueNode();

    if (!newEntry) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return -1;
    }

    newEntry->data = data;

    newEntry->next = queue->tail;
    newEntry->prev = queue->sentinel;

    newEntry->next->prev = newEntry;
    newEntry->prev->next = newEntry;

    queue->tail = newEntry;
    if (queue->head == queue->sentinel)
        queue->head = newEntry;

    return 0;
}

// Pop element from queue
void dequeue(Queue *queue) {
    if (queue->head == queue->sentinel)
        return;

    if (queue->head == queue->tail)
        queue->tail = queue->sentinel;

    QueueNode *removedEntry = queue->head;
    queue->sentinel->prev = queue->head->prev;
    queue->head->prev->next = queue->sentinel;
    queue->head = queue->head->prev;
    free(removedEntry);
}

// Free memory allocated for queue
void freeQueue(Queue *queue) {
    while (queue->head != queue->sentinel)
        dequeue(queue);
    free(queue->sentinel);
    free(queue);
}
