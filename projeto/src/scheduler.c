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

Package roundRobin(Scheduler *s, IdQueue *idQueue){
    int userEscolhido = userDequeue(idQueue);
    Queue *q = get_from_hashRR(s->hashRR, userEscolhido);
    Package p = dequeue(q);
    if (q != NULL && q->head != NULL) userEnqueue(idQueue, userEscolhido);
    else remove_from_hashRR(s->hashRR, userEscolhido);
    return p;  
}

Package lastInFirstOut(Scheduler *s){
    
}

Package schedPolicy(Scheduler *s, IdQueue *idQueue){
    if (s->policy == FCFS) return firstComeFirstServe(s);
    else return roundRobin(s, idQueue);
    // else return lastInFirstOut(s);
}





