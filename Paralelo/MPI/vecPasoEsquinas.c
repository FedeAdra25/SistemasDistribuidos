#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/time.h>

#define ROOT_P 0
#ifndef USE_FLOAT
    #define DATA_T double
    #define MPI_DATA_T MPI_DOUBLE
#else
    #define DATA_T float
    #define MPI_DATA_T MPI_FLOAT
#endif

#define precision 0.01

//Utils
double dwalltime();
void printVector(int N, DATA_T *M);
DATA_T randFP(DATA_T min, DATA_T max);

void funcionSlave(int, int, int);
void funcionDelMaster(int N, int P);

int main(int argc, char **argv)
{
    int miID;
    int nrProcesos;
    int N;

    MPI_Init(&argc, &argv);                     //Inicializa el ambiente
    MPI_Comm_rank(MPI_COMM_WORLD, &miID);       // Obtiene el identificador de cada proceso (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcesos); // Obtiene el numero de procesos

    if(argc<2){
        printf("Error: debe enviar el tamaño del vector \nRecibido %d argumentos\n",argc);
        printf("Forma: ./out.o N (ej: ./out.o 256)\n");
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

    if (miID == 0)
    {
        funcionDelMaster(N, nrProcesos);
    }
    else
    {
        funcionSlave(miID, N, nrProcesos);
    }

    MPI_Finalize(); // Finaliza el ambiente MPI. No debe haber sentencias después
    
    return (0);
}

void funcionDelMaster(int N, int nrProcesos) {
	DATA_T *A, *B, *swapAux;
	int bloque = N / nrProcesos;
	int converge, numIteraciones = 0;
	int convergeG = 0;
	double timetick;

    #ifdef PRUEBA
	printf("Root\n");
    #endif

	// Aloca memoria para los vectores
	A = (DATA_T *) malloc(sizeof(DATA_T) * N);
	B = (DATA_T *) malloc(sizeof(DATA_T) * bloque+1);

	// Inicializacion del arreglo
	for(int i=0;i<N;i++) {
            A[i] = randFP(0.0,1.0);
            //A[i] = (double)rand() / (double)(RAND_MAX); 
            #ifdef PRINT_VEC
            printf("%.5f ",A[i]);
            #endif
    }
    #ifdef PRINT_VEC
    printf("\n\n --------------------\n\n");
    sleep(3);
    #endif

	timetick = dwalltime();

    // Enviar los bloques a cada proceso
    MPI_Scatter(A, bloque, MPI_DATA_T, A, bloque, MPI_DATA_T, 0, MPI_COMM_WORLD);

    #ifdef PRUEBA        
    printf("fin del evio\n");
    sleep(3);
    #endif

  while (!convergeG) 
  {
    B[0] = (A[0]+A[1]) * 0.5;
    //envio el dato para chequear convergencia
   	MPI_Bcast(B, 1, MPI_DATA_T, 0, MPI_COMM_WORLD);

    MPI_Request request;
	MPI_Status status;

	MPI_Isend(&A[bloque-1], 1, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
    // recibo el valor del vecino derecho
    MPI_Irecv(&A[bloque], 1, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);
    
    #ifdef PRUEBA        
    printf("root envio el primer valor\n");
    sleep(3);
    #endif

    #ifdef PRINT_VEC
        printf("VecA Master :");
        for(int i=0;i<bloque+1;i++){
            printf("%.5f ",A[i]);
        }
        printf("\n");
        sleep(3);
    #endif

  	// Procesamiento
    int i;
    converge = 1;
	for(i=1;i<bloque;i++){
        B[i] = (A[i-1] + A[i] + A[i+1]) * (1.0/3); 
        #ifdef CALCULO
        printf("root B[%d] = A[%d] + A[%d] + A[%d]",i,i-1,i,i+1);
        #endif
        if (fabs(B[0]-B[i]) > precision){
            
            printf("Master ERROR CONVERGENCIA pos %d B[%d]= %f, B[%d]=%f \n",i,0,B[0],i,B[i]);
            converge = 0;
            i++;
            break;
        }
    }
    //Calculo el promedio de los numeros que me faltaron antes de descubrir la divergencia
    for (;i<bloque;i++){
        B[i] = (A[i-1] + A[i] + A[i+1])* (1.0/3);
    }

    #ifdef PRINT_VEC
        printf("MASTER iteracion %d :",numIteraciones);
        for(int i=0;i<bloque+1;i++){
            printf("%.5f ",B[i]);
        }
        printf("\n");
        sleep(3);
    #endif

  	#ifdef PRUEBA        
    printf("root fin de procesamiento iteracion:%d\n",numIteraciones);
    sleep(3);
    #endif
    
    // Chequeo convergencia global
  	MPI_Allreduce(&converge, &convergeG, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

    #ifdef PRUEBA        
    printf("root converge: %d\n",convergeG);
    sleep(3);
    #endif

    if(!convergeG){
        swapAux = A;
        A = B;
        B = swapAux;
        numIteraciones++;
    }


  }
    printf("Iteraciones: %d\n", numIteraciones);

    printf("Tiempo en segundos: %f\n", dwalltime() - timetick);

    free(A);
    free(B);
    
}

void funcionSlave(int tid, int N, int nrProcesos) {
	DATA_T *A, *B, *swapAux;
	int bloque = (N / nrProcesos) ;
	int converge,i,convergeG = 0, numIteracion=0;
	DATA_T data0;

    #ifdef PRUEBA
	printf("Proceso %d\n", tid);
    sleep(3);
    #endif

	// Aloca memoria para los vectores
	A = (DATA_T *) malloc(sizeof(DATA_T) * bloque + 2 - (tid == nrProcesos-1) );
	B = (DATA_T *) malloc(sizeof(DATA_T) * bloque + 2 - (tid == nrProcesos-1) );

    // Recibir el bloque
    MPI_Scatter(&A[1], 0, MPI_DATA_T, &A[1], bloque, MPI_DATA_T, 0, MPI_COMM_WORLD);

    #ifdef PRINT_VEC
        printf("Scatter Hilo%d :",tid);
        for(int i=0;i<bloque + 2 - (tid == nrProcesos-1);i++){
            printf("%.5f ",A[i]);
        }
        printf("\n");
        sleep(3);
    #endif

    #ifdef PRUEBA
    printf("%d: Recibi mi vector\n", tid);
    sleep(3);
    #endif

    while (!convergeG ) {
        // Recibo B[0] en data0
        MPI_Bcast(&data0, 1, MPI_DATA_T, 0, MPI_COMM_WORLD);
        
        MPI_Request request;
		MPI_Status status;

        if (tid != nrProcesos-1) {
			// envio mis valores a los vecinos
			MPI_Isend(&A[1], 1, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Isend(&A[bloque], 1, MPI_DATA_T, tid+1, 1, MPI_COMM_WORLD, &request);

			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, 1, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
			// recibo el valor del vecino derecho
			MPI_Irecv(&A[bloque+1], 1, MPI_DATA_T, tid+1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);

		} else {

			MPI_Isend(&A[1], 1, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, 1, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
		}

        #ifdef PRUEBA        
        printf("Hilo%d recibo B[0]\n",tid);
        sleep(3);
        #endif

        #ifdef PRINT_VEC
        printf("VecA Hilo%d :",tid);
        for(int i=0;i<bloque + 2 - (tid == nrProcesos-1);i++){
            printf("%.5f ",A[i]);
        }
        printf("\n");
        sleep(3);
        #endif

        converge = 1;
        // calculo Promedio y convergencia
        for(i=1;i<bloque+1 - (tid == nrProcesos-1) ;i++){
			B[i] = (A[i-1] + A[i] + A[i+1]) * (1.0/3); 
			if (fabs(data0-B[i]) > precision){
                printf("Hilo%d ERROR CONVERGENCIA pos %d data0= %f, B[%d]=%f\n",tid,i,data0,i,B[i]);
				converge = 0;
				i++;
				break;
			}
		}
		//Calculo el promedio de los numeros que me faltaron antes de descubrir la divergencia
		for (;i<bloque+1 - (tid == nrProcesos-1) ;i++){
			B[i] = (A[i-1] + A[i] + A[i+1])* (1.0/3);
		}


        if(tid == nrProcesos-1){
            B[bloque] = (A[bloque-1]+A[bloque]) *0.5;
			if (converge && fabs(data0-B[i])>precision){
				converge = 0;
			}
        }

        #ifdef PRUEBA        
        printf("Hilo%d finalizo calculo\n",tid);
        sleep(3);
        #endif

        #ifdef PRINT_VEC
        printf("VecB Hilo%d iteracion %d :",tid,numIteracion);
        for(int i=0;i<bloque + 2 - (tid == nrProcesos-1);i++){
            printf("%.5f ",B[i]);
        }
        printf("\n ");
        #endif

        // Chequeo de convergencia global
        MPI_Allreduce(&converge, &convergeG, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

        #ifdef PRINT_VEC
        if (tid == 1)
            printf("\n\n -------------------------- \n ");
        else
            sleep(2);
        #endif

        #ifdef CONVERGE
        printf("Hilo%d converge: %d convergeG: %d",tid,converge,convergeG);
        #endif

        #ifdef PRUEBA        
        printf("Hilo%d convergencia:%d\n",tid,convergeG);
        sleep(3);
        #endif

        if(!convergeG){
            //swap
            swapAux = A;
            A = B;
            B= swapAux;
            numIteracion++;
        }

	}

    free(A);
    free(B);
}

DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}
void printVector(int N, DATA_T *M){
	printf("Vector = [");
	for(int i= 0;i<N;i++){
		printf("%.8f, ",M[i]);
	}
	printf("]\n");
}