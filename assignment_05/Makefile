GCC=gcc
GCC_FLAGS=-std=c99 -Wall -Wextra -pedantic

.PHONEY: all
all: nBody2D 

nBody2D: nBody2D.c
	@$(GCC) $(GCC_FLAGS) $< -o nBody2D -lm -O3 -ffast-math -D_DEFAULT_SOURCE 
	
.PHONEY: clean
clean:
	@rm nBody2D
