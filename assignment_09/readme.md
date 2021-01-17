# Assignment 09, due January 18th 2021

The goal of this assignment is to parallelize an unknown application using profiling and OpenMP.

## Exercise 1

### Description

The file [real.tar.gz](real.tar.gz) contains a realistic implementation of a (simple) numerical algorithm. Imagine you are tasked with making this implementation faster by parallelizing it with OpenMP, without any further information.

### Tasks

- Familiarize yourself with the code. You are not required to look at every source line, but rather profile the code using the means discussed in the lecture and get a grasp on its computational hotspots and performance characteristics (computation-heavy, memory-heavy, etc.).

Some notes to the program:
* no commandline arguments, but setting flags with files, e.g. to enable timer the file `timer.flag` has to be created
* zero3(void *oz, int n1, int n2, int n3): this functions sets all values of `oz` to 0.0 in the range oz[0:n3][0:n2][0:n1]
  but it has to be mentioned that `oz` is 1D and is previously transformed to 3D
* bubble(double ten[][2], int j1[][2], int j2[][2], int j3[][2], int m, int ind): as the name let us assume this one does bubble sort into the direction `ind` (ascending for ind=1, descending otherwise)
* power(double a, int n): computes a to the power of n
* zran3(void *oz, int n1, int n2, int n3, int nx1, int ny1, int k): setting all values of `oz` to 0.0 and increment 10 random numbers and decrement 10 by 1
* comm3(void *ou, int n1, int n2, int n3, int kk): exchanging cells between borders of cube: |0|1|2|...|n-2|n-1| where 0 = n-2 and n-1 = 1; kk is never used and can be deleted from the signature
* rep_nrm(void *u, int n1, int n2, int n3, char *title, int kk): simply printing the results that were computed by norm2u3
* norm2u3(void *or, int n1, int n2, int n3, double *rnm2, double *rnmu, int nx, int ny, int nz): evaluates approximations to the L2 norm
* interp(void *oz, int mm1, int mm2, int mm3, void *ou, int n1, int n2, int n3, int k): adds the trilinear interpolation of the correction from the coarser grid to the current approximation:  u = u + Qu'
* rprj3(void *or, int m1k, int m2k, int m3k, void *os, int m1j, int m2j, int m3j, int k): projects onto the next coarser grid, using a trilinear Finite Element projection:  s = r' = P r
* resid(void *ou, void *ov, void *or, int n1, int n2, int n3, double a[4], int k): Computes the residual from ou
* psinv(void *or, void *ou, int n1, int n2, int n3, double c[4], int k): this applys smoothing by an approximate inverse working similar to convolution
* mg3P(double u[], double v[], double r[], double a[4], double c[4], int n1, int n2, int n3): multigrid V-cycle solver: 


With enabling the timer.flag and together with perf stat we get the following data:

SECTION   Time (secs)
benchmk :    3.441  (100.00%)
mg3P    :    2.585  ( 75.13%)
psinv   :    0.853  ( 24.78%)
resid   :    1.669  ( 48.50%)
--> mg-resid:    0.864  ( 25.12%)
rprj3   :    0.401  ( 11.66%)
interp  :    0.352  ( 10.23%)
norm2   :    0.051  (  1.49%)
comm3   :    0.080  (  2.31%)

          4.404,56 msec task-clock                #    1,000 CPUs utilized          
                14      context-switches          #    0,003 K/sec                  
                 0      cpu-migrations            #    0,000 K/sec                  
           110.583      page-faults               #    0,025 M/sec                  
    14.273.630.340      cycles                    #    3,241 GHz                    
    32.685.242.882      instructions              #    2,29  insn per cycle         
     1.717.809.025      branches                  #  390,007 M/sec                  
        14.190.424      branch-misses             #    0,83% of all branches 

5,007853840 seconds time elapsed

4,867632000 seconds user
0,139989000 seconds sys

So we can conclude that the section `benchmk` is the most computationally heavy one, which is clear because it comprises the whole benchmarking, followed by `mg3P` and `resid`. So for those three sections the biggest potential is given to improve the overall performance.
Far the majority of the execution time is spend in user mode, so we can conclude that there are less idle states and time spend to wait for I/O operations.

A more detailed view is got with gprof:
```console
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  ms/call  ms/call  name    
 39.77      1.67     1.67      147    11.36    13.89  resid
 22.15      2.60     0.93   131072     0.01     0.01  vranlc
 17.86      3.35     0.75      168     4.47     6.65  psinv
  8.10      3.69     0.34      147     2.31     4.50  rprj3
  7.15      3.99     0.30      147     2.04     2.04  interp
  3.10      4.12     0.13      485     0.27     2.19  showall
  1.91      4.20     0.08                             norm2u3
  0.00      4.20     0.00   131642     0.00     0.00  randlc
  0.00      4.20     0.00     2332     0.00     0.00  wtime_
  0.00      4.20     0.00     1123     0.00     0.00  timer_start
  0.00      4.20     0.00     1119     0.00     0.00  timer_stop
```

From this table we can derive which functions were called the most times and which ones took the most time.
For instance `vranlc` required overall more than 22% of the execution time but was at the same time with 131072 calls
very frequently triggered. On the other side `resid` for example took nearly 40% but was only 147 times executed. 
This means that `resid` is much more computational expensive and potentially easier to optimize than `vranlc`. 
From this table the most interesting functions are: `resid`, `psinv`, `rprj3` and `interp` because they have the highest 
execution times per call and contribute the most to the overall execution time.    


For checking the memory consumption we have used `valgrind`:
```console
==23078== HEAP SUMMARY:
==23078==     in use at exit: 0 bytes in 0 blocks
==23078==   total heap usage: 3 allocs, 3 frees, 1,968 bytes allocated
````

With only 3 allocations with a total amount of 1,968 bytes this programm does not allocate a lot of memory of the heap.

There are, however, 3 arrays of double with in the default case a size of of 19.704.488 values - which means 19704488*8 = 157.635.904 bytes.
There are 3 of such arrays and this makes the program memory heavy.


- Investigate any loops that carry larger workloads and determine if and how they can be parallelized. Parallelize them with OpenMP. Ensure that any code modification does not violate program correctness with respect to its output.
  As mentioned above the functions `resid`, `psinv`, `rprj3` are most valuable to investigate on. Therefore we tried to parallelize parts of those functions.



- Benchmark the original, sequential program and your parallelized version for 1, 2, 4 and 8 threads on LCC2 and enter your results into the comparison spreadsheet linked on Discord..

  The measurments therefore can be found int the `measurments.xls`file. 
  We can see that execution for 4 threads works best for this example. 

## General Notes

All the material required by the tasks above (e.g. code, figures, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source code, please provide a makefile or other, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given problem, your solution, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute nodes, not on the login node.**
If you want to do some interactive experimentation, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.
