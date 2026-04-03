#include <unistd.h>
#include <stdio.h>
#include "comandos.h"
#include "package.h"
#include "queue.h"
#include "scheduler.h"

void executaComando(Comando *c){
    char buffer2[64];
    int n2 = sprintf(buffer2, "[runner] executing command %d...\n", getpid());
    write(1, &buffer2, n2);

    //criar array de args
    char *argv[ARGS_LIM + 1];
    for(int i = 0; i < c->num_args; i++){
        argv[i] = c->args[i];
    }
    argv[c->num_args] = NULL;

    //executar o comando
    execvp(c->args[0], argv);
}

void controller_enviaInfoComandos(Package *array, int executions, Queue *fila, int fd) {
    write(fd, &executions, sizeof(int));
    write(fd, &fila->size, sizeof(int));

    // do array — estão em execução
    for (int i = 0; i < executions; i++) {
        InfoComando info = {
            array[i].pid,
            array[i].comando.userId,
        };
        write(fd, &info, sizeof(InfoComando));
    }

    // da fila — estão em espera
    Node *atual = fila->head;
    while (atual != NULL) {
        InfoComando info = {
            atual->pkg.pid,
            atual->pkg.comando.userId,
        };
        write(fd, &info, sizeof(InfoComando));
        atual = atual->next;
    }
}

void consultaComandos(int fd){
    int execucao, fila;
    read(fd, &execucao, sizeof(int));
    read(fd, &fila, sizeof(int));

    write(1, "---", 3);
    char executing[10] = "Executing";
    executing[9] = '\0';
    write(1, &executing, sizeof(executing));
    for(int i=0; i < execucao; i++){
        InfoComando info;
        read(fd, &info, sizeof(info));

        char cmd1[64];
        int n1 = sprintf(cmd1, "user-id %d - command-id %d\n", info.user_id, info.comando_id);
        write(1, &cmd1, n1);
    }

    write(1, "---", 3);
    char scheduled[10] = "Scheduled";
    scheduled[9] = '\0';
    write(1, &scheduled, sizeof(scheduled));
    for(int i=0; i < fila; i++){
        InfoComando info;
        read(fd, &info, sizeof(info));

        char cmd2[64];
        int n2 = sprintf(cmd2, "user-id %d - command-id %d\n", info.user_id, info.comando_id);
        write(1, &cmd2, n2);
    }
}

void executerMain(Comando *c, int fd){
    TipoComando tipo = c->tipo;
    if (tipo == CMD_EXEC) executaComando(c);
    else if (tipo == CMD_CONSULTA) consultaComandos(fd);
    else if (tipo == CMD_OFF) {
        char buffer[128];
        int n = sprintf(buffer, "[runner] sent shutdown notification\n[runner] waiting for controller to shutdown...\n");
        write(1, &buffer, n);
    }
}


