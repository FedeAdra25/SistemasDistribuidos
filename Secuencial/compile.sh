debug=${1:-""}

#Este script genera los archivos compilados de vector.c y matriz.c
gcc $debug -o ./out/matriz.o ./matriz.c
gcc $debug -o ./out/vector.o ./vector.c

#Matriz
./out/matriz.o 512 "Matriz size 512"> ./out/results.txt
./out/matriz.o 1024 "Matriz size 1024">> ./out/results.txt
./out/matriz.o 2048 "Matriz size 2048">> ./out/results.txt

#Vector
./out/vector.o 512 "Vector size 512">> ./out/results.txt
./out/vector.o 1024 "Vector size 1024">> ./out/results.txt
./out/vector.o 2048 "Vector size 2048">> ./out/results.txt