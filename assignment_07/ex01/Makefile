CC= mpicc
GCC=gcc
CC_FLAGS=-O2 -std=c99 -Wall -Wextra -pedantic

.PHONEY: all
all: heat_stencil_1D_seq heat_stencil_1D_mpi heat_stencil_checker

heat_stencil_1D_seq: heat_stencil_1D_seq.c
	@$(GCC) $(CC_FLAGS) $< -o heat_stencil_1D_seq

heat_stencil_1D_mpi: heat_stencil_1D_mpi.c
	@$(CC) $(CC_FLAGS) $< -o heat_stencil_1D_mpi

heat_stencil_checker: heat_stencil_checker.c
	@$(GCC) $(CC_FLAGS) $< -o heat_stencil_checker
	

.PHONEY: clean
clean:
	@rm heat_stencil_1D_seq
	@rm heat_stencil_1D_mpi
	@rm heat_stencil_checker

.PHONEY: run
run: all
	@echo "1D-Sequential:"
	@./heat_stencil_1D_seq
	@echo "1D-Parallel:"
	@./heat_stencil_1D_mpi
	@echo "Heat stencil checker:"
	@./heat_stencil__checker
	@echos
