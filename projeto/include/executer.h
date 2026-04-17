#ifndef EXECUTER
#define EXECUTER
#include "comandos.h"
#include "package.h"
#include "queue.h"
#include "scheduler.h"
#include "hashRR.h"

void executaComando(Comando *c);
void controller_enviaInfoComandos(Package *array, int executions, Queue *fila, GHashTable *hash, int policy, int fd);
void consultaComandos(int fd);
void executerMain(Comando *c, int fd);

#endif