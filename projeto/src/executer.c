#include <unistd.h>
#include "comandos.h"
#include "package.h"
#include "queue.h"

void executaComando(Comando *c){
    execvp(c->args[0], c->args);
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

        char cmd1[256];
        sprintf("user-id %d - command-id %d\n", info.user_id, info.comando_id);
        write(1, &cmd1, sizeof(cmd1));
    }

    write(1, "---", 3);
    char scheduled[10] = "Scheduled";
    scheduled[9] = '\0';
    write(1, &scheduled, sizeof(scheduled));
    for(int i=0; i < fila; i++){
        InfoComando info;
        read(fd, &info, sizeof(info));

        char cmd2[256];
        sprintf("user-id %d - command-id %d\n", info.user_id, info.comando_id);
        write(1, &cmd2, sizeof(cmd2));
    }
}

