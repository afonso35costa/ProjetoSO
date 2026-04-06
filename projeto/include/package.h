#ifndef PACKAGE
#define PACKAGE
#include "comandos.h"
#include <fcntl.h>

#define NAME_FIFO 256 // TAMANHO MAXIMO DO FIFO

typedef enum{
    WAITING = 0, // quando o comando ainda nao foi executado
    RUNNING = 1, // quando o comando recebe autorização para executar
} PackageStatus;

typedef struct {
    Comando comando;
    char fifo_runner[NAME_FIFO];
    PackageStatus status;
    int arrival_time;
    pid_t pid;
} Package;

typedef struct{
    int comando_id;
    int user_id;
} InfoComando;

void package_init(Package *pkg, Comando *cmd);

Comando* get_comando(Package *p);
int get_status(Package *p);
int get_time(Package *p);
pid_t get_pid(Package *p);

void set_comando(Comando *c, Package *p);
void set_status(int status, Package *p);

void addPackageArray (Package* pkgs, int nr_cmds, Package *pkg);
void removePackageArray(Package*pkgs, int executions, pid_t pid);

#endif