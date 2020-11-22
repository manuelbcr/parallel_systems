# Assignment 4, due November 23rd 2020
Buchauer, De Sclavis, Handl

The goal of this assignment is to parallelize several applications with OpenMP.

## Exercise 1

### Tasks

- Use OpenMP to parallelize the Monte Carlo computation of π of Assignment 2.
- Measure the execution time for several problem sizes and for 1 to 8 threads.
- Illustrate the data in appropriate speedup/efficiency figures and discuss them. What can you observe? How does the performance compare to your MPI implementation of Assignment 2?

For this task we have measured the wall time from 10 up to 1.000.000.000 samples and as requested with 1 to 8 threads.
The results are given in the sheet ex01/measurements.xslx.

In our case the performance was significantly higher for the Open MP program.
For example running the MPI program with 4 ranks and 100.000.000 samples the execution time was about 5.577 seconds, however, the Open MP program with 4 threads took
only 0.850719 s and is orders of magnitudes faster. 

- Vary between the use of `atomic`, `critical`, and `reduction`, and vary between aggregating the intermediate `int` and final `float`/`double` data. What can you observe?

The best performance we achieved by aggregating the intermediate `int` value and use `reduction` and set the other variables to private.

If we use `critical` the execution time was not comparative. We also did not run tests for more than 2 rank for 1.000.000.000 samples because it just took more than 10 min. 

A similar, even tough not so bad, the performance also for `atomic` got worse. 
Also doing the reduction manually andleaving `reduction` out in the pragma causes very bad results.
Aggregating the final `double` value instead of the `int` value shows the same effect.

Especially it has to be mentioned that the performance decreased when the number of threads was increased for those cases. This is an effect we do not expect
while parallizing a program.

- Consider the performance-related topics discussed in the lecture so far. What optimization strategies would you choose and why?
In this task the data is not dependent and therefore no communication of intermediate results between threads is needed. So all of them can independently compute
their subresults. 
The workload is known a priori and hence it can be distributed balanced statically. 

- Add your best wall times for 10^8 samples for 1 and 8 threads in the comparison spreadsheet linked on Discord.

First we ran the code without optimization. After seeing that our times weren't that fast, we decided to compile with `-O3` for optimization.

## Exercise 2

### Tasks

- Use OpenMP to parallelize the 2D heat stencil simulation of Assignment 3.
- Measure the execution time for several problem sizes and for 1 to 8 threads.
- Illustrate the data in appropriate speedup/efficiency figures and discuss them. What can you observe? How does the performance compare to your MPI implementation of Assignment 3?

Again the measurements results can be obtained from the spreadsheet ex02/measurements.xslx.
We did the tests again with 1 up to 8 threads with problem sizes varying from N=50x50 up to N=400x400 with T=N*100 timesteps and in addition also with 1 to 8 threads
N=4000x4000 but only T=100 timesteps.  

If we compare the execution times with our MPI version of the previous assignment, each version has a setting where it outperforms the other one. For instance for 
smaller problem sizes N the Open MP version is mainly faster, especially when we increase the number of threads/ranks. Then Open MP ist significantly better performing 
than our MPI program, e.g. N=50x50 and 8 threads/ranks: Open MP =  0.0827s and MPI = 0.4419s.
On the other side, if the problem size N is increased beyond 200x200 MPI is up to 8 ranks performing better. For about N=100x100 and 4 threads/ranks both programs
are performing nearly identically well. 


- Consider the performance-related topics discussed in the lecture so far. What optimization strategies would you choose and why?
Considering the 2D heat_stencil problem 
For the 2D heat stencil problem one optimization that can be used is to used a even-odd send-receive pattern for sending ghost cells, if MPI is used. If the rank is always waiting for the previous one we get serial dependencies and a bigger overhead for waiting is introduced. This can be reduced if all even ranks are sending first and then all odd ranks are sending because it is ensured that in 2 runs everyone has sended and received. 

An additional strategy can be to seek for sending messages that fits in a lower level of memory hierarchy, e.g. instead of communicating only with the upper and lower neighbour, split the matrix into rectangles/ squares and send smaller portions of ghost cells to 2 neighbours instead one big line that potentially does not fit in the cache anymore to only one neighbour.

Another point that has to be taken into consideration is that not always increasing the number of ranks brings leads to better performances. If the problem size is rather small, say N=64x64 and it is ran on 64 ranks the communication overhead is immense and has no improvement in the performance. So the order of parallelizations should be appropriate to get most out of it.

- Add your best wall times for N=4000x4000 and T=100 for 1 and 8 threads in the comparison spreadsheet linked on Discord.

First we ran the code without optimization. After seeing that our times weren't that fast, we decided to compile with `-O3` for optimization.


### All measurments from both exercises can be found in the respective measurment.xls files.

## General Notes

All the material required by the tasks above (e.g. code, figures, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source code, please provide a makefile or other, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given problem, your solution, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute nodes, not on the login node.**
If you want to do some interactive experimentation, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.