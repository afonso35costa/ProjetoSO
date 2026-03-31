#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"
#include "queue.h"

void schedInit(Scheduler *s, Queue *q, const char* a1, const char* a2){
    s->comandosParalelo = atoi(a1);
    s->policy = atoi(a2);
    s->arrayComandos = malloc(s->comandosParalelo * sizeof(Comando));
    s->filaEspera = q;
}

void policyExists(int policy){
    if (policy != FCFS || policy != POLICY2 || policy != POLICY3){
        char buffer[256];
        sprintf(buffer, "Política de escalonamento inexistente, por favor tente com 1, 2 ou 3\n");
    }
}

Comando firstComeFirstServe(Scheduler* s){
    Comando aux = s->arrayComandos[0];
    for(int i=0; i < s->executions; i++){
        s->arrayComandos[i] = s->arrayComandos[i+1];
    }
    if (s->filaEspera->size > 0){
        s->arrayComandos[s->executions] = dequeue(s->filaEspera);
    }
    else s->executions--;

    return aux;
}

// FAZER FUNCAO GERAL




