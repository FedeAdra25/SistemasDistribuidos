#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#define DATA_T double
#define precision 0.01

//Para calcular tiempo
double dwalltime();
//para crear valores DATA_T
DATA_T randFP(DATA_T min, DATA_T max);
//para imprimir matriz
void printMatriz(int, DATA_T*);
//para imprimir vector converge
void printConverge(int*);
//tarea de los hilos
void *funcion(void *arg);

//variables compartidas
int N,T, *converge, convergeG=0,numIteracion=0;
DATA_T *A,*B,*swapAux;

pthread_barrier_t barrera;

int main(int argc, char** argv) { 
	double timetick;

    if(argc<3){
        printf("Error: debe enviar el tamaño de la matriz y cantidad de threads \nRecibido %d argumentos\n",argc);
        printf("Forma: ./out.o N T (ej: ./out.o 256 4)\n");
        return 2;
    }
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    if(N<8){
        printf("N debe ser mayor a 8 (N=%d)",N);
        return 0;
    }
    if(argc>3){
        printf("%s\n",argv[3]);
    }


	A = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	B = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	converge = (int*) malloc(sizeof(int)*T);

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

	//Inicializo vector de booleanas
	for (i = 0; i< T;i++){
		converge[i]= 0;
	}
	//Inicializacion de Pthreads
	pthread_t misThreads[T];
	pthread_barrier_init(&barrera, NULL, T);
    int threads_ids[T];

    #ifdef DEBUG
    printf("Comienza el algoritmo N=%d T=%d\n",N,T);
    #endif
    //Comienzo a correr el tiempo
	timetick = dwalltime();
    //Antes de crear los procesos calculo el primer elemento para tenerlo de referencia cuando verifico convergencia
    B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25; //Esquina izquierda superior B[0,0]
	for(int id=0;id<T;id++){ 
		threads_ids[id]=id; 
		pthread_create(&misThreads[id],NULL,&funcion,(void*)&threads_ids[id]); 
	}
	for(int id=0;id<T;id++){ 
		pthread_join(misThreads[id],NULL); 
	}
    //Fin de calculo
    #ifdef DEBUG
    printf("Terminaron todos los procesos\n");
    #endif
	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);

    //Libero recursos
	pthread_barrier_destroy(&barrera);
	free(A);
	free(B);
    free(converge);
	return 0;	
  }

void *funcion(void *arg){
    //Variables para el algoritmo y tid
    int tid= *(int*)arg;
	int start, end,i,j,inj;

    //Calculo 
    start = tid * (N/T) + (tid == 0);
	end = ((tid+1) * (N/T)) - (tid == T-1);
    #ifdef DEBUG
    printf("TID: %d START y calcula desde fila %d hasta fila %d\n",tid,start,end);
    #endif

	//Algoritmo de filtrado
    while (!convergeG && numIteracion < 1){
		converge[tid] = 1;

        if(tid==0){
            //Cálculo fila 0 revisando convergencia: B[0,x]
            for(j=1;j<N-1;j++){
                printf("Hilo %d calcul B[%d][%d]\n",tid,0,j);
                B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
                //Calculo convergencia
                if (fabs(B[0]-B[j])>precision){
                    converge[tid] = 0;
                    break;
                }
            }
            //Calculo el resto de la fila 0 si entré al if de la convergencia en alguna iteración
            for(;j<N-1;j++){
                printf("Hilo %d calcul B[%d][%d]\n",tid,0,j);
                B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
            }
            //Esquina derecha superior B[0,N-1]
            printf("Hilo %d calcul B[%d][%d]\n",tid,0,N-1);
            B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;
            //Reviso convergencia esquina derecha superior
            if (converge[tid] && fabs(B[0]-B[N-1])>precision){
                    converge[tid] = 0;
            }
        }

        #ifdef DEBUGA
            printf("Inicio cálculo de fila %d - %d\n--------------\n",start,end);
        #endif
        //Este primer doble for calcula verificando la convergencia
        //Si no converge en esta iteración sigue con el segundo doble for que simplemente actualiza los valores
        for(i=start,j=1;converge[tid] && i<end;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            //B[i,0] = ...
            printf("Hilo %d calcul B[%d][%d]\n",tid,i,0);
            B[i*N]=(  A[(i-1)*N] + A[(i-1)*N+1]     //2 elems de fila anterior
                    + A[i*N] + A[i*N+1]             //2 elems de fila actual
                    + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
                    ) * (1.0/6);                    //Divido por 6
            //Reviso su convergencia
            if (fabs(B[0]-B[i*N])>precision){
                converge[tid] = 0;
                break;
		    }
            //Calculo de la parte central de la fila
            //B[i,1] hasta B[i,N-2]
            for(j=1;j<N-1;j++){
                inj = i*N+j;
                printf("Hilo %d calcul B[%d][%d] inj = %d \n",tid,i,j, inj);
                B[inj]=(  A[inj-N-1] + A[inj-N] + A[inj-N+1]    //3 elems de fila anterior
                        + A[inj-1]+ A[inj] + A[inj+1]           //3 elems de fila actual
                        + A[inj+N-1] +A[inj+N] +A[inj+N+1]      //3 elems de fila siguiente
                        ) * (1.0/9);                            //Divido por 9
                if (fabs(B[0]-B[inj])>precision){
                    converge[tid] = 0;
                    break;
		        }
            }
            //Calculo último elemento de la fila (calculo todas las últimas columnas)
            //B[i,N-1]
            printf("Hilo %d calcul B[%d][%d]\n",tid,i,N-1);
            B[i*N+N-1]= ( A[(i-1)*N-1+N-1] + A[(i-1)*N+N-1] //2 elems de fila anterior
                        + A[i*N-1+N-1] + A[i*N+N-1]         //2 elems de fila actual
                        + A[(i+1)*N-1+N-1] + A[(i+1)*N+N-1] //2 elems de fila siguiente
                        ) * (1.0/6);                      //Divido por 6
            //Verifico convergencia
            if (fabs(B[0]-B[i*N+N-1])>precision){
                converge[tid] = 0;
		    }
        }

        //Actualizo los valores que quedaron pendientes sin verificar convergencia
        for(;i<end;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            //B[i,0] = ...
            printf("Hilo %d calcul B[%d][%d]\n",tid,i,0);
            B[i*N] = (A[(i-1)*N] + A[(i-1)*N+1] //2 elems de fila anterior
                    + A[i*N] + A[i*N+1]         //2 elems de fila actual
                    + A[(i+1)*N] + A[(i+1)*N+1] //2 elems de fila siguiente
                    ) * (1.0/6);                  //Divido por 6

            //Calculo de la parte central de la fila
            //B[i,1] hasta B[i,N-2]
            for(;j<N-1;j++){
                inj = i*N+j;
                printf("Hilo %d calcul B[%d][%d] inj = %d \n",tid,i,j, inj);
                B[inj] = (A[inj-N-1] + A[inj-N] + A[inj-N+1]    //3 elems de fila anteriors
                        + A[inj-1]+ A[inj] + A[inj+1]           //3 elems de fila actuals
                        + A[inj+N-1] +A[inj+N] +A[inj+N+1]      //3 elems de fila siguientes
                         ) * (1.0/9);                             //Divido por 9s
            }
            //Calculo último elemento de la fila (calculo todas las últimas columnas)
            //B[i,N-1], j=N-1
            printf("Hilo %d calcul B[%d][%d]\n",tid,i,j);
            B[i*N+j] = ( A[(i-1)*N-1+j] + A[(i-1)*N+j]  //2 elems de fila anterior
                        +A[i*N-1+j] + A[i*N+j]          //2 elems de fila actual
                        +A[(i+1)*N-1+j] + A[(i+1)*N+j]  //2 elems de fila siguiente
                        ) * (1.0/6);                      //Divido por 6
            //Preparo j para la siguiente iteración ya que el for de esta parte no inicializa j
            j=1;
        }
        #ifdef DEBUGA
            printf("FIN cálculo de fila %d hasta %d by %d\n--------------\n",start,end,tid);
        #endif

        if(tid==T-1){
            //Calculo ULTIMA FILA
            //Primer elemento: Calculo esquina izquierda inferior B[N-1,0]
            printf("Hilo %d calcul B[%d][%d]\n",tid,N-1,0);
            B[(N-1)*N] = (A[(N-2)*N] +  A[(N-2)*N+1] 
                        + A[(N-1)*N] + A[(N-1)*N+1]
                        ) * 0.25;
            //Verifico convergencia
            if (converge[tid] && fabs(B[0]-B[(N-1)*N])>precision){
                converge[tid] = 0;
            }
            //Calculo ultima fila verificando convergencia
            //B[N-1,j]
            i=N-1;
            for(j=1;j<N-1;j++){
                inj= i*N+j;
                printf("Hilo %d calcul B[%d][%d] inj = %d\n",tid,i,j,inj);
                B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                        + A[inj-1-N] + A[inj-N] + A[inj+1-N]   //3 elems de fila anterior
                        ) * (1.0/6);
                
                //calculo de convergencia
                if (fabs(B[0]-B[inj])>precision){
                    converge[tid] = 0;
                    break;
                }
            }
            //Calculo del resto de columnas sin verificar convergencia
            for(;j<N-1;j++){
                inj= i*N+j;
                printf("Hilo %d calcul B[%d][%d] inj = %d\n",tid,i,j,inj);
                B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                        + A[inj-1-N] + A[inj-N] + A[inj+1-N]   //3 elems de fila anterior
                        ) * (1.0/6);
            }
            //Ultimo elemento: Esquina derecha inferior, B[N-1,N-1]
            //j=N-1
            printf("Hilo %d calcul B[%d][%d] \n",tid,i,j);
            B[N*N-1] = (A[N*N-2] + A[N*N-1] + A[N*j-2] + A[N*j-1])*0.25;
            //Verifico convergencia de ultimo elemento
            if (converge[tid] && fabs(B[0]-B[N*N-1])>precision){
                converge[tid] = 0;
            }
        }

        //Barrera para todos los hilos
		pthread_barrier_wait(&barrera);
        //Si soy el tid 0
        //Reviso la convergencia de los demas y swapeo los vectores 		
		if ((tid == 0)){

                #ifdef DEBUG_POR_ITERACION
                if(numIteracion==10000){
                    printf("MATRIZ ITERACION: %d - FIN\n\n\n", numIteracion);
                    printf("i=%d j=%d converge=%d\n",i,j,convergeG);
                    printConverge(converge);
                    printMatriz(N,B);
                    convergeG=1;
                    pthread_barrier_wait(&barrera);
                    break;
                }
                #endif

				convergeG = 1;
				for(i= 0;i < T && convergeG;i++){
					convergeG = convergeG && converge[i];
				}
				if (!convergeG){
					swapAux = A;
					A = B;
					B = swapAux;
                    //Calculo esquina izquierda superior B[0,0] para la siguiente iteración
                    printf("soy el hilo %d caluclo B[0]",tid);
                    B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25;
				}
				numIteracion++;
				
		}
        else{
            printf("Soy el Hilo %d, esperando al cero",tid);
        }
        //Barrera para que esperen a que el hilo cero valide convergencia global y haga el swap
		pthread_barrier_wait(&barrera);

	}
    #ifdef DEBUG
    printf("TID: %d FINISH\n",tid);
    #endif
	pthread_exit(NULL);
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

void printMatriz(int N, DATA_T* M){
    
    int i,j,f,c;
    printf("\n");
    //imprimir la matriz
    for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            printf("%.2f ",M[f+j]);
        }
    printf("\n");
    }
    printf("\n");
}

void printConverge(int* p){
    int i;
    printf("\n");
    for(i=0;i<T;i++){
        printf("converge[%d]=%d - ",i,p[i]);
    }
    printf("\n");
}