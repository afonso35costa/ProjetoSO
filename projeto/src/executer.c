#include <unistd.h>
#include <stdio.h>
#include "comandos.h"
#include "package.h"
#include "queue.h"
#include "scheduler.h"
#include "hashRR.h"
#include <wait.h>
#include <glib.h>

void executaComando(Package *p, int temOp) {

    Comando *c = &p->comando;

    char buffer2[64];
    int n2 = sprintf(buffer2,
                     "[runner] executing command %d...\n",
                     p->runnerPid);
    write(1, buffer2, n2);

    // CASO SIMPLES 
    if (!temOp) {

        char *argv[ARGS_LIM + 1];

        for (int i = 0; i < c->num_args; i++)
            argv[i] = c->args[i];

        argv[c->num_args] = NULL;

        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    }

    // PIPELINE
    ComandoParsed parsed = parseComando(c);

    int prev_read = -1;

    for (int i = 0; i < parsed.num_segmentos; i++) {

        int pipefd[2] = {-1, -1};
        Segmento *seg = &parsed.segmentos[i];

        if (i < parsed.num_segmentos - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                exit(1);
            }
        }

        pid_t pid = fork();

        if (pid == 0) {

            // INPUT (< ou pipe)
            if (seg->stdin_file[0] != '\0') {

                int fd = open(seg->stdin_file, O_RDONLY);
                if (fd < 0) {
                    perror("open stdin");
                    exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);

            } else if (prev_read != -1) {

                dup2(prev_read, STDIN_FILENO);
            }

            // OUTPUT (> ou pipe)

            if (seg->stdout_file[0] != '\0') {

                int fd = open(seg->stdout_file,
                              O_CREAT | O_WRONLY | O_TRUNC,
                              0666);

                if (fd < 0) {
                    perror("open stdout");
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);

            } else if (i < parsed.num_segmentos - 1) {

                dup2(pipefd[1], STDOUT_FILENO);
            }

            // STDERR (2>)
            if (seg->stderr_file[0] != '\0') {

                int fd = open(seg->stderr_file,
                              O_CREAT | O_WRONLY | O_TRUNC,
                              0666);

                if (fd < 0) {
                    perror("open stderr");
                    exit(1);
                }

                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            // FECHAR FDs
            if (prev_read != -1)
                close(prev_read);

            if (i < parsed.num_segmentos - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            // ARGV
            char *argv[ARGS_LIM + 1];

            for (int j = 0; j < seg->num_args; j++)
                argv[j] = seg->args[j];

            argv[seg->num_args] = NULL;

            if (argv[0] == NULL) {
                fprintf(stderr, "Erro: comando vazio\n");
                exit(1);
            }

            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        }

        // PAI
        if (prev_read != -1)
            close(prev_read);

        if (i < parsed.num_segmentos - 1) {
            close(pipefd[1]);
            prev_read = pipefd[0];
        }
    }

    // WAIT
    int status;

    for (int i = 0; i < parsed.num_segmentos; i++) {

        wait(&status);

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) < 0){
                printf("[runner] command %d failed\n", p->runnerPid);
                exit(1);
            }
        }
    }
}
/*
 * controller_enviaInfoComandos
 * ----------------------------
 * Envia para o runner as informações necessárias para a consulta de
 * comandos: quantos estão em execução e quais estão na fila/estruturas.
 * Parâmetros:
 *  - array: array de Packages em execução
 *  - executions: número de packages em execução
 *  - fila: fila de espera (FCFS/LIFO)
 *  - hash: tabela de filas por utilizador (RR)
 *  - policy: política de escalonamento (FCFS/RR/LIFO)
 *  - fd: descritor do FIFO para onde escrever as informações
 *  - idQueue: fila de IDs de utilizadores (apenas para RR)
 */
void controller_enviaInfoComandos(Package *array, int executions, Queue *fila, GHashTable *hash, int policy, int fd, IdQueue *idQueue) {
    if (policy == FCFS || policy == LIFO) {
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

    if (policy == FCFS || policy == LIFO) {
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
/*
 * executerMain
 * ------------
 * Dispatcher para executar o tipo correto de ação consoante o pacote:
 *  - executa comandos (-e)
 *  - envia a lista de execução e fila para o runner (-c)
 *  - mostra mensagem de desligamento para -s
 */
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




