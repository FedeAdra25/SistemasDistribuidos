#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ROOT_PID 0
#ifndef USE_FLOAT
#define DATA_T double
#define MPI_DATA_T MPI_DOUBLE
#else
#define DATA_T float
#define MPI_DATA_T MPI_FLOAT
#endif

#define precision 0.01

// Utils
double dwalltime();
void printVector(int N, DATA_T* M);
DATA_T randFP(DATA_T min, DATA_T max);

void funcionSlave(int, int, int);
void funcionDelMaster(int N, int P);

int main(int argc, char** argv) {
  int miID;
  int nrProcesos;
  int N;

  MPI_Init(&argc, &argv);  // Inicializa el ambiente
  MPI_Comm_rank(MPI_COMM_WORLD,
                &miID);  // Obtiene el identificador de cada proceso (rank)
  MPI_Comm_size(MPI_COMM_WORLD, &nrProcesos);  // Obtiene el numero de procesos

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

  if (miID == 0) {
    funcionDelMaster(N, nrProcesos);
  } else {
    funcionSlave(miID, N, nrProcesos);
    printf("Proceso %d sali de la funcion",miID);
  }

  MPI_Finalize();  // Finaliza el ambiente MPI. No debe haber sentencias después

  return (0);
}


void funcionDelMaster(int N, int nrProcesos) {
  //A guarda vector entero, B guarda el resultado de la iteracion actual
  DATA_T *A, *B, *swapAux,*originalA,*originalB; 
  int tamBloque = N / nrProcesos;
  int converge, numIteraciones = 0;
  int convergeG = 0;
  double timetick;

  // Aloca memoria para los vectores
  A = (DATA_T*)malloc(sizeof(DATA_T) * N);
  B = (DATA_T*)malloc(sizeof(DATA_T) * tamBloque + 1);

  // Inicializacion del arreglo
  for (int i = 0; i < N; i++) {
    A[i] = randFP(0.0, 1.0);
  }
  #ifdef DEBUG
  printVector(N,A);
  #endif

  //Variables para comunicacion
  MPI_Request request;
  MPI_Status status;
  originalA=A; //Esto para asegurarme en el gather que agarro A y no B (porque swapeo)
  originalB=B;
  timetick = dwalltime();
  // Enviar los bloques a cada proceso
  MPI_Scatter(A, tamBloque, MPI_DATA_T, //Pointer to data, length, type
              A, tamBloque, MPI_DATA_T, //Pointer to data received, length, type
              ROOT_PID, MPI_COMM_WORLD);

  while (!convergeG) {
    B[0] = (A[0] + A[1]) * 0.5;
    //Envio el dato B[0] para chequear convergencia
    //Broadcast(FROM,SIZE,TYPE,ROOT,COMM)
    MPI_Bcast(B, 1, MPI_DATA_T, ROOT_PID, MPI_COMM_WORLD);
   
    //Isend -> No bloqueante, guardo estado en request
    MPI_Isend(&A[tamBloque - 1], 1, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
    //Irecv -> No bloqueante, guardo estado en request
    //Recibo el valor del vecino derecho para calcular ultimo elemento
    MPI_Irecv(&A[tamBloque], 1, MPI_DATA_T, 1, 1, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status); //Espero que termine el receive (entiendo que el send se pisa)

    // Procesamiento
    int i;
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

    // Chequeo convergencia global
    MPI_Allreduce(&converge, &convergeG, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
    if (!convergeG) {
      swapAux = A;
      A = B;
      B = swapAux;
      numIteraciones++;
    }
  }


  #ifdef DEBUG
  printf("Proceso: %d - Envia: B+1=%p - val0: %.5f - A=%p - tamBloque=%d\n",0,B+1,B[1],A,tamBloque);
  #endif

  //Si convergeG=1 no hice swap
  printf("Iteraciones: %d\n", numIteraciones);
  MPI_Gather(B+1,tamBloque,MPI_DATA_T,originalA,tamBloque,MPI_DATA_T,ROOT_PID,MPI_COMM_WORLD);
  printf("Tiempo en segundos: %f\n", dwalltime() - timetick);
  
  printf("Resultado: \n");
  
  #ifdef DEBUG
  printVector(N,A);
  #endif

  #ifdef DEBUG
  printf("P: %d hace Free de %p y %p\nPunteros: A=%p,B=%p,swapaux=%p,tamBloque=%p,originalA=%p,originalB=%p,",0,A,B,A,B,swapAux,&tamBloque,originalA,originalB);
  printf("P: %d hace Free de %p y %p\n",0,A,B);
  #endif

  free(originalA);
  free(originalB);
}

void funcionSlave(int tid, int N, int nrProcesos) {
  DATA_T *A, *B, *swapAux;
  int tamBloque = N / nrProcesos;
  int *convergencias, i,numIteraciones=0;
  // int converge, convergeG = 0;
  DATA_T data0;

  // Aloca memoria para los vectores
  A = (DATA_T*)malloc(sizeof(DATA_T) * tamBloque + 2 - (tid == nrProcesos - 1));
  B = (DATA_T*)malloc(sizeof(DATA_T) * tamBloque + 2 - (tid == nrProcesos - 1));
  convergencias = (int*)malloc(sizeof(int) * 2);
  convergencias[1]= 0;

  //Datos para send y receive
  MPI_Request request;
  MPI_Status status;

  // Recibir el bloque
  MPI_Scatter(&A[1], 0, MPI_DATA_T,         //Pointer to data, length, type
              &A[1], tamBloque, MPI_DATA_T, //Pointer to data received, length, type
              ROOT_PID, MPI_COMM_WORLD);
  while (!convergencias[1]) {
    // Recibo B[0] en data0
    MPI_Bcast(&data0, 1, MPI_DATA_T, 0, MPI_COMM_WORLD);

    //Si no soy el último proceso
    if (tid != nrProcesos - 1) {
      //Envio mis valores a los vecinos
      MPI_Isend(&A[1], 1, MPI_DATA_T, tid - 1, 1, MPI_COMM_WORLD, &request);
      MPI_Isend(&A[tamBloque], 1, MPI_DATA_T, tid + 1, 1, MPI_COMM_WORLD,&request);

      //Recibo el valor del vecino izquierdo
      MPI_Irecv(A, 1, MPI_DATA_T, tid - 1, 1, MPI_COMM_WORLD, &request);
      MPI_Wait(&request, &status);

      //Recibo el valor del vecino derecho
      MPI_Irecv(&A[tamBloque + 1], 1, MPI_DATA_T, tid + 1, 1, MPI_COMM_WORLD,&request);
      MPI_Wait(&request, &status);

    }
    //Si soy el ultimo
    else {
      //Envio valor vecino izquierdo
      MPI_Isend(&A[1], 1, MPI_DATA_T, tid - 1, 1, MPI_COMM_WORLD, &request);
      //Recibo el valor del vecino izquierdo
      MPI_Irecv(A, 1, MPI_DATA_T, tid - 1, 1, MPI_COMM_WORLD, &request);
      MPI_Wait(&request, &status);
    }
    convergencias[0] = 1;

    //Calculo promedio y convergencia
    for (i = 1; i < tamBloque + 1 - (tid == nrProcesos - 1); i++) {
      B[i] = (A[i - 1] + A[i] + A[i + 1]) * (1.0 / 3);
      if (fabs(data0 - B[i]) > precision) {
        convergencias[0] = 0;
        i++;
        break;
      }
    }
    //Calculo el promedio de los numeros que me faltaron
    for (; i < tamBloque + 1 - (tid == nrProcesos - 1); i++) {
      B[i] = (A[i - 1] + A[i] + A[i + 1]) * (1.0 / 3);
    }
    
    //Calculo de ultimo elemento si soy el ultimo proceso
    if (tid == nrProcesos - 1) {
      B[tamBloque] = (A[tamBloque - 1] + A[tamBloque]) * 0.5;
      if (convergencias[0] && fabs(data0 - B[i]) > precision) {
        convergencias[0] = 0;
      }
    }
    // Chequeo de convergencia global
    MPI_Allreduce(&convergencias[0], &convergencias[1], 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

    #ifdef DEBUG
    printf("iteracion: %d convergenciaLocal: %d, convergenciaGlobal: %d\n",numIteraciones, convergencias[0],convergencias[1]);
    #endif

    if (!convergencias[1]) {
      numIteraciones++;
      swapAux = A;
      A = B;
      B = swapAux;
    }
  }
  #ifdef DEBUG
  printf("Proceso: %d - Envia: B+1=%p - val0: %.5f - A=%p - tamBloque=%d\n",tid,B+1,B[1],A,tamBloque);
  #endif

  //Si convergencias[1]=1, no hice el swap, envio datos finales
  MPI_Gather(B+1,tamBloque,MPI_DATA_T,A,0,MPI_DATA_T,ROOT_PID,MPI_COMM_WORLD);

  #ifdef DEBUG
  printf("P: %d hace Free de %p y %p\nPunteros: A=%p,B=%p,swapaux=%p,tamBloque=%p,data0=%p,converge=%p,",tid,A,B,A,B,swapAux,&tamBloque,&data0,&convergencias[0]);
  printVector(tamBloque,B+1);
  #endif

  printf("free convergencia\n");
  free(convergencias);
  printf("free convergencia reaalizada\n");
  printf("free A\n");
  free(A);
  printf("free A realizada\n");
  printf("free B \n");
  free(B);
  printf("free B realizada\n");
}

DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}

double dwalltime() {
  double sec;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  sec = tv.tv_sec + tv.tv_usec / 1000000.0;
  return sec;
}

void printVector(int N, DATA_T* M) {
  printf("Vector = [");
  for (int i = 0; i < N; i++) {
    printf("%.8f, ", M[i]);
  }
  printf("]\n");
}