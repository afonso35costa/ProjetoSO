#ifndef QUEUE
#define QUEUE
#include "package.h"

typedef struct Node{
    Package pkg;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int   size;
} Queue;

void enqueue(Queue *q, Package *pkg);
Package dequeue(Queue *q);

#endif