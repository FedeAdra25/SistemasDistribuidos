#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>

#define DATA_T double
#define precision (1.0/100)


//Para calcular tiempo
double dwalltime();
//para generar nro random entre min y max
DATA_T randFP(DATA_T min, DATA_T max);
void printVector(int N, DATA_T *M);


int main(int argc, char** argv) {
	int N = atoi(argv[1]);
	double timetick;

	DATA_T *A = (DATA_T*) malloc(sizeof(DATA_T)*N);
	DATA_T *B = (DATA_T*) malloc(sizeof(DATA_T)*N);
	DATA_T *swapAux;

	if(argc<2){
        printf("Error: debe enviar el tamaño de la matriz. \nRecibido %d argumentos\n",argc);
        printf("Forma: ./out.o N\n");
        return 2;
    }
    N = atoi(argv[1]);
    if(N<8){
        printf("N debe ser mayor a 8 (N=%d)",N);
        return 0;
    }
    if(argc>2){
        printf("%s\n",argv[2]);
    }

	//Inicialización
	int i;
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
	}
	#ifdef DEBUG
	printVector(N,A);
	#endif

	//Cálculo del promedio
	int converge=0,numIteracion= 0;

	timetick = dwalltime();
	while (!converge){
		numIteracion++;
		B[0] = (A[0]+A[1]) * 0.5;

		converge = 1;

		//Calcula el elemento i del resultado. Si le da que no converge actualiza la variable
		for (i=1;i<N-1;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (1.0/3);
			if (fabs(B[0]-B[i])>precision){
				converge = 0;
				break;
			}
		}
		//Calcula el resto de los elementos en caso de haber entrado al break
		for (;i<N-1;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (1.0/3);
		}
		B[N-1] = (A[N-2]+A[N-1]) *0.5;

		//Calcula la última posición para ver si converge
		if (converge && fabs(B[0]-B[N-1])>precision){
			converge = 0;
		}
		//Verifico si converge hago el swap
		if (!converge){
				swapAux = A;
				A = B;
				B = swapAux;
		}
		#ifdef DEBUG
		if(numIteracion<20 || numIteracion%1000==0 || numIteracion>377470){
			//Imprimo  calculo
			printf("Iteracion: %d\n",numIteracion);
			printVector(N,A);				
			printf("\n");
		}
		#endif
		
	}
	/**
	 * TODO: Agregar tests
	 * */
	
	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);
	printf("-------------------\n");
	free(A);
	free(B);
	return(0);	
  }



DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}

double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}
void printVector(int N, DATA_T *M){
	printf("Vector = [");
	for(int i= 0;i<N;i++){
		printf("%.8f, ",M[i]);
	}
	printf("]\n");
}