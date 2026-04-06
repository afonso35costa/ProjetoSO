#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "package.h"
#include "comandos.h"
#include "executer.h"

void runner_enviaPackage (const char* fifo_controllerToRunner, Package *p, int fd_controller) {
    
    if (fd_controller < 0) {
        perror("Erro ao abrir fifo_controller para escrita");
        unlink(fifo_controllerToRunner);
        exit(1);
    }

    ssize_t size = write(fd_controller, p, sizeof(Package));
    if (size != sizeof(Package)) {
        printf("[runner] Erro ao enviar package\n");
        close(fd_controller);
        unlink(fifo_controllerToRunner);
        exit(1);
    }
    if (p->comando.tipo == CMD_EXEC){
        char buffer1[256];
        int n1 = sprintf(buffer1, "[runner] command %d submitted\n", p->pid);
        write(1, &buffer1, n1);
    }
    printf("sizeof(Package) = %zu\n", sizeof(Package));
    close(fd_controller);
}

pid_t runner_recebePidFilho (const char* fifo_controllerToRunner, int fd_controller){
    if (fd_controller < 0){
        printf("Erro a abrir fifo do controller\n");
        unlink(fifo_controllerToRunner);
        exit(1);
    }

    pid_t pid;
    int n = read(fd_controller, &pid, sizeof(pid));
    if (n < 0){
        printf("[runner] Erro ao receber package do controller\n");
    }
    printf("Pid filho: %d\n", pid);
    return pid;
}

void runner_avisaFim (const char* fifo_controllerToRunner, int fd_filho){
    if (fd_filho < 0) {
        perror("Erro ao abrir fifo_controller para escrita");
        unlink(fifo_controllerToRunner);
        exit(1);
    }

    ssize_t size = write(fd_filho, "FIM", 3);
    if (size != 3) {
        printf("[runner] Erro ao enviar mensagem de término\n");
        close(fd_filho);
        unlink(fifo_controllerToRunner);
        exit(1);
    }
}

int main(int argc, char* argv[]){
    //mensagem de debug
    char debug[256];
    if (getcwd(debug, sizeof(debug)) != NULL) {
        printf("[DEBUG]: Servidor a correr no diretório: %s\n", debug);
    } else {
        perror("[DEBUG]: Erro ao obter o diretório atual (getcwd)");
    }

    if (argc < 2) {
        fprintf(stderr, "O executável runner precisa de mais argumentos para ser usado\n");
        exit(1);
    }

    Comando cmd = criaComando(argc, argv);
    if (cmd.tipo == CMD_UNKNOWN) {
        printf("Comando desconhecido\n");
        exit(1);
    }

    // Criação de um FIFO único para cada runner
    char fifo_controllerToRunner[256];
    snprintf(fifo_controllerToRunner, sizeof(fifo_controllerToRunner), "/tmp/controllerToRunner_%d", getpid());

    // Cria o FIFO para resposta
    unlink(fifo_controllerToRunner); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_controllerToRunner, 0666) < 0) {
        perror("Erro ao criar FIFO do runner");
        exit(1);
    }

    Package p;
    package_init(&p, &cmd);

    snprintf(p.fifo_runner, sizeof(p.fifo_runner), "%s", fifo_controllerToRunner);

    // FIFO para enviar a mensagem para o controller
    const char *fifo_runnerToController = "/tmp/runnerToController";
    int fd_controller = open(fifo_runnerToController, O_WRONLY);
    runner_enviaPackage(fifo_controllerToRunner, &p, fd_controller);

    int fd = open(fifo_controllerToRunner, O_RDONLY);
    if (cmd.tipo == CMD_EXEC){
        pid_t pidFilho = runner_recebePidFilho(fifo_controllerToRunner, fd);
        
        printf("gay\n");

        // FIFO para enviar a mensagem para o controller
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/tmp/runnerTofilho%d", pidFilho);
        
        printf("gay2\n");
        int fd_filho = open(buffer, O_WRONLY);

        printf("gay3\n");

        //Executar o comando
        executerMain(&cmd, fd);
        runner_avisaFim(fifo_controllerToRunner, fd_filho);
    }
    else executerMain(&cmd, fd);

    if (p.comando.tipo == CMD_EXEC){
        char buffer[256];
        int n = sprintf(buffer, "[runner] command %d finished\n", getpid());
        write(1, buffer, n);
    }
    else if (p.comando.tipo == CMD_OFF) {
        // Runner que enviou -s fica à espera da confirmação
        char buf[6];
        int n = read(fd, &buf, 5);
        if (n > 0) {
            printf("[runner] controller exited.\n");
        }
        else perror("Erro a ler término");
    }
    close(fd);

    return 0;
}