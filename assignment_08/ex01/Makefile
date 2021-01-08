GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic
OMP_FLAGS=-std=gnu99 -Wall -Wextra -pedantic -fopenmp -O3

.PHONEY: all
all: matmul_seq matmul_omp

matmul_seq: matmul_seq.c
	@$(GCC) $(GCC_FLAGS) $< -o matmul_seq

matmul_omp: matmul_omp.c
	@$(GCC) $(OMP_FLAGS) $< -o matmul_omp

.PHONEY: clean
clean:
	@rm matmul_seq
	@rm matmul_omp