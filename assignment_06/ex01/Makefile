GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic

.PHONEY: all
all: nBody2D_omp 

nBody2D_omp: nBody2D_omp.c
	@$(GCC) $(GCC_FLAGS) $< -o nBody2D_omp -O3 -ffast-math -lm 
	
.PHONEY: clean
clean:
	@rm nBody2D_omp
