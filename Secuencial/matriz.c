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
//para imprimir matriz
void printMatriz(int, DATA_T*);



int main(int argc, char** argv) {
	int N;
	double timetick;

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
        }
	}
    #ifdef PRINT_MATRIZ
    printMatriz(N,A);
    #endif

    //Variables para algoritmo
	int converge = 0,numIteracion= 0,inj;
	//Algoritmo de filtrado
	timetick = dwalltime();    
    while (!converge ){
    	numIteracion++;
		converge = 1;

        //-------------------------------//
        //Cálculo de fila 0

        //Esquina izquierda superior B[0,0]
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25;
        //Cálculo fila 0 revisando convergencia: B[0,x]
        for(j=1;j<N-1;j++){
            B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
            //Calculo convergencia
            if (fabs(B[0]-B[j])>precision){
				converge = 0;
				break;
			}
        }
        //Calculo el resto de la fila 0 si entré al if de la convergencia en alguna iteración
        for(;j<N-1;j++){
            B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
        }

        //Esquina derecha superior B[0,N-1]
        B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;
        //Reviso convergencia esquina derecha superior
        if (converge && fabs(B[0]-B[N-1])>precision){
				converge = 0;
		}
        //-------------------------------------
        //Fila 1 hasta N-2 Comienzo cálculos
        
        /* ACLARACIÓN de dudosa relevancia
        Si j viene seteada de arriba al valor j=N-1 y converge=0, va a entrar al siguiente for
        Que no setea j=1 en la primer iteración por lo que muere el algoritmo.
        j=1 (se hace en el for de i)
        */


        //Este primer doble for calcula verificando la convergencia
        //Si no converge en esta iteración sigue con el segundo doble for
        //que simplemente actualiza los valores
        for(i=1,j=1;converge && i<N-1;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            //B[i,0] = ...
            B[i*N]=(  A[(i-1)*N] + A[(i-1)*N+1]     //2 elems de fila anterior
                    + A[i*N] + A[i*N+1]             //2 elems de fila actual
                    + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
                    ) * (1.0/6);                    //Divido por 6
            //Reviso su convergencia
            if (fabs(B[0]-B[i*N])>precision){
                converge = 0;
                break;
		    }
            //Calculo de la parte central de la fila
            //B[i,1] hasta B[i,N-2]
            for(j=1;j<N-1;j++){
                inj = i*N+j;
                B[inj]=(  A[inj-N-1] + A[inj-N] + A[inj-N+1]    //3 elems de fila anterior
                        + A[inj-1]+ A[inj] + A[inj+1]           //3 elems de fila actual
                        + A[inj+N-1] +A[inj+N] +A[inj+N+1]      //3 elems de fila siguiente
                        ) * (1.0/9);                            //Divido por 9
                if (fabs(B[0]-B[inj])>precision){
                    converge = 0;
                    break;
		        }
            }
            //Calculo último elemento de la fila (calculo todas las últimas columnas)
            //B[i,N-1]
            B[i*N+N-1]=(A[(i-1)*N-1+N-1] + A[(i-1)*N+N-1] //2 elems de fila anterior
                    + A[i*N-1+N-1] + A[i*N+N-1]         //2 elems de fila actual
                    + A[(i+1)*N-1+N-1] + A[(i+1)*N+N-1] //2 elems de fila siguiente
                    )*(1.0/6);                      //Divido por 6
            //Verifico convergencia
            if (fabs(B[0]-B[i*N+N-1])>precision){
                converge = 0;
		    }
        }


        //Actualizo los valores que quedaron pendientes sin verificar convergencia
        for(;i<N-1;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            //B[i,0] = ...
            B[i*N] = (A[(i-1)*N] + A[(i-1)*N+1] //2 elems de fila anterior
                    + A[i*N] + A[i*N+1]         //2 elems de fila actual
                    + A[(i+1)*N] + A[(i+1)*N+1] //2 elems de fila siguiente
                    )*(1.0/6);                  //Divido por 6
            //printf("pos [%d], A[%d] = %f + A[%d]= %f\n",i*N,(i-1)*N,A[(i-1)*N],(i-1)*N+1,A[(i-1)*N+1]);
            //Calculo de la parte central de la fila
            //B[i,1] hasta B[i,N-2]
            for(;j<N-1;j++){
                inj = i*N+j;
                B[inj] = (A[inj-N-1] + A[inj-N] + A[inj-N+1]    //3 elems de fila anteriors
                        + A[inj-1]+ A[inj] + A[inj+1]           //3 elems de fila actuals
                        + A[inj+N-1] +A[inj+N] +A[inj+N+1]      //3 elems de fila siguientes
                         )*(1.0/9);                             //Divido por 9s
            }
            //Calculo último elemento de la fila (calculo todas las últimas columnas)
            //B[i,N-1], j=N-1
            B[i*N+j] = ( A[(i-1)*N-1+j] + A[(i-1)*N+j]  //2 elems de fila anterior
                        +A[i*N-1+j] + A[i*N+j]          //2 elems de fila actual
                        +A[(i+1)*N-1+j] + A[(i+1)*N+j]  //2 elems de fila siguiente
                        )*(1.0/6);                      //Divido por 6
            //Preparo j para la siguiente iteración ya que el for de esta parte no inicializa j
            j=1;
        }
        //----------------------------------------
        //Calculo esquina izquierda inferior B[N-1,0]
        B[(N-1)*N] = (A[(N-2)*N] +  A[(N-2)*N+1] 
                    + A[(N-1)*N] + A[(N-1)*N+1]
                    ) * 0.25;
        //Verifico convergencia
        if (converge && fabs(B[0]-B[(N-1)*N])>precision){
            converge = 0;
        }
        //-------------------------------------

        //Calculo ultima fila verificando convergencia
        //B[N-1,j]
        i=N-1;
        for(j=1;j<N-1;j++){
            inj= i*N+j;
            B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                     + A[inj-1-N] + A[inj-N] + A[inj+1-N]   //3 elems de fila anterior
                     ) * (1.0/6);
            //calculo de convergencia
            if (fabs(B[0]-B[inj])>precision){
				converge = 0;
				break;
			}
        }
        //Calculo del resto de columnas sin verificar convergencia
        for(;j<N-1;j++){
            inj= i*N+j;
            B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                     + A[inj-1-N] + A[inj-N] + A[inj+1-N]   //3 elems de fila anterior
                     ) * (1.0/6);
        }
		//Esquina derecha inferior, B[N-1,N-1]
        //j=N-1
        B[N*N-1] = (A[N*N-2] + A[N*N-1] + A[N*j-2] + A[N*j-1])*0.25;
        //Verifico convergencia de ultimo elemento
        if (converge && fabs(B[0]-B[N*N-1])>precision){
            converge = 0;
        }

        //-------------------------------------

        //SI TODAVÍA NO CONVERGE SWAPEO PARA LA SIGUIENTE ITERACIÓN
        if(!converge){
            swapAux=A;
            A=B;
            B=swapAux;
        }

        #ifdef DEBUG_POR_ITERACION
        if(numIteracion > 230){
            printf("MATRIZ A ITERACION: %d\n", numIteracion);
            printMatriz(N,A);
        }
        #endif
	}


    
	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);
    printf("-------------------\n");
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
    printf("\n");
    //imprimir la matriz
    for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            printf("%.7f ",B[f+j]);
        }
    printf("\n");
    }
    printf("\n");
}