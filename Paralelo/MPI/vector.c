#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef USE_FLOAT
    #define DATA_T double
    #define MPI_DATA_T MPI_DOUBLE
#else
    #define DATA_T float
    #define MPI_DATA_T MPI_FLOAT
#endif

double dwalltime();
int funcionSlave(int, int, int);
int funcionDelMaster(int N, int P);


int main(int argc, char **argv)
{
    int miID;
    int nrProcesos;
    int N;
    double timetick;

    MPI_Init(&argc, &argv);                     //Inicializa el ambiente
    MPI_Comm_rank(MPI_COMM_WORLD, &miID);       // Obtiene el identificador de cada proceso (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcesos); // Obtiene el numero de procesos

    if(argc<3){
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

int funcionDelMaster(int N, int P)
{
    DATA_T *A, *B;
    A = (DATA_T *)malloc(sizeof(DATA_T) * N);
    B = (DATA_T *)malloc(sizeof(DATA_T) * N);
    int converge[T];
    //Inicialización del vector
	int i,start,end,converge=0;
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
	}
	#ifdef DEBUG
	printVector(N,A);
	#endif
    //Inicialización del converge
    for(i=0;i<P;i++){
        converge[i]=0;
    }

    timetick = dwalltime();
    while(!converge){
        for (i = 1; i < P; i++)
        {
            //*Enviar B[0]*
            MPI_Send(&B[0],MPI_DATA_T,i,99,MPI_COMM_WORLD); //revisar
            //*Enviar A para calcular*
            MPI_Send(&(A[i * (N * (N / P))]), N * (N / P), MPI_DATA_T, i, 99, MPI_COMM_WORLD);
        }
        start=1;
        end=N/P;
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

        for (i = 1; i < P; i++)
        {
            //Recibo parte de B calculada por slave
            MPI_Recv(B + (N * (N / P)) * i, N * (N / P), MPI_DOUBLE, i, 97, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //Recibo si le dió que converge
            //*hacer*
            printf("Llego bien el resultado\n");
        }

        //Revisar convergencia y swapear
    }
    printf("Tiempo en segundos %f\n", dwalltime() - timetick);
    free(A);
    free(B);

    return 0;
}

int funcionSlave(int id, int N, int P)
{
    DATA_T *A, *B, *C;
    A = (DATA_T *)malloc(sizeof(DATA_T) * N * (N / P));
    B = (DATA_T *)malloc(sizeof(DATA_T) * N * N);
    C = (DATA_T *)malloc(sizeof(DATA_T) * N * (N / P));
    int i, j, k;

    MPI_Recv(A, N * (N / P), MPI_DATA_T, 0, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Slave id: %d\n", id);

    MPI_Recv(B, N * N, MPI_DATA_T, 0, 98, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Llego bien el mensaje 2 (Y), id: %d\n", id);

    for (i = 0; i < N / P; i++)
    {
        for (j = 0; j < N; j++)
        {
            for (k = 0; k < N; k++)
            {
                C[i * N + j] += A[i * N + k] * B[k + j * N];
            }
        }
    }
    MPI_Send(C, N * (N / P), MPI_DATA_T, 0, 97, MPI_COMM_WORLD);
    printf("Termine hilo: %d\n", id);

    free(A);
    free(B);
    free(C);
    return 0;
}

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}