# Assignment 3, due November 16th 2020
Buchauer Manuel, De Sclavis Davide, Handl Philip 

The goal of this assignment is to extend the heat stencil application and measure its performance.

## Exercise 1

This exercise consists in extending the heat stencil application of Assignment 2 to two and three dimensions.

### Tasks

- Extend the heat stencil application to the two- and three-dimensional cases and name them `heat_stencil_2D` and `heat_stencil_3D`.

For this we have created 6 programs:
* heat_stencil_1D_seq.c: sequential implementation of a 1D heat stencil
* heat_stencil_1D_mpi.c: parallel implementation using MPI
* heat_stencil_2D_seq.c: sequential implementation of a 2D heat stencil using 4 points (upper, lower, left, right) 
* heat_stencil_2D_mpi.c: parallel implementation of 2D case using MPI and again 4 points for computing temperature propagation
* heat_stencil_3D_seq.c: sequential implementation of a 3D heat stencil using 6 points (upper, lower, left, right, front, back)
* heat_stencil_3D_mpi.c: parallel implementation using MPI and again 6 points

We have used 2 different approaches for 2D and 3D to parallelize the heat stencil.
While 

For the 3D solution we have used derived datatypes to try to extract the edge cases, however, we had some problems with ghost cells while MPI_Gather.
For this we had again to serialize and deserialize the matrix A.

The probelm we faced was that matrix A serialized resulted into:
- - - - - - - - - - -
| 01 | 02 | 03 | 04 |
- - - - - - - - - - - 
| 05 | 06 | 07 | 08 |  ====> | 01 | 02 | 03 | 04 | 00 | 00 | 05 | 06 | 07 | 08 | 00 | 00 | 09 | 10 | 11 | 12 | 00 | 00 |
- - - - - - - - - - - 
| 09 | 10 | 11 | 12 |
- - - - - - - - - - -

So we managed to get rid of the | 00 | cells by serializing by "hand".

- Provide a sequential and an MPI implementation, and use MPI's virtual topologies and derived data types features for the latter
We have used in both cases (2D and 3D) a 1D cartesian topology. 
A multidimensional topology is according to us not useful because then only the number of ranks that have to communicate to is
increased and thus the overhead is increased. But with such a choice the performance is not improved.

- Run your programs with multiple problem and machine sizes.
Results are given below and in Exercise 2

- How can you verify the correctness of your applications?
We have check the correctness by pairwise comparing the cells of the sequential and parallel program.
For this we have written the program heat_stencil_checker.c that gets as input the two files that should be compared
All programs write the final output into a file _D-output-seq.dat or _D-output-mpi.dat respectively. 

The first line is the dimension N and all further lines are the i-th entry of the final output.
Since all corrsponding entries matched we assumend the program to be correct. 

## Exercise 2

This exercise consists in measuring all heat stencil variants (1D, 2D and 3D) to get a grasp of their performance behavior.

### Tasks

- Measure the speedup and efficiency of all three stencil codes for varying problem and machine sizes/mappings. Consider using strong scalability, weak scalability, or both. Justify your choice.
- Illustrate the data in appropriate figures and discuss them. What can you observe?
- Measure and illustrate an application throughput metric. What can you observe?

## General Notes

All the material required by the tasks above (e.g. code, figures, etc...) must be part of the solution that is handed in. Your experiments should be reproducible and comparable to your own measurements using the solution materials that you hand in. For source code, please provide a makefile or other, intuitive means of compiling with the required flags and settings.

**Every** member of your group must be able to explain the given problem, your solution, and possible findings. You may also need to answer detailed questions about any of these aspects.

**Please run any benchmarks or heavy CPU loads only on the compute nodes, not on the login node.**
If you want to do some interactive experimentation, use an *interactive job* as outlined in the tutorial. Make sure to stop any interactive jobs once you are done.
