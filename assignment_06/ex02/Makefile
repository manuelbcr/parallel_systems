GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic
OMP_FLAGS=-std=gnu99 -Wall -Wextra -pedantic -fopenmp -O3

.PHONEY: all
all: nBody2D_omp nBody2D_omp_plot 

nBody2D_omp: nBody2D_omp.c
	@$(GCC) $(OMP_FLAGS) $< -o nBody2D_omp -ffast-math -lm
	
nBody2D_omp_plot: nBody2D_omp.c
	@$(GCC) -c pbPlots.c -std=c99 -O3 -march=native
	@$(GCC) -c supportLib.c -std=c99 -O3 -march=native
	@$(GCC) -c nBody2D_omp.c -std=c99 -O3 -march=native -DPLOT
	@$(GCC) nBody2D_omp.o pbPlots.o supportLib.o -lm -o nBody2D_omp_plot 
	
.PHONEY: clean
clean:
	@rm nBody2D_omp
	@rm nBody2D_omp_plot
