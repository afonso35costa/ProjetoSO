#ifndef PACKAGE

#define NAME_FIFO 256 // TAMANHO MAXIMO DO FIFO

typedef struct package{
    Comando comando;
    char fifo_runner[NAME_FIFO];
    int status;
} Package;

void package_init(Package *pkg, Comando *cmd);

Comando* get_comando(Package *p);
int get_status(Package *p);

void set_comando(Comando *c, Package *p);
void set_status(int status, Package *p);

#endif