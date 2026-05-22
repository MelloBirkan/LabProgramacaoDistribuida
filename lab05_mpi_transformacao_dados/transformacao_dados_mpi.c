#include <mpi.h>
#include <stdio.h>

#define DATA_SIZE 100
#define NUM_PROCS 5
#define CHUNK_SIZE (DATA_SIZE / NUM_PROCS)

static void print_vector(const char *label, const int vector[], int size) {
    printf("%s: [", label);
    for (int i = 0; i < size; i++) {
        printf("%d", vector[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main(int argc, char *argv[]) {
    int rank, size;
    int original_data[DATA_SIZE];
    int transformed_data[DATA_SIZE];
    int local_data[CHUNK_SIZE];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != NUM_PROCS) {
        if (rank == 0) {
            fprintf(stderr, "Erro: execute com exatamente %d processos.\n", NUM_PROCS);
            fprintf(stderr, "Exemplo: mpirun -np %d ./transformacao_dados_mpi\n", NUM_PROCS);
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        for (int i = 0; i < DATA_SIZE; i++) {
            original_data[i] = i + 1;
        }

        print_vector("[Processo 0] Vetor original", original_data, DATA_SIZE);
    }

    MPI_Scatter(original_data, CHUNK_SIZE, MPI_INT, local_data, CHUNK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < CHUNK_SIZE; i++) {
        local_data[i] = local_data[i] * local_data[i];
    }

    MPI_Gather(local_data, CHUNK_SIZE, MPI_INT, transformed_data, CHUNK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        print_vector("[Processo 0] Vetor transformado", transformed_data, DATA_SIZE);
    }

    MPI_Finalize();
    return 0;
}
