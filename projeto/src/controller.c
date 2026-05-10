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
#include <assert.h>
#include <glib.h>

void controller_enviaPidFilho(const char* caminho_fifo_runner, pid_t pid_filho, int fd_runner){
    // Envia o PID do filho para o runner através do FIFO especificado.
    // Parâmetros:
    // caminho_fifo_runner: caminho do FIFO para o runner.
    // pid_filho: PID do processo filho a ser enviado.
    // fd_runner: descritor de arquivo do FIFO aberto para escrita.
    if (fd_runner < 0){
        perror("[controller] erro ao abrir FIFO do runner para enviar autorizacao");
        return;
    }
    if (write(fd_runner, &pid_filho, sizeof(pid_filho)) == -1) {
        //printf("[controller] erro ao dar autorização\n");
    }
}


int main(int argc, char* argv[]){

    struct timespec inicio, fim;
    // Função principal do programa que gerencia a execução de comandos.
    // Parâmetros:
    // argc: número de argumentos da linha de comando.
    // argv: vetor de argumentos da linha de comando.
    double tempo_execucao;


    if (argc < 3) {
        //printf("<ParallellComands> e <SchedPolicy> necessários\n");
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

        // RECOLHER FILHOS TERMINADOS 
        while ((pid_recolhido = waitpid(-1, &estado, WNOHANG)) > 0);

        // Colocar todos os packages lidos na fila de espera

        if ((bytes_lidos = read(fd_fifo_controller, &pacote, sizeof(pacote))) > 0) {
            assert(bytes_lidos == sizeof(pacote));

            clock_gettime(CLOCK_MONOTONIC, &inicio);

            //Abrir um fifo com o pid do runner
            char caminho_fifo_runner[256];
            snprintf(caminho_fifo_runner, sizeof(caminho_fifo_runner), "%s", pacote.fifo_runner);

            // controller abre fifo para escrita para o runner
            int fd_runner_escrita = open(caminho_fifo_runner, O_WRONLY);

            if (pacote.comando.tipo == CMD_EXEC) {
                if (escalonador.policy == FCFS) enqueue(escalonador.filaEspera, &pacote);
                else if (escalonador.policy == LIFO) push_head(escalonador.filaEspera, &pacote);
                else {
                   Queue *userCmdQueue = get_from_hashRR(escalonador.hashRR, pacote.comando.userId);

                    if (!userCmdQueue) {
                        userCmdQueue = malloc(sizeof(Queue));
                        queueInit(userCmdQueue);
                        add_to_hashRR(escalonador.hashRR, pacote.comando.userId, userCmdQueue);
                        if (!userInQueue(&idQueue, pacote.comando.userId))
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
                    
                    if (q && q->head != NULL) {
                        if (!userInQueue(&idQueue, pacote.comando.userId))  // ← ADICIONAR
                            userEnqueue(&idQueue, pacote.comando.userId);
                    }
                }
                escalonador.executions--;
            }
        
        }

        // Realizar o escalonamento tendo em conta o limite de comandos em paralelo

        while (escalonador.executions < escalonador.comandosParalelo && (escalonador.filaEspera->size > 0 || nr_comandos_hash > 0)) {

            printf("nr execucoes :%d\n", escalonador.executions);

            Package pacote_selecionado = schedPolicy(&escalonador, &idQueue);
            if (escalonador.policy == RR) nr_comandos_hash--;
            

            pid_t pid_filho = fork();

            if (pid_filho == 0) {

                pid_t pid_corrente_filho = getpid();

                // Processo filho envia o seu pid para o runner saber que pode executar o comando

                int fd_runner_selecionado = open(pacote_selecionado.fifo_runner, O_WRONLY);

                if (fd_runner_selecionado < 0) {
                    _exit(0);
                }

                controller_enviaPidFilho(pacote_selecionado.fifo_runner, pid_corrente_filho, fd_runner_selecionado);
                close(fd_runner_selecionado);
                _exit(0);
            }

            else {

                pacote_selecionado.filhoPid = pid_filho;

                addPackageArray(escalonador.arrayPackages, escalonador.executions, &pacote_selecionado);

                escalonador.executions++;
            }
        }
    }

    write(fd_pacote_termina, "ADEUS", 5);
    close(fd_fifo_controller);
    unlink(caminho_fifo_controller);

    return 0;
}
