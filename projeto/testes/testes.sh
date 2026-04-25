#!/bin/bash

RUNNER="../bin/runner"
CONTROLLER="../bin/controller"
OUTDIR="outputs"

mkdir -p "$OUTDIR"

clean_env() {
    rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*
    rm -f infocomandos out.txt err.txt input.txt
}

start_controller() {
    PAR=$1
    POLICY=$2
    echo "A iniciar controller: paralelismo=$PAR politica=$POLICY"
    $CONTROLLER "$PAR" "$POLICY" > "$OUTDIR/controller_p${PAR}_pol${POLICY}.log" 2>&1 &
    CONTROLLER_PID=$!
    sleep 1
}

stop_controller() {
    echo "A terminar controller..."
    $RUNNER -s >> "$OUTDIR/shutdown.log" 2>&1
    wait $CONTROLLER_PID 2>/dev/null
}

wait_runners() {
    for pid in "$@"; do
        wait "$pid"
    done
}

echo "=== TESTES GRANDES DO PROJETO SO ==="

make -C .. clean
make -C ..

# -------------------------------------------------
# TESTE 1 — Execução básica
# -------------------------------------------------
clean_env
echo "=== TESTE 1: Execução básica ===" | tee "$OUTDIR/teste1.log"
start_controller 1 1

$RUNNER -e 1 "echo ola" | tee -a "$OUTDIR/teste1.log"

stop_controller

# -------------------------------------------------
# TESTE 2 — Consulta com execução e fila
# -------------------------------------------------
clean_env
echo "=== TESTE 2: Consulta -c com fila ===" | tee "$OUTDIR/teste2.log"
start_controller 1 1

$RUNNER -e 1 "sleep 5" >> "$OUTDIR/teste2.log" 2>&1 &
R1=$!
$RUNNER -e 2 "sleep 5" >> "$OUTDIR/teste2.log" 2>&1 &
R2=$!
$RUNNER -e 3 "echo terceiro" >> "$OUTDIR/teste2.log" 2>&1 &
R3=$!

sleep 1
echo "--- CONSULTA ---" | tee -a "$OUTDIR/teste2.log"
$RUNNER -c | tee -a "$OUTDIR/teste2.log"

wait_runners $R1 $R2 $R3
stop_controller

# -------------------------------------------------
# TESTE 3 — Pipes
# -------------------------------------------------
clean_env
echo "=== TESTE 3: Pipe | ===" | tee "$OUTDIR/teste3.log"
start_controller 1 1

$RUNNER -e 1 "echo ola mundo | wc -w" | tee -a "$OUTDIR/teste3.log"

stop_controller

# -------------------------------------------------
# TESTE 4 — Redirecionamento stdout >
# -------------------------------------------------
clean_env
echo "=== TESTE 4: Redirecionamento > ===" | tee "$OUTDIR/teste4.log"
start_controller 1 1

$RUNNER -e 1 "echo teste_redir > out.txt" | tee -a "$OUTDIR/teste4.log"

echo "--- conteúdo de out.txt ---" | tee -a "$OUTDIR/teste4.log"
cat out.txt | tee -a "$OUTDIR/teste4.log"

stop_controller

# -------------------------------------------------
# TESTE 5 — Redirecionamento stdin <
# -------------------------------------------------
clean_env
echo "linha1" > input.txt
echo "linha2" >> input.txt
echo "linha3" >> input.txt

echo "=== TESTE 5: Redirecionamento < ===" | tee "$OUTDIR/teste5.log"
start_controller 1 1

$RUNNER -e 1 "wc -l < input.txt" | tee -a "$OUTDIR/teste5.log"

stop_controller

# -------------------------------------------------
# TESTE 6 — Redirecionamento stderr 2>
# -------------------------------------------------
clean_env
echo "=== TESTE 6: Redirecionamento 2> ===" | tee "$OUTDIR/teste6.log"
start_controller 1 1

$RUNNER -e 1 "ls ficheiro_que_nao_existe 2> err.txt" | tee -a "$OUTDIR/teste6.log"

echo "--- conteúdo de err.txt ---" | tee -a "$OUTDIR/teste6.log"
cat err.txt | tee -a "$OUTDIR/teste6.log"

stop_controller

# -------------------------------------------------
# TESTE 7 — Pipe + redirecionamento
# -------------------------------------------------
clean_env
echo "=== TESTE 7: Pipe + > ===" | tee "$OUTDIR/teste7.log"
start_controller 1 1

$RUNNER -e 1 "grep root /etc/passwd | wc -l > out.txt" | tee -a "$OUTDIR/teste7.log"

echo "--- conteúdo de out.txt ---" | tee -a "$OUTDIR/teste7.log"
cat out.txt | tee -a "$OUTDIR/teste7.log"

stop_controller

# -------------------------------------------------
# TESTE 8 — Paralelismo 1 vs 2 vs 3
# -------------------------------------------------
for PAR in 1 2 3
do
    clean_env
    echo "=== TESTE 8: Paralelismo $PAR ===" | tee "$OUTDIR/teste8_par${PAR}.log"
    start_controller "$PAR" 1

    START=$(date +%s)

    $RUNNER -e 1 "sleep 4" >> "$OUTDIR/teste8_par${PAR}.log" 2>&1 &
    R1=$!
    $RUNNER -e 2 "sleep 4" >> "$OUTDIR/teste8_par${PAR}.log" 2>&1 &
    R2=$!
    $RUNNER -e 3 "sleep 4" >> "$OUTDIR/teste8_par${PAR}.log" 2>&1 &
    R3=$!
    $RUNNER -e 4 "sleep 4" >> "$OUTDIR/teste8_par${PAR}.log" 2>&1 &
    R4=$!

    wait_runners $R1 $R2 $R3 $R4

    END=$(date +%s)
    echo "Tempo total: $((END-START)) segundos" | tee -a "$OUTDIR/teste8_par${PAR}.log"

    stop_controller
done

# -------------------------------------------------
# TESTE 9 — Políticas: FCFS, RR, LIFO
# 1 = FCFS, 2 = RR, 3 = LIFO
# -------------------------------------------------
# for POLICY in 1 2 3
# do
#     clean_env
#     echo "=== TESTE 9: Política $POLICY ===" | tee "$OUTDIR/teste9_policy${POLICY}.log"
#     start_controller 1 "$POLICY"

#     $RUNNER -e 1 "sleep 3" >> "$OUTDIR/teste9_policy${POLICY}.log" 2>&1 &
#     R1=$!
#     sleep 0.2
#     $RUNNER -e 2 "echo user2" >> "$OUTDIR/teste9_policy${POLICY}.log" 2>&1 &
#     R2=$!
#     sleep 0.2
#     $RUNNER -e 1 "echo user1_segundo" >> "$OUTDIR/teste9_policy${POLICY}.log" 2>&1 &
#     R3=$!
#     sleep 0.2
#     $RUNNER -e 3 "echo user3" >> "$OUTDIR/teste9_policy${POLICY}.log" 2>&1 &
#     R4=$!

#     sleep 1
#     echo "--- CONSULTA DURANTE POLÍTICA $POLICY ---" | tee -a "$OUTDIR/teste9_policy${POLICY}.log"
#     $RUNNER -c | tee -a "$OUTDIR/teste9_policy${POLICY}.log"

#     wait_runners $R1 $R2 $R3 $R4
#     stop_controller
# done

# -------------------------------------------------
# TESTE 10 — Carga grande
# -------------------------------------------------
clean_env
echo "=== TESTE 10: Carga grande ===" | tee "$OUTDIR/teste10.log"
start_controller 3 2

PIDS=()
for i in 1 2 3 4 5 6 7 8 9 10
do
    USER=$(( (i % 3) + 1 ))
    $RUNNER -e "$USER" "sleep 2" >> "$OUTDIR/teste10.log" 2>&1 &
    PIDS+=($!)
done

sleep 1
echo "--- CONSULTA DURANTE CARGA ---" | tee -a "$OUTDIR/teste10.log"
$RUNNER -c | tee -a "$OUTDIR/teste10.log"

wait_runners "${PIDS[@]}"
stop_controller

echo "=== TODOS OS TESTES TERMINARAM ==="
echo "Outputs guardados na pasta: $OUTDIR"