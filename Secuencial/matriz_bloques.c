#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>

#define DATA_T float
#define precision 0.01

//Para calcular tiempo
double dwalltime();
//para crear valores DATA_T
DATA_T randFP(DATA_T min, DATA_T max);

int main(int argc, char** argv) {
	int N = atoi(argv[1]);
	double timetick;


    //TODO: Validar recibir N antes de castear a atoi
    if(N<8){
        printf("N debe ser mayor a 8 (N=%d)",N);
        return 0;
    }


	DATA_T *A = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	DATA_T *B = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	DATA_T *swapAux;

	//Inicialización
	int i,j,f;
	for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            A[f+j] = randFP(0.0,1.0);
            #ifdef DEBUG
            printf("%.2f ",A[f+j]);
            #endif
        }
        #ifdef DEBUG
        printf("\n");
        #endif
	}
	#ifdef DEBUG
    printf("Llegue a aca");
    #endif
	//Cálculo del promedio
	int condRecalculo = 1,numIteracion= 0,fila,col;
	timetick = dwalltime();
	while (condRecalculo){
		numIteracion++;
		condRecalculo = 0;


        //Borde izquierdo superior
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25;
        //-------------------------------------

        //Cálculo superior
        /*
            Intento aprovechar la localidad, entonces primero sumo los 3 datos
            de la primera fila para todos los resultados, y despues hago lo mismo 
            para la segunda fila.

            PD:no se si esto es mejor, deberiamos probar haciendo todo junto
            entiendo que dependera de la capacidad de almacenamiento en memoria
        */
        for(j=1;j<N-1;j++){
            B[j]=0;
            for(col= j-1; col < j+3; col++){
                B[j] += A[col]+A[N+col];
            }
            B[j]*= 0.16666666666;
        }
        //----------------------------------------

        //Borde derecho superior
        B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;
        //-------------------------------------

        //Cálculo sin vertices
        //Este lo hice sin la idea de localidad anterior
        for(i=1;i<N-1;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            B[i*N]=(A[(i-1)*N]+A[(i-1)*N+1]+A[i*N]+A[i*N+1]+A[(i+1)*N]+A[(i+1)*N+1])/6;
            for(j=1;j<N-1;j++){
                B[i*N+j]=0;
                for(fila= i-1; fila < i+3; fila++){
                    for(col= j-1; col < j+3; col++){
                        B[i*N+j] += A[fila*N+col];
                    }
                }
                B[i*N+j] *= 0.11111111;
            }
            //Calculo último elemento de la fila (calculo de todas las últimas columnas)
            //j=N-1
            B[i*N+j]=(A[(i-1)*N-1+j]+A[(i-1)*N+j]+A[i*N-1+j]+A[i*N+j]+A[(i+1)*N-1+j]+A[(i+1)*N+j])/6;
        }
        //----------------------------------------

		//Borde izquierdo inferior
        B[(N-1)*N] = (A[(N-2)*N] +  A[(N-2)*N+1] + A[(N-1)*N] + A[(N-1)*N+1]) * 0.25;
        //-------------------------------------
        
        //Cálculo inferior
        for(j=1;j<N-1;j++){
            B[(N-1)*N+j]=0;
            for(col= j-1; col < j+3; col++){
                B[(N-1)*N+j] += A[(N-2)*N+col]+A[(N-1)*N*col];
            }
            B[(N-1)*N+j] *= 0.16666666666;
        }
        //----------------------------------------

		//Borde derecho inferior, j=N-1
        B[N*N-1] = (A[N*N-2] + A[N*N-1] + A[N*j-2] +  A[N*j-1]) * 0.25;
        //-------------------------------------
        
        #ifdef DEBUG
            for(i=0;i<N;i++) {
                f=i*N;
                for(j=0;j<N;j++){
                    printf("%.2f ",A[f+j]);
                }
            printf("\n");
	        }
        #endif


        //Verificaion de convergencia
		for (i= 0;i< N;i++){
            for(j=1;j<N;j++){
                if (fabs( B[0] - B[i*N+j] ) > 0.01 ){
                    condRecalculo = 1;
                    #ifdef DEBUG
                    //printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                    printf("Iteriacion: %d \n",numIteracion);
                    #endif
                    swapAux = A;
                    A = B;
                    B = swapAux;
                    break;
                }
            }
		}
	}
	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);

    free(A);
    free(B);
	return 0;	
  }



double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}