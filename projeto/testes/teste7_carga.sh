#!/bin/bash

echo "=== TESTE 7: CARGA ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho* infocomandos

../bin/controller 3 1 &
CONTROLLER_PID=$!

sleep 1

PIDS=""
for i in 1 2 3 4 5 6 7 8
do
    ../bin/runner -e $i sleep 2 &
    PIDS="$PIDS $!"
done

wait $PIDS

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 7 ==="