#include <unistd.h>
#include <stdio.h>
#include "comandos.h"
#include "package.h"
#include "queue.h"
#include "scheduler.h"
#include "hashRR.h"
#include <glib.h>

void executaComando(Package *p, int temOp){
    Comando *c = &p->comando;
    char buffer2[64];
    int n2 = sprintf(buffer2, "[runner] executing command %d...\n", p->runnerPid);
    write(1, &buffer2, n2);

    if (!temOp) {
        //criar array de args
        char *argv[ARGS_LIM + 1];
        for(int i = 0; i < c->num_args; i++){
            argv[i] = c->args[i];
        }
        argv[c->num_args] = NULL;

        //executar o comando
        execvp(c->args[0], argv);
    }
    else {
        
    }
}

void controller_enviaInfoComandos(Package *array, int executions, Queue *fila, GHashTable *hash, int policy, int fd, IdQueue *idQueue) {
    if (policy == FCFS) {
        write(fd, &(fila->size), sizeof(int));
    }
    else if (policy == RR) {
        int total_fila = cmd_count_in_all_queues(hash);
        write(fd, &total_fila, sizeof(int));
    }
    write(fd, &executions, sizeof(int));

    // do array — estão em execução
    for (int i = 0; i < executions; i++) {
        InfoComando info = {
            array[i].runnerPid,
            array[i].comando.userId,
        };
        write(fd, &info, sizeof(InfoComando));
    }

    if (policy == FCFS) {
            // da fila — estão em espera
        Node *atual = fila->head;
        while (atual != NULL) {
            InfoComando info = {
                atual->pkg.runnerPid,
                atual->pkg.comando.userId,
            };
            write(fd, &info, sizeof(InfoComando));
            atual = atual->next;
        }
    }
    else if (policy == RR) {
        int total = cmd_count_in_all_queues(hash);
        int pids[total];
        int userIds[total];

        int i = 0;
        printf("Size idqueue = %d\n", idQueue->size);
        UserNode *tmp = idQueue->head;
        while (tmp != NULL) {
            Queue *fila = get_from_hashRR(hash, tmp->user_id);
            if (fila != NULL) {
                Node *cur = fila->head;
                printf("User id %d\n", tmp->user_id);
                int idx = i;
                while (cur != NULL) {
                    pids[idx]    = cur->pkg.runnerPid;
                    userIds[idx] = cur->pkg.comando.userId;
                    idx += idQueue->size;
                    cur = cur->next;
                }
                i++;
            }
            tmp = tmp->next;
        }

        for (int j = 0; j < total; j++) {
            InfoComando info = { pids[j], userIds[j] };
            printf("Enviando info comando: runnerPid=%d, userId=%d\n", info.comando_id, info.user_id);
            write(fd, &info, sizeof(InfoComando));
        }
    }
}

void consultaComandos(int fd){
    int execucao, fila;
    read(fd, &fila, sizeof(int));
    read(fd, &execucao, sizeof(int));

    printf("Exec: %d, Fila: %d\n", execucao, fila);


    write(1, "---", 3);
    char executing[11] = "Executing\n";
    executing[10] = '\0';
    write(1, &executing, sizeof(executing));
    for(int i=0; i < execucao; i++){
        InfoComando info;
        read(fd, &info, sizeof(info));

        char cmd1[64];
        int n1 = sprintf(cmd1, "user-id %d - command-id %d\n", info.user_id, info.comando_id);
        write(1, &cmd1, n1);
    }

    write(1, "---", 3);
    char scheduled[11] = "Scheduled\n";
    scheduled[10] = '\0';
    write(1, &scheduled, sizeof(scheduled));
    for(int i=0; i < fila; i++){
        InfoComando info;
        read(fd, &info, sizeof(info));

        char cmd2[64];
        int n2 = sprintf(cmd2, "user-id %d - command-id %d\n", info.user_id, info.comando_id);
        write(1, &cmd2, n2);
    }
}

void executerMain(Package *p, int fd, int temOp){
    Comando *c = &p->comando;
    TipoComando tipo = c->tipo;
    if (tipo == CMD_EXEC) executaComando(p, temOp);
    else if (tipo == CMD_CONSULTA) consultaComandos(fd);
    else if (tipo == CMD_OFF) {
        char buffer[128];
        int n = sprintf(buffer, "[runner] sent shutdown notification\n[runner] waiting for controller to shutdown...\n");
        write(1, &buffer, n);
    }
}




