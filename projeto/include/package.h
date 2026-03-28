#ifndef PACKAGE
#include "comandos.h"

#define NAME_FIFO 256 // TAMANHO MAXIMO DO FIFO

typedef enum{
    WAITING = 0, // quando o comando ainda nao foi executado
    RUNNING = 1, // quando o comando recebe autorização para executar
} PackageStatus;

typedef struct package{
    Comando comando;
    char fifo_runner[NAME_FIFO];
    PackageStatus status;
    int arrival_time;
} Package;


void package_init(Package *pkg, Comando *cmd);

Comando* get_comando(Package *p);
int get_status(Package *p);
int get_time(Package *p);

void set_comando(Comando *c, Package *p);
void set_status(int status, Package *p);

#endif