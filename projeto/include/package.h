#ifndef PACKAGE
#define PACKAGE
#include "comandos.h"
#include <fcntl.h>

#define NAME_FIFO 256 // TAMANHO MAXIMO DO FIFO
#define CMDFILE "infocomandos"

typedef enum{
    WAITING = 0, // quando o comando ainda nao foi executado
    RUNNING = 1, // quando o comando recebe autorização para executar
} PackageStatus;

typedef struct {
    Comando comando;
    char fifo_runner[NAME_FIFO];
    PackageStatus status;
    double arrival_time;
    pid_t runnerPid;
    pid_t filhoPid;
} Package;

typedef struct{
    int comando_id;
    int user_id;
    double arrival_time;
} InfoComando;

void package_init(Package *pkg, Comando *cmd);

Comando* get_comando(Package *p);
int get_status(Package *p);
double get_time(Package *p);
pid_t get_runnerPid(Package *p);
pid_t get_filhoPid(Package *p);

void set_comando(Comando *c, Package *p);
void set_status(int status, Package *p);

void addPackageArray (Package* pkgs, int nr_cmds, Package *pkg);
void removePackageporFilhoPid(Package*pkgs, int executions, pid_t pid);
void escreveInfoComandosEmFicheiro (InfoComando info, const char* file);

#endif