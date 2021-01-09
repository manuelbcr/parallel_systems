# Assignment 8, due January 11th 2021

Buchauer, De Sclavis, Handl


The goal of this assignment is to implement and optimize two types of dwarfs using OpenMP.

## Exercise 1

### Tasks

- Use OpenMP to develop a parallel matrix multiplication program. In order to facilitate verification:
    - initialize matrix A with `a(i,j) = i * j`,
    - make matrix B the identity matrix (`b(i,j) = (i == j) ? 1 : 0`), 
    - use `double` as the element type, and 
    - only consider the actual matrix multiplication for your time measurements, not the initialization or any verification.
- Measure the execution time of your OpenMP program for several matrix sizes and for 1 and 8 threads.
- Illustrate the data in appropriate speedup/efficiency figures and discuss them. What can you observe?
- Try to maximize the performance by considering all sequential and parallelism-related optimizations we discussed so far. Which did you choose and why?
- Add your best wall times for 1 and 8 threads for `N = 3000` into the comparison spreadsheet linked on Discord.

## Exercise 2

### Description

N-queens is a popular branch-and-bound problem. The goal is to compute the number of possible ways to place N chess queens on a NxN chess board without them attacking each other. See https://en.wikipedia.org/wiki/Eight_queens_puzzle for further information.

### Tasks

- Implement a sequential version of the n-queens problem. Benchmark your program for several problem sizes (e.g. 8, 10, and 12). What can you observe?

The sequential version can be found in ex02/nQueens_seq.c. We have used the backtracking algorithm with recursion. What was interesting to observe in the
execution times is that there is a strong exponential growth. for a problem size of 8 it was not really measureable, we have got for N = 10 about 0.012s, for
only 2 more with N=12 about 0.59s and with 13 already 3.6s. All the concrete numbers can be obtained from data/measurments_ex02.xslx.

- Parallelize your program using OpenMP. Which OpenMP constructs are suitable candidates and why?

The parallelized version can be found in nQueens_omp.c
Here, we need task parallelization instead of data parallelization. This means, the most important concept ist "#pragma omp task" that is used
for each recursion call "place_queen". Then each recursion call is added to the task queue and every time a thread is available it can poll from it.

- Benchmark your problem for several numbers of threads. What can you observe?

Again the results can be obtained from measurments_ex02.xslx.
We have measured for N = 13 the performance for 1 up to 8 threads. Interestingly, at the beginning an additional thread brings much speedup, however, for more threads the difference is marginal. The performance between 7 (0.1835126s) and 8 (0.1793696s) threads is nearly identical.

- Optional: Try to maximize the performance by considering all sequential and parallelism-related optimizations we discussed so far. Which did you choose and why?
- Add your best wall times for 1 and 8 threads for `N = 13` into the comparison spreadsheet linked on Discord.

Our best wall times were:
1 thread: 1.1350116
8 threads: 0.1793696s

## General Notes

All the material required by the tasks above (e.g. code, figures, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source code, please provide a makefile or other, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given problem, your solution, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute nodes, not on the login node.**
If you want to do some interactive experimentation, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.