#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#define PI 3.1415926535897932384626433832795029L

double f(double a) 
{    
  return 4.0/(1.0+(a*a));   
}
int main(int argc, char *argv[]) {
int myrank, numproc;
 
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numproc);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
#pragma omp parallel for
for(int i=0; i<8; i++)
{
        int iam = omp_get_thread_num();
        printf("Hello World is Printed the %d-th time by Process %d and Threadid %d\n", i, myrank, iam);
}
 
MPI_Finalize();
return EXIT_SUCCESS;
}
