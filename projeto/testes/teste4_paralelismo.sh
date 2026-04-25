#!/bin/bash

echo "=== TESTE 4: PARALELISMO ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*

echo "---- Paralelismo = 1 ----"
../bin/controller 1 1 &
CONTROLLER_PID=$!

sleep 1

time (
    ../bin/runner -e 1 "sleep 4" &
    ../bin/runner -e 2 "sleep 4" &
    ../bin/runner -e 3 "sleep 4" &
    wait
)

../bin/runner -s
wait $CONTROLLER_PID

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho*

echo "---- Paralelismo = 2 ----"
../bin/controller 2 1 &
CONTROLLER_PID=$!

sleep 1

time (
    ../bin/runner -e 1 "sleep 4" &
    ../bin/runner -e 2 "sleep 4" &
    ../bin/runner -e 3 "sleep 4" &
    wait
)

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 4 ==="