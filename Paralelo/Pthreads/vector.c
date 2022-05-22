#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>
#include <pthread.h>


#define DATA_T double
#define precision 0.01


//Para calcular tiempo
double dwalltime();
//para generar nro random entre min y max
DATA_T randFP(DATA_T min, DATA_T max);


//variables compartidas
int N,T, *converge, convergeG=0,numIteracion= 0;
DATA_T *A,*B,*swapAux;

pthread_barrier_t barrera;



void *funcion(void *arg){
    int tid= *(int*)arg;
	#ifdef DEBUG
    printf("Hilo id: %d\n",tid);
	#endif
    /**
     * TODO: arreglar el debuf y poner los ifdef donde conrrespondan
     **/
    
	int start, end,i;
	start = tid * (N/T) + (tid == 0);
	end = ((tid+1) * (N/T)) - (tid == T-1);
	//DATA_T compare;
	while(!convergeG){
		//compare = (A[0]+A[1]) * 0.5;
		B[0] = (A[0]+A[1]) * 0.5;

		//calculo mi parte del promedio y a la vez reviso si converge
		converge[tid] = 1;
		for (i= start;i< end;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (0.33333333); 
			if (fabs(B[0]-B[i])>precision){
				#ifdef DEBUG2
				printf("Hilo %d ,B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",tid,i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge[tid] = 0;
				break;
			}
		}

		//calculo el promedio de los numeros que me faltaron antes de descubrir la divergencia
		for (;i< end;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (0.33333333);
		}
		
		#ifdef DEBUG
			printf("Hilo id: %d ralizo %d iteraciones en la %d iteracion\n",tid,i-start, numIteracion);
		#endif

		//reviso el ultimo valor
		if (tid == T-1){
			B[N-1] = (A[N-2]+A[N-1]) *0.5;
			if (converge[tid] && fabs(B[0]-B[i])>precision){
				#ifdef DEBUG
				printf("Hilo %d ,B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",tid,i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge[tid] = 0;
			}
		}

		//barrera para todos los hilos
		pthread_barrier_wait(&barrera);

		//el hilo 0 revisa las convergencia de los demas y swapea los vectores 		
		if ((tid == 0)){
				convergeG = 1;
				for(i= 0;i < T && convergeG;i++){
					convergeG = convergeG && converge[i];
				}
				if (!convergeG){
					swapAux = A;
					A = B;
					B = swapAux;
				}
				numIteracion++;
				

				#ifdef DEBUG
				printf("vector: ");
				for(i= 0;i<N;i++){
					printf("%.10f-",A[i]);
				}
				printf("\n");
				printf("-------------------------------------------------\n");
				#endif
		} 

		//barrera para todos los hilos
		pthread_barrier_wait(&barrera);
	}

    pthread_exit(NULL);
}



int main(int argc, char** argv) {
	N = atoi(argv[1]); 
	T = atoi(argv[2]); 
	double timetick;

	A = (DATA_T*) malloc(sizeof(DATA_T)*N);
	B = (DATA_T*) malloc(sizeof(DATA_T)*N);
	converge = (int*) malloc(sizeof(int)*T);

	//Inicialización
	int i;
	//inicializo vector
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
		#ifdef DEBUG
		printf("%.2f ",A[i]);
		#endif
	}
	#ifdef DEBUG
	printf("\n\n\n---\n\n\n");
	#endif
	//inicializo converge
	for (i = 0; i< T;i++){
		converge[i]= 0;
	}

	//inicializacion de Pthreads
	pthread_t misThreads[T];
	pthread_barrier_init(&barrera, NULL, T);

	timetick = dwalltime();

	int threads_ids[T];
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