# Assignment 6, due December 07th 2020

Buchauer, De Sclavis, Handl

The goal of this assignment is to implement your parallelization and optimization plan of the n-body simulation of Assignment 5 and experiment with load imbalance.

## Exercise 1

### Tasks

- Provide an OpenMP implementation of your parallelization and optimization plan for the n-body simulation of Assignment 5.

    To parallelize the implementation of Assignment 5 we used a parallel for loop. As optimization strategy we have chosen to implement barnes-hut. To measure the impact of our optimization and parallelization seperately we implemented a sequential barnes-hut version and a parallel one, which was parallelized using OpenMP. <br>

- Measure the speedup and efficiency for multiple problem and machine sizes as in previous exercises. If your parallelization and optimization are orthogonal code modifications, try to measure the impact of your optimization separately.
- Illustrate the data in appropriate figures and discuss them. What can you observe?

     The first figure bellow visualizes the measured speedup for the sequential nBody 2D program from Assignment 5 and the parallelized version using OpenMP without any optimization. As it can be seen some significant speedup could be achieved however due to the complexity of the problem, a simple parallelization is not sufficient especially for big numbers of particles.<br>

    <img src="./ex01/img/speedup.png" alt="speedup" width="800"/> <br>

    The second image shows a walltime comparison of the nBody version of Assignment 5 and the optimized barnes hut version. In this image the naive approach of Assignment 5 is marked as "seq" and "8threads" where "seq_b" and "8threads_b" denote the walltimes for the barnes-hut implementation. As it can be seen the barnes hut implementation is significantly faster. 

    <img src="./ex01/img/comparison_barnes.png" alt="speedup" width="800"/> <br>

    To show the effect of the barnes-hut optimization compared to the naive version of the nBody implementation, we visualized the speedup of the barnes version in relation to the naive one. While just parallelizing the naive version has a speedup of aproximately 7.9 with 8 threads, the barnes-hut version indicates a speedup of 9354.3.      

    <img src="./ex01/img/speedup_barnes.png" alt="speedup" width="800"/>

- Did the implementation meet your expectations from Assignment 5?

    In terms of the parallelization of the naive version from Assignemnt 5, we expected the speedup to fall in the range of the results we achieved, as for example the 8 thread version yields a speedup of approximately 7.9. In terms of parallelization and optimization with barnes-hut, we expected an increase of speedup compared to the normal parallelization of the naive approach, however our expectations were exceeded by the results we achieved. 
    All the provided measurements and some more are provided in the excel spreadsheet of this Assignement.




- Add your best parallel wall time for 8 threads, 10000 particles, and 100 time steps into the comparison spreadsheet linked on Discord.

## Exercise 2

### Tasks

- Modify your n-body simulation to introduce spatial load imbalance.
- Try to mitigate your newly introduced load imbalance using any means discussed so far, and measure your efforts.
- Illustrate the data in appropriate figures and discuss them. What can you observe?

## General Notes

All the material required by the tasks above (e.g. code, figures, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source code, please provide a makefile or other, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given problem, your solution, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute nodes, not on the login node.**
If you want to do some interactive experimentation, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.