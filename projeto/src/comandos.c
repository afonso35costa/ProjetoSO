#include "comandos.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

TipoComando verificaTipo(char *tipo) {
    if (strcmp(tipo, "-e") == 0) return CMD_EXEC;
    if (strcmp(tipo, "-c") == 0) return CMD_CONSULTA;
    if (strcmp(tipo, "-s") == 0) return CMD_OFF;
    else return CMD_UNKNOWN;
}

Comando criaComando(int argc, char *argv[], int *temOperadores) {
    Comando cmd;
    cmd.num_args = 0;
    cmd.tipo = verificaTipo(argv[1]);
    *temOperadores = 0;

    if (cmd.tipo == CMD_EXEC) {
        cmd.userId = atoi(argv[2]);

        char cmd_str[ARGS_TAM];
        strncpy(cmd_str, argv[3], ARGS_TAM - 1);
        cmd_str[ARGS_TAM - 1] = '\0';

        // split da string por espaços
        char *token = strtok(cmd_str, " ");
        while (token != NULL && cmd.num_args < ARGS_LIM) {
            if (strcmp(token, "|") == 0 ||
                strcmp(token, ">") == 0 ||
                strcmp(token, "<") == 0 ||
                strcmp(token, "2>") == 0) {
                *temOperadores = 1;
            }
            strncpy(cmd.args[cmd.num_args], token, ARGS_TAM - 1);
            cmd.args[cmd.num_args][ARGS_TAM - 1] = '\0';
            cmd.num_args++;
            token = strtok(NULL, " ");
        }
    }

    return cmd;
}

TipoComando get_tipo(Comando *c){
    return c->tipo;
}

int get_numArgs(Comando *c){
    return c->num_args;
}

char* get_arg_porIndice(Comando *c, int index){
    return c->args[index];
}

int get_userId(Comando *c){
    return c->userId;
}

ComandoParsed parseComando(Comando *cmd) {

    ComandoParsed parsed;
    memset(&parsed, 0, sizeof(ComandoParsed));

    int seg = 0;

    for (int i = 0; i < cmd->num_args; i++) {

        if (strcmp(cmd->args[i], "|") == 0) {

            seg++;

            if (seg >= ARGS_LIM)
                break;

        }

        else if (strcmp(cmd->args[i], ">") == 0) {

            if (i + 1 < cmd->num_args)
                strcpy(parsed.segmentos[seg].stdout_file, cmd->args[++i]);

        }

        else if (strcmp(cmd->args[i], "<") == 0) {

            if (i + 1 < cmd->num_args)
                strcpy(parsed.segmentos[seg].stdin_file, cmd->args[++i]);

        }

        else if (strcmp(cmd->args[i], "2>") == 0) {

            if (i + 1 < cmd->num_args)
                strcpy(parsed.segmentos[seg].stderr_file, cmd->args[++i]);

        }

        else {

            Segmento *s = &parsed.segmentos[seg];

            strcpy(s->args[s->num_args], cmd->args[i]);
            s->num_args++;
        }
    }

    parsed.num_segmentos = seg + 1;

    return parsed;
}