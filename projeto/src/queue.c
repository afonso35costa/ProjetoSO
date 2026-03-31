#include <stdlib.h>
#include "queue.h"

void enqueue(Queue *q, Comando *cmd) {
    Node *novo = malloc(sizeof(Node));
    novo->cmd  = *cmd;
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

Comando dequeue(Queue *q) {
    Node   *tmp = q->head;
    Comando cmd = tmp->cmd;
    q->head = q->head->next;
    if (q->head == NULL)
        q->tail = NULL;
    free(tmp);
    q->size--;
    return cmd;
}