#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>

#define DATA_T double
#define precision 0.01


//Para calcular tiempo
double dwalltime();
//para generar nro random entre min y max
DATA_T randFP(DATA_T min, DATA_T max);


int main(int argc, char** argv) {
	int N = atoi(argv[1]);
	double timetick;

	DATA_T *A = (DATA_T*) malloc(sizeof(DATA_T)*N);
	DATA_T *B = (DATA_T*) malloc(sizeof(DATA_T)*N);
	DATA_T *swapAux;

	/**
	 * TODO: revisar porque siempre devuelve las mismas iteraciones
	 * */

	//Inicialización
	int i;
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
		#ifdef DEBUG
		printf("%.2f ",A[i]);
		#endif
	}
	#ifdef DEBUG
	printf("\n\n\n---\n\n\n");
	#endif


	//Cálculo del promedio
	int converge=0,numIteracion= 0;

	timetick = dwalltime();
	while (!converge ){
		numIteracion++;
		B[0] = (A[0]+A[1]) * 0.5;

		converge = 1;
		for (i=1;i<N-1;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (0.33333333);
			if (fabs(B[0]-B[i])>precision){
				#ifdef DEBUG
				printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge = 0;
				break;
			}
		}

		for (;i<N-1;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (0.33333333);
		}

		B[N-1] = (A[N-2]+A[N-1]) *0.5;
		#ifdef DEBUG3
		printf("B[%d]= %.15f, B[0] = %.15f \n",N-1,B[N-1], B[0]);
		#endif

		if (converge && fabs(B[0]-B[N-1])>precision){
			#ifdef DEBUG
			printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",N-1,fabs(B[0]-B[N-1]),B[0],i,B[N-1]);
			#endif
			converge = 0;
		}

		if (!converge){
				swapAux = A;
				A = B;
				B = swapAux;
		}
		#ifdef DEBUG
		//Imprimo  calculo
		printf("Iteracion no: %d: \n",numIteracion);
		for(i=0;i<N;i++) {
			printf("%.3f-",B[i]);
		}
		printf("\n");
		#endif
		
	}
	/**
	 * TODO: Agregar tests
	 * */
	
	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);
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