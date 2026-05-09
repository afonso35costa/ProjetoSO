#!/bin/bash

CONTROLLER="${1:-../bin/controller}"
RUNNER="${2:-../bin/runner}"

RESULTS_DIR="resultados_escalonamento"
INFOFILE="infocomandos"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
RED='\033[0;31m'
BOLD='\033[1m'
NC='\033[0m'

mkdir -p "$RESULTS_DIR"

check_binaries() {
    if [ ! -x "$CONTROLLER" ]; then
        echo -e "${RED}ERRO: controller não encontrado em $CONTROLLER${NC}"
        exit 1
    fi

    if [ ! -x "$RUNNER" ]; then
        echo -e "${RED}ERRO: runner não encontrado em $RUNNER${NC}"
        exit 1
    fi
}

policy_name() {
    case "$1" in
        1) echo "FCFS" ;;
        2) echo "RR" ;;
        3) echo "LIFO" ;;
        *) echo "UNKNOWN" ;;
    esac
}

clean_env() {
    pkill -x controller 2>/dev/null
    pkill -x runner 2>/dev/null

    rm -f "$INFOFILE"
    rm -f /tmp/runnerToController
    rm -f /tmp/controllerToRunner_*
    rm -f /tmp/runnerTofilho*
}

start_controller() {
    local parallel=$1
    local policy=$2
    local pname
    pname=$(policy_name "$policy")

    clean_env

    "$CONTROLLER" "$parallel" "$policy" > "$RESULTS_DIR/controller_${pname}_par${parallel}.log" 2>&1 &
    CTRL_PID=$!

    sleep 1
}

stop_controller() {
    timeout 3 "$RUNNER" -s 2>/dev/null

    local count=0

    while kill -0 "$CTRL_PID" 2>/dev/null; do
        sleep 0.2
        count=$((count + 1))

        if [ "$count" -ge 15 ]; then
            echo -e "${YELLOW}[WARN] controller bloqueado -> kill -9${NC}"
            kill -9 "$CTRL_PID" 2>/dev/null
            break
        fi
    done

    wait "$CTRL_PID" 2>/dev/null

    pkill -x controller 2>/dev/null
    pkill -x runner 2>/dev/null

    rm -f /tmp/runnerToController
    rm -f /tmp/controllerToRunner_*
    rm -f /tmp/runnerTofilho*

    sleep 0.5
}

wait_runners() {
    local max_wait=25
    local elapsed=0
    local still_running=1

    while [ "$elapsed" -lt "$max_wait" ]; do
        still_running=0

        for pid in "$@"; do
            if kill -0 "$pid" 2>/dev/null; then
                still_running=1
            fi
        done

        if [ "$still_running" -eq 0 ]; then
            break
        fi

        sleep 1
        elapsed=$((elapsed + 1))
    done

    if [ "$still_running" -eq 1 ]; then
        echo -e "${YELLOW}[WARN] runners bloqueados -> kill${NC}"
        for pid in "$@"; do
            kill -9 "$pid" 2>/dev/null
        done
    fi

    for pid in "$@"; do
        wait "$pid" 2>/dev/null
    done
}

save_infofile() {
    local name=$1

    if [ -f "$INFOFILE" ]; then
        cp "$INFOFILE" "$RESULTS_DIR/${name}_infocomandos.bin"
    else
        echo "[WARN] sem infocomandos para $name" >> "$RESULTS_DIR/avisos.log"
    fi
}

consulta() {
    local logfile=$1
    timeout 3 "$RUNNER" -c >> "$logfile" 2>&1
}

run_test_fairness() {
    local policy=$1
    local pname
    pname=$(policy_name "$policy")

    echo -e "${CYAN}[01] Fairness carga igual — $pname${NC}"

    start_controller 3 "$policy"

    PIDS=()

    for user in 1 2 3; do
        for i in 1 2 3 4; do
            timeout 10 "$RUNNER" -e "$user" "sleep 1" >> "$RESULTS_DIR/fairness_${pname}.log" 2>&1 &
            PIDS+=($!)
            sleep 0.05
        done
    done

    wait_runners "${PIDS[@]}"
    sleep 1
    consulta "$RESULTS_DIR/fairness_${pname}.log"

    stop_controller
    save_infofile "fairness_${pname}"
}

run_test_convoy() {
    local policy=$1
    local pname
    pname=$(policy_name "$policy")

    echo -e "${CYAN}[02] Convoy effect — $pname${NC}"

    start_controller 1 "$policy"

    PIDS=()

    timeout 15 "$RUNNER" -e 1 "sleep 5" >> "$RESULTS_DIR/convoy_${pname}.log" 2>&1 &
    PIDS+=($!)

    sleep 0.2

    for user in 2 3 4 5; do
        timeout 10 "$RUNNER" -e "$user" "echo curto_$user" >> "$RESULTS_DIR/convoy_${pname}.log" 2>&1 &
        PIDS+=($!)
        sleep 0.1
    done

    sleep 1
    consulta "$RESULTS_DIR/convoy_${pname}.log"

    wait_runners "${PIDS[@]}"
    sleep 1

    stop_controller
    save_infofile "convoy_${pname}"
}

run_test_starvation() {
    local policy=$1
    local pname
    pname=$(policy_name "$policy")

    echo -e "${CYAN}[03] Starvation heavy vs light — $pname${NC}"

    start_controller 2 "$policy"

    PIDS=()

    for i in 1 2 3 4 5 6 7 8; do
        timeout 15 "$RUNNER" -e 1 "sleep 1" >> "$RESULTS_DIR/starvation_${pname}.log" 2>&1 &
        PIDS+=($!)
        sleep 0.03
    done

    sleep 0.2

    for i in 1 2; do
        timeout 15 "$RUNNER" -e 2 "sleep 1" >> "$RESULTS_DIR/starvation_${pname}.log" 2>&1 &
        PIDS+=($!)
        sleep 0.03
    done

    sleep 1
    consulta "$RESULTS_DIR/starvation_${pname}.log"

    wait_runners "${PIDS[@]}"
    sleep 1

    stop_controller
    save_infofile "starvation_${pname}"
}

run_test_order() {
    local policy=$1
    local pname
    pname=$(policy_name "$policy")

    echo -e "${CYAN}[04] Ordem de execução — $pname${NC}"

    start_controller 1 "$policy"

    PIDS=()

    for i in 1 2 3 4 5; do
        timeout 10 "$RUNNER" -e 1 "echo job_$i" >> "$RESULTS_DIR/order_${pname}.log" 2>&1 &
        PIDS+=($!)
        sleep 0.2
    done

    sleep 1
    consulta "$RESULTS_DIR/order_${pname}.log"

    wait_runners "${PIDS[@]}"
    sleep 1

    stop_controller
    save_infofile "order_${pname}"
}

run_test_throughput() {
    local policy=$1
    local pname
    pname=$(policy_name "$policy")

    echo -e "${CYAN}[05] Throughput jobs curtos — $pname${NC}"

    start_controller 3 "$policy"

    PIDS=()

    START=$(date +%s)

    for i in 1 2 3 4 5 6 7 8 9 10 11 12; do
        user=$(( (i % 3) + 1 ))

        timeout 15 "$RUNNER" -e "$user" "sleep 0.5" >> "$RESULTS_DIR/throughput_${pname}.log" 2>&1 &
        PIDS+=($!)

        sleep 0.03
    done

    wait_runners "${PIDS[@]}"

    END=$(date +%s)
    TOTAL=$((END - START))

    echo "Tempo total throughput $pname: ${TOTAL}s" >> "$RESULTS_DIR/throughput_${pname}.log"
    echo "$TOTAL" > "$RESULTS_DIR/throughput_${pname}_walltime.txt"

    sleep 1

    stop_controller
    save_infofile "throughput_${pname}"
}

run_test_parallelism() {
    local policy=1
    local pname="FCFS"

    echo ""
    echo -e "${YELLOW}============== TESTE DE PARALELISMO ==============${NC}"
    echo -e "${CYAN}[06] Impacto do grau de paralelismo — FCFS${NC}"

    echo "parallel_commands,n_runners,n_comandos,tempo_total_s,throughput_cmd_s" > "$RESULTS_DIR/parallelism_results.csv"

    for parallel in 1 2 4 8; do
        echo -e "${CYAN}     parallel-commands = $parallel${NC}"

        start_controller "$parallel" "$policy"

        PIDS=()

        START=$(date +%s)

        for i in $(seq 1 16); do
            user=$(( (i % 4) + 1 ))

            timeout 20 "$RUNNER" -e "$user" "sleep 1" >> "$RESULTS_DIR/parallelism_p${parallel}.log" 2>&1 &
            PIDS+=($!)

            sleep 0.03
        done

        wait_runners "${PIDS[@]}"

        END=$(date +%s)
        TOTAL=$((END - START))

        if [ "$TOTAL" -eq 0 ]; then
            THROUGHPUT="16"
        else
            THROUGHPUT=$(echo "scale=2; 16 / $TOTAL" | bc 2>/dev/null)
            if [ -z "$THROUGHPUT" ]; then
                THROUGHPUT="N/A"
            fi
        fi

        echo "$parallel,4,16,$TOTAL,$THROUGHPUT" >> "$RESULTS_DIR/parallelism_results.csv"
        echo "parallel=$parallel tempo_total=${TOTAL}s throughput=${THROUGHPUT} cmd/s" >> "$RESULTS_DIR/parallelism_summary.log"

        sleep 1

        stop_controller
        save_infofile "parallelism_p${parallel}_${pname}"
    done
}

run_policy_tests() {
    local policy=$1
    local pname
    pname=$(policy_name "$policy")

    echo ""
    echo -e "${YELLOW}============== POLÍTICA $pname ==============${NC}"

    run_test_fairness "$policy"
    run_test_convoy "$policy"
    run_test_starvation "$policy"
    run_test_order "$policy"
    run_test_throughput "$policy"
}

main() {
    check_binaries
    clean_env

    echo -e "${GREEN}${BOLD}"
    echo "============================================================"
    echo " TESTES DE ESCALONAMENTO — FCFS vs RR vs LIFO"
    echo "============================================================"
    echo -e "${NC}"

    echo "Controller : $CONTROLLER"
    echo "Runner     : $RUNNER"
    echo "Resultados : $RESULTS_DIR"
    echo ""

    run_policy_tests 1
    run_policy_tests 2
    run_policy_tests 3

    run_test_parallelism

    clean_env

    echo ""
    echo -e "${GREEN}${BOLD}Testes terminados.${NC}"
    echo "Resultados guardados em: $RESULTS_DIR"
    echo ""
    echo "Para a secção 6.4 usa:"
    echo "  $RESULTS_DIR/parallelism_results.csv"
    echo "  $RESULTS_DIR/parallelism_summary.log"
}

main