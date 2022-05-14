#Este script genera los archivos compilados de vector.c y matriz.c
gcc -o ./out/matriz.o -DDEBUG matriz.c
gcc -o ./out/vector.o vector.c