#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>

#define DATA_T double
#define precision 0.01

//Para calcular tiempo
double dwalltime();
//para crear valores DATA_T
DATA_T randFP(DATA_T min, DATA_T max);
//para imprimir matriz
void printMatriz(int, DATA_T*);



int main(int argc, char** argv) {
	int N;
	double timetick;

    if(argc<1){
        printf("Error: debe enviar el tamaño de la matriz. \nRecibido %d argumentos\n",argc);
        printf("Forma: ./out.o N\n");
        return 2;
    }
    N = atoi(argv[1]);
    if(N<8){
        printf("N debe ser mayor a 8 (N=%d)",N);
        return 0;
    }

    //Alocación de memoria para matrices
	DATA_T *A = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	DATA_T *B = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	DATA_T *swapAux;

	//Inicialización de matriz A
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



    //variables para algoritmo
	int converge = 0,numIteracion= 0,inj;

	//Algoritmo de filtrado
	timetick = dwalltime();    
    while (!converge){
    	numIteracion++;
		converge = 1;


        //Borde izquierdo superior B[0,0]
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25;
        //-------------------------------------

        //Cálculo fila 0 revisando convergencia: B[0,x]
        for(j=1;j<N-1;j++){
            B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
            //Calculo de convergencia
            if (fabs(B[0]-B[j])>precision){
				converge = 0;
				break;
			}
        }

        //Calculo el resto de la fila 0
        for(;j<N-1;j++){
            B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
        }
        #ifdef DEBUG
            printf("Fila 0 (parte superior) calculada\n--------------\n");
        #endif
        //----------------------------------------
        //Esquina derecha superior B[0,N-1]
        B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;
        //Reviso convergencia esquina derecha superior
        if (converge && fabs(B[0]-B[N-1])>precision){
				converge = 0;
		}
        #ifdef DEBUG
            printf("Hice el borde derecho superior\n");
        #endif
        //-------------------------------------


        #ifdef DEBUG
        printf("MATRIZ ANTES:\n\n\n");
        printf("i=%d j=%d converge=%d\n",i,j,converge);
        printMatriz(N,B);
        printf("\n\nFINMATRIZ\n\n");
        #endif
        j=1; //Esto se hace porque j=15 y puedo no entrar al for de acá entonces el siguiente
                //que retoma el valor de j se rompería
        //Cálculo sin vertices checkeando convergencia
        for(i=1,j=1;i<N-1 && converge;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            B[i*N]=( A[(i-1)*N] + A[(i-1)*N+1] + A[i*N] 
                    + A[i*N+1] + A[(i+1)*N] + A[(i+1)*N+1])/6;
            if (fabs(B[0]-B[i*N])>precision){
                #ifdef DEBUG
                printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                #endif
                converge = 0;
                break;
		    }
            for(j=1;j<N-1;j++){
                inj = i*N+j;
                B[inj]= (A[inj-N-1] + A[inj-N] + A[inj-N+1] 
                + A[inj-1]+ A[inj] + A[inj+1] 
                + A[inj+N-1] +A[inj+N] +A[inj+N+1])  * 0.1111111111111111;
                if (fabs(B[0]-B[inj])>precision){
                    #ifdef DEBUG
                    printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                    #endif
                    converge = 0;
                    break;
		        }
            }
            //Calculo último elemento de la fila (calculo de todas las últimas columnas)
            //j=N-1
            B[i*N+j]=(A[(i-1)*N-1+j] + A[(i-1)*N+j] + A[i*N-1+j] + A[i*N+j] + A[(i+1)*N-1+j] + A[(i+1)*N+j])/6;
            if (fabs(B[0]-B[i*N+j])>precision){
                #ifdef DEBUG
                printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                #endif
                converge = 0;
		    }
        }
        #ifdef DEBUG
        printf("MATRIZ:\n\n\n");
        printf("i=%d j=%d converge=%d\n",i,j,converge);
        printMatriz(N,B);
        printf("\n\nFINMATRIZ\n\n");
        #endif


        //Cálculo sin vertices(el resto en caso de no convergir) sin checkeo de convergencia 
        for(;i<N-1;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            B[i*N]=(A[(i-1)*N] + A[(i-1)*N+1] + A[i*N] + A[i*N+1] + A[(i+1)*N] + A[(i+1)*N+1])/6;
            for(;j<N-1;j++){
                inj = i*N+j;
                B[inj]= (A[inj-N-1] + A[inj-N] + A[inj-N+1] 
                + A[inj-1]+ A[inj] + A[inj+1] 
                + A[inj+N-1] +A[inj+N] +A[inj+N+1])  * 0.1111111111111111;
            }
            //Calculo último elemento de la fila (calculo de todas las últimas columnas)
            //j=N-1
            B[i*N+j]=(   A[(i-1)*N-1+j]+A[(i-1)*N+j]+A[i*N-1+j]
                        +A[i*N+j]+A[(i+1)*N-1+j]+A[(i+1)*N+j])/6;
            
            j=1; //esto se hace porque el for de arriba no tiene inicializado j
        }

        //----------------------------------------

        #ifdef DEBUG
            printf("Hice la matriz sin vertices\n");
        #endif

		//Borde izquierdo inferior
        B[(N-1)*N] = (A[(N-2)*N] +  A[(N-2)*N+1] + A[(N-1)*N] + A[(N-1)*N+1]) * 0.25;

        if (converge && fabs(B[0]-B[(N-1)*N])>precision){
            #ifdef DEBUG
            printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
            #endif
            converge = 0;
        }
        //-------------------------------------
        
        #ifdef DEBUG
            printf("hice el borde izquierdo inferior\n");
        #endif

        //Cálculo inferior verificando convegencia
        i=N-1;
        for(j=1;j<N-1;j++){
            inj= i*N+j;

            B[inj] = (A[inj-1] + A[inj] + A[inj+1] 
                    +A[inj-1-N] + A[inj-N] + A[inj+1-N])*0.1666666666666;
            
            //calculo de convergencia
            if (fabs(B[0]-B[inj])>precision){
				#ifdef DEBUG
				printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge = 0;
				break;
			}
        }

        for(;j<N-1;j++){
            inj= i*N+j;

            B[inj] = (A[inj-1] + A[inj] + A[inj+1] 
                    +A[inj-1-N] + A[inj-N] + A[inj+1-N])*0.1666666666666;
        }
        //----------------------------------------

        #ifdef DEBUG
            printf("hice el calculo inferior\n");
        #endif

		//Borde derecho inferior, j=N-1
        B[N*N-1] = (A[N*N-2] + A[N*N-1] + A[N*j-2] + A[N*j-1])*0.25;

        if (converge && fabs(B[0]-B[N*N-1])>precision){
            #ifdef DEBUG
            printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
            #endif
            converge = 0;
        }
        //-------------------------------------
        if(!converge){
            swapAux=A;
            A=B;
            B=swapAux;
            
        }
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

void printMatriz(int N, DATA_T* B){
    int i,j,f,c;
    //imprimir la matriz
    for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            printf("%.2f ",B[f+j]);
        }
    printf("\n");
    }
}