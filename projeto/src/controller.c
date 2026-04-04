#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include "package.h"
#include "comandos.h"
#include "scheduler.h"
#include "queue.h"
#include "executer.h"

void controller_enviaAutorizacao(const char* fifo_controllerToRunner, char* autorizacao, int fd_runner){
    if (fd_runner < 0){
        printf("[controller] erro ao abrir FIFO do runner para enviar autorizacao\n");
        return;
    }
    if (write(fd_runner, autorizacao, sizeof(autorizacao)) == -1) {
        printf("[controller] erro ao dar autorização\n");
    }
}


int main(int argc, char* argv[]){
    if (argc < 3) {
        printf("<ParallellComands> e <SchedPolicy> necessários\n");
        return 0;
    }
    Scheduler s;
    Queue q;
    schedInit(&s, &q, argv[1], argv[2]);
    if (!policyExists(s.policy)) return 0;
    
    // FIFO do controller
    const char *fifo_runnerToController = "/tmp/runnerToController";
    unlink(fifo_runnerToController);
    if (mkfifo(fifo_runnerToController, 0666) == -1) {
        perror("Erro ao criar fifo_runnerToController");
        return 1;
    }
    printf("[controller] FIFO '%s' criado. A esperar por mensagens...\n", fifo_runnerToController);

    int fd_controller = open(fifo_runnerToController, O_RDONLY);
    if (fd_controller == -1) {
        perror("[controller] Erro ao abrir fifo_runnerToController");
        return 1;
    }

    ssize_t nbytes;
    int terminar_pai = 0; 
    int status;
    // Inicializar a estrutura onde se vão guardar e escalonar os comandos
    Package pkg;

    while(!terminar_pai || s.filaEspera->size > 0 || s.executions > 0){

		if ((nbytes = read(fd_controller, &pkg, sizeof(pkg))) > 0){

            //Adicionar package ao array ou à fila de espera
            if (s.executions < s.comandosParalelo) {
                if (!terminar_pai) {
                    addPackageArray(s.arrayPackages, s.executions, &pkg);
                    s.executions++;
                }

                //Executar o escalonamento
                Package chosen = schedPolicy(&s);
                if (chosen.comando.tipo == CMD_OFF) terminar_pai++;

                //Criar processo filho para executar em paralelo
                pid_t pidFilho = fork();
                if (pidFilho == 0){
                    //Abrir um fifo com o pid do runner e enviar a autorização
                    char fifo[256];
                    snprintf(fifo, sizeof(fifo), "%s", pkg.fifo_runner);

                    char autorizacao[4] = "OLA";
                    autorizacao[3] = '\0';
                    int fd = open(fifo, O_WRONLY);
                    controller_enviaAutorizacao(fifo, autorizacao, fd);

                    //Enviar a informação de consulta caso seja esse o comando
                    if (chosen.comando.tipo == CMD_CONSULTA) controller_enviaInfoComandos(s.arrayPackages, s.executions, s.filaEspera, fd);
                    else if (chosen.comando.tipo == CMD_OFF) {
                        write(fd, "ADEUS", 5);
                    }
                    close(fd);
                    _exit(0);
                }
                pid_t pid = waitpid(-1, &status, 0);
                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                    perror("Erro a receber status");
                    return 1;
                }
                if (s.filaEspera->size > 0){
                    s.arrayPackages[s.executions] = dequeue(s.filaEspera);
                }
                else s.executions--;
            }
            else if (!terminar_pai) enqueue(s.filaEspera, &pkg);
            else {
                char msgTerminar[64];
                int n = sprintf(msgTerminar, "O controller irá encerrar, impossível realizar o comando");
                write(1, &msgTerminar, n);
            }
		}
	}
    close(fd_controller);
    unlink(fifo_runnerToController);

    return 0;
}