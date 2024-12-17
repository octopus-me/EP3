#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Função fornecida para calcular os valores RGB do pixel
int compute_julia_pixel(int x, int y, int width, int height, float tint_bias, unsigned char *rgb) {
    if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
        fprintf(stderr, "Coordenadas inválidas (%d,%d) para uma imagem de %d x %d\n", x, y, width, height);
        return -1;
    }

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

// Função fornecida para gravar o cabeçalho do BMP
int write_bmp_header(FILE *f, int width, int height) {
    unsigned int row_size_in_bytes = width * 3 + ((width * 3) % 4 == 0 ? 0 : (4 - (width * 3) % 4));

    char id[3] = "BM";
    unsigned int filesize = 54 + row_size_in_bytes * height;
    short reserved[2] = {0, 0};
    unsigned int offset = 54;

    unsigned int size = 40;
    unsigned short planes = 1;
    unsigned short bits = 24;
    unsigned int compression = 0;
    unsigned int image_size = row_size_in_bytes * height;
    int x_res = 0;
    int y_res = 0;
    unsigned int ncolors = 0;
    unsigned int importantcolors = 0;

    size_t ret = 0;
    ret += fwrite(id, sizeof(char), 2, f);
    ret += fwrite(&filesize, sizeof(int), 1, f);
    ret += fwrite(reserved, sizeof(short), 2, f);
    ret += fwrite(&offset, sizeof(int), 1, f);
    ret += fwrite(&size, sizeof(int), 1, f);
    ret += fwrite(&width, sizeof(int), 1, f);
    ret += fwrite(&height, sizeof(int), 1, f);
    ret += fwrite(&planes, sizeof(short), 1, f);
    ret += fwrite(&bits, sizeof(short), 1, f);
    ret += fwrite(&compression, sizeof(int), 1, f);
    ret += fwrite(&image_size, sizeof(int), 1, f);
    ret += fwrite(&x_res, sizeof(int), 1, f);
    ret += fwrite(&y_res, sizeof(int), 1, f);
    ret += fwrite(&ncolors, sizeof(int), 1, f);
    ret += fwrite(&importantcolors, sizeof(int), 1, f);

    return (ret != 17);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "n deve ser um inteiro positivo.\n");
        return 1;
    }

    int width = 2 * n;
    int height = n;

    unsigned char *pixels = (unsigned char *)malloc(3 * width * height * sizeof(unsigned char));
    if (!pixels) {
        fprintf(stderr, "Erro ao alocar memória para pixels.\n");
        return 1;
    }

    // Computa os valores dos pixels
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            compute_julia_pixel(x, y, width, height, 1.0, &pixels[(y * width + x) * 3]);
        }
    }

    // Salva a imagem em um arquivo BMP
    FILE *output_file = fopen("julia.bmp", "wb");
    if (!output_file) {
        fprintf(stderr, "Erro ao criar o arquivo julia.bmp\n");
        free(pixels);
        return 1;
    }

    // Escreve o cabeçalho
    if (write_bmp_header(output_file, width, height)) {
        fprintf(stderr, "Erro ao escrever o cabeçalho BMP.\n");
        fclose(output_file);
        free(pixels);
        return 1;
    }

    // Escreve os pixels
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fwrite(&pixels[(y * width + x) * 3], sizeof(unsigned char), 3, output_file);
        }
        unsigned char padding[3] = {0, 0, 0};
        fwrite(padding, sizeof(unsigned char), (width * 3) % 4, output_file);
    }

    fclose(output_file);
    free(pixels);
    printf("Imagem salva em julia.bmp\n");
    return 0;
}
