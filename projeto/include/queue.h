#ifndef QUEUE
#define QUEUE
#include "package.h"

typedef struct Node{
    Package pkg;
    struct Node *next;
} Node;

typedef struct UserNode{
    int user_id;
    struct UserNode *next;
} UserNode;

typedef struct {
    UserNode *head;
    UserNode *tail;
    int size;
} IdQueue;

typedef struct {
    Node *head;
    Node *tail;
    int   size;
} Queue;

void queueInit (Queue *q);
void enqueue(Queue *q, Package *pkg);
void push_head(Queue *q, Package *pkg);
Package dequeue(Queue *q);

void idQueueInit (IdQueue *q);
void userEnqueue(IdQueue *q, int user_id);
int userDequeue(IdQueue *q);

#endif