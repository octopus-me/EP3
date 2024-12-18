CC = gcc
MPICC = mpicc
CFLAGS = -O3
LDFLAGS = -lm

all: sequential_julia 1D_parallel_julia

sequential_julia: sequential_julia.c
	@$(CC) $(CFLAGS) sequential_julia.c -o sequential_julia $(LDFLAGS)

1D_parallel_julia: 1D_parallel_julia.c
	@$(MPICC) $(CFLAGS) 1D_parallel_julia.c -o 1D_parallel_julia

clean:
	@rm -f sequential_julia 1D_parallel_julia julia.bmp
