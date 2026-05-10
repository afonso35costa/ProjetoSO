# Orquestração de um Ambiente Multi-Runner

**Trabalho Prático — Sistemas Operativos**  
Universidade do Minho — 2025/2026

---

## Descrição

Este projeto implementa um ambiente de execução de comandos bash multi-utilizador assente num modelo cliente-servidor. Um programa **controller** centraliza a gestão e o escalonamento de comandos, enquanto múltiplos programas **runner** submetem pedidos de execução de forma concorrente. A comunicação é feita exclusivamente via **named pipes (FIFOs)**.

---

## Estrutura do Projeto

```
projeto/
├── src/
│   ├── controller.c      # Processo central de escalonamento
│   ├── runner.c          # Cliente: submete e executa comandos
│   ├── executer.c        # Execução de comandos com suporte a pipes e redirecionamentos
│   ├── scheduler.c       # Lógica das políticas de escalonamento (FCFS, RR, LIFO)
│   ├── queue.c           # Estrutura de fila de espera
│   ├── package.c         # Estrutura de mensagem (Package) e I/O persistente
│   ├── comandos.c        # Parsing de argumentos e tipos de comando
│   └── hashRR.c          # Hash table para Round-Robin por utilizador
├── include/
│   ├── package.h
│   ├── scheduler.h
│   ├── queue.h
│   ├── comandos.h
│   ├── executer.h
│   └── hashRR.h
├── testes/
│   ├── testes.sh                  # Script de testes funcionais (10 testes)
│   ├── run_scheduler_tests.sh     # Script de testes de políticas de escalonamento
│   └── limpar_testes.sh           # Limpeza de artefactos de teste
├── bin/                  # Binários compilados (gerado pelo make)
├── obj/                  # Ficheiros objeto (gerado pelo make)
├── tmp/                  # Ficheiros temporários
└── Makefile
```

---

## Dependências

- **GCC** (compilador C)
- **GLib 2.0** — utilizada para a hash table do escalonamento Round-Robin

Em Ubuntu/Debian:
```bash
sudo apt install build-essential libglib2.0-dev
```

Em Arch Linux:
```bash
sudo pacman -S base-devel glib2
```

---

## Compilação

```bash
cd projeto
make
```

Para limpar os binários e ficheiros objeto:
```bash
make clean
```

Os binários ficam em `bin/controller` e `bin/runner`.

---

## Utilização

### Iniciar o controller

```bash
./bin/controller <parallel-commands> <sched-policy>
```

| Argumento | Descrição |
|---|---|
| `parallel-commands` | Número máximo de comandos a executar em simultâneo |
| `sched-policy` | `1` = FCFS · `2` = Round-Robin · `3` = LIFO |

**Exemplo:**
```bash
./bin/controller 2 1    # Máximo 2 em paralelo, política FCFS
./bin/controller 4 2    # Máximo 4 em paralelo, política Round-Robin
```

---

### Executar um comando

```bash
./bin/runner -e <user-id> "<comando> <args...>"
```

**Exemplos:**
```bash
./bin/runner -e 1 "echo hello"
./bin/runner -e 2 "sleep 5"
./bin/runner -e 1 "grep root /etc/passwd | wc -l > out.txt"
./bin/runner -e 3 "wc -l < input.txt"
./bin/runner -e 1 "ls ficheiro_invalido 2> err.txt"
```

**Output esperado:**
```
[runner] command 12345 submitted
[runner] executing command 12345...
hello
[runner] command 12345 finished
```

---

### Consultar comandos em execução e em espera

```bash
./bin/runner -c
```

**Output esperado:**
```
---
Executing
user-id 1 - command-id 12345
---
Scheduled
user-id 2 - command-id 12346
user-id 3 - command-id 12347
```

---

### Terminar o controller

```bash
./bin/runner -s
```

O controller só termina após todos os runners ativos concluírem a sua execução.

---

## Políticas de Escalonamento

| ID | Nome | Descrição |
|---|---|---|
| `1` | FCFS | First-Come, First-Served — ordem de chegada |
| `2` | Round-Robin | Alternância entre utilizadores para garantir justiça |
| `3` | LIFO | Last-In, First-Out — o último pedido a chegar é o primeiro a executar |

---

## Mecanismos de Comunicação

| FIFO | Direção | Finalidade |
|---|---|---|
| `/tmp/runnerToController` | Runner → Controller | Todos os pedidos (REQUEST, QUERY, SHUTDOWN, DONE) |
| `/tmp/controllerToRunner_<PID>` | Controller → Runner | Autorização de execução (ACK) e respostas de consulta |

O controller cria um processo filho para cada autorização, mantendo o ciclo principal sempre disponível para receber novos pedidos.

---

## Registo Persistente

O controller regista em ficheiro binário (`infocomandos`) a informação de cada comando concluído:

- **user-id** — identificador do utilizador
- **command-id** — PID do runner que submeteu o comando
- **duração** — tempo total desde a submissão até à conclusão (em segundos)

---

## Operadores Suportados

O runner suporta os seguintes operadores na string de comando:

- `|` — pipe entre processos
- `>` — redirecionamento de stdout para ficheiro
- `<` — redirecionamento de stdin a partir de ficheiro
- `2>` — redirecionamento de stderr para ficheiro

---

## Testes

```bash
cd testes
bash testes.sh
```

Os resultados são guardados em `testes/outputs/`. O script cobre:

1. Execução básica
2. Consulta com fila de espera
3. Pipes
4. Redirecionamento `>`
5. Redirecionamento `<`
6. Redirecionamento `2>`
7. Pipe + redirecionamento combinados
8. Comparação de paralelismo (1, 2, 3 comandos simultâneos)
9. Comparação das 3 políticas de escalonamento
10. Teste de carga com 10 runners simultâneos

Para testes específicos às políticas de escalonamento:
```bash
bash run_scheduler_tests.sh
```

Para limpar artefactos:
```bash
bash limpar_testes.sh
```
