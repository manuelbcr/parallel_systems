GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic
OMP_FLAGS=-std=gnu99 -Wall -Wextra -pedantic -fopenmp

.PHONEY: all
all: pi_seq pi_openmp

pi_seq: pi_seq.c
	@$(GCC) $(GCC_FLAGS) $< -o pi_seq

pi_openmp: pi_openmp.c
	@$(GCC) $(OMP_FLAGS) $< -o pi_openmp

.PHONEY: clean
clean:
	@rm pi_seq
	@rm pi_openmp
