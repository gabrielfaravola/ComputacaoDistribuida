#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char *argv[]){

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = atoi(argv[1]);
    float vetor[N];
    float somaLocal = 0;
    float mediaLocal = 0;
    float somaGlobal = 0;
    srand(time(NULL) + rank);
    
    for(int i = 0; i < N; i++){
        vetor[i] = rand() / (float)RAND_MAX;        
        somaLocal += vetor[i]; 
    }

    mediaLocal = somaLocal/N;
    
    printf("[Processo %d] Soma local = %f   Media local = %f\n", rank, somaLocal, mediaLocal);
    
    MPI_Reduce(&somaLocal, &somaGlobal, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank == 0){
        float mediaGlobal = somaGlobal / (N * size);
        printf("\n[Soma global] %f\n", somaGlobal);
        printf("[Media global] %f\n", mediaGlobal);
    }

    MPI_Finalize();
    return 0;
}