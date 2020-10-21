GCC = gcc
C_FLAGS = -std=c99 -Wall -Werror

.PHONEY: clean

TARGETS = pi_seq

pi_seq: pi_seq.c
	$(GCC) $(C_FLAGS) $< -o pi_seq

clean:
	$(RM) $(TARGETS)