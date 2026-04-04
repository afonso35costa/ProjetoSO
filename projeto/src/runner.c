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

void runner_enviaPackage (const char* fifo_controllerToRunner, Package *p) {
    // FIFO para enviar a mensagem para o controller
    const char *fifo_runnerToController = "/tmp/runnerToController";

    // Abre o FIFO para escrita 
    int fd_controller = open(fifo_runnerToController, O_WRONLY);
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

    close(fd_controller);
}

void runner_recebeAutorizacao (const char* fifo_controllerToRunner, int fd_controller){
    if (fd_controller < 0){
        printf("Erro a abrir fifo do controller\n");
        unlink(fifo_controllerToRunner);
        exit(1);
    }

    char a[4];
    int n = read(fd_controller, &a, sizeof(a));
    if (n < 0){
        printf("[runner] Erro ao receber package do controller\n");
    }
    printf("Autorizacao: %s\n", a);
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
        perror("Erro ao criar FIFO do cliente");
        exit(1);
    }

    Package p;
    package_init(&p, &cmd);

    snprintf(p.fifo_runner, sizeof(p.fifo_runner), "%s", fifo_controllerToRunner);

    runner_enviaPackage(fifo_controllerToRunner, &p);

    int fd = open(fifo_controllerToRunner, O_RDONLY);
    runner_recebeAutorizacao(fifo_controllerToRunner, fd);

    //Executar o comando
    executerMain(&cmd, fd);

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