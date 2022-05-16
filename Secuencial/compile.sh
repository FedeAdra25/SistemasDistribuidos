#Este script genera los archivos compilados de vector.c y matriz.c
gcc -o ./out/matriz.o -DDEBUG2 ./matriz_bloques.c
gcc -o ./out/vector.o -DDEBUG2 ./vector.c