#include "package.h"
#include "comandos.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


void package_init(Package *pkg, Comando *cmd) {
    if (!cmd) {
        printf("Command ou MetaInformation inválido ao iniciar mensagem.\n");
        return;
    }
    if (!pkg) {
        printf("Mensagem inválida.\n");
        return;
    }
    pkg->comando = *cmd;
    pkg->status = WAITING;
    pkg->arrival_time = time(NULL);
    pkg->runnerPid = getpid();
    memset(pkg->fifo_runner, 0, NAME_FIFO);  // Inicializar o nome do FIFO
         // Initialize the package with the given command and set its status and arrival time
}

Comando* get_comando(Package *p){
    if(p){
        return &p->comando;
    }
    return NULL;
         // Return the command associated with the package, or NULL if the package is invalid
}

int get_status(Package *p){
    if (p) {
        return p->status;
    }
    return 0;
         // Return the status of the package, or 0 if the package is invalid
}

double get_time(Package *p){
    if (p) return p->arrival_time;
    return 0;
         // Return the arrival time of the package, or 0 if the package is invalid
}

pid_t get_runnerPid(Package *p){
    if (p) return p->runnerPid;
    return 0;
         // Return the runner PID of the package, or 0 if the package is invalid
}

pid_t get_filhoPid(Package *p){
    if (p) return p->filhoPid;
    return 0;
         // Return the child PID of the package, or 0 if the package is invalid
}

void set_comando(Comando *c, Package *p){
    if (p && c){
        p->comando = *c;
    }
         // Set the command for the package
}

void set_status(int status, Package *p){
    if (p) p->status = status;
         // Set the status of the package
}

void set_fifo(char* fifo, Package *p){
    if (fifo && p){
        strcpy(p->fifo_runner, fifo);
    }
         // Set the FIFO name for the package
}

void addPackageArray (Package* pkgs, int nr_cmds, Package *pkg){
    pkgs[nr_cmds] = *pkg;
         // Add a package to the array of packages at the specified index
}

void removePackageporFilhoPid(Package*pkgs, int executions, pid_t pid) {
    for (int i = 0; i < executions; i++) {
        if (pkgs[i].filhoPid == pid) {
            pkgs[i] = pkgs[executions - 1];
            break;
        }
    }
         // Remove a package from the array based on the child PID
}

void escreveInfoComandosEmFicheiro (InfoComando info, const char* file) {
    int fd = open(file, O_CREAT | O_WRONLY | O_APPEND, 0666);
    if (fd < 0) {
        perror("Erro a criar ficheiro");
        exit(1);
    }
    if (fd > 0) {
        write(fd, &info, sizeof(InfoComando));
    }
    close(fd);
         // Write command information to a specified file
}



