#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wait.h"
#include "package.h"
#include "comandos.h"
#include "executer.h"

void runner_enviaPackage (const char* fifo_controller_para_runner, Package *pacote, int fd_para_controller) {
    
    if (fd_para_controller < 0) {
        perror("Erro ao abrir fifo do controller para escrita");
        unlink(fifo_controller_para_runner);
        exit(1);
    }

    ssize_t escrito = write(fd_para_controller, pacote, sizeof(Package));
    if (escrito != sizeof(Package)) {
        printf("[runner] Erro ao enviar package\n");
        close(fd_para_controller);
        unlink(fifo_controller_para_runner);
        exit(1);
    }
    if (pacote->comando.tipo == CMD_EXEC){
        char buffer_msg[256];
        int tam_msg = sprintf(buffer_msg, "[runner] comando %d submetido\n", pacote->runnerPid);
        write(1, buffer_msg, tam_msg);
    }
    printf("sizeof(Package) = %zu\n", sizeof(Package));
    close(fd_para_controller);
}

pid_t runner_recebePidFilho (const char* fifo_controller_para_runner, int fd_resposta_controller){
    if (fd_resposta_controller < 0){
        printf("Erro a abrir fifo do controller\n");
        unlink(fifo_controller_para_runner);
        exit(1);
    }

    pid_t pid_filho;
    int n = read(fd_resposta_controller, &pid_filho, sizeof(pid_filho));
    if (n < 0){
        printf("[runner] Erro ao receber pid do controller\n");
    }
    printf("Pid filho: %d\n", pid_filho);
    return pid_filho;
}

void runner_avisaFim (const char* fifo_controller_para_runner, int fd_para_filho){
    if (fd_para_filho < 0) {
        perror("Erro ao abrir fifo do filho para escrita");
        unlink(fifo_controller_para_runner);
        exit(1);
    }

    ssize_t size = write(fd_para_filho, "FIM", 3);
    if (size != 3) {
        printf("[runner] Erro ao enviar mensagem de término\n");
        close(fd_para_filho);
        unlink(fifo_controller_para_runner);
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

    Comando comando = criaComando(argc, argv);
    if (comando.tipo == CMD_UNKNOWN) {
        printf("Comando desconhecido\n");
        exit(1);
    }

    // Criação de um FIFO único para cada runner
    char fifo_controller_para_runner[256];
    snprintf(fifo_controller_para_runner, sizeof(fifo_controller_para_runner), "/tmp/controllerToRunner_%d", getpid());

    // Cria o FIFO para resposta
    unlink(fifo_controller_para_runner); // Se já existir, apaga antes de criar
    if (mkfifo(fifo_controller_para_runner, 0666) < 0) {
        perror("Erro ao criar FIFO do runner");
        exit(1);
    }

    Package pacote;
    package_init(&pacote, &comando);

    snprintf(pacote.fifo_runner, sizeof(pacote.fifo_runner), "%s", fifo_controller_para_runner);

    // FIFO para enviar a mensagem para o controller
    const char *fifo_runner_para_controller = "/tmp/runnerToController";
    int fd_para_controller = open(fifo_runner_para_controller, O_WRONLY);

     
    runner_enviaPackage(fifo_controller_para_runner, &pacote, fd_para_controller);

    int fd_resposta_controller = open(fifo_controller_para_runner, O_RDONLY);
    if (comando.tipo == CMD_EXEC){
        pid_t pid_filho = runner_recebePidFilho(fifo_controller_para_runner, fd_resposta_controller);
        
        // FIFO para enviar a mensagem para o filho
        char fifo_runner_para_filho[256];
        snprintf(fifo_runner_para_filho, sizeof(fifo_runner_para_filho), "/tmp/runnerTofilho%d", pid_filho);
        
        int fd_para_filho = open(fifo_runner_para_filho, O_WRONLY);

        //Executar o comando
        pid_t pid = fork();
        if (pid == 0) {
            // filho executa o comando
            executerMain(&comando, fd_resposta_controller);
            perror("Erro no execvp");
            _exit(1);
        }
        // pai espera que o filho termine
        int status;
        waitpid(pid, &status, 0);

        // agora sim podes imprimir
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            char mensagem_fim[256];
            int tam_mensagem_fim = sprintf(mensagem_fim, "[runner] comando %d terminado\n", getpid());
            write(1, mensagem_fim, tam_mensagem_fim);
        }
        else
            perror("[runner] command failed\n");

        runner_avisaFim(fifo_controller_para_runner, fd_para_filho);
    }
    else executerMain(&comando, fd_resposta_controller);

    if (pacote.comando.tipo == CMD_OFF) {
        // Runner que enviou -s fica à espera da confirmação
        char buf[6];
        int n = read(fd_resposta_controller, &buf, 5);
        if (n > 0) {
            printf("[runner] controller exited.\n");
        }
        else perror("Erro a ler término");
    }
    close(fd_resposta_controller);

    return 0;
}
