#ifndef SCHEDULER
#define SCHEDULER
#include "package.h"
#include "queue.h"
#include "hashRR.h"

typedef enum{
    FCFS = 1, // AINDA A DEFINIR
    RR = 2, // AINDA A DEFINIR
    LIFO = 3 // AINDA A DEFINIR
    // PODEM SER DEFINIDAS MAIS POLITICAS
} SchedPolicies;

typedef struct {
    SchedPolicies policy;
    Package* arrayPackages;
    Queue* filaEspera;
    GHashTable* hashRR;
    int comandosParalelo;
    int executions;

} Scheduler;

void schedInit(Scheduler *s, Queue *q, GHashTable *hash, const char* a1, const char* a2);
int policyExists(SchedPolicies policy);

Package firstComeFirstServe(Scheduler* s);
Package roundRobin(Scheduler *s, IdQueue *idQueue);
Package lastInFirstOut(Scheduler *s);

Package schedPolicy(Scheduler *s, IdQueue *idQueue);

#endif