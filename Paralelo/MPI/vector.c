#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


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

int funcionSlave(int, int, int);
int funcionDelMaster(int N, int P);

int main(int argc, char **argv)
{
    int miID;
    int nrProcesos;
    int N;

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

int funcion(int id, int N, int numP){
    DATA_T *A, *B;
    int *convergeP; //Esto aca porque si no no compila ;p
    double timetick; //Esto aca porque si no no compila ;p
    if(id==ROOT_P){
        A = (DATA_T *)malloc(sizeof(DATA_T) * N);
        convergeP = (int *)malloc(sizeof(int)*numP);
        double timetick;
        //Inicialización del vector
        int i,start,end,converge=0,offset;
        for(i=0;i<N;i++) {
            A[i] = randFP(0.0,1.0);
        }
        #ifdef DEBUG
        printVector(N,A);
        #endif
        //Inicialización del converge
        for(i=0;i<numP;i++){
            convergeP[i]=0;
        }
    }

    DATA_T *part = (DATA_T *)malloc(sizeof(DATA_T) * N/numP);
    DATA_T *partB = (DATA_T *)malloc(sizeof(DATA_T) * N/numP);
    DATA_T *swapAux;
    int ant,sig,start,end;
    int i,converge=0;
    DATA_T primerElem;
    start=id*N/numP + id==ROOT_P;
    end=(id+1)*N/numP - id==numP;
    MPI_Scatter(A,N/numP,MPI_DATA_T,part+1,N/numP,MPI_DATA_T,ROOT_P,MPI_COMM_WORLD);
    //Algoritmo, primero broadcast del primer elemento    
    if(id==ROOT_P){
        timetick = dwalltime(); //Arranco a contar.
        primerElem = (part[0]+part[1])*0.5;
        //BROADCAST DE PRIMERELEM!! HACER
    }
    else{
        //Recibo el primerElem
        //HACER
        //primerElem = ???
    }
    while(!converge){
        converge=1;
        //Enviamos su vecindad los threads (Cambiar por master enviando esto?)
        MPI_Send(part,1, MPI_DATA_T, id-1, 99, MPI_COMM_WORLD); //Envio mi primer elemento a thread anterior part[0]
        MPI_Send(part+(N/numP)-1,1, MPI_DATA_T, id+1, 99, MPI_COMM_WORLD); //Envio mi ultimo elemento a thread siguiente part[N/numP-1]
        MPI_Recv(&ant,1,MPI_DATA_T,id-1,99,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recibo el ultimo elemento del thread anterior
        MPI_Recv(&sig,1,MPI_DATA_T,id-1,99,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recibo el primer elemento del thread siguiente
        //part -> arreglo con mi parte
        //ant -> Elem ultimo de thread anterior
        //sig -> Elem primero de thrad siguiente


        //Calculo mi parte
        for(i=start;i<N/numP;i++){
            partB[i] = (part[i-1] + part[i] + part[i+1]) * (1.0/3); 
            if (fabs(primerElem-partB[i]) > precision){
                converge=0;
                break;
            }
        }
        //Calculo el promedio de los numeros que me faltaron antes de descubrir la divergencia
        for (;i<end;i++){
            partB[i] = (part[i-1] + part[i] + part[i+1])* (1.0/3);
        }
        if(id==numP){
            partB[N-1] = (part[N-2]+part[N-1]) *0.5;
			if (converge && fabs(primerElem-partB[i])>precision){
				converge = 0;
			}
        }
        if(id!=0){
            //Si no soy el master envio la booleana de convergencia y sigo
            MPI_Send(&converge,1,MPI_INT,ROOT_P, 99, MPI_COMM_WORLD); //Envio mi ultimo elemento a thread siguiente part[N/numP-1]
            MPI_Recv(&converge,1,MPI_INT,ROOT_P, 99,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recibo el ultimo elemento del thread anterior
            //primerElem = ???? RECIBO BROADCAST DE MASTER
            part=swapAux;
            part=partB;
            partB=swapAux;
            continue;
        }
        //Soy el master, verifico la convergencia
        for(i=1;i<numP;i++){ //i = id de proceso que recibo
            MPI_Recv(convergeP+i,1,MPI_INT,i, 99,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            converge=converge && convergeP[i];
        }
        //Ver de hacer un broadcast con el valor B[0] y converge
        primerElem = (part[0]+part[1])*0.5;
        //BROADCAST PRIMER ELEM
        //BROADCAST DE CONVERGE

    }
    MPI_Gather(part,N/numP,MPI_DATA_T,A,N/numP,MPI_DATA_T,0,MPI_COMM_WORLD);
    if(id==ROOT_P){
        printf("Tiempo en segundos %f\n", dwalltime() - timetick);
    }
    free(A);
    free(B);
    return 0;
}


/*
int funcionDelMaster(int N, int numP)
{
    DATA_T *A, *B;
    A = (DATA_T *)malloc(sizeof(DATA_T) * N);
    B = (DATA_T *)malloc(sizeof(DATA_T) * N);
    int converge2[numP];
    double timetick;

    //Inicialización del vector
	int i,start,end,converge=0,offset;
	for(i=0;i<N;i++) {
		A[i] = randFP(0.0,1.0);
	}
	#ifdef DEBUG
	printVector(N,A);
	#endif
    
    //Inicialización del converge
    for(i=0;i<numP;i++){
        converge2[i]=0;
    }


    //Algoritmo principal
    timetick = dwalltime();
    B[0] = (A[0]+A[1])*0.5; //Calculo primer elemento antes de entrar
    while(!converge){
        //Sin scatter
        // for (i = 1; i < numP; i++)
        // {
        //     offset = i*N/numP;
        //     //*Enviar B[0]*
        //     MPI_Send(&B[0],1,MPI_DATA_T,i,99,MPI_COMM_WORLD);
        //     //*Enviar A para calcular*
        //     MPI_Send(A+(i*N/numP), N / numP, MPI_DATA_T, i, 99, MPI_COMM_WORLD);
        // }

       MPI_Scatter(A,N/numP,MPI_DATA_T,B,N/numP,MPI_DATA_T,0,MPI_COMM_WORLD);
        start=1;
        end=N/numP;
        for(i=1;i<N/numP;i++){
            B[i] = (A[i-1] + A[i] + A[i+1]) * (1.0/3); 
            if (fabs(B[0]-B[i]) > precision){
                converge2[0] = 0;
                i++;
                break;
            }
        }
        //Calculo el promedio de los numeros que me faltaron antes de descubrir la divergencia
        for (;i<end;i++){
            B[i] = (A[i-1] + A[i] + A[i+1])* (1.0/3);
        }

        for (i = 1; i < numP; i++)
        {
            //Recibo parte de B calculada por slave
            MPI_Recv(B + (N * (N / numP)) * i, N * (N / numP), MPI_DOUBLE, i, 97, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
*/
/*
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
*/
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