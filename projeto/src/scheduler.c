#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"
#include "queue.h"

void schedInit(Scheduler *s, Queue *q, const char* a1, const char* a2){
    s->comandosParalelo = atoi(a1);
    s->policy = atoi(a2);
    s->arrayPackages = malloc(s->comandosParalelo * sizeof(Package));
    s->filaEspera = q;
}

void policyExists(int policy){
    if (policy != FCFS || policy != POLICY2 || policy != POLICY3){
        char buffer[256];
        sprintf(buffer, "Política de escalonamento inexistente, por favor tente com 1, 2 ou 3\n");
    }
}

Package firstComeFirstServe(Scheduler* s){
    Package aux = s->arrayPackages[0];
    for(int i=0; i < s->executions; i++){
        s->arrayPackages[i] = s->arrayPackages[i+1];
    }
    if (s->filaEspera->size > 0){
        s->arrayPackages[s->executions] = dequeue(s->filaEspera);
    }
    else s->executions--;

    return aux;
}



// FAZER FUNCAO GERAL




