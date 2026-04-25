#!/bin/bash

echo "=== TESTE 2: FILA DE ESPERA ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*

../bin/controller 1 1 &
CONTROLLER_PID=$!

sleep 1

../bin/runner -e 1 "sleep 5" &
R1=$!

../bin/runner -e 2 "echo rapido" &
R2=$!

../bin/runner -e 3 "sleep 2" &
R3=$!

wait $R1 $R2 $R3

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 2 ==="