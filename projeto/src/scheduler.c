#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "scheduler.h"
#include "queue.h"
#include "hashRR.h"

void schedInit(Scheduler *s, Queue *q, GHashTable *hash, const char* a1, const char* a2){
    s->comandosParalelo = atoi(a1);
    s->policy = atoi(a2);
    s->arrayPackages = malloc(s->comandosParalelo * sizeof(Package));
    s->filaEspera = q;
    s->executions = 0;
    s->hashRR = hash;
}

/*
 * schedInit
 * ---------
 * Inicializa a estrutura Scheduler com a política e número de comandos
 * paralelos especificados (a1, a2 são strings representando esses valores).
 */

int policyExists(SchedPolicies policy){
    if (policy != FCFS && policy != RR && policy != LIFO){
        char buffer[256];
        int n = sprintf(buffer, "Pol\u00edtica de escalonamento inexistente, por favor tente com 1, 2 ou 3\n");
        write(1, &buffer, n);
        return 0;
    }
    return 1;
}

/*
 * firstComeFirstServe
 * -------------------
 * Implementação FCFS: retira o próximo Package da fila de espera.
 */
Package firstComeFirstServe(Scheduler* s){
    return dequeue(s->filaEspera);
}

/*
 * roundRobin
 * ----------
 * Implementação RR: escolhe o próximo user_id da IdQueue, devolve o
 * primeiro Package dessa Queue de utilizador e re-enfileira o user
 * caso ainda tenha comandos.
 */
Package roundRobin(Scheduler *s, IdQueue *idQueue){
    printf("Round Robin: idQueue tem %d elementos\n", idQueue->size);

    int userEscolhido = userDequeue(idQueue);

    printf("User escolhido: %d\n", userEscolhido);

    Queue *q = get_from_hashRR(s->hashRR, userEscolhido);

    if (!q || q->size == 0 || q->head == NULL) {
        printf("Fila inv\u00e1lida/vazia do user %d\n", userEscolhido);
        remove_from_hashRR(s->hashRR, userEscolhido);
        return (Package){0};
    }

    Package p = dequeue(q);

    if (q->size > 0) {
        if (!userInQueue(idQueue, userEscolhido))
            userEnqueue(idQueue, userEscolhido);
    } else {
        remove_from_hashRR(s->hashRR, userEscolhido);
    }

    return p;
}

/*
 * lastInFirstOut
 * ---------------
 * Implementação LIFO: comporta-se igual a dequeue porque push_head
 * insere no início; dequeue retira também do início.
 */
Package lastInFirstOut(Scheduler *s){
    return dequeue(s->filaEspera);
}

/*
 * schedPolicy
 * -----------
 * Encapsula qual função de escalonamento usar com base em s->policy.
 */
Package schedPolicy(Scheduler *s, IdQueue *idQueue){
    if (s->policy == FCFS) return firstComeFirstServe(s);
    else if (s->policy == RR) return roundRobin(s, idQueue);
    else return lastInFirstOut(s);
}





