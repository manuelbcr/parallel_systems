CC = mpicc
GCC = gcc
C_FLAGS = -std=c99 -Wall -Werror

.PHONEY: clean all

TARGETS = pi_seq pi_mpi

all: pi_seq pi_mpi

pi_seq: pi_seq.c
	$(GCC) $(C_FLAGS) $< -o pi_seq

pi_mpi: pi_mpi.c
	$(CC) $(C_FLAGS) $< -o pi_mpi

clean:
	$(RM) $(TARGETS)
