#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <wait.h>
#include "package.h"
#include "comandos.h"
#include "scheduler.h"
#include "queue.h"
#include "executer.h"
#include "hashRR.h"
#include <glib.h>

void controller_enviaPidFilho(const char* caminho_fifo_runner, pid_t pid_filho, int fd_runner){
    if (fd_runner < 0){
        perror("[controller] erro ao abrir FIFO do runner para enviar autorizacao");
        return;
    }
    if (write(fd_runner, &pid_filho, sizeof(pid_filho)) == -1) {
        printf("[controller] erro ao dar autorização\n");
    }
}


int main(int argc, char* argv[]){

    struct timespec inicio, fim;
    double tempo_execucao;


    if (argc < 3) {
        printf("<ParallellComands> e <SchedPolicy> necessários\n");
        return 0;
    }
    Scheduler escalonador;
    Queue fila;
    GHashTable *hash = NULL;
    int nr_comandos_hash = 0;
    IdQueue idQueue;
    queueInit(&fila);
    idQueueInit(&idQueue);
    schedInit(&escalonador, &fila, hash, argv[1], argv[2]);
    if (!policyExists(escalonador.policy)) return 0;
    if (escalonador.policy == RR) escalonador.hashRR = hash_create();
    
    // FIFO do controller
    const char *caminho_fifo_controller = "/tmp/runnerToController";
    unlink(caminho_fifo_controller);
    if (mkfifo(caminho_fifo_controller, 0666) == -1) {
        perror("Erro ao criar fifo_runnerToController");
        return 1;
    }
    printf("[controller] FIFO '%s' criado. A esperar por mensagens...\n", caminho_fifo_controller);

    int fd_fifo_controller = open(caminho_fifo_controller, O_RDWR);
    if (fd_fifo_controller == -1) {
        perror("[controller] Erro ao abrir fifo_runnerToController");
        return 1;
    }

    ssize_t bytes_lidos;
    int terminar = 0; 
    int estado;
    // Inicializar a estrutura onde se vão guardar e escalonar os comandos
    Package pacote;
    pid_t pid_recolhido;
    int fd_pacote_termina = 0;

    while(!terminar || escalonador.filaEspera->size > 0 || nr_comandos_hash > 0 || escalonador.executions > 0){

        // Fazer read tantas vezes quantos os elementos à espera de executar

        // Colocar todos os packages lidos na fila de espera

        if ((bytes_lidos = read(fd_fifo_controller, &pacote, sizeof(pacote))) > 0){

            printf("READ BYTES = %ld\n", bytes_lidos);

            clock_gettime(CLOCK_MONOTONIC, &inicio);

            printf("---\nsize fila: %d\n", escalonador.filaEspera->size);

            //Abrir um fifo com o pid do runner
            char caminho_fifo_runner[256];
            snprintf(caminho_fifo_runner, sizeof(caminho_fifo_runner), "%s", pacote.fifo_runner);

            // controller abre fifo para escrita para o runner
            int fd_runner_escrita = open(caminho_fifo_runner, O_WRONLY);


            printf("Info do package\nTipo comando: %d, Status: %d, Args: %s\n", pacote.comando.tipo, pacote.status, pacote.comando.args[1]);
            printf("Exec: %d\n", escalonador.executions);

            if (pacote.comando.tipo == CMD_EXEC) {
                if (escalonador.policy == FCFS) enqueue(escalonador.filaEspera, &pacote);
                else if (escalonador.policy == LIFO) push_head(escalonador.filaEspera, &pacote);
                else {
                    /* Queue *userCmdQueue = get_from_hashRR(escalonador.hashRR, pacote.comando.userId);
                    if (userCmdQueue) {
                        printf("Fila do user %d já existe, adicionando comando à fila\n", pacote.comando.userId);
                        enqueue(userCmdQueue, &pacote);
                        nr_comandos_hash++;
                        if (userCmdQueue != NULL && userCmdQueue->head != NULL && !userInQueue(&idQueue, pacote.comando.userId))
                            userEnqueue(&idQueue, pacote.comando.userId);
                    }
                    else {
                        Queue *userQueue2 = malloc(sizeof(Queue));
                        queueInit(userQueue2);
                        enqueue(userQueue2, &pacote);
                        if (userQueue2->size == 0) printf("Fila do user %d vazia\n", pacote.comando.userId);
                        nr_comandos_hash++;
                        printf("comando do user %d adicionado à hash\n", pacote.comando.userId);
                        add_to_hashRR(escalonador.hashRR, pacote.comando.userId, userQueue2);
                        userEnqueue(&idQueue, pacote.comando.userId);
                    }
                    */
                   Queue *userCmdQueue = get_from_hashRR(escalonador.hashRR, pacote.comando.userId);

                    if (!userCmdQueue) {
                        userCmdQueue = malloc(sizeof(Queue));
                        queueInit(userCmdQueue);
                        add_to_hashRR(escalonador.hashRR, pacote.comando.userId, userCmdQueue);
                        userEnqueue(&idQueue, pacote.comando.userId);
                    }

                    enqueue(userCmdQueue, &pacote);
                    nr_comandos_hash++;
                }
            }

            else if (pacote.comando.tipo == CMD_CONSULTA) {

                pid_t pid_filho_consulta = fork();
                if (pid_filho_consulta == 0) {
                    controller_enviaInfoComandos(escalonador.arrayPackages, escalonador.executions, escalonador.filaEspera, escalonador.hashRR, escalonador.policy, fd_runner_escrita, &idQueue);
                    close(fd_runner_escrita);
                    _exit(0);
                }
                if (waitpid(pid_filho_consulta, &estado, 0) > 0) {
                    if (!WIFEXITED(estado) || WEXITSTATUS(estado) != 0) {
                        perror("Erro a receber status da consulta");
                        return 1;
                    }
                }

            }

            else if (pacote.comando.tipo == CMD_OFF) {

                terminar = 1;

                fd_pacote_termina = fd_runner_escrita;

                unlink(caminho_fifo_controller);

            }

            else if (pacote.comando.tipo == CMD_FIM) {
                // Escrever em ficheiro e contar tempo de execução
                clock_gettime(CLOCK_MONOTONIC, &fim);
                tempo_execucao = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
                InfoComando info = {
                    pacote.runnerPid,
                    pacote.comando.userId,
                    tempo_execucao,
                };
                escreveInfoComandosEmFicheiro (info, CMDFILE);

                if (escalonador.policy == FCFS) removePackageporFilhoPid(escalonador.arrayPackages, escalonador.executions, pacote.runnerPid);
                else if (escalonador.policy == RR) {
                    Queue *q = get_from_hashRR(escalonador.hashRR, pacote.comando.userId);
                    if (q) {
                        if (q->head != NULL) {
                            userEnqueue(&idQueue, pacote.comando.userId);  // se ainda houver comandos do user, re-enfileirar o userId
                        }
                    }
                    printf("CMD_FIM user %d → q=%p, head=%p\n", pacote.comando.userId, q, q ? q->head : NULL);
                }
                escalonador.executions--;
            }
        
        }

        printf("afonso\n");

        // RECOLHER FILHOS TERMINADOS 
        while ((pid_recolhido = waitpid(-1, &estado, WNOHANG)) > 0);

        // Realizar o escalonamento tendo em conta o limite de comandos em paralelo

        printf("executions: %d e tamanho : %d\n", escalonador.executions, escalonador.filaEspera->size);
        printf("debug hash: %d\n", nr_comandos_hash);
        while (escalonador.executions < escalonador.comandosParalelo && (escalonador.filaEspera->size > 0 || nr_comandos_hash > 0)) {

            printf("costa\n");

            Package pacote_selecionado = schedPolicy(&escalonador, &idQueue);
            if (escalonador.policy == RR) nr_comandos_hash--;
            printf("2 debug hash: %d\n", nr_comandos_hash);

            pid_t pid_filho = fork();

            if (pid_filho == 0) {

                pid_t pid_corrente_filho = getpid();

                /* Filho do controller cria FIFO para enviar autorizacao ao runner para executar
                char caminho_fifo_filho[256];
                sprintf(caminho_fifo_filho, "/tmp/runnerTofilho%d", pid_corrente_filho);

                unlink(caminho_fifo_filho); // Se já existir, apaga antes de criar
                if (mkfifo(caminho_fifo_filho, 0666) < 0) {
                    perror("Erro ao criar FIFO do runner para o filho");
                    exit(1);
                }
                */


                // Processo filho envia o seu pid para o runner saber que pode executar o comando

                printf("Fifo nome: %s\n", pacote_selecionado.fifo_runner);

                int fd_runner_selecionado = open(pacote_selecionado.fifo_runner, O_WRONLY);

                if (fd_runner_selecionado < 0) {
                    printf("caminho fifo runner: %s\n", pacote_selecionado.fifo_runner);
                    perror("FIFO do runner não existe ou já terminou");

                    // NÃO continuar
                    _exit(0);
                }

                controller_enviaPidFilho(pacote_selecionado.fifo_runner, pid_corrente_filho, fd_runner_selecionado);

                printf("ola\n");

                close(fd_runner_selecionado);

                /* Processo filho abre fifo para ler a resposta do runner

                int fd_leitura_filho = open(caminho_fifo_filho, O_RDONLY);

                printf("bbbb\n");
                
                pid_t pid_resposta_runner;
                if (read(fd_leitura_filho, &pid_resposta_runner, sizeof(pid_t)) > 0) {
                    printf("aaaa\n");
                    clock_gettime(CLOCK_MONOTONIC, &fim);
                    tempo_execucao = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
                    InfoComando info = {
                        pacote_selecionado.runnerPid,
                        pacote_selecionado.comando.userId,
                        tempo_execucao,
                    };
                    escreveInfoComandosEmFicheiro (info, CMDFILE);
                }
                */

                _exit(0);
            }

            else {

                pacote_selecionado.filhoPid = pid_filho;

                addPackageArray(escalonador.arrayPackages, escalonador.executions, &pacote_selecionado);

                escalonador.executions++;
            }
            printf("Executions nr : %d\n", escalonador.executions);
        }
    }

    write(fd_pacote_termina, "ADEUS", 5);
    //close(fd_fifo_controller);
    //unlink(caminho_fifo_controller);

    return 0;
}