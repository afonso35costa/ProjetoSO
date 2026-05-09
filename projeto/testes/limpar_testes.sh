#!/bin/bash
# Limpa FIFOs, processos e resultados dos testes.

pkill controller 2>/dev/null || true
pkill runner 2>/dev/null || true

rm -f /tmp/runnerToController
rm -f /tmp/controllerToRunner_*
rm -f /tmp/runnerTofilho*
rm -f infocomandos
rm -rf resultados_escalonamento

echo "Ambiente de testes limpo."
