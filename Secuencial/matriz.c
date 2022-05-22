#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <sys/time.h>

#define DATA_T double
#define precision 0.01

//Para calcular tiempo
double dwalltime();
//para crear valores DATA_T
DATA_T randFP(DATA_T min, DATA_T max);

int main(int argc, char** argv) {
	int N = atoi(argv[1]);
	double timetick;


    //TODO: Validar recibir N antes de castear a atoi
    if(N<8){
        printf("N debe ser mayor a 8 (N=%d)",N);
        return 0;
    }


	DATA_T *A = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	DATA_T *B = (DATA_T*) malloc(sizeof(DATA_T)*N*N);
	DATA_T *swapAux;

	//Inicialización
	int i,j,f;
	for(i=0;i<N;i++) {
        f=i*N;
        for(j=0;j<N;j++){
            A[f+j] = randFP(0.0,1.0);
            #ifdef DEBUG
            printf("%.2f ",A[f+j]);
            #endif
        }
        #ifdef DEBUG
        printf("\n");
        #endif
	}
	#ifdef DEBUG
    printf("Llegue a aca\n");
    #endif


	//Cálculo del promedio
	int converge = 0,numIteracion= 0,inj;
	timetick = dwalltime();
	
    
    while (!converge){
	//while (numIteracion < 5){
    	numIteracion++;
		converge = 1;


        //Borde izquierdo superior
        B[0] = (A[0] + A[1] + A[N] + A[N+1]) * 0.25;
        //-------------------------------------

        #ifdef DEBUG
            printf("A[0]=%.2f ,B[0]=%.2f \n",A[0], B[0]);   

        #endif

        //Cálculo superior revisando convergencia
        for(j=1;j<N-1;j++){
            B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (0.1666666666666);

            //calculo de convergencia
            if (fabs(B[0]-B[j])>precision){
				#ifdef DEBUG
				printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge = 0;
				break;
			}
        }

        //calculo superior sin convergencia
        for(;j<N-1;j++){
            B[j]= (A[j] + A[j-1] + A [j+1] + A[N+j] + A[N+j-1] + A[N+j+1]) * (0.1666666666666);
        }


        //----------------------------------------

        #ifdef DEBUG
            printf("hice la parte superior\n");
        #endif


        //Borde derecho superior
        B[N-1] = (A[N-1] + A[N-2] + A[N+(N-1)] +  A[N+(N-2)]) * 0.25;
        //revisa convergencia Borde derecho superior
        if (converge && fabs(B[0]-B[N-1])>precision){
				#ifdef DEBUG
				printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge = 0;
		}
        //-------------------------------------

        #ifdef DEBUG
            printf("hice el borde derecho superior\n");
        #endif

        //Cálculo sin vertices checkeando convergencia 
        for(i=1;i<N-1 && converge;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            B[i*N]=(A[(i-1)*N]+A[(i-1)*N+1]+A[i*N]+A[i*N+1]+A[(i+1)*N]+A[(i+1)*N+1])/6;
            if (fabs(B[0]-B[i*N])>precision){
                #ifdef DEBUG
                printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                #endif
                converge = 0;
                break;
		    }
            for(j=1;j<N-1;j++){
                inj = i*N+j;
                B[inj]= (A[inj-N-1] + A[inj-N] + A[inj-N+1] 
                + A[inj-1]+ A[inj] + A[inj+1] 
                + A[inj+N-1] +A[inj+N] +A[inj+N+1])  * 0.1111111111111111;
                if (fabs(B[0]-B[inj])>precision){
                    #ifdef DEBUG
                    printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                    #endif
                    converge = 0;
                    break;
		        }
            }
            //Calculo último elemento de la fila (calculo de todas las últimas columnas)
            //j=N-1
            B[i*N+j]=(A[(i-1)*N-1+j]+A[(i-1)*N+j]+A[i*N-1+j]+A[i*N+j]+A[(i+1)*N-1+j]+A[(i+1)*N+j])/6;
            if (fabs(B[0]-B[i*N+j])>precision){
                #ifdef DEBUG
                printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
                #endif
                converge = 0;
		    }
        }


        //Cálculo sin vertices(el resto en caso de no convergir) sin checkeo de convergencia 
        for(;i<N-1 && converge;i++) {
            //Calculo primer elemento de la fila (calculo de todas las primeras columnas)
            B[i*N]=(A[(i-1)*N]+A[(i-1)*N+1]+A[i*N]+A[i*N+1]+A[(i+1)*N]+A[(i+1)*N+1])/6;
            for(;j<N-1;j++){
                inj = i*N+j;
                B[inj]= (A[inj-N-1] + A[inj-N] + A[inj-N+1] 
                + A[inj-1]+ A[inj] + A[inj+1] 
                + A[inj+N-1] +A[inj+N] +A[inj+N+1])  * 0.1111111111111111;
            }
            //Calculo último elemento de la fila (calculo de todas las últimas columnas)
            //j=N-1
            B[i*N+j]=(A[(i-1)*N-1+j]+A[(i-1)*N+j]+A[i*N-1+j]+A[i*N+j]+A[(i+1)*N-1+j]+A[(i+1)*N+j])/6;
            
            j=1; //esto se hace porque el for de arriba no tiene inicializado j
        }

        //----------------------------------------

        #ifdef DEBUG
            printf("hice la matriz sin vertices\n");
        #endif

		//Borde izquierdo inferior
        B[(N-1)*N] = (A[(N-2)*N] +  A[(N-2)*N+1] + A[(N-1)*N] + A[(N-1)*N+1]) * 0.25;

        if (converge && fabs(B[0]-B[(N-1)*N])>precision){
            #ifdef DEBUG
            printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
            #endif
            converge = 0;
        }
        //-------------------------------------
        
        #ifdef DEBUG
            printf("hice el borde izquierdo inferior\n");
        #endif

        //Cálculo inferior verificando convegencia
        i=N-1;
        for(j=1;j<N-1;j++){
            inj= i*N+j;

            B[inj] = A[inj-1] + A[inj] + A[inj+1] 
                    +A[inj-1-N] + A[inj-N] + A[inj+1-N];
            
            //calculo de convergencia
            if (fabs(B[0]-B[inj])>precision){
				#ifdef DEBUG
				printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
				#endif
				converge = 0;
				break;
			}
        }

        for(;j<N-1;j++){
            inj= i*N+j;

            B[inj] = A[inj-1] + A[inj] + A[inj+1] 
                    +A[inj-1-N] + A[inj-N] + A[inj+1-N];
        }
        //----------------------------------------

        #ifdef DEBUG
            printf("hice el calculo inferior\n");
        #endif

		//Borde derecho inferior, j=N-1
        B[N*N-1] = (A[N*N-2] + A[N*N-1] + A[N*j-2] +  A[N*j-1]) * 0.25;

        if (converge && fabs(B[0]-B[N*N-1])>precision){
            #ifdef DEBUG
            printf("B[0]-B[%d] = %.15f - B[0]=%.15f y B[%d]=%.15f\n",i,fabs(B[0]-B[i]),B[0],i,B[i]);
            #endif
            converge = 0;
        }
        //-------------------------------------
        
        #ifdef DEBUG
            //imprimir la matriz
            for(i=0;i<N;i++) {
                f=i*N;
                for(j=0;j<N;j++){
                    printf("%.2f ",B[f+j]);
                }
            printf("\n");
	        }
        #endif
	}



	printf("Tiempo en segundos %f, con %d iteraciones \n", dwalltime() - timetick,numIteracion);

    free(A);
    free(B);
	return 0;	
  }



double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

DATA_T randFP(DATA_T min, DATA_T max) {
  DATA_T range = (max - min);
  DATA_T div = RAND_MAX / range;
  return min + (rand() / div);
}