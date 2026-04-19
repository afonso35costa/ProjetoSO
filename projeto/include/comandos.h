#ifndef COMANDOS
#define COMANDOS

#define ARGS_LIM 16
#define ARGS_TAM 128

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
    int userId;
} Comando;

typedef struct {
    char args[ARGS_LIM][ARGS_TAM];  // argumentos do comando
    int  num_args;
    char stdin_file[ARGS_TAM];      // ficheiro para 
    char stdout_file[ARGS_TAM];     // ficheiro para >
    char stderr_file[ARGS_TAM];     // ficheiro para 2>
} Segmento;

typedef struct {
    Segmento segmentos[ARGS_LIM];   // cada segmento é um comando do pipe
    int num_segmentos;
} ComandoParsed;

TipoComando verificaTipo(char *tipo);
Comando criaComando(int argc, char *argv[], int *temOperadores);

TipoComando get_tipo(Comando *c);
int get_numArgs(Comando *c);
char* get_arg_porIndice(Comando *c, int index);
int get_userId(Comando *c);
ComandoParsed parseComando(Comando *cmd);

#endif