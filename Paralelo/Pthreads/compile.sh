#Este script genera los archivos compilados de vector.c y matriz.c
gcc -pthread -o ./out/vector.o -DDEBUG2 ./vector.c 