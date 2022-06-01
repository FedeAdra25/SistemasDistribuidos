#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/time.h>


#define MAX_ITERACIONES 100000
#define ROOT_P 0
#ifndef USE_FLOAT
    #define DATA_T double
    #define MPI_DATA_T MPI_DOUBLE
#else
    #define DATA_T float
    #define MPI_DATA_T MPI_FLOAT
#endif

#define precision 0.01

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

void printMatriz(int,int, DATA_T*);
DATA_T randFP(DATA_T min, DATA_T max);
int funcionSlave(int, int, int);
int funcionDelMaster(int, int);
int procesamientoMaster(DATA_T *A, DATA_T *B, int N, int filasCalculo);
int procesamientoSlave(DATA_T *A, DATA_T *B, int N, int filasCalculo, int tid,DATA_T data0,int nrProcesos);

int main(int argc, char **argv)
{
    int miID;
    int nrProcesos;
    int N;
    double timetick;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &miID);       // Obtiene el identificador de cada proceso (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcesos); // Obtiene el numero de procesos


    timetick = dwalltime();
    if (argc < 2) {
    printf("Error: debe enviar el tamaño del vector \nRecibido %d argumentos\n",
           argc);
    printf("Forma: ./out.o N (ej: ./out.o 256)\n");
    return 2;
    }
    N = atoi(argv[1]);
    if (N < 8) {
        printf("N debe ser mayor a 8 (N=%d)", N);
        return 0;
    }
    if (argc > 2 && miID==ROOT_P) {
        printf("%s\n", argv[2]);
    }

    N = atoi(argv[1]);

    if (miID == ROOT_P)
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

int funcionDelMaster(int N, int nrProcesos)
{
    DATA_T *A, *B, *swapAux,*originalA,*originalB;
	int tamBloque = (N * N) / nrProcesos;
	int converge, i,inj,numIteraciones = 0;
	int convergeG = 0;
	double timetick;

    // Aloca memoria para los vectores
	A = (DATA_T *) malloc(sizeof(DATA_T) * N * N);
	B = (DATA_T *) malloc(sizeof(DATA_T) * (tamBloque + N) );
    originalA=A;
    originalB=B;
    // Inicializacion de la matriz
	int j,f;
	for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            A[f+j] = randFP(0.0,1.0);
        }
	}

    #ifdef DEBUG
    printf("Matriz inicial: \n");
    printMatriz(N,N,A);
    #endif

    MPI_Barrier(MPI_COMM_WORLD); 

    timetick = dwalltime();

    // Enviar los bloques a cada proceso
    MPI_Scatter(A, tamBloque, MPI_DATA_T, A, tamBloque, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);
    while(!convergeG && numIteraciones<MAX_ITERACIONES){
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25; //Esquina izquierda superior B[0,0]
        
        //Envio el dato para chequear convergencia
   	    MPI_Bcast(B, 1, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);

        MPI_Request request;
	    MPI_Status status;

        //Envio valor a vecino derecho
        MPI_Isend(&A[tamBloque-N], N, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
        //Recibo el valor del vecino derecho
        MPI_Irecv(&A[tamBloque], N, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);

        //procesamiento
        converge = procesamientoMaster(A,B,N,N/nrProcesos);

        // Chequeo convergencia global
  	    MPI_Allreduce(&converge, &convergeG, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
        swapAux = A;
        A = B;
        B = swapAux;
        numIteraciones++;
        #ifdef DEBUG
        printf("Iteracion: %d. Matriz: \n",numIteraciones);
        printMatriz(N/nrProcesos,N,A);
        #endif
    }
    
    MPI_Gather(A,tamBloque,MPI_DATA_T,originalA,tamBloque,MPI_DATA_T,ROOT_P,MPI_COMM_WORLD);
    printf("Tiempo en segundos %f - Iteraciones: %d\n", dwalltime() - timetick,numIteraciones);
    
    #ifdef DEBUG
    printf("Final: %d. Matriz: \n",numIteraciones);
    printMatriz(N,N,originalA);
    #endif

    free(originalA);
    free(originalB);
    return 0;
}

int funcionSlave(int tid, int N, int nrProcesos) {
    DATA_T *A, *B, *swapAux;
	int cantFilas = N / nrProcesos;
	int i,numIteraciones=0;
	DATA_T data0;
    int *convergencias;

    // Aloca memoria para los vectores
	A = (DATA_T *) malloc(sizeof(DATA_T) * ((cantFilas+2) * N - N*(tid == nrProcesos-1)) );
	B = (DATA_T *) malloc(sizeof(DATA_T) * ((cantFilas+2) * N - N*(tid == nrProcesos-1)) );
    convergencias = (int *) malloc(sizeof(int) * 2 );

    MPI_Barrier(MPI_COMM_WORLD); 

    convergencias[1] = 0;
    // Recibir el bloque
    MPI_Scatter(NULL, 0, MPI_DATA_T, A+N, cantFilas*N, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);
    #ifdef DEBUG
    printf("Matriz inicial: \n");
    printMatriz(N/nrProcesos+1,N,A);
    #endif


    while (!convergencias[1] && numIteraciones<MAX_ITERACIONES) {
        // Recibo B[0] en data0
        MPI_Bcast(&data0, 1, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);

        
        MPI_Request request;
		MPI_Status status;

        if(tid != nrProcesos-1){
            // envio mis valores a los vecinos
			MPI_Isend(&A[N], N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Isend(&A[cantFilas*N], N, MPI_DATA_T, tid+1, 1, MPI_COMM_WORLD, &request);

			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
			// recibo el valor del vecino derecho
			MPI_Irecv(&A[cantFilas*N+N], N, MPI_DATA_T, tid+1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
        }else{
            
            MPI_Isend(&A[N], N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);

			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
        }

        #ifdef DEBUG
        if(numIteraciones==0){
            printf("Matriz inicial: \n");
            printMatriz(N/nrProcesos+2-(tid==(nrProcesos-1)),N,A);
        }
        #endif
        
        //Calculo el Promedio y convergencia
        convergencias[0] = procesamientoSlave(A, B, N, cantFilas - (tid==(nrProcesos-1)) , tid,data0,nrProcesos);
        // Chequeo convergencia global
  	    MPI_Allreduce(convergencias, &convergencias[1], 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);


        if(!convergencias[1]){
            //swap
            swapAux = A;
            A = B;
            B= swapAux;
            numIteraciones++;
        }
        #ifdef DEBUG
        printf("MATRIZ A ITERACION: %d\n",numIteraciones);
        printMatriz(N/nrProcesos,N,A+N);
        #endif
    }
    MPI_Gather(A+N,cantFilas*N,MPI_DATA_T,NULL,cantFilas*N,MPI_DATA_T,ROOT_P,MPI_COMM_WORLD);
    free(convergencias);
    free(A);
    free(B);
}

int procesamientoMaster(DATA_T *A, DATA_T *B, int N, int filasCalculo){
    int i,j,inj,converge;

    converge=1;
    //Cálculo fila 0 revisando convergencia: B[0,x]

    for(j=1;converge && j<N-1;j++){
        B[j]= (A[j-1] + A[j] + A [j+1] + A[N+j-1] + A[N+j] + A[N+j+1]) * (1.0/6);

        //Calculo convergencia
        if (fabs(B[0]-B[j])>precision){
            converge= 0;
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

    //------------------------


    for(i= 1;converge && i< filasCalculo;i++) {
        //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
        //B[i,0] = ...
        B[i*N]=(  A[(i-1)*N] + A[(i-1)*N+1]     //2 elems de fila anterior
            + A[i*N] + A[i*N+1]             //2 elems de fila actual
            + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
            ) * (1.0/6);                    //Divido por 6

        //Reviso su convergencia
        if (fabs(B[0]-B[i*N])>precision){
            converge = 0;
        }
        //Calculo de la parte central de la fila
        //B[i,1] hasta B[i,N-2]
        for(j=1;j<N-1;j++){
            inj = i*N+j;
            B[inj]=(  A[inj-N-1] + A[inj-N] + A[inj-N+1]    //3 elems de fila anterior
                    + A[inj-1]+ A[inj] + A[inj+1]           //3 elems de fila actual
                    + A[inj+N-1] +A[inj+N] +A[inj+N+1]      //3 elems de fila siguiente
                    ) * (1.0/9);                            //Divido por 9
            if (converge && fabs(B[0]-B[inj])>precision){
                converge = 0;
            }
        }
        //Calculo último elemento de la fila (calculo todas las últimas columnas)
        //B[i,N-1]
        B[i*N+N-1]= ( A[(i-1)*N-1+N-1] + A[(i-1)*N+N-1] //2 elems de fila anterior
                    + A[i*N-1+N-1] + A[i*N+N-1]         //2 elems de fila actual
                    + A[(i+1)*N-1+N-1] + A[(i+1)*N+N-1] //2 elems de fila siguiente
                    ) * (1.0/6.0);                      //Divido por 6
        //Verifico convergencia
        if (converge && fabs(B[0]-B[i*N+N-1])>precision){
            converge = 0;
        }
    }

    //Actualizo los valores que quedaron pendientes sin verificar convergencia
    for(;i<filasCalculo;i++) {
        //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
        //B[i,0] = ...
        B[i*N] = (A[(i-1)*N] + A[(i-1)*N+1] //2 elems de fila anterior
            + A[i*N] + A[i*N+1]             //2 elems de fila actual
            + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
            )*(1.0/6);                      //Divido por 6
        //Calculo de la parte central de la fila
        //B[i,1] hasta B[i,N-2]
        for(j=1;j<N-1;j++){
            B[i*N+j]=(  A[(i-1)*N+(j-1)] + A[(i-1)*N+j] + A[(i-1)*N+j+1]    //3 elems de fila anterior
                    + A[(i)*N+(j-1)]+ A[(i)*N+j] + A[(i)*N+j+1]             //3 elems de fila actual
                    + A[(i+1)*N+(j-1)] +A[(i+1)*N+j] +A[(i+1)*N+j+1]        //3 elems de fila siguiente
                    ) * (1.0/9);                                            //Divido por 9
        }
        //Calculo último elemento de la fila (calculo todas las últimas columnas)
        //B[i,N-1]
        B[i*N+j] = ( A[(i-1)*N-1+j] + A[(i-1)*N+j]  //2 elems de fila anterior
                    +A[i*N-1+j] + A[i*N+j]          //2 elems de fila actual
                    +A[(i+1)*N-1+j] + A[(i+1)*N+j]  //2 elems de fila siguiente
                    )*(1.0/6);                      //Divido por 6
    }
    return converge;
}

int procesamientoSlave(DATA_T *A, DATA_T *B, int N, int filasCalculo, int tid,DATA_T data0,int nrProcesos){
    int i,j,inj,converge;

    converge=1;
    for(i=1;converge && i<filasCalculo+1;i++) {
        //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
        //B[i,0] = ...
        B[i*N]=( A[(i-1)*N] + A[(i-1)*N+1]     //2 elems de fila anterior
            + A[i*N] + A[i*N+1]             //2 elems de fila actual
            + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
            ) * (1.0/6);                    //Divido por 6

        //Reviso su convergencia
        if (fabs(data0-B[i*N])>precision){
            converge = 0;
        }
        //Calculo de la parte central de la fila
        //B[i,1] hasta B[i,N-2]
        for(j=1;j<N-1;j++){
            inj = i*N+j;
            B[inj]=(  A[inj-N-1] + A[inj-N] + A[inj-N+1]    //3 elems de fila anterior
                    + A[inj-1]+ A[inj] + A[inj+1]           //3 elems de fila actual
                    + A[inj+N-1] +A[inj+N] +A[inj+N+1]      //3 elems de fila siguiente
                    ) * (1.0/9);                            //Divido por 9
            if (converge && fabs(data0-B[inj])>precision){
                converge = 0;
            }
        }
        //Calculo último elemento de la fila (calculo todas las últimas columnas)
        //B[i,N-1]
        B[i*N+N-1]= ( A[(i-1)*N-1+N-1] + A[(i-1)*N+N-1] //2 elems de fila anterior
                    + A[i*N-1+N-1] + A[i*N+N-1]         //2 elems de fila actual
                    + A[(i+1)*N-1+N-1] + A[(i+1)*N+N-1] //2 elems de fila siguiente
                    ) * (1.0/6.0);                      //Divido por 6
        //Verifico convergencia
        if (converge && fabs(data0-B[i*N+N-1])>precision){
            converge = 0;
        }
    }

    //Actualizo los valores que quedaron pendientes sin verificar convergencia
    for(;i<filasCalculo+1;i++) {
        //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
        //B[i,0] = ...
        B[i*N] = (A[(i-1)*N] + A[(i-1)*N+1] //2 elems de fila anterior
            + A[i*N] + A[i*N+1]             //2 elems de fila actual
            + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
            )*(1.0/6);                      //Divido por 6
        //Calculo de la parte central de la fila
        //B[i,1] hasta B[i,N-2]
        for(j=1;j<N-1;j++){
            B[i*N+j]=(  A[(i-1)*N+(j-1)] + A[(i-1)*N+j] + A[(i-1)*N+j+1]    //3 elems de fila anterior
                    + A[(i)*N+(j-1)]+ A[(i)*N+j] + A[(i)*N+j+1]             //3 elems de fila actual
                    + A[(i+1)*N+(j-1)] +A[(i+1)*N+j] +A[(i+1)*N+j+1]        //3 elems de fila siguiente
                    ) * (1.0/9);                                            //Divido por 9
        }
        //Calculo último elemento de la fila (calculo todas las últimas columnas)
        //B[i,N-1]
        B[i*N+j] = ( A[(i-1)*N-1+j] + A[(i-1)*N+j]  //2 elems de fila anterior
                    +A[i*N-1+j] + A[i*N+j]          //2 elems de fila actual
                    +A[(i+1)*N-1+j] + A[(i+1)*N+j]  //2 elems de fila siguiente
                    )*(1.0/6);                      //Divido por 6
    }

        
    if(tid==nrProcesos-1){
        filasCalculo++;
        //Calculo ULTIMA FILA
        //Primer elemento: Calculo esquina izquierda inferior B[N-1,0]
        B[(filasCalculo)*N] = (A[(filasCalculo-1)*N] +  A[(filasCalculo-1)*N+1] 
                + A[(filasCalculo)*N] + A[(filasCalculo)*N+1]
                ) * 0.25;
        //Verifico convergencia
        if (converge && fabs(data0-B[(filasCalculo)*N])>precision){
            converge = 0;
        }
        //Calculo ultima fila verificando convergencia
        //B[N-1,j]
        i=filasCalculo;
        for(j=1;converge && j<N-1;j++){
            inj= i*N+j;
            B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                    + A[inj-1-N] + A[inj-N] + A[inj+1-N]       //3 elems de fila anterior
                    ) * (1.0/6);                               //divido por 6
            //Calculo convergencia
            if (converge && fabs(data0-B[inj])>precision){
                converge = 0;
            }
        }
        //Calculo del resto de columnas sin verificar convergencia
        for(;j<N-1;j++){
            inj= i*N+j;
            B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                    + A[inj-1-N] + A[inj-N] + A[inj+1-N]       //3 elems de fila anterior
                    ) * (1.0/6);
        }
        //Ultimo elemento: Esquina derecha inferior, B[N-1,N-1]
        //j=N-1
        B[(filasCalculo)*N+(N-1)] = (A[(filasCalculo-1)*N+(N-2)] + A[(filasCalculo-1)*N+(N-1)] + A[(filasCalculo)*N+(N-2)] + A[(filasCalculo)*N+(N-1)])*0.25;
            //Verifico convergencia de ultimo elemento
        if (converge && fabs(data0-B[(filasCalculo)*N+(N-1)])>precision){
            converge = 0;
        }
    }
    return converge;
}
void printMatriz(int Ni,int Nj, DATA_T* M){
    int i,j,f,c;
    printf("\n[");
    for(int i=0;i<Ni;i++) {
        for (int j= 0;j<Nj;j++){
            printf("%.7f, ",M[i*Nj+j]);
        }
            printf("\n");
    }
    printf("]\n");
}

DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}