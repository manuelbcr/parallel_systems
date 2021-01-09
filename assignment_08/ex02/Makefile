GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic
OMP_FLAGS=-std=gnu99 -Wall -Wextra -pedantic -fopenmp -O3

.PHONEY: all
all: nQueens_seq nQueens_omp 

nQueens_seq: nQueens_seq.c
	@$(GCC) $(GCC_FLAGS) $< -o nQueens_seq

nQueens_omp: nQueens_omp.c
	@$(GCC) $(OMP_FLAGS) $< -o nQueens_omp

.PHONEY: clean
clean:
	@rm nQueens_seq
