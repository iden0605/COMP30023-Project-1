#ifndef QUEUE_H
#define QUEUE_H

/* Singly linked list node, contains 1 int value. 
 *  value - int stored in the node
 *  next  - pointer to the next node in the list
 */
typedef struct QNode {
    int value;
    struct QNode *next;
} QNode;

/* FIFO queue of ints, implemented as a singly linked list.
 *  head  - pointer to the front node
 *  tail  - pointer to the back node
 *  count - current number of elements in the queue
 */
typedef struct {
    QNode *head;
    QNode *tail;
    int count;
} Queue;

/* Initialise an empty queue. */
void q_init(Queue *q);

/* Insert v at the back of the queue. */
void q_push(Queue *q, int v);

/* Remove and return the element at the head of the queue. */
int q_pop(Queue *q);

/* Return 1 if queue is empty, else 0. */
int q_empty(const Queue *q);

/* Free all remaining nodes in the queue. */
void q_free(Queue *q);

#endif
