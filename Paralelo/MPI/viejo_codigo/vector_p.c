#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <sys/time.h>

#define N 512
#define precision 0.01

int procesamientoMaster(int tamBloque,double *A,double *B);
int procesamientoSlave(int tamBloque, double *A, double *B,int tid,int nrProcesos,double dato0);
void swap(double **x, double **y);
double dwalltime();


void funciondelmaster(int nrProcesos) {
	double *A, *B, *Vaux;
	int tamBloque = N / nrProcesos;
	int converge, iteraciones = 0;
	int convergeGlobal = 0;
	double timetick, aux;

	// Aloca memoria para los vectores
	A = (double *) malloc(sizeof(double) * N);
	B = (double *) malloc(sizeof(double) * (tamBloque +1));
	Vaux = A;
	// Inicializacion del arreglo
	for (int i = 0; i < N; i++)
	{
		A[i] = (double)rand() / (double)(RAND_MAX);
	}
	MPI_Barrier(MPI_COMM_WORLD); //evitar tiempo de alocamiento
	timetick = dwalltime();

	MPI_Scatter(A, tamBloque, MPI_DOUBLE, A, tamBloque, MPI_DOUBLE, 0, MPI_COMM_WORLD);


	while (!convergeGlobal) 
	{
		MPI_Request request;
		MPI_Status status;

		B[0] = (A[0] + A[1]) * 0.5;
	
		MPI_Bcast(B, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		MPI_Isend(A+tamBloque-1, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &request);
		//vecino derecho
		MPI_Irecv(A+tamBloque, 1, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		
		// Procesamiento y convergencia
		iteraciones++;
		converge = procesamientoMaster(tamBloque,A,B);

		// Chequeo de convergencia global
		MPI_Allreduce(&converge, &convergeGlobal, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

		swap(&A, &B);
	}

	//MPI_Gather(part, N/nProcs, MPI_CHAR, message, N/nProcs, MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Gather(A, tamBloque, MPI_DOUBLE, Vaux, tamBloque , MPI_DOUBLE, 0, MPI_COMM_WORLD); 

	printf("Tiempo en segundos: %f\n", dwalltime() - timetick);
  	printf("Iteraciones: %d\n",iteraciones);

  	free(A);
	free(B);
  
}

void funcionslave(int tid, int nrprocesos) {
	double *A, *B;
	int tamBloque = N / nrprocesos + 2 - (tid == nrprocesos-1);
	int converge, iteraciones = 0;
	int convergeGlobal = 0;
	double aux;

	A = (double *) malloc(sizeof(double) * tamBloque);
	B = (double *) malloc(sizeof(double) * tamBloque);


	MPI_Barrier(MPI_COMM_WORLD); //evitar tiempo de alocamiento

	MPI_Scatter(A+1, N / nrprocesos, MPI_DOUBLE, A+1, N / nrprocesos, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	while (!convergeGlobal) 
	{
		MPI_Request request;
		MPI_Status status;

		// Recibo V2[0] en aux
		MPI_Bcast(&aux, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		if (tid != nrprocesos-1) {
			// envio esquinas
			MPI_Isend(A+1, 1, MPI_DOUBLE, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Isend(A+tamBloque-2, 1, MPI_DOUBLE, tid+1, 1, MPI_COMM_WORLD, &request);

			// vecino izquierdo
			MPI_Irecv(A, 1, MPI_DOUBLE, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
			// vecino derecho
			MPI_Irecv(A+tamBloque-1, 1, MPI_DOUBLE, tid+1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);

		} else {
			MPI_Isend(A+1, 1, MPI_DOUBLE, tid-1, 1, MPI_COMM_WORLD, &request);
			// recibo el valor del vecino izquierdo
			MPI_Irecv(A, 1, MPI_DOUBLE, tid-1, 1, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
		}

		// Procesamiento y convergencia
   		converge= procesamientoSlave(tamBloque,A,B,tid,nrprocesos,aux);

		// convergencia global
		MPI_Allreduce(&converge, &convergeGlobal, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

		swap(&A, &B);
	}
	MPI_Gather(A+1, N / nrprocesos, MPI_DOUBLE, A, N / nrprocesos, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	free(A);
	free(B);

}


int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int tid, nrProcesos;
  MPI_Comm_rank(MPI_COMM_WORLD, &tid);
  MPI_Comm_size(MPI_COMM_WORLD, &nrProcesos);

  if (tid == 0) {
    funciondelmaster(nrProcesos);
  } else {
    funcionslave(tid, nrProcesos);
  }
 
  MPI_Finalize();
  
  return 0;
}

int procesamientoMaster(int tamBloque,double *A,double *B){
	int i,converge;
    converge = 1;
    for (i = 1; i < tamBloque; i++) {
      B[i] = (A[i - 1] + A[i] + A[i + 1]) * (1.0 / 3);
      if (fabs(B[0] - B[i]) > precision) {
        converge = 0;
        i++;
        break;
      }
    }
    // Calculo el promedio de los numeros que me faltaron
    for (; i < tamBloque; i++) {
      B[i] = (A[i - 1] + A[i] + A[i + 1]) * (1.0 / 3);
    }

	return converge;
}


int procesamientoSlave(int tamBloque, double *A, double *B,int tid,int nrProcesos,double data0){
	int i,converge=1;

	//Calculo promedio y convergencia
    for (i = 1; i < tamBloque -1 ; i++) {
      B[i] = (A[i - 1] + A[i] + A[i + 1]) * (1.0 / 3);
      if (fabs(data0 - B[i]) > precision) {
        converge = 0;
        i++;
        break;
      }
    }

    //Calculo el promedio de los numeros que me faltaron
    for (; i < tamBloque -1 ; i++) {
      B[i] = (A[i - 1] + A[i] + A[i + 1]) * (1.0 / 3);
    }
    
    //Calculo de ultimo elemento si soy el ultimo proceso
    if (tid == nrProcesos - 1) {
      B[tamBloque-1] = (A[tamBloque - 1] + A[tamBloque-2]) * 0.5;
      if (converge && fabs(data0 - B[i]) > precision) {
        converge = 0;
      }
    }

	return converge;
}

void swap(double **x, double **y)
{
	double *temp = *x;
	*x = *y;
	*y = temp;
}

double dwalltime()
{
	double sec;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	sec = tv.tv_sec + tv.tv_usec / 1000000.0;
	return sec;
}