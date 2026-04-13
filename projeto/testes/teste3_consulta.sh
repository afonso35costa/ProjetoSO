#!/bin/bash

echo "=== TESTE 3: CONSULTA ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho* infocomandos

../bin/controller 1 1 &
CONTROLLER_PID=$!

sleep 1

../bin/runner -e 1 sleep 5 &
R1=$!
../bin/runner -e 2 sleep 5 &
R2=$!
../bin/runner -e 3 echo ola &
R3=$!

sleep 1

echo "----- CONSULTA -----"
../bin/runner -c
echo "--------------------"

wait $R1 $R2 $R3

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 3 ==="