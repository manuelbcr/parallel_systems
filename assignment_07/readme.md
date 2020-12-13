# Assignment |7, due December 14th 2020

Buchauer, De Sclavis, Handl

The goal of this assignment is to explore shared memory MPI and hybrid parallelization.

## Exercise 1

### Tasks

- Implement a hybrid parallel version of any of the 1D/2D/3D heat stencil implementation|s, using MPI+OpenMP. For simplicit|y, you can assume periodic boundary conditions.
    The solution file heat_stencil_1D_mpi_omp.c is located in the folder ex01. 

- Vary your rank and thread setup (e.g. one rank per nod|e, one rank per CP|U, etc.). What performance effects did you expect and what effects can you observe? How does the performance compare to your MPI-only implementation? 
 
 - We executed our program in the following manner:
    - 1perhost8: so 1 rank on each node with a total of 8 ranks
        - 'mpiexec -n N ./heat_stencil_1D_mpi_omp 1000 NT' where N denotes to the number of processes and NT to the number of threads. 
    - 8perhost8: so 8 ranks on the same node
        - 'mpiexec -n N ./heat_stencil_1D_mpi_omp 1000 NT' where N denotes to the number of processes and NT to the number of threads. 
 
- The solutions can be seen in the output folder under measurments.xlsx
 
- The numbers for the mpi-only implementation from previous homework can be seen together with all the other execution times from our hybrid implementation in the table below. MPI only numbers are those from  1P mpi, 2P mpi, 3P mpi and 4P mpi. 

- You can clearly see that when the number of threads is increased, the execution times also go down in the 1perhost8 column. However if the ranks are located on one node, an increase in the number of thread doesn't result in a decrease in the execution time. For example with 4 processes the mpi-only version took 1,6 seconds and the hybrid version with 8 ranks on one node took: 
    - 2,2 seconds with 1 thread per rank
    - 3,6 seconds with 2 threads per rank
    - 5,2 seconds with 3 threads per rank 
    - 6,8 seconds with 4 threads per rank 

 We don't exactly know why this is happening.

What can also be clearly seen is that the mpi-only implementation took nearly the same amount of time regardless it was executed on 1perhost8 or 8perhost8.

| 	            |1perhost8|	8perhost8|
|---------------|---------|-----------|
|1P mpi	        |5,78133	|5,71018|
|1P2T mpi_omp	|3,995060	|3,99481|
|1P3T mpi_omp	|2,742630	|2,738310|
|1P4T mpi_omp	|2,186640	|2,195280|
|2P mpi	        |3,01723	|2,98674|
|2P1T mpi_omp	|4,034690	|4,118980|
|2P2T mpi_omp	|2,124360	|2,196710|
|3P mpi	        |2,08198	|2,057277|
|3P1T mpi_omp	|2,773810	|2,815360|
|3P2T mpi_omp	|1,46185    |4,22862|
|3P3T mpi_omp	|1,111400	|4,984310|
|4P mpi	        |1,64159	|1,58577|
|4P1T mpi_omp	|2,176860	|2,232800|
|4P2T mpi_omp	|1,17503	|3,63009|
|4P3T mpi_omp	|0,876593	|5,266070|
|4P4T mpi_omp	|0,840502	|6,839650|


- Add error handling to your code by checking all return values of MPI function calls.

Also the code for error handling is implemented in the file heat_stencil_1D_mpi_omp.c
We saved the return value of the MPI-function calls and reported them using our handleError() function, which basically only prints out the error and terminates all MPI-processes with MPI_Abort().

## Exercise 2

- Modify your implementation of exercise 1 using MPI one-sided shared memory communication instead of OpenMP for intra-node parallelism.
- Is there any performance difference to your implementation of exericse 1? How do the implementations compare in terms of effort?

## General Notes

All the material required by the tasks above (e.g. cod|e, figure|s, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source cod|e, please provide a makefile or othe|r, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given proble|m, your solutio|n, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute node|s, not on the login node.**
If you want to do some interactive experimentatio|n, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.
