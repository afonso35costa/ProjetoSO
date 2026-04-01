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

void controller_enviaAutorizacao(const char* fifo_controllerToRunner, char* autorizacao){
    int fd_runner = open(fifo_controllerToRunner, O_WRONLY);
    if (fd_runner < 0){
        printf("[controller] erro ao abrir FIFO do runner para enviar autorizacao\n");
        return;
    }
    if (write(fd_runner, autorizacao, sizeof(autorizacao)) == -1) {
        printf("[controller] erro ao dar autorização\n");
    }
    close(fd_runner);
}

int main(int argc, char* argv[]){
    Scheduler s;
    Queue q;
    schedInit(&s, &q, argv[1], argv[2]);
    
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

    // Inicializar a estrutura onde se vão guardar e escalonar os comandos

    Package pkg;
    ssize_t nbytes;
    int terminar_pai = 0; 
    int status;

    while(1){
		if ((nbytes = read(fd_controller, &pkg, sizeof(pkg))) > 0){
			pid_t pidFilho = fork();
			if (pidFilho == 0){

                //Adicionar comando ao array ou à fila de espera
                if (s.executions < s.comandosParalelo) addPackageArray(s.arrayPackages, s.executions, &pkg);
                else enqueue(s.filaEspera, &pkg);

				// Executar o escalonamento
				

				//Abrir um fifo com o pid do runner e enviar a autorização

				char fifo[256];
				snprintf(fifo, sizeof(fifo), "%s", pkg.fifo_runner);

                char autorizacao[4] = "OLA";
                autorizacao[3] = '\0';
				controller_enviaAutorizacao(fifo, autorizacao);
				_exit(0);
			}
			wait(&status);
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
				perror("Erro a receber status");
				return 1;
			}
		}
	}
    close(fd_controller);
    unlink(fifo_runnerToController);

    return 0;
}