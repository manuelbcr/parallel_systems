CC=gcc
OMP_FLAGS= -fopenmp -g3

.PHONEY: all
all: real

SRC=$(wildcard *.c)

real: $(SRC)
	@$(CC) $(OMP_FLAGS) $(SRC) -o real -lm -O3

real_profiled: $(SRC)
	@$(CC) $(OMP_FLAGS) $(SRC) -o real -lm -pg

.PHONEY: clean
clean:
	@rm real

.PHONEY: run
run: all
	@echo "Sequential:"
	@./real
	@echo
