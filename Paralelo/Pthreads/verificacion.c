#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>

#define DATA_T double

DATA_T *A, *B;
int N ,numIteracion;

static const char* filenameA = "out.txt";
static const char* filenameB = "outB.txt";

FILE* fileA;

int main(int argc, char** argv) {
    N = atoi(argv[1]);
	A = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	B = (DATA_T*) malloc(sizeof(DATA_T)*N*N);

    FILE* fileA = fopen(filenameA, "rb");
    if (!fileA) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

	return 0;	
  }


void compararMatrices(){
    for (int i=0; i <N ; i++){
        for (int j=0;j<N;j++){
            if (A[i*N+j] =! B[i*N+j])
                printf("error en la iteracion %d, posicion i:%d, j:%d \n",numIteracion,i,j);
        }
    }
}

void leerMatrices(){
    
    fread(A , sizeof(DATA_T), N*N, fileA);
}