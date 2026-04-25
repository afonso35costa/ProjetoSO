#!/bin/bash

echo "=== TESTE 1: EXECUÇÃO BÁSICA ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*

../bin/controller 1 1 &
CONTROLLER_PID=$!

sleep 1

../bin/runner -e 1 "echo ola"

sleep 1

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 1 ==="