#include <stdio.h>
#include <mpi.h>

#define N 40

int main(int argc, char *argv[]){
    MPI_Init(&argc, &argv);

    int rank;
    int size;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int chunk = N / size;

    int vetor[N];
    int parte[chunk];
    int soma = 0;

    if(rank == 0){
        for(int i = 0; i < N; i++){
            vetor[i] = i + 1;
        }
    }


    MPI_Scatter(vetor, chunk, MPI_INT, parte, chunk, MPI_INT, 0, MPI_COMM_WORLD);
    
    int somaLocal = 0;

    for(int i = 0; i < chunk; i++){
        somaLocal += parte[i] * parte[i];
    }
    
    for(int i = 0; i < size; i++){
        if(rank == i){
            printf("Processo %d recebeu: ", rank);
            
            for(int i = 0; i < chunk; i++){
                printf("%d ", parte[i]);
            }
            printf("\n");

            printf("Soma local = %d\n\n", somaLocal);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Reduce(&somaLocal, &soma, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    

    if (rank == 0){
        printf("---------- Resultados ----------\n");
        printf("Soma dos quadrados = %d\n", soma);
        printf("Resultado esperado = 22140\n\n");

        if(soma == 22140){
            printf("✅ Os valores conferem!\n");
        } else{
            printf("❌ Os valores não conferem!\n");
        }
    }

    MPI_Finalize();
    return 0;
}