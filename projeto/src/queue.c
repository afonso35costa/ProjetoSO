#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "package.h"

void queueInit (Queue *q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void enqueue(Queue *q, Package *pkg) {
    Node *novo = malloc(sizeof(Node));
    novo->pkg  = *pkg;
    novo->next = NULL;
    if (q->tail == NULL) {
        q->head = novo;
        q->tail = novo;
    } else {
        q->tail->next = novo;
        q->tail = novo;
    }
    q->size++;
}

void push_head(Queue *q, Package *pkg) {
    Node *novo = malloc(sizeof(Node));
    novo->pkg  = *pkg;
    novo->next = q->head;
    q->head = novo;
    if (q->tail == NULL)
        q->tail = novo;
    q->size++;
}

Package dequeue(Queue *q) {
    printf("Dequeue: fila tem %d elementos\n", q->size);
    Node   *tmp = q->head;
    Package pkg = tmp->pkg;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    free(tmp);
    q->size--;
    return pkg;
}

void idQueueInit (IdQueue *q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void userEnqueue(IdQueue *q, int user_id) {
    UserNode *novo = malloc(sizeof(UserNode));
    novo->user_id  = user_id;
    novo->next = NULL;
    if (q->tail == NULL) {
        q->head = novo;
        q->tail = novo;
    } else {
        q->tail->next = novo;
        q->tail = novo;
    }
    q->size++;
}

int userDequeue(IdQueue *q) {
    UserNode *tmp = q->head;
    int id = tmp->user_id;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    free(tmp);
    q->size--;
    return id;
}

int userInQueue(IdQueue *q, int userId) {
    UserNode *tmp = q->head;
    while (tmp != NULL) {
        if (tmp->user_id == userId) return 1;
        tmp = tmp->next;
    }
    return 0;
}