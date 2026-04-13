#!/bin/bash

echo "=== TESTE 5: VÁRIOS RUNNERS ==="

rm -f /tmp/runnerToController /tmp/controllerToRunner_* /tmp/runnerTofilho* infocomandos

../bin/controller 2 1 &
CONTROLLER_PID=$!

sleep 1

../bin/runner -e 1 echo user1 &
R1=$!
../bin/runner -e 2 sleep 2 &
R2=$!
../bin/runner -e 3 echo user3 &
R3=$!
../bin/runner -e 4 sleep 1 &
R4=$!
../bin/runner -e 5 echo user5 &
R5=$!

wait $R1 $R2 $R3 $R4 $R5

../bin/runner -s
wait $CONTROLLER_PID

echo "=== FIM TESTE 5 ==="