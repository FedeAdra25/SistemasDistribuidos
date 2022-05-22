#Este script genera los archivos compilados de vector.c y matriz.c
gcc -O3 -o ./out/matriz.o  ./matriz.c
gcc -o ./out/vector.o   ./vector.c