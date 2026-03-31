#ifndef EXEC
#include "comandos.h"
#include "queue.h"

typedef enum{
    FCFS = 1, // AINDA A DEFINIR
    POLICY2 = 2, // AINDA A DEFINIR
    POLICY3 = 3 // AINDA A DEFINIR
    // PODEM SER DEFINIDAS MAIS POLITICAS
} SchedPolicies;

typedef struct {
    SchedPolicies policy;
    Comando* arrayComandos;
    Queue* filaEspera;
    int comandosParalelo;
    int executions;
} Scheduler;

void schedInit(Scheduler *s, Queue *q, const char* a1, const char* a2);
void policyExists(int policy);

#endif