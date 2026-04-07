#include "queue.h"

#include <stdlib.h>

void q_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}

void q_push(Queue *q, int v) {
    // Allocate a new node and add it to tail of the list
    QNode *node = malloc(sizeof(QNode));
    node->value = v;
    node->next = NULL;

    if (q->tail == NULL) {
        // Queue empty, the new node is both head and tail
        q->head = node;
    }
    else {
        q->tail->next = node;
    }
    q->tail = node;
    q->count++;
}

int q_pop(Queue *q) {
    // Save head node value and free it
    QNode *node = q->head;
    int v = node->value;
    q->head = node->next;

    if (q->head == NULL) {
        // Queue empty, clear tail pointer too
        q->tail = NULL;
    }

    free(node);
    q->count--;
    return v;
}

int q_empty(const Queue *q) {
    return q->count == 0;
}

void q_free(Queue *q) {
    // Traverse the list and free every node
    QNode *cur = q->head;
    while (cur != NULL) {
        QNode *next = cur->next;
        free(cur);
        cur = next;
    }
    
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}