#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

int funcionSlave (int , int , int );
int funcionDelMaster (int, int);

int main(int argc, char** argv) {
    int miID;
	int nrProcesos;
	int N;
	double timetick;
	
	timetick = dwalltime();
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&miID); // Obtiene el identificador de cada proceso (rank)
	MPI_Comm_size(MPI_COMM_WORLD,&nrProcesos);  // Obtiene el numero de procesos
	N = atoi(argv[1]);


	if (miID == 0) {	
		funcionDelMaster(N, nrProcesos);

	} else {
		funcionSlave(miID, N, nrProcesos);
	}
	

	MPI_Finalize(); // Finaliza el ambiente MPI. No debe haber sentencias después
	printf("Tiempo en segundos %f\n", dwalltime() - timetick);
	return(0);
	
}

  int funcionDelMaster (int N, int P) {
	double *A, *B, *C;
	A = (double*) malloc(sizeof(double)*N*N);
	B = (double*) malloc(sizeof(double)*N*N);
	C = (double*) malloc(sizeof(double)*N*N);
	
	int i, j, k;
	for(i=0;i<N;i++) {
		for(j=0;j<N;j++)  {
			A[i*N+j] = 1;
			B[i+j*N] = 1;
	    		C[i*N+j] = 0;
		}
	}

	for (i = 1; i < P; i ++) {
		MPI_Send(&(A[i*(N*(N/P))]), N*(N/P), MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
		MPI_Send(B, N*N, MPI_DOUBLE, i, 98, MPI_COMM_WORLD);	
	}

	for  (i = 0; i < N/P; i++) {
		for (j = 0; j < N; j++) {
		    for (k = 0; k < N; k++) {
			 C[i*N + j] += A[i*N + k] * B[k + j*N];
		    }
		}
	}
	
	for (i = 1; i < P; i ++) {
		MPI_Recv(C + (N*(N/P))*i, N*(N/P), MPI_DOUBLE, i, 97, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Llego bien el resultado\n");			
	 
	}

	int check = 1; 
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
//		printf("%.2f ", C[i*N+j]);
	        check=check&&(C[i*N+j]==N);
        }
//	printf("\n");
    }   

    if(check){
        printf("Multiplicacion de matrices resultado correcto\n");
    } else {
        printf("Multiplicacion de matrices resultado erroneo\n");
    }
	free(A);
	free(B);
	free(C);

	return 0;
  }

  int funcionSlave (int id, int N, int P) {
	double *A, *B, *C;
	A = (double*) malloc(sizeof(double)*N*(N/P));
	B = (double*) malloc(sizeof(double)*N*N);
	C = (double*) malloc(sizeof(double)*N*(N/P));	
	int i, j, k;	
	
	MPI_Recv(A, N*(N/P), MPI_DOUBLE, 0, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("Slave id: %d\n", id);
	
	
	MPI_Recv(B, N*N, MPI_DOUBLE, 0, 98, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("Llego bien el mensaje 2 (Y), id: %d\n", id);
	 
	for  (i = 0; i < N/P; i++) {
		for (j = 0; j < N; j++) {
		    for (k = 0; k < N; k++) {
			 C[i*N + j] += A[i*N + k] * B[k + j*N];
		    }
		}
	}
	MPI_Send(C, N*(N/P), MPI_DOUBLE, 0, 97, MPI_COMM_WORLD);
	printf("Termine hilo: %d\n", id);
	
	free(A);
	free(B);
	free(C);
	return 0; 
  }
	

	