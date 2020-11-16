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

However, we had there some problems that this only worked for certain values. So if N=16 for example we have got correct results but for
N=32 no results. The reason for this was, that the edge planes were for the middle rank for even iterations full of zeros but for odd ones correct.
This was also only for one rank the case. The screenshots at /bug show the behaviour. As in bug1.jpg can be seen all values are 273 as expected.
Then in the first iteration from the bottom and top correct planes with full of 273 are received (bug_2.png).
But in 2nd iteration from the top a very strange plane was received as it van be obtained from bug_3.png. Interestingly, in the following iteration
(bug_3.png) it was correct again.

We have tried to debug it as intensively as possible but could not manage to solve it...
Hence, we have written 3D mpi the same way as 2D by linearization of the matrices. 

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

We have measured it as it can be obtained from the excelsheet ex02/measurements.xslx
We have observed strong scalability.

1D:
As it an be seen for small problem sizes the sequntial program is fairly compatitive because the overhaed of sending and receiving is higher than the parallization speeds it up.

For lager sizes > 1000 parallel is already twice as fast and this effects stronger and stronger.

2D:
Here the parallel execution is already for small problemsizes
significantly faster. When the problem sizes are getting bigger the speedup is tremenduous and for N=400 and 8 ranks it is even 180 

3D:
While testing 3D we had the problem that we could only test it for a problem size up to 16 because then the programm has to be aborted after 10 min. of execution. We expect that out implementation did not scale for some reason we couldn't determine while solving the exercise... 
