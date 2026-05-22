#include <errno.h>
#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

static int parse_local_size(const char *value, int *local_size) {
    char *endptr = NULL;
    long parsed_value;

    errno = 0;
    parsed_value = strtol(value, &endptr, 10);

    if (errno != 0 || endptr == value || *endptr != '\0' || parsed_value <= 0 || parsed_value > INT_MAX) {
        return 0;
    }

    *local_size = (int)parsed_value;
    return 1;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int local_size = 0;
    float *local_values = NULL;
    double local_sum = 0.0;
    double local_avg = 0.0;
    double global_sum = 0.0;
    int allocation_failed = 0;
    int any_allocation_failed = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2 || !parse_local_size(argv[1], &local_size)) {
        if (rank == 0) {
            fprintf(stderr, "Uso: mpirun -np <processos> ./media_mpi <N>\n");
            fprintf(stderr, "N deve ser um inteiro positivo.\n");
        }
        MPI_Finalize();
        return 1;
    }

    local_values = malloc((size_t)local_size * sizeof(float));
    allocation_failed = (local_values == NULL);
    MPI_Allreduce(&allocation_failed, &any_allocation_failed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    if (any_allocation_failed) {
        if (rank == 0) {
            fprintf(stderr, "Erro: nao foi possivel alocar o vetor local.\n");
        }
        free(local_values);
        MPI_Finalize();
        return 1;
    }

    srand((unsigned int)time(NULL) + (unsigned int)rank);

    for (int i = 0; i < local_size; i++) {
        local_values[i] = rand() / (float)RAND_MAX;
        local_sum += local_values[i];
    }

    local_avg = local_sum / local_size;

    wait_for_turn(rank);
    printf("[Processo %d] Soma local: %.3f, Media local: %.4f\n", rank, local_sum, local_avg);
    fflush(stdout);
    signal_next_rank(rank, size);

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double global_avg = global_sum / ((double)local_size * size);

        printf("\n[Soma global] %.3f\n", global_sum);
        printf("[Media global] %.4f\n", global_avg);
        fflush(stdout);
    }

    free(local_values);
    MPI_Finalize();
    return 0;
}
