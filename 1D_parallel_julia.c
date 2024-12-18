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
    rgb[0] = (unsigned char)(num_iter == 0 ? 200 : (-500.0 * pow(tint_bias, 1.2) * pow(color_bias, 1.6)));
    rgb[1] = (unsigned char)(num_iter == 0 ? 100 : (-255.0 * pow(color_bias, 0.3)));
    rgb[2] = (unsigned char)(num_iter == 0 ? 100 : (255 - 255.0 * pow(tint_bias, 1.2) * pow(color_bias, 3.0)));

    return 0;
}

void write_bmp_header(FILE *f, int width, int height) {
    unsigned int row_size = width * 3 + (width * 3) % 4; // Alinhamento a múltiplos de 4
    unsigned int filesize = 54 + row_size * height;

    fwrite("BM", 2, 1, f);
    fwrite(&filesize, 4, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
    unsigned int offset = 54;
    fwrite(&offset, 4, 1, f);

    unsigned int header_size = 40;
    unsigned short planes = 1, bits = 24;
    fwrite(&header_size, 4, 1, f);
    fwrite(&width, 4, 1, f);
    fwrite(&height, 4, 1, f);
    fwrite(&planes, 2, 1, f);
    fwrite(&bits, 2, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
    fwrite("\0\0\0\0", 4, 1, f);
}

int main(int argc, char *argv[]) {
    int rank, size, n, start_row, end_row, total_rows;

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

    // Calcular linhas atribuídas a cada processo
    if (rank < remainder) {
        start_row = rank * (lines_per_process + 1);
        end_row = start_row + lines_per_process;
    } else {
        start_row = rank * lines_per_process + remainder;
        end_row = start_row + lines_per_process - 1;
    }
    total_rows = end_row - start_row + 1;

    unsigned char *local_pixels = (unsigned char *)malloc(3 * width * total_rows);

    double start_time = MPI_Wtime();
    printf("Rank %d iniciando cálculo em %f\n", rank, start_time);
    fflush(stdout);

    // Calculo dos pixels locais
    for (int y = 0; y < total_rows; y++) {
        int global_y = start_row + y; 
        for (int x = 0; x < width; x++) {
            compute_julia_pixel(x, global_y, width, n, 1.0, &local_pixels[(y * width + x) * 3]);
        }
    }

    double end_time = MPI_Wtime();
    printf("Rank %d finalizando cálculo em %f (tempo de cálculo = %f s)\n", rank, end_time, end_time - start_time);
    fflush(stdout);

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        FILE *file = fopen("julia_parallel.bmp", "wb");
        write_bmp_header(file, width, n);
        fwrite(local_pixels, 3, width * total_rows, file);
        fclose(file);
        MPI_Send(NULL, 0, MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(NULL, 0, MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        FILE *file = fopen("julia_parallel.bmp", "ab");
        fwrite(local_pixels, 3, width * total_rows, file);
        fclose(file);
        if (rank < size - 1) {
            MPI_Send(NULL, 0, MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD);
        }
    }

    free(local_pixels);
    MPI_Finalize();
    return 0;
}
