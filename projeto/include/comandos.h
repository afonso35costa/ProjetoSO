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

const char* stringComando(TipoComando tipo);
TipoComando verificaTipo(char *tipo);
Comando criaComando(int argc, char *argv[]);

TipoComando get_tipo(Comando *c);
int get_numArgs(Comando *c);
char** get_args(Comando *c);

#endif