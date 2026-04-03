#ifndef EXECUTER
#define EXECUTER
#include "comandos.h"
#include "package.h"
#include "queue.h"
#include "scheduler.h"

void executaComando(Comando *c);
void controller_enviaInfoComandos(Package *array, int executions, Queue *fila, int fd);
void consultaComandos(int fd);
void executerMain(Comando *c, int fd);

#endif