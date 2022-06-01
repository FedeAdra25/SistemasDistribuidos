#!/usr/bin/env bash
#Este script genera los archivos compilados de vector.c y matriz.c
debug=${1:-""} # -DPRINT_VEC -DPRUEBA -DPRINT_MATRIZ -DCONVERGE
                                                # -DPROCE_SLAVE
# Guardamos el directorio del script
SCRIPT_DIR=$(dirname $0)
echo "El script está en: $SCRIPT_DIR"
echo "Todos los argumentos = $@"
cd $SCRIPT_DIR

# mpicc $debug -o out/vector2.o vecPasoEsquinas.c
mpicc $debug -o out/vector.o vector.c

#comando que estoy usando ahora mismo
#mpirun -v -output-filename out/ -np 4 out/vector.o 512

mpicc $debug -o out/matriz.o matriz.c
# mpicc $debug -o out/matriz.o matriz.c
# mpirun -v -output-filename out/ -np 4 out/matriz.o 1024
#mpirun -np 2 vector.o 16

#Matriz
echo "Algoritmo Paralelo" > ./out/results.txt
echo "Matriz" >> ./out/results.txt
mpirun -v -output-filename out/ -np 4 ./out/matriz.o 512 "Matriz de 512x512" >> ./out/results.txt
mpirun -v -output-filename out/ -np 4 ./out/matriz.o 1024 "Matriz de 1024x1024">> ./out/results.txt
#mpirun -v -output-filename out/ -np 4 ./out/matriz.o 2048 "Matriz de 2048x2048">> ./out/results.txt

# #Vector
echo "=======" >> ./out/results.txt
echo "Vector" >> ./out/results.txt
mpirun -v -output-filename out/ -np 4 ./out/vector.o 512 "Vector de 512">> ./out/results.txt
mpirun -v -output-filename out/ -np 4 ./out/vector.o 1024 "Vector de 1024">> ./out/results.txt
mpirun -v -output-filename out/ -np 4 ./out/vector.o 2048 "Vector de 2048">> ./out/results.txt