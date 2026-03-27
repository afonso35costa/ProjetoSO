#include "comandos.h"


const char* stringComando(TipoComando tipo){
    if (tipo == CMD_EXEC) return "Executar comando";
    else if (tipo == CMD_CONSULTA) return "Consultar comandos em execução";
    else if (tipo == CMD_OFF) return "Terminar o programa controller";
    else return "Comando desconhecido";
}

TipoComando verificaTipo(char *tipo) {
    if (strcmp(tipo, "-e") == 0) return CMD_EXEC;
    if (strcmp(tipo, "-c") == 0) return CMD_CONSULTA;
    if (strcmp(tipo, "-d") == 0) return CMD_OFF;
    return CMD_UNKNOWN;
}

Comando criaComando(int argc, char *argv[]) {

    Comando cmd;
    cmd.num_args = 0;
    cmd.tipo = verificaTipo(argv[1]);

    for (int i = 2; i < argc && cmd.num_args < ARGS_LIM; i++) {
        strcpy(cmd.args[cmd.num_args], argv[i]);
        cmd.args[cmd.num_args][ARGS_LIM - 1] = '\0';
        cmd.num_args++;
    }

    return cmd;
}

TipoComando get_tipo(Comando *c){
    return c->tipo;
}

int get_numArgs(Comando *c){
    return c->num_args;
}

char** get_args(Comando *c){
    return c->args;
}
