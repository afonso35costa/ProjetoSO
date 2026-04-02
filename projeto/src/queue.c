#include <stdlib.h>
#include "queue.h"
#include "package.h"

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
    Node   *tmp = q->head;
    Package pkg = tmp->pkg;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    free(tmp);
    q->size--;
    return pkg;
}