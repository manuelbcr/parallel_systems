#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

typedef double value_t;

#define RESOLUTION 120

// -- vector utilities --

typedef value_t *Vector;

Vector createVector(int N);

void releaseVector(Vector m);

void printTemperature(Vector m, int N);

// -- simulation code ---
// -- MPI version for heat stencil ---
int main(int argc, char **argv) {
   
  int N = 2000;
  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 500;

  Vector A = createVector(N);

  for (int i = 0; i < N; i++) {
    A[i] = 273; // temperature is 0° C everywhere (273 K)
  }

  int rank, size;
  int root_proc = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int min_index = (rank == root_proc) ? 0 : (rank*N)/size;
  int max_index = (rank == size-1) ? N-1 : (rank+1)*N/size;
  int receive_counts[8] = {250, 250, 250, 250, 250, 250, 250, 250};
  int* displs = (int *) malloc(sizeof(int)*size);
  displs[0] = 0;
  displs[1] = 250;
  displs[2] = 500;
  displs[3] = 750;
  displs[4] = 1000;
  displs[5] = 1250;
  displs[6] = 1500;
  displs[7] = 1750;


  printf("I am rank #%d and I serve [%d, %d] from [0, %d]\n", rank, min_index, max_index, N-1);
 
  Vector sub_array = createVector(max_index-min_index);

   
  for(int i=0; i<max_index-min_index; i++){
    sub_array[i] = i+rank*250;
  }
  
  MPI_Gatherv(sub_array, max_index-min_index, MPI_DOUBLE, 
             A, receive_counts, displs, MPI_DOUBLE, 
             0, MPI_COMM_WORLD);

  releaseVector(sub_array);

  MPI_Barrier(MPI_COMM_WORLD);
  if(rank == root_proc){
    printf("[");
    for(int i=0; i<N-1; i++){
      printf("%f, ", A[i]);
    }
    printf("]");
  }
  
  MPI_Finalize();
  releaseVector(A);

  return 0;
}

Vector createVector(int N) {
  // create data and index vector
     return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) { free(m); }

