#!/usr/bin/env bash
#Este script genera los archivos compilados de vector.c y matriz.c
debug=${1:-""} # -DPRINT_VEC -DPRUEBA

# Guardamos el directorio del script
SCRIPT_DIR=$(dirname $0)
echo "El script está en: $SCRIPT_DIR"
echo "Todos los argumentos = $@"
cd $SCRIPT_DIR

mpicc $debug -o out/vector2.o vecPasoEsquinas.c
mpicc $debug -o out/vector.o vector_triple.c

#comando que estoy usando ahora mismo
mpirun -v -output-filename out/ -np 4 out/vector.o 32 > out/result.txt 

#mpirun -np 2 vector.o 16