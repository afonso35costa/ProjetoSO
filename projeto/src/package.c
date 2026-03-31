#include "package.h"
#include "comandos.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>


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
    memset(pkg->fifo_runner, 0, NAME_FIFO);  // Inicializar o nome do FIFO
}

Comando* get_comando(Package *p){
    if(p){
        return &p->comando;
    }
    return NULL;
}

int get_status(Package *p){
    if (p) {
        return p->status;
    }
    return 0;
}

int get_time(Package *p){
    if (p) return p->arrival_time;
    return 0;
}

void set_comando(Comando *c, Package *p){
    if (p && c){
        p->comando = *c;
    }
}

void set_status(int status, Package *p){
    if (p) p->status = status;
}

void set_fifo(char* fifo, Package *p){
    if (fifo && p){
        strcpy(p->fifo_runner, fifo);
    }
}



