#!/usr/bin/env bash
# debug=${1:-"-DPRINT_MATRIZ -DPRINT_OPERACION"}  -DPRUEBA_COL0 -DPRUEBA -DDEBUG_POR_ITERACION -DPRUEBA
debug=${1:-""} #debug = $1==null ? "" : $1

# Guardamos el directorio del script
SCRIPT_DIR=$(dirname $0)
echo "El script está en: $SCRIPT_DIR"
echo "Todos los argumentos = $@"
cd $SCRIPT_DIR
#echo $(ls -aF $SCRIPT_DIR)

#Este script genera los archivos compilados de vector.c y matriz.c
gcc -pthread $debug -o ./out/matriz.o ./matriz.c
gcc -pthread $debug -o ./out/vector.o ./vector.c

if [ $? -eq 0 ]; then
    echo "Compilación correcta"
else
    echo "Compilación incorrecta"
    exit 1
fi
#borro el out/results.txt
rm ./out/results.txt

for i in "$@"
do
    if [ "$i" = "-test" ]; then
        echo "Se ejecutará con N=32 y N=16 y T=2"
        echo "Ejecutando matriz.o"
        ./out/matriz.o 32 2 "-------Matriz 32-------">> ./out/results.txt
        ./out/matriz.o 16 2 "-------Matriz 16-------">> ./out/results.txt
        echo "Ejecutando vector.o"
        ./out/vector.o 32 2 "-------Vector 32-------">> ./out/results.txt
        ./out/vector.o 16 2 "-------Vector 16-------">> ./out/results.txt
        exit 0
    fi
done

#Matriz
echo "Algoritmo Paralelo" > ./out/results.txt
echo "Matriz 4 threads" >> ./out/results.txt
./out/matriz.o 512 4 "Matriz de 512x512 T=4" >> ./out/results.txt
./out/matriz.o 1024 4 "Matriz de 1024x1024 T=4">> ./out/results.txt
./out/matriz.o 2048 4 "Matriz de 2048x2048 T=4">> ./out/results.txt
echo "Matriz 8 threads" >> ./out/results.txt
./out/matriz.o 512 8 "Matriz de 512x512 T=8" >> ./out/results.txt
./out/matriz.o 1024 8 "Matriz de 1024x1024 T=8">> ./out/results.txt
./out/matriz.o 2048 8 "Matriz de 2048x2048 T=8">> ./out/results.txt

#Vector
echo "=======" >> ./out/results.txt
echo "Vector 4 threads" >> ./out/results.txt
./out/vector.o 512 4 "Vector de 512 T=4">> ./out/results.txt
./out/vector.o 1024 4 "Vector de 1024 T=4">> ./out/results.txt
./out/vector.o 2048 4 "Vector de 2048 T=4">> ./out/results.txt
echo "Vector 8 threads" >> ./out/results.txt
./out/vector.o 512 8 "Vector de 512 T=8">> ./out/results.txt
./out/vector.o 1024 8 "Vector de 1024 T=8">> ./out/results.txt
./out/vector.o 2048 8 "Vector de 2048 T=8">> ./out/results.txt