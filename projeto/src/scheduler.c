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

int policyExists(SchedPolicies policy){
    if (policy != FCFS && policy != RR && policy != LIFO){
        char buffer[256];
        int n = sprintf(buffer, "Política de escalonamento inexistente, por favor tente com 1, 2 ou 3\n");
        write(1, &buffer, n);
        return 0;
    }
    return 1;
}

Package firstComeFirstServe(Scheduler* s){
    return dequeue(s->filaEspera);
}

/* Package roundRobin(Scheduler *s, IdQueue *idQueue){
    printf("Round Robin: idQueue tem %d elementos\n", idQueue->size);
    int userEscolhido = userDequeue(idQueue);
    printf("User escolhido: %d\n", userEscolhido);
    Queue *q = get_from_hashRR(s->hashRR, userEscolhido);
    if (q->head == NULL) {
        printf("Fila do user %d vazia ou inexistente\n", userEscolhido);
    }
    Package p = dequeue(q);

    return p;  
}
*/

Package roundRobin(Scheduler *s, IdQueue *idQueue){
    printf("Round Robin: idQueue tem %d elementos\n", idQueue->size);

    int userEscolhido = userDequeue(idQueue);

    printf("User escolhido: %d\n", userEscolhido);

    Queue *q = get_from_hashRR(s->hashRR, userEscolhido);

    // 🔴 PROTEÇÃO CRÍTICA
    if (!q || q->size == 0 || q->head == NULL) {
        printf("Fila inválida/vazia do user %d\n", userEscolhido);

        // garante consistência
        remove_from_hashRR(s->hashRR, userEscolhido);
        return (Package){0};
    }

    Package p = dequeue(q);

    // 🔴 se ainda há trabalho, re-coloca user no RR
    if (q->size > 0) {
        userEnqueue(idQueue, userEscolhido);
    } else {
        // cleanup total
        remove_from_hashRR(s->hashRR, userEscolhido);
    }

    return p;
}

Package lastInFirstOut(Scheduler *s){
    return dequeue(s->filaEspera);
}

Package schedPolicy(Scheduler *s, IdQueue *idQueue){
    if (s->policy == FCFS) return firstComeFirstServe(s);
    else if (s->policy == RR) return roundRobin(s, idQueue);
    else return lastInFirstOut(s);
}





