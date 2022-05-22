#Este script genera los archivos compilados de vector.c y matriz.c
gcc -o ./out/matriz.o ./matriz.c
gcc -o ./out/vector.o ./vector.c

./out/matriz.o 1024