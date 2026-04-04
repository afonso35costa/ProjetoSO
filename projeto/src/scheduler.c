#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "scheduler.h"
#include "queue.h"

void schedInit(Scheduler *s, Queue *q, const char* a1, const char* a2){
    s->comandosParalelo = atoi(a1);
    s->policy = atoi(a2);
    s->arrayPackages = malloc(s->comandosParalelo * sizeof(Package));
    s->filaEspera = q;
    s->executions = 0;
}

int policyExists(SchedPolicies policy){
    if (policy != FCFS && policy != RR && policy && LIFO){
        char buffer[256];
        int n = sprintf(buffer, "Política de escalonamento inexistente, por favor tente com 1, 2 ou 3\n");
        write(1, &buffer, n);
        return 0;
    }
    return 1;
}

Package firstComeFirstServe(Scheduler* s){
    Package aux = s->arrayPackages[0];
    for(int i=0; i < s->executions; i++){
        s->arrayPackages[i] = s->arrayPackages[i+1];
    }
    return aux;
}

Package roundRobin(Scheduler *s){

}

Package lastInFirstOut(Scheduler *s){

}

Package schedPolicy(Scheduler *s){
    //if (s->policy == FCFS) 
    return firstComeFirstServe(s);
    // else if (s->policy == LIFO) return lastInFirstOut(s);
    // else return roundRobin(s);
}



// FAZER FUNCAO GERAL




