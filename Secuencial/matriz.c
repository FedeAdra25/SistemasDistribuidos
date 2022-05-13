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
//para crear valores float
double randFP(double min, double max) {
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + (rand() / div);
}

int main(int argc, char** argv) {
	int N = atoi(argv[1]);
	double timetick;

	float *A = (float*) malloc(sizeof(float)*N*N);
	float *B = (float*) malloc(sizeof(float)*N*N);
	float *swapAux;

	//inicializacion
	int i,j;
	for(i=0;i<N;i++) {
        for(j=0;j<N;j++){
            A[i*N+j] = randFP(0.0,1.0);
            printf("%.2f ",A[i]);
        }
        printf("\n");
	}
	


	//calculo del promedio
	int condRecalculo = 1,numIteracion= 0,fila,col;
	timetick = dwalltime();
	while (condRecalculo){
		numIteracion++;
		condRecalculo = 0;


        //borde izquierdo superior
        B[0] = (A[0] + A[1] + A[N] + A[N+1])/4
        //-------------------------------------

        //calculo superior
        /*
            Intento aporbechar la localidad, entonces primero sumo los 3 datos
            de la primera fila para todos los resultados, y despues hago lo mismo 
            para la segunda fila.

            PD:no se si esto es mejor, deberiamos probar haciendo todo junto
            entiendo que dependera de la capacidad de almacenamiento en memoria
        */
        for(j=1;j<N-1;j++){
            B[j]=0;
            for(col= j-1; col < j+3; j++){
                B[j] += A[col];
            }
        }
        for(j=1;j<N-1;j++){
            for(col= j-1; col < j+3; j++){
                B[j] += A[N+col];
            }
            B[j] /= 9;
        }
        //----------------------------------------

        //borde derecho superior
        B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)])/4
        //-------------------------------------

        //calculo sin vertices
        //este lo hice sin la idea de localidad anterior
        for(i=1;i<N-1;i++) {
            for(j=1;j<N-1;j++){
                B[i*N+j]=0;

                for(fila= i-1; fila < i+3; i++){
                    for(col= j-1; col < j+3; j++){
                        B[i*N+j] += A[fila*N+col];
                    }
                }
                B[i*N+j] /= 9;
            }
        }
        //----------------------------------------

		//borde izquierdo inferior
        B[(N-1)*N] = (A[(N-1)*N] + A[(N-1)*N+1] + A[(N-2)*N] +  A[(N-2)*N+1])/4
        //-------------------------------------
        
        //calculo inferior
        for(j=1;j<N-1;j++){
            B[(N-1)*N+j]=0;
            for(col= j-1; col < j+3; j++){
                B[(N-1)*N+j] += A[(N-2)*N+col];
            }
        }
        for(j=1;j<N-1;j++){
            for(col= j-1; col < j+3; j++){
                B[(N-1)*N+j] += A[(N-1)*N*col];
            }
            B[(N-1)*N+j] /= 9;
        }
        //----------------------------------------

		//borde drecho inferior
        B[(N-1)*N+ N-1] = (A[(N-1)*N +N-2] + A[(N-1)*N +N-1] + A[(N-2)*N +N-2] +  A[(N-2)*N +N-1])/4
        //-------------------------------------

        //verificaion de convergencia
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

