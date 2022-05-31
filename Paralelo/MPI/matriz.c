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
int porcesamientoSlave(DATA_T *A, DATA_T *B, int N, int filasCalculo, int tid,int data0,int nrProcesos);

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
    if (argc > 2) {
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

    // printf("Tiempo en segundos %f\n", dwalltime() - timetick);
    MPI_Finalize(); // Finaliza el ambiente MPI. No debe haber sentencias después
    
    return (0);
}

int funcionDelMaster(int N, int nrProcesos)
{
    DATA_T *A, *B, *swapAux;
	int bloque = (N * N) / nrProcesos;
	int converge, i,inj,numIteraciones = 0;
	int convergeG = 0;
	double timetick;

    // Aloca memoria para los vectores
	A = (DATA_T *) malloc(sizeof(DATA_T) * N * N);
	B = (DATA_T *) malloc(sizeof(DATA_T) * (bloque + N) );   //sus filas a procesar mas una extra para el calculo de convergencia

    // Inicializacion de la matriz
	int j,f;
	for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            A[f+j] = randFP(0.0,1.0);
        }
	}
    #ifdef PRINT_MATRIZ
    printMatriz(N,N,A);
    sleep(3);
    #endif

    timetick = dwalltime();

    // Enviar los bloques a cada proceso
    MPI_Scatter(A, bloque, MPI_DATA_T, A, bloque, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);

    #ifdef PRINT_MATRIZ
        printf("Scatter MASTER:\n");
        printMatriz(N/nrProcesos+1,N,A);
        sleep(3);
    #endif

    while(!convergeG && numIteraciones< 1){
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25; //Esquina izquierda superior B[0,0]
        
        //Envio el dato para chequear convergencia
   	    MPI_Bcast(B, 1, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);

        MPI_Request request;
	    MPI_Status status;

        //Envio valor a vecino derecho
        MPI_Isend(&A[bloque-N], N, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
        //Recibo el valor del vecino derecho
        MPI_Irecv(&A[bloque], N, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);

        #ifdef PRINT_MATRIZ
        printf("MASTER postPasaje: \n");
        printMatriz(N/nrProcesos+1,N,A);
        sleep(3);
        #endif

        //procesamiento
        converge = procesamientoMaster(A,B,N,N/nrProcesos);

        #ifdef PROCE_MASTER
        printf("MASTER termino procesamiento iteracion:%d\n",numIteraciones);
        sleep(1);
        #endif

        #ifdef PRINT_MATRIZ
        printf("Postporcesamiento MASTER:\n");
        printMatriz(N/nrProcesos+1,N,A);
        sleep(3);
        #endif
        
        // Chequeo convergencia global
  	    MPI_Allreduce(&converge, &convergeG, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

        #ifdef CONVERGE
        printf("MASTER converge:%d convergeG: %d numIteracion: %d\n",converge,convergeG,numIteraciones);
        #endif

        //swapeo
        // if(!convergeG){
        // swapAux = A;
        // A = B;
        // B = swapAux;
        // numIteraciones++;
        // }

    }

    printf("Tiempo en segundos %f\n", dwalltime() - timetick);
    free(A);
    free(B);
    return 0;
}

int funcionSlave(int tid, int N, int nrProcesos) {
    DATA_T *A, *B, *swapAux;
	int bloque = ((N*N) / nrProcesos) ;
	int i,numIteracion=0;
	DATA_T data0;
    int *convergencias;

    // Aloca memoria para los vectores
	A = (DATA_T *) malloc(sizeof(DATA_T) * bloque + 2*N - N*(tid == nrProcesos-1) );
	B = (DATA_T *) malloc(sizeof(DATA_T) * bloque + 2*N - N*(tid == nrProcesos-1) );
    convergencias = (int *) malloc(sizeof(int) * 2 );   //pos0:local pos1:global

    convergencias[1] = 0; 
    // Recibir el bloque
    MPI_Scatter(NULL, 0, MPI_DATA_T, &A[N], bloque, MPI_DATA_T, ROOT_P, MPI_COMM_WORLD);

    #ifdef PRINT_MATRIZ
    sleep(tid);
    printf("Scatter Proces %d:\n",tid);
    printMatriz(N/nrProcesos+1,N,A);
    sleep(3);
    #endif

    while (!convergencias[1] ) {
        // Recibo B[0] en data0
        MPI_Bcast(&data0, 1, MPI_DATA_T, 0, MPI_COMM_WORLD);

        
        MPI_Request request;
		MPI_Status status;

        if(tid != nrProcesos-1){
            // envio mis valores a los vecinos
			MPI_Isend(&A[N], N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Isend(&A[bloque], N, MPI_DATA_T, tid+1, 1, MPI_COMM_WORLD, &request);

			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
			// recibo el valor del vecino derecho
			MPI_Irecv(&A[bloque+N], N, MPI_DATA_T, tid+1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
        }else{
            
            MPI_Isend(&A[N], N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);

			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, N, MPI_DATA_T, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
        }

        #ifdef PRINT_MATRIZ
        printf("Proces %d postPasaje:\n",tid);
        printMatriz(N/nrProcesos+1,N,A);
        sleep(3);
        #endif

        
        // calculo Promedio y convergencia
        convergencias[0] = porcesamientoSlave(A, B, N, (N/nrProcesos) - (tid==nrProcesos) , tid,data0,nrProcesos);

        #ifdef CONVERGE  
        printf("Hilo%d converge: %d conveergeG:%d iteracion:%d\n",tid,convergencias[0],convergencias[1],numIteracion);
        #endif
        
        #ifdef DEBUG
        printf("Proces%d termino procesamiento iteracion:%d\n",tid,numIteracion);
        sleep(1);
        #endif

        #ifdef PRINT_MATRIZ
        printf("Proces %d postCalculo:\n",tid);
        printMatriz(N/nrProcesos+1,N,A);
        sleep(3);
        #endif

        printf("intento hacer el converge");
        // Chequeo convergencia global
  	    MPI_Allreduce(convergencias, &convergencias[1], 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

        #ifdef CONVERGE  
        printf("Hilo%d converge: %d conveergeG:%d iteracion:%d\n",tid,convergencias[0],convergencias[1],numIteracion);
        #endif

        if(!convergencias[1]){
            //swap
            swapAux = A;
            A = B;
            B= swapAux;
            numIteracion++;
            printf("Hilo %d no deberia terminar iteracion: %d convergeG: %d\n",tid,numIteracion,convergencias[1]);
        }else{
            printf("Hilo %d deberia termino",tid);
            }
            

    }

    free(A);
    free(B);
}


void printMatriz(int Ni,int Nj, DATA_T* M){
    int i,j,f,c;
    printf("\n");
    for(int i=0;i<Ni;i++) {
        for (int j= 0;j<Nj;j++){
            printf("%.5f ",M[i*Nj+j]);
        }
            printf("\n");
    }
    printf("\n\n --------------------\n\n");
    printf("\n\n\n");
}

DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}

int procesamientoMaster(DATA_T *A, DATA_T *B, int N, int filasCalculo){
    int i,j,inj,converge;

    converge =1;
    //Cálculo fila 0 revisando convergencia: B[0,x]
    #ifdef PROCE_MASTER
    printf("MASTER comienzo a calcular el procesamiento\n");
    sleep(1);
    #endif


    for(j=1;j<N-1;j++){
        B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);

        #ifdef PROCE_MASTER
        printf("Calculo B[0,%d]\n",j);
        sleep(1);
        #endif

        //Calculo convergencia
        if (fabs(B[0]-B[j])>precision){
            #ifdef PROCE_MASTER
            printf("Divergio el valor B[0,%d]\n",j);
            sleep(1);
            #endif
            converge= 0;
            j++;
            break;
        }
    }
    //Calculo el resto de la fila 0 si entré al if de la convergencia en alguna iteración
    for(;j<N-1;j++){
        B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (1.0/6);
        #ifdef PROCE_MASTER
        printf("Calculo B[0,%d]\n",j);
        sleep(1);
        #endif
    }
    //Esquina derecha superior B[0,N-1]
    B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;

    #ifdef PROCE_MASTER
    printf("Calculo B[0,N-1]\n");
    sleep(1);
    #endif

    //Reviso convergencia esquina derecha superior
    if (converge && fabs(B[0]-B[N-1])>precision){
            #ifdef PROCE_MASTER
            printf("Divergio el valor B[0,%d]\n",j);
            sleep(1);
            #endif
            converge = 0;
    }
    
    #ifdef PROCE_MASTER
    printf("Finalizo calculo fila0 \n");
    sleep(1);
    #endif

    //------------------------


    for(i= 1;converge && i< filasCalculo;i++) {
        //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
        //B[i,0] = ...
        B[i*N]=(  A[(i-1)*N] + A[(i-1)*N+1]     //2 elems de fila anterior
            + A[i*N] + A[i*N+1]             //2 elems de fila actual
            + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
            ) * (1.0/6);                    //Divido por 6

        #ifdef PROCE_MASTER
        printf("Calculo B[%d,0]\n",i);
        sleep(1);
        #endif

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
            #ifdef PROCE_MASTER
            printf("Calculo B[%d,%d]\n",i,j);
            sleep(1);
            #endif
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
        #ifdef PROCE_MASTER
        printf("Calculo B[%d,%d]\n",i,N-1);
        sleep(1);
        #endif
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

        #ifdef PROCE_MASTER
        printf("Calculo B[%d,0]\n",i);
        sleep(1);
        #endif
        //Calculo de la parte central de la fila
        //B[i,1] hasta B[i,N-2]
        for(j=1;j<N-1;j++){
            B[i*N+j]=(  A[(i-1)*N+(j-1)] + A[(i-1)*N+j] + A[(i-1)*N+j+1]    //3 elems de fila anterior
                    + A[(i)*N+(j-1)]+ A[(i)*N+j] + A[(i)*N+j+1]             //3 elems de fila actual
                    + A[(i+1)*N+(j-1)] +A[(i+1)*N+j] +A[(i+1)*N+j+1]        //3 elems de fila siguiente
                    ) * (1.0/9);                                            //Divido por 9
            #ifdef PROCE_MASTER
            printf("Calculo B[%d,%d]\n",i,j);
            sleep(1);
            #endif
        }
        //Calculo último elemento de la fila (calculo todas las últimas columnas)
        //B[i,N-1]
        B[i*N+j] = ( A[(i-1)*N-1+j] + A[(i-1)*N+j]  //2 elems de fila anterior
                    +A[i*N-1+j] + A[i*N+j]          //2 elems de fila actual
                    +A[(i+1)*N-1+j] + A[(i+1)*N+j]  //2 elems de fila siguiente
                    )*(1.0/6);                      //Divido por 6
        #ifdef PROCE_MASTER
        printf("Calculo B[%d,%d]\n",i,j);
        sleep(1);
        #endif
    }

    #ifdef PROCE_MASTER
    printf("fin del calculoMaster\n");
    sleep(1);
    #endif

    return converge;
}

int porcesamientoSlave(DATA_T *A, DATA_T *B, int N, int filasCalculo, int tid,int data0,int nrProcesos){
    int i,j,inj,converge;

    converge =1;

    #ifdef PROCE_SLAVE
    printf("Proces %d comienza procesamiento\n",tid);
    sleep(1);
    #endif

    for(i= 1;converge && i<filasCalculo;i++) {
        //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
        //B[i,0] = ...
        B[i*N]=(  A[(i-1)*N] + A[(i-1)*N+1]     //2 elems de fila anterior
            + A[i*N] + A[i*N+1]             //2 elems de fila actual
            + A[(i+1)*N] + A[(i+1)*N+1]     //2 elems de fila siguiente
            ) * (1.0/6);                    //Divido por 6

        #ifdef PROCE_SLAVE
        printf("calculo B[%d,0]\n",i);
        sleep(1);
        #endif
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
            #ifdef PROCE_SLAVE
            printf("calculo B[%d,%d]\n",i,j);
            sleep(1);
            #endif
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
        #ifdef PROCE_SLAVE
        printf("calculo B[%d,%d]\n",i,N-1);
        sleep(1);
        #endif
        //Verifico convergencia
        if (converge && fabs(data0-B[i*N+N-1])>precision){
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
        #ifdef PROCE_SLAVE
        printf("calculo B[%d,0]\n",i);
        sleep(1);
        #endif
        //Calculo de la parte central de la fila
        //B[i,1] hasta B[i,N-2]
        for(j=1;j<N-1;j++){
            B[i*N+j]=(  A[(i-1)*N+(j-1)] + A[(i-1)*N+j] + A[(i-1)*N+j+1]    //3 elems de fila anterior
                    + A[(i)*N+(j-1)]+ A[(i)*N+j] + A[(i)*N+j+1]             //3 elems de fila actual
                    + A[(i+1)*N+(j-1)] +A[(i+1)*N+j] +A[(i+1)*N+j+1]        //3 elems de fila siguiente
                    ) * (1.0/9);                                            //Divido por 9
            #ifdef PROCE_SLAVE
            printf("calculo B[%d,%d]\n",i,j);
            sleep(1);
            #endif
        }
        //Calculo último elemento de la fila (calculo todas las últimas columnas)
        //B[i,N-1]
        B[i*N+j] = ( A[(i-1)*N-1+j] + A[(i-1)*N+j]  //2 elems de fila anterior
                    +A[i*N-1+j] + A[i*N+j]          //2 elems de fila actual
                    +A[(i+1)*N-1+j] + A[(i+1)*N+j]  //2 elems de fila siguiente
                    )*(1.0/6);                      //Divido por 6
        #ifdef PROCE_SLAVE
        printf("calculo B[%d,%d]\n",i,N-1);
        sleep(1);
        #endif
    }

        
        if(tid==nrProcesos-1){
            //Calculo ULTIMA FILA
            //Primer elemento: Calculo esquina izquierda inferior B[N-1,0]
            B[(filasCalculo)*N] = (A[(filasCalculo-1)*N] +  A[(filasCalculo-1)*N+1] 
                    + A[(filasCalculo)*N] + A[(filasCalculo)*N+1]
                    ) * 0.25;
            #ifdef PROCE_SLAVE
            printf("calculo B[%d,%d]\n",filasCalculo,0);
            sleep(1);
            #endif
            //Verifico convergencia
            if (converge && fabs(data0-B[(filasCalculo)*N])>precision){
                converge = 0;
            }
            //Calculo ultima fila verificando convergencia
            //B[N-1,j]
            i=filasCalculo;
            for(j=1;j<N-1;j++){
                inj= i*N+j;
                B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                     + A[inj-1-N] + A[inj-N] + A[inj+1-N]       //3 elems de fila anterior
                     ) * (1.0/6);                               //divido por 6
                #ifdef PROCE_SLAVE
                printf("calculo B[%d,%d]\n",i,j);
                sleep(1);
                #endif
                //Calculo convergencia
                if (fabs(data0-B[inj])>precision){
                    converge = 0;
                    j++;
                    break;
                }
            }
            //Calculo del resto de columnas sin verificar convergencia
            for(;j<N-1;j++){
                inj= i*N+j;
                B[inj] = ( A[inj-1] + A[inj] + A[inj+1]         //3 elems de fila actual
                     + A[inj-1-N] + A[inj-N] + A[inj+1-N]       //3 elems de fila anterior
                     ) * (1.0/6);
                #ifdef PROCE_SLAVE
                printf("calculo B[%d,%d]\n",i,j);
                sleep(1);
                #endif
            }
            //Ultimo elemento: Esquina derecha inferior, B[N-1,N-1]
            //j=N-1
            B[(filasCalculo)*N+(N-1)] = (A[(filasCalculo-1)*N+(N-2)] + A[(filasCalculo-1)*N+(N-1)] + A[(filasCalculo)*N+(N-2)] + A[(filasCalculo)*N+(N-1)])*0.25;
            #ifdef PROCE_SLAVE
            printf("calculo B[%d,%d]\n",filasCalculo,N-1);
            sleep(1);
            #endif
            //Verifico convergencia de ultimo elemento
            if (converge && fabs(data0-B[N*N-1])>precision){
                converge = 0;
            }
        }

    return converge;
}
