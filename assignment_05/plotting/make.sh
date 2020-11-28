gcc -c pbPlots.c -std=c99 -O3 -march=native
gcc -c supportLib.c -std=c99 -O3 -march=native
gcc -c nBody2D.c -std=c99 -O3 -march=native
gcc nBody2D.o pbPlots.o supportLib.o -lm -o nBody2D
strip nBody2D
./nBody2D


