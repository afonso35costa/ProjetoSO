#!/bin/bash

echo "=== TESTE 2: FILA DE ESPERA ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*

../bin/controller 1 1 &
CONTROLLER_PID=$!

sleep 1

../bin/runner -e 1 sleep 5 &
../bin/runner -e 2 echo rapido &
../bin/runner -e 3 sleep 2 &

wait

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 2 ==="