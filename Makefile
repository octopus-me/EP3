all: sequential_julia

sequential_julia: sequential_julia.c
	@gcc -O3 sequential_julia.c -o sequential_julia -lm

clean:
	@rm -f sequential_julia julia.bmp
