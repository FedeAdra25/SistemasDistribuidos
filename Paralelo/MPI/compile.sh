#!/usr/bin/env bash
#Este script genera los archivos compilados de vector.c y matriz.c
debug=${1:-""} # -DPRINT_VEC -DPRUEBA

# Guardamos el directorio del script
SCRIPT_DIR=$(dirname $0)
echo "El script está en: $SCRIPT_DIR"
echo "Todos los argumentos = $@"
cd $SCRIPT_DIR

mpicc $debug -o vector.o vecPasoEsquinas.c

#mpirun -np 2 vector.o 16