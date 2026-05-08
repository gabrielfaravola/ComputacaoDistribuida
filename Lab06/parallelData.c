#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define DATA_SIZE 100

void local_func(int vector[], int data){
    for(int i = 0; i < data; i++){
        vector[i] = vector[i] * vector[i];
    }
}

int main(int argc, char *argv[]){
    
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int vector[DATA_SIZE];
    int new_vector[DATA_SIZE];
    int chunk = DATA_SIZE / size;
    int part[chunk];

    if(rank == 0){
        for(int i = 0; i < DATA_SIZE; i++){
            vector[i] = i+1;
        }
    }

    MPI_Scatter(vector, chunk, MPI_INT, part, chunk, MPI_INT, 0, MPI_COMM_WORLD);
    local_func(part, chunk);
    for(int i = 0; i < size; i++){

        MPI_Barrier(MPI_COMM_WORLD);

        if(rank == i){
            printf("[PROCESSO %d] ", rank);

            for(int j = 0; j < chunk; j++){
                printf("%d ", part[j]);
            }

            printf("\n");
            fflush(stdout);
        }
    }

    MPI_Gather(part, chunk, MPI_INT, new_vector, chunk, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(rank == 0){
        printf("\n---------- OLD VECTOR ----------\n");
        for(int i = 0; i < DATA_SIZE; i++){
            printf("%d ", vector[i]);
        }

        printf("\n");
        printf("\n---------- NEW VECTOR ----------\n");
        
        for(int i = 0; i < DATA_SIZE; i++){
            printf("%d ", new_vector[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}