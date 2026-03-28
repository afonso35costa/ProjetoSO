#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void controller_enviaAutorizacao(const char* fifo_controllerToRunner, int autorizacao){
    int fd_runner = open(fifo_controllerToRunner, O_WRONLY);
    if (fd_runner < 0){
        printf("[controller] erro ao abrir FIFO do runner para enviar autorizacao\n");
        return;
    }
    if (write(fd_runner, &autorizacao, sizeof(int)) == -1) {
        printf("[controller] erro ao dar autorização\n");
    }
    close(fd_runner);
}

int main(int argc, char* argv[]){
    
}