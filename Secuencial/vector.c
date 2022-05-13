#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

double randFP(double min, double max) {
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + (rand() / div);
}

int main(int argc, char** argv) {
	int N = atoi(argv[1]);
	double timetick;

	float *A = (float*) malloc(sizeof(float)*N);
	float *B = (float*) malloc(sizeof(float)*N);
	float *swapAux;

	//inicializacion
	int i;
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
		printf("%.2f ",A[i]);
	}
	printf("\n");


	//calculo del promedio
	int condRecalculo = 1,numIteracion= 0;

	timetick = dwalltime();
	while (condRecalculo){
		numIteracion++;
		condRecalculo = 0;

		B[0] = (A[0]+A[1])/2;
		B[N-1] = (A[N-2]+A[N-1])/2;

		for (i= 1;i< N-1;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])/3; 
		}

		//print calculo
		/*printf("%d iteracion: ",numIteracion);
		for(i=0;i<N;i++) {
			printf("%.3f ",B[i]);
		}
		printf("\n");*/

		for (i= 1;i< N;i++){
			if (fabs( B[0] - B[i] ) > 0.01 ){
				condRecalculo = 1;
				swapAux = A;
				A = B;
				B = swapAux;
				break;
			}
		}
	}
	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);

	return(0);	
  }

