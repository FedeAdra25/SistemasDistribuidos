#!/usr/bin/env bash
#-DDEBUG_POR_ITERACION
debug=${1:-""} #debug = $1==null ? "" : $1

# Guardamos el directorio del script
SCRIPT_DIR=$(dirname $0)
echo "El script está en: $SCRIPT_DIR"
echo "Todos los argumentos = $@"
cd $SCRIPT_DIR
#echo $(ls -aF $SCRIPT_DIR)

#Este script genera los archivos compilados de vector.c y matriz.c
gcc $debug -o ./out/matriz.o ./matriz.c
gcc $debug -o ./out/vector.o ./vector.c

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
        echo "Se ejecutará con N=32 y N=16"
        echo "Ejecutando matriz.o"
        ./out/matriz.o 32 "Matriz 32"> ./out/results_M32.txt
        ./out/matriz.o 16 "Matriz 16"> ./out/results_M16.txt
        echo "Ejecutando vector.o"
        ./out/vector.o 32 "Vector 32"> ./out/results_V32.txt
        ./out/vector.o 16 "Vector 16"> ./out/results_V16.txt
        exit 0
    fi
done

#Matriz
echo "Algoritmo secuencial" > ./out/results.txt
echo "Matriz" >> ./out/results.txt
./out/matriz.o 512 "Matriz de 512x512" >> ./out/results.txt
./out/matriz.o 1024 "Matriz de 1024x1024">> ./out/results.txt
./out/matriz.o 2048 "Matriz de 2048x2048">> ./out/results.txt

# #Vector
echo "=======" >> ./out/results.txt
echo "Vector" >> ./out/results.txt
./out/vector.o 512 "Vector de 512">> ./out/results.txt
./out/vector.o 1024 "Vector de 1024">> ./out/results.txt
./out/vector.o 2048 "Vector de 2048">> ./out/results.txt