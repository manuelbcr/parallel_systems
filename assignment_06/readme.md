# Assignment 6, due December 07th 2020

Buchauer, De Sclavis, Handl

The goal of this assignment is to implement your parallelization and optimization plan of the n-body simulation of Assignment 5 and experiment with load imbalance.

## Exercise 1

### Tasks

- Provide an OpenMP implementation of your parallelization and optimization plan for the n-body simulation of Assignment 5.

We have done once the parallelization by using OpenMP and mainly by applying the pragma on the for loops, then we have implemented the optimization Barens Hut to gain 
satisfying speedup and bring the complexity from O(n^2) to O(n*log(n)).

- Measure the speedup and efficiency for multiple problem and machine sizes as in previous exercises. If your parallelization and optimization are orthogonal code modifications, try to measure the impact of your optimization separately.

- Illustrate the data in appropriate figures and discuss them. What can you observe? Did the implementation meet your expectations from Assignment 5?

The measurements are given in the spradsheet measurements_omp.xslx
As it can be seen already the parallelization brings an significant speedup for large problemsizes. However, for very small numbers of particles like 10. Up to 100 particles the parallelization overhead is decreasing and then it outperforms the sequential version. For our benchmarking setup of 10000 particles with 100 timesteps the OpenMP programm with 8 threads is even nearly 10 times faster.

The optimization with Barens Hut brought a huge impact. The runtime was reduced for instance for 1000 particles from about 310s down to 0.13sfor the sequential version. With 8 threads it was even achieved a best performance of about 0.033s.

- Add your best parallel wall time for 8 threads, 10000 particles, and 100 time steps into the comparison spreadsheet linked on Discord.
Our  best performance was achieved with -O3 and -ffast-math. We neededto compile with -O3 because otherwise we have got a segmentation fault on the LCC2. On our local machines it always worked perfectly: 0.033s
With -O2 and without -ffast-math on our local machines the runtime increased from about 0.02s to 0.3s in average.

We did expect that the runtime is improved significantly but such a huge impact was rather a surprise.

## Exercise 2

### Tasks

- Modify your n-body simulation to introduce spatial load imbalance.

We achieved load imbalancing by making sure that every second particle is the left-upper (=north-west) quarter. 

- Try to mitigate your newly introduced load imbalance using any means discussed so far, and measure your efforts.

We have tried out different scheduling strategies: static, dynamic, guided with chunksize 2, 4 and 8.

- Illustrate the data in appropriate figures and discuss them. What can you observe?

The results are provided in the spreadsheet measurements_load_imbalancing.xslx. We did all measurements with 10000 particles and 100 timesteps and 8 threads.
It can be seen, that the static decomposition does not deal that well with load imbalancing. Because we first put every second particle 
in the space [0-50, 0-50], then in the next setup [0-25, 0-25] and then [0-10, 0-10].
With increasing imbalancing the execution time increased from 0.037s up to 0.387s.
The dynamic strategy was worst in all cases but did not vary with increasing imbalancing: about 0.42s
The guided versions worked out the best in all cases. Especially the one with chunksize 8 had constantly a runtime of about 0.05s.

## General Notes

All the material required by the tasks above (e.g. code, figures, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source code, please provide a makefile or other, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given problem, your solution, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute nodes, not on the login node.**
If you want to do some interactive experimentation, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.