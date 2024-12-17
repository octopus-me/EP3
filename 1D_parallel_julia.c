#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size, n;

    // Inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Validação do argumento
    if (argc != 2) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Erro: n deve ser um inteiro positivo.\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Cálculo da distribuição de linhas entre processos
    int lines_per_process = n / size;    // Número base de linhas por processo
    int remainder = n % size;            // Linhas extras a distribuir

    int start_row, end_row;

    if (rank < remainder) {
        // Processos com "linhas extras" pegam uma linha a mais
        start_row = rank * (lines_per_process + 1);
        end_row = start_row + lines_per_process;
    } else {
        // Processos restantes
        start_row = rank * lines_per_process + remainder;
        end_row = start_row + lines_per_process - 1;
    }

    int total_rows = end_row - start_row + 1;

    // Impressão da distribuição
    printf("[Process %d out of %d]: I should compute pixel rows %d to %d, for a total of %d rows\n",
           rank, size, start_row, end_row, total_rows);

    MPI_Finalize();
    return 0;
}
