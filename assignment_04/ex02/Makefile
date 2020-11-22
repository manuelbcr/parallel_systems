GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic
OMP_FLAGS=-std=gnu99 -Wall -Wextra -pedantic -fopenmp -O3

.PHONEY: all
all: heat_stencil_2D_omp

heat_stencil_2D_omp: heat_stencil_2D_omp.c
	@$(GCC) $(OMP_FLAGS) $< -o heat_stencil_omp

.PHONEY: clean
clean:
	@rm heat_stencil_2D_omp
