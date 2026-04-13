#!/bin/bash

echo "=== TESTE 6: SHUTDOWN ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*

../bin/controller 1 1 &
CONTROLLER_PID=$!

sleep 1

../bin/runner -e 1 sleep 5 &

sleep 1

../bin/runner -s &

wait
wait $CONTROLLER_PID

echo "=== FIM TESTE 6 ==="