#ifndef EXECUTER
#define EXECUTER
#include "comandos.h"
#include "package.h"
#include "queue.h"
#include "scheduler.h"
#include "hashRR.h"

void executaComando(Package *p, int temOp);
void controller_enviaInfoComandos(Package *array, int executions, Queue *fila, GHashTable *hash, int policy, int fd, IdQueue *idQueue);
void consultaComandos(int fd);
void executerMain(Package *p, int fd, int temOp);

#endif