#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define WIDTH_SCALE 2 // Largura da imagem será 2 * n

// Função fornecida para calcular os valores RGB de um pixel
int compute_julia_pixel(int x, int y, int width, int height, float tint_bias, unsigned char *rgb) {
    float X_MIN = -1.6, X_MAX = 1.6, Y_MIN = -0.9, Y_MAX = +0.9;
    float float_y = (Y_MAX - Y_MIN) * (float)y / height + Y_MIN;
    float float_x = (X_MAX - X_MIN) * (float)x / width + X_MIN;

    float julia_real = -0.79;
    float julia_img = 0.15;
    int max_iter = 300;

    float real = float_y, img = float_x;
    int num_iter = max_iter;
    while ((img * img + real * real < 2 * 2) && (num_iter > 0)) {
        float xtemp = img * img - real * real + julia_real;
        real = 2 * img * real + julia_img;
        img = xtemp;
        num_iter--;
    }

    float color_bias = (float)num_iter / max_iter;
    rgb[0] = (num_iter == 0 ? 200 : -500.0 * pow(tint_bias, 1.2) * pow(color_bias, 1.6));
    rgb[1] = (num_iter == 0 ? 100 : -255.0 * pow(color_bias, 0.3));
    rgb[2] = (num_iter == 0 ? 100 : 255 - 255.0 * pow(tint_bias, 1.2) * pow(color_bias, 3.0));

    return 0;
}

int main(int argc, char *argv[]) {
    int rank, size, n, start_row, end_row, total_rows;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        if (rank == 0) fprintf(stderr, "Erro: n deve ser um inteiro positivo.\n");
        MPI_Finalize();
        return 1;
    }

    int width = WIDTH_SCALE * n;
    int lines_per_process = n / size;
    int remainder = n % size;

    // Calcular as linhas atribuídas a cada processo
    if (rank < remainder) {
        start_row = rank * (lines_per_process + 1);
        end_row = start_row + lines_per_process;
    } else {
        start_row = rank * lines_per_process + remainder;
        end_row = start_row + lines_per_process - 1;
    }
    total_rows = end_row - start_row + 1;

    // Alocar espaço para os pixels locais
    unsigned char *local_pixels = (unsigned char *)malloc(3 * width * total_rows);
    if (!local_pixels) {
        fprintf(stderr, "Erro: Falha na alocação de memória\n");
        MPI_Finalize();
        return 1;
    }

    // Medir o tempo de cálculo
    start_time = MPI_Wtime();

    // Calcular os pixels locais
    for (int y = 0; y < total_rows; y++) {
        for (int x = 0; x < width; x++) {
            compute_julia_pixel(x, start_row + y, width, n, 1.0, &local_pixels[(y * width + x) * 3]);
        }
    }

    end_time = MPI_Wtime();

    // Imprimir o tempo gasto por cada processo
    printf("[Process %d out of %d]: Computed rows %d to %d (Total: %d rows). Time: %.6f seconds\n",
           rank, size, start_row, end_row, total_rows, end_time - start_time);

    free(local_pixels);
    MPI_Finalize();
    return 0;
}
