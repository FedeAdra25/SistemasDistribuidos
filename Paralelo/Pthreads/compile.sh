# debug=${1:-"-DPRINT_MATRIZ -DPRINT_OPERACION"}  -DPRUEBA_COL0 -DPRUEBA
debug=${1:-"-DDEBUG_POR_ITERACION -DPRUEBA"}


#Este script genera los archivos compilados de vector.c y matriz.c
gcc -pthread $debug -o ./out/matriz.o ./matriz.c
gcc -pthread $debug -o ./out/vector.o ./vector.c


#Pruebas actuales
gcc -pthread $debug -o ./out/matriz.o ./matriz.c
#gcc -pthread -o ./out/matriz_prueba.o ./matriz_prueba.c
./out/matriz.o 32 2 "Matriz 8x4" > out/results.txt
# ./out/matriz.o 8 4 "Matriz 8x4" > out/results4.txt
# ./out/matriz.o 8 2 "Matriz 8x2" > out/results2.txt

#(descomentar cuando este andando)
#Matriz
# ./out/matriz.o 512 2 "Matriz size 512 2hilos"> ./out/results.txt
# ./out/matriz.o 1024 2 "Matriz size 1024 2 hilos">> ./out/results.txt
# ./out/matriz.o 1024 4 "Matriz size 1024 4 hilos">> ./out/results.txt
#./out/matriz.o 2048 "Matriz size 2048">> ./out/results.txt

#Vector
#./out/vector.o 512 "Vector size 512">> ./out/results.txt
#./out/vector.o 1024 "Vector size 1024">> ./out/results.txt
#./out/vector.o 2048 "Vector size 2048">> ./out/results.txt