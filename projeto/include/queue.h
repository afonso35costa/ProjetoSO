#ifndef QUEUE
#define QUEUE
#include "comandos.h"

typedef struct Node{
    Comando cmd;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int   size;
} Queue;

void enqueue(Queue *q, Comando *cmd);
Comando dequeue(Queue *q);

#endif