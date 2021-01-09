GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic
OMP_FLAGS=-std=gnu99 -Wall -Wextra -pedantic -fopenmp -O3

.PHONEY: all
all: nQueens_seq nQueens_omp nQueens_omptask 

nQueens_seq: nQueens_seq.c
	@$(GCC) $(GCC_FLAGS) $< -o nQueens_seq

nQueens_omp: nQueens_omp.c
	@$(GCC) $(OMP_FLAGS) $< -o nQueens_omp

nQueens_omptask: nQueens_omptask.c
	@$(GCC) $(OMP_FLAGS) $< -o nQueens_omptask

.PHONEY: clean
clean:
	@rm nQueens_seq
	@rm nQueens_omp
	@rm nQueens_omptask
