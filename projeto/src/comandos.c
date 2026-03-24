#ifndef COMANDOS
#define COMANDOS

#define ARGS_LIM 5
#define ARGS_TAM 256

typedef enum {
    CMD_UNKNOWN = -1,          /**< Comando inexistente. */
    CMD_EXEC = 1,               /**< Comando para executar (-e). */
    CMD_CONSULTA = 2,           /**< Comando para consultar comandos em execução (-c). */
    CMD_OFF = 3,            /**< Comando de terminação do controller (-d). */
} TipoComando;

/**
 * @struct Comando
 * @brief Estrutura que caracteriza um comando e os seus argumentos.
 */
typedef struct Comando {
    TipoComando tipo;                        /**< Tipo do comando. */
    int num_args;                            /**< Número de argumentos do comando verdadeiramente armazenados. */
    char args[ARGS_LIM][ARGS_TAM];       /**< Lista de argumentos do comando. */
} Comando;

const char* stringComando(TipoComando tipo){
    if (tipo == CMD_EXEC) return "Executar comando";
    else if (tipo == CMD_CONSULTA) return "Consultar comandos em execução";
    else if (tipo == CMD_OFF) return "Terminar o programa controller";
    else return "Comando desconhecido";
}

TipoComando verificaTipo(char *tipo, int numArgs) {
    if (strcmp(tipo, "-e") == 0) return CMD_EXEC;
    if (strcmp(tipo, "-c") == 0) return CMD_CONSULTA;
    if (strcmp(tipo, "-d") == 0) return CMD_OFF;
    return CMD_UNKNOWN;
}

Comando criaComando()

#endif