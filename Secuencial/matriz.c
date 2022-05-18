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
    printf("Llegue a aca\n");
    #endif
	//Cálculo del promedio
	int condRecalculo = 1,numIteracion= 0,fila,col;
	timetick = dwalltime();
	while (condRecalculo){
	//while (numIteracion < 5){
    	numIteracion++;
		condRecalculo = 0;


        //Borde izquierdo superior
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25;
        //-------------------------------------

        #ifdef DEBUG
            printf("A[0]=%.2f ,B[0]=%.2f \n",A[0], B[0]);   

        #endif

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
            for(col= j-1; col < j+2; col++){
                //printf("col = %d, j= %d, N+col= %d",col,j,N+col);
                B[j] += A[col]+A[N+col];
            }
            B[j]/= 6;
        }
        //----------------------------------------

        #ifdef DEBUG
            printf("hice la parte superior\n");
        #endif


        //Borde derecho superior
        B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;
        //-------------------------------------

        #ifdef DEBUG
            printf("hice el borde derecho superior\n");
        #endif

        //Cálculo sin vertices
        //Este lo hice sin la idea de localidad anterior
        for(i=1;i<N-1;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            B[i*N]=(A[(i-1)*N]+A[(i-1)*N+1]+A[i*N]+A[i*N+1]+A[(i+1)*N]+A[(i+1)*N+1])/6;
            for(j=1;j<N-1;j++){
                B[i*N+j]=0;
                for(fila= i-1; fila < i+2; fila++){
                    for(col= j-1; col < j+2; col++){
                        B[i*N+j] += A[fila*N+col];
                    }
                }
                B[i*N+j] /= 9;
            }
            //Calculo último elemento de la fila (calculo de todas las últimas columnas)
            //j=N-1
            B[i*N+j]=(A[(i-1)*N-1+j]+A[(i-1)*N+j]+A[i*N-1+j]+A[i*N+j]+A[(i+1)*N-1+j]+A[(i+1)*N+j])/6;
        }
        //----------------------------------------

        #ifdef DEBUG
            printf("hice la matriz sin vertices\n");
        #endif

		//Borde izquierdo inferior
        B[(N-1)*N] = (A[(N-2)*N] +  A[(N-2)*N+1] + A[(N-1)*N] + A[(N-1)*N+1]) * 0.25;
        //-------------------------------------
        
        #ifdef DEBUG
            printf("hice el borde izquierdo inferior\n");
        #endif

        //Cálculo inferior
        i=N-1;
        for(j=1;j<N-1;j++){
            B[i*N+j]=0;
            #ifdef DEBUG
            printf("------------\n");
            #endif 
            for(col= j-1; col < j+2; col++){
                #ifdef DEBUG
                printf("col= %d, j= %d, i= %d, N=%d \n",col,j,i,N);
                printf("i*N+j = %d, (i-1)*N+col= %d, i*N+col= %d \n",i*N+j,(i-1)*N+col,i*N+col);
                #endif
                B[i*N+j] += A[(i-1)*N+col]+A[i*N+col];
            }
            B[i*N+j] /= 6;
        }
        //----------------------------------------

        #ifdef DEBUG
            printf("hice el calculo inferior\n");
        #endif

		//Borde derecho inferior, j=N-1
        B[N*N-1] = (A[N*N-2] + A[N*N-1] + A[N*j-2] +  A[N*j-1]) * 0.25;
        //-------------------------------------
        
        #ifdef DEBUG
            //imprimir la matriz
            for(i=0;i<N;i++) {
                f=i*N;
                for(j=0;j<N;j++){
                    printf("%.2f ",B[f+j]);
                }
            printf("\n");
	        }
        #endif


        //Verificaion de convergencia
		for (i= 0;i< N && !condRecalculo ;i++){
            for(j=1;j<N && !condRecalculo;j++){
                if (fabs( B[0] - B[i*N+j] ) > 0.01 ){
                    condRecalculo = 1;
                    #ifdef DEBUG2
                    printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i*N+j,fabs(B[0]-B[i*N+j]),B[0],i*N+j,B[i]);
                    printf("Iteriacion: %d \n",numIteracion);
                    #endif
                    swapAux = A;
                    A = B;
                    B = swapAux;
                }
            }
		}
	}

    #ifdef DEBUG2
        //imprimir la matriz
        for(i=0;i<N;i++) {
            f=i*N;
            for(j=0;j<N;j++){
                printf("%.2f ",B[f+j]);
            }
        printf("\n");
        }
    #endif

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