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

void controller_enviaPidFilho(const char* fifo_controllerToRunner, pid_t pid, int fd_runner){
    if (fd_runner < 0){
        printf("[controller] erro ao abrir FIFO do runner para enviar autorizacao\n");
        return;
    }
    if (write(fd_runner, &pid, sizeof(pid)) == -1) {
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
    queueInit(&q);
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
            printf("---\nsize fila: %d\n", s.filaEspera->size);

            if (pkg.comando.tipo == CMD_OFF) terminar_pai++;
            pid_t pid;

            //Abrir um fifo com o pid do runner
            char fifo[256];
            snprintf(fifo, sizeof(fifo), "%s", pkg.fifo_runner);
            int fd = open(fifo, O_WRONLY);
            printf("Info do package\nTipo comando: %d, Status: %d, Args: %s\n", pkg.comando.tipo, pkg.status, pkg.comando.args[1]);
            printf("Exec: %d\n", s.executions);

            //Adicionar package ao array ou à fila de espera
            if (s.executions < s.comandosParalelo && pkg.comando.tipo == CMD_EXEC) {

                // LIGAR O TIMER

                if (!terminar_pai) {
                    addPackageArray(s.arrayPackages, s.executions, &pkg);
                    s.executions++;
                }
                //Executar o escalonamento
                Package chosen = schedPolicy(&s);

                //Criar processo filho para executar em paralelo
                pid_t pidFilho = fork();
                if (pidFilho == 0){
                    // Enviar a autorização para o RUNNER executar
                    printf("EXEC: %d\n", s.executions);
                    pid_t pid = getpid();

                    // Filho do controller cria FIFO e fica à espera da mensagem de término do runner
                    char buffer[256];
                    sprintf(buffer, "/tmp/runnerTofilho%d", pid);

                    unlink(buffer); // Se já existir, apaga antes de criar
                    if (mkfifo(buffer, 0666) < 0) {
                        perror("Erro ao criar FIFO do runner para o filho");
                        exit(1);
                    }

                    if (chosen.pid != pkg.pid) {
                        //Abrir um fifo com o pid do runner
                        char fifoAuto[256];
                        snprintf(fifoAuto, sizeof(fifoAuto), "%s", chosen.fifo_runner);
                        int fd1 = open(fifoAuto, O_WRONLY);
                        controller_enviaPidFilho(fifoAuto, pid, fd1);
                        close(fd1);
                    }
                    else {
                        controller_enviaPidFilho(fifo, pid, fd);
                        printf("Enviei pidFilho\n");
                        close(fd);
                    }

                    // Abrir fifo para ler resposta do runner
                    int fd_filho = open(buffer, O_RDONLY);

                    pid_t resposta;
                    read(fd_filho, &resposta, sizeof(pid_t));
                    _exit(0);
                }
                pid = waitpid(-1, &status, 0);
                if (pid > 0){
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                        perror("Erro a receber status");
                        return 1;
                    }
                    printf("FIM\n");
                    removePackageArray(s.arrayPackages, s.executions, chosen.pid);
                    s.executions--;
                }
            }
            else if (!terminar_pai && pkg.comando.tipo == CMD_EXEC) {
                printf("peito\n");
                pid = waitpid(-1, &status, 0);
                if (pid > 0){
                    if (s.filaEspera->size > 0){
                        Package p = dequeue(s.filaEspera);
                        addPackageArray(s.arrayPackages, s.executions, &p);
                    }
                    else addPackageArray(s.arrayPackages, s.executions, &pkg);
                }
                else enqueue(s.filaEspera, &pkg);
                printf("Fila: %d\n", s.filaEspera->size);
            }
            else if (pkg.comando.tipo != CMD_EXEC){
                //Enviar a informação de consulta caso seja esse o comando
                if (pkg.comando.tipo == CMD_CONSULTA) controller_enviaInfoComandos(s.arrayPackages, s.executions, s.filaEspera, fd);
                else if (pkg.comando.tipo == CMD_OFF) {
                    write(fd, "ADEUS", 5);
                }
            }
            else {
                char msgTerminar[64];
                int n = sprintf(msgTerminar, "O controller irá encerrar, impossível realizar o comando");
                write(1, &msgTerminar, n);
            }
            close(fd);
		}
        else if (s.executions > 0 && s.executions < s.comandosParalelo){

            printf("AQUI\n");
        //Executar o escalonamento
            Package chosen = schedPolicy(&s);

            //Criar processo filho para executar em paralelo
            pid_t pidFilho = fork();
            if (pidFilho == 0){
                // Enviar a autorização para o RUNNER executar
                printf("EXEC: %d\n", s.executions);
                pid_t pid = getpid();

                //Abrir um fifo com o pid do runner
                char fifoAuto[256];
                snprintf(fifoAuto, sizeof(fifoAuto), "%s", chosen.fifo_runner);
                int fd1 = open(fifoAuto, O_WRONLY);
                controller_enviaPidFilho(fifoAuto, pid, fd1);
                close(fd1);

                // Filho do controller fica à espera da mensagem de término do runner
                char buffer[256];
                sprintf(buffer, "/tmp/runnerTofilho%d", pid);
                int fd_filho = open(buffer, O_RDONLY);

                pid_t resposta;
                read(fd_filho, &resposta, sizeof(pid_t));
                _exit(0);
            }
            pid_t pid = waitpid(-1, &status, 0);
            if (pid > 0){
                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                    perror("Erro a receber status");
                    return 1;
                }
                removePackageArray(s.arrayPackages, s.executions, chosen.pid);
            }
	    }
    }
    close(fd_controller);
    unlink(fifo_runnerToController);

    return 0;
}