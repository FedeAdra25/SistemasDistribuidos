#!/usr/bin/env bash
#Este script genera los archivos compilados de vector.c y matriz.c
debug=${1:-"-DDEBUG -DPRINT_MATRIZ -DCONVERGE"} # -DPRINT_VEC -DPRUEBA -DPRINT_MATRIZ -DCONVERGE

# Guardamos el directorio del script
SCRIPT_DIR=$(dirname $0)
echo "El script está en: $SCRIPT_DIR"
echo "Todos los argumentos = $@"
cd $SCRIPT_DIR

mpicc $debug -o out/vector2.o vecPasoEsquinas.c
mpicc $debug -o out/vector.o vector_triple.c

#comando que estoy usando ahora mismo
#mpirun -v -output-filename out/ -np 4 out/vector.o 32

mpicc $debug -o out/matriz.o matriz_fede.c
#mpicc $debug -o out/matriz.o matriz.c
mpirun -v -output-filename out/ -np 4 out/matriz.o 16
#mpirun -np 2 vector.o 16