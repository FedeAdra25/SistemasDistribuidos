#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>
#include <pthread.h>


#define DATA_T double
#define precision (1.0/100)


//Para calcular tiempo
double dwalltime();
//para generar nro random entre min y max
DATA_T randFP(DATA_T min, DATA_T max);
//para imprimir matriz
void printVector(int N, DATA_T *M);

//variables compartidas
int N,T, *converge, convergeG=0,numIteracion= 0;
DATA_T *A,*B,*swapAux;

pthread_barrier_t barrera;


void *funcion(void *arg){
    int tid= *(int*)arg;
	int start, end,i;
	start = tid * (N/T) + (tid == 0);
	end = ((tid+1) * (N/T)) - (tid == T-1);
	#ifdef DEBUG
    printf("Hilo id: %d START from: %d to %d\n",tid,start,end);
	#endif


	//DATA_T compare;
	while(!convergeG){
		//Calculo mi parte del promedio y a la vez reviso si converge
		converge[tid] = 1;
		for(i=start;i<end;i++){
			B[i] = (A[i-1] + A[i] + A[i+1]) * (1.0/3); 
			if (fabs(B[0]-B[i]) > precision){
				converge[tid] = 0;
				i++;
				break;
			}
		}
		//Calculo el promedio de los numeros que me faltaron antes de descubrir la divergencia
		for (;i<end;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (1.0/3);
		}

		//Calculo el ultimo valor si soy el ultimo thread
		if (tid==T-1){
			B[N-1] = (A[N-2]+A[N-1]) *0.5;
			if (converge[tid] && fabs(B[0]-B[i])>precision){
				converge[tid] = 0;
			}
		}

		//Barrera para todos los hilos
		pthread_barrier_wait(&barrera);

		//El hilo 0 revisa las convergencia de los demás y swapea los vectores 		
		if (tid == 0){
				convergeG = 1;
				for(i=0; i<T && convergeG;i++){
					convergeG = convergeG && converge[i];
				}
				if (!convergeG){
					swapAux = A;
					A = B;
					B = swapAux;
					//Calculo el elemento de referencia para la siguiente iteración
					B[0] = (A[0]+A[1]) * 0.5;
				}
				numIteracion++;
				#ifdef DEBUG
				printf("Iteracion: %d\n",numIteracion);
				printVector(N,A);				
				printf("-------------------------------------------------\n");
				#endif
		} 

		//barrera para todos los hilos
		pthread_barrier_wait(&barrera);
	}


	#ifdef DEBUG
    printf("Hilo id: %d STOP\n",tid);
	#endif
    pthread_exit(NULL);
}



int main(int argc, char** argv) {
	double timetick;

	if(argc<3){
        printf("Error: debe enviar el tamaño del vector y cantidad de threads \nRecibido %d argumentos\n",argc);
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

	A = (DATA_T*) malloc(sizeof(DATA_T)*N);
	B = (DATA_T*) malloc(sizeof(DATA_T)*N);
	converge = (int*) malloc(sizeof(int)*T);

	//Inicialización
	int i;
	//inicializo vector
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
	}
	#ifdef DEBUG
	printVector(N,A);
	#endif
	//Inicializo converge
	for (i = 0; i< T;i++){
		converge[i]= 0;
	}

	//inicializacion de Pthreads
	pthread_t misThreads[T];
	pthread_barrier_init(&barrera, NULL, T);
	int threads_ids[T];

	timetick = dwalltime();
	B[0] = (A[0]+A[1]) * 0.5; //Calculo la primera vez el elemento primero para comparar
	for(int id=0;id<T;id++){ 
		threads_ids[id]=id; 
		pthread_create(&misThreads[id],NULL,&funcion,(void*)&threads_ids[id]); 
	}

	for(int id=0;id<T;id++){ 
		pthread_join(misThreads[id],NULL); 
	} 

	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);
	pthread_barrier_destroy(&barrera);
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