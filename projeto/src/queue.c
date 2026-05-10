#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "package.h"

void queueInit (Queue *q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

/*
 * enqueue
 * -------
 * Adiciona um Package ao final da fila.
 * Parâmetros:
 *  - q: ponteiro para a Queue
 *  - pkg: Package a inserir
 */
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

/*
 * push_head
 * ---------
 * Insere um Package no início da fila (usado para LIFO).
 */
void push_head(Queue *q, Package *pkg) {
    Node *novo = malloc(sizeof(Node));
    novo->pkg  = *pkg;
    novo->next = q->head;
    q->head = novo;
    if (q->tail == NULL)
        q->tail = novo;
    q->size++;
}

/*
 * dequeue
 * -------
 * Remove e retorna o primeiro Package da fila. Se a fila estiver vazia
 * retorna um Package com zero inicializado.
 */
Package dequeue(Queue *q) {
    if (!q || !q->head) {
        printf("DEQUEUE em fila inválida\n");
        return (Package){0};
    }

    printf("Dequeue: fila tem %d elementos\n", q->size);

    Node *tmp = q->head;
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

/*
 * userEnqueue
 * -----------
 * Enfileira um user_id na IdQueue (usada pela política RR).
 */
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

/*
 * userDequeue
 * -----------
 * Remove e retorna o primeiro user_id da IdQueue.
 */
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

/*
 * userInQueue
 * -----------
 * Verifica se um dado userId já está presente na IdQueue.
 */
int userInQueue(IdQueue *q, int userId) {
    UserNode *tmp = q->head;
    while (tmp != NULL) {
        if (tmp->user_id == userId) return 1;
        tmp = tmp->next;
    }
    return 0;
}