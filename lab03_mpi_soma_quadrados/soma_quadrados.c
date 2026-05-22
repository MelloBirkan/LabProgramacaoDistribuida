#include <mpi.h>
#include <stdio.h>

#define N 40
#define NUM_PROCS 4
#define CHUNK (N / NUM_PROCS)
#define PRINT_TOKEN_TAG 100

static void wait_for_turn(int rank) {
    int token = 1;
    MPI_Status status;

    if (rank > 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, PRINT_TOKEN_TAG, MPI_COMM_WORLD, &status);
    }
}

static void signal_next_rank(int rank, int size) {
    int token = 1;

    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, PRINT_TOKEN_TAG, MPI_COMM_WORLD);
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int vector[N];
    int local_data[CHUNK];
    int local_sum = 0;
    int global_sum = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Fase 1: geracao do vetor e distribuicao via Scatter */
    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            vector[i] = i + 1;
        }
    }

    MPI_Scatter(vector, CHUNK, MPI_INT, local_data, CHUNK, MPI_INT, 0, MPI_COMM_WORLD);

    /* Impressao dos dados recebidos por cada processo */
    wait_for_turn(rank);
    printf("Processo %d recebeu:", rank);
    for (int i = 0; i < CHUNK; i++) {
        printf(" %d", local_data[i]);
    }
    printf("\n");
    fflush(stdout);
    signal_next_rank(rank, size);

    /* Fase 2: calculo da soma local dos quadrados */
    for (int i = 0; i < CHUNK; i++) {
        local_sum += local_data[i] * local_data[i];
    }

    /* Impressao das somas locais */
    wait_for_turn(rank);
    printf("Processo %d: soma local dos quadrados = %d\n", rank, local_sum);
    fflush(stdout);
    signal_next_rank(rank, size);

    /* Fase 3: reducao e validacao */
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int seq_sum = N * (N + 1) * (2 * N + 1) / 6;

        printf("\nSoma paralela dos quadrados     = %d\n", global_sum);
        printf("Soma sequencial esperada        = %d\n", seq_sum);
        printf("\nResultado: ");
        if (global_sum == seq_sum) {
            printf("Os valores conferem!\n");
        } else {
            printf("Os valores NAO conferem!\n");
        }
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
