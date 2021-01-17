CC=gcc
OMP_FLAGS= -fopenmp -g3 -O3

.PHONEY: all
all: real real_profiled

SRC=$(wildcard *.c)

real: $(SRC)
	@$(CC) $(OMP_FLAGS) $(SRC) -o real -lm

real_profiled: $(SRC)
	@$(CC) $(OMP_FLAGS) $(SRC) -o real_profiled -lm -pg -O3

.PHONEY: clean
clean:
	@rm real

.PHONEY: run
run: all
	@echo "Sequential:"
	@./real
	@./real_profiled
	@echo
