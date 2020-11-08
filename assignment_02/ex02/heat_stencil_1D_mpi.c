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

  // initialize A with 273K
  for (int i = 0; i < N; i++) {
    A[i] = 273; 
  }

  // and there is a heat source in one corner
  int source_x = N / 4;
  A[source_x] = 273 + 60;


  // setup of parallel part
  int rank, size;
  int root_proc = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // initialize displacements of sub arrays for gathering subresults
  int* displs = (int *) malloc(sizeof(int)*size);
  // initialize array containing number of elements of each subresult array
  int* receive_counts = (int *) malloc(sizeof(int)*size);

  // set displacement and receive-counts for each rank
  for(int i=0; i<size; i++){
    int min = (i*N)/size;
    int max = (i == size-1) ? N : (i+1)*N/size;
    //displs[i] = (i == 0) ? 0 : min+1;
    displs[i] = min;
    receive_counts[i] = max-min+1;
  }

  // get min and max index of A for which current rank is responsible
  int min_index = (rank == root_proc) ? 0 : (rank*N)/size;
  int max_index = (rank == size-1) ? N : (rank+1)*N/size;

  printf("I am rank #%d and I serve [%d, %d] from [0, %d]\n", rank, min_index, max_index, N-1);
 
  if(rank == root_proc){
    printf("Initial:\t");
    printTemperature(A, N);
    printf("\n");
  }

  // sub_array contains the subresult of each rank (similar to B) 
  Vector sub_array = createVector(max_index-min_index);
  for(int timestep=0; timestep<T; timestep++){
    // Broadcast A such that all have same data again
    MPI_Bcast(A, N, MPI_DOUBLE, root_proc, MPI_COMM_WORLD);

    // index_A contains corresponding index to i in A
    // sub_array has range [0, max_index-min-index] <-> A has range [displs[rank], displs[rank]+max_index-min_index]
    int index_A = displs[rank];
    for(int i=0; i<max_index-min_index; i++){

      // heat source has always same heat
      if (index_A == source_x) {
        sub_array[i] = A[source_x];
        index_A++; 
        continue;
      }

      value_t tc = A[index_A];

      // get temperatures of adjacent cells
      value_t tl = (index_A != 0) ? A[index_A - 1] : tc;
      value_t tr = (index_A != N - 1) ? A[index_A + 1] : tc;
      
      // compute new temperature at current position
      sub_array[i] = tc + 0.2 * (tl + tr + (-2 * tc));

      index_A++;
    }
  

    // end some testing ---------------
    // gather subresults again and write them to A
    MPI_Gatherv(sub_array, max_index-min_index, MPI_DOUBLE, 
               A, receive_counts, displs, MPI_DOUBLE, 
               0, MPI_COMM_WORLD);

    if(rank == root_proc){
      if (!(timestep % 1000)) {
        printf("Step t=%d:\t", timestep);
        printTemperature(A, N);
        printf("\n");
      }
    }

  }

  // final print
  if(rank == root_proc){  
    printf("Final:\t\t");
    printTemperature(A, N);
    printf("\n");

    int success = 1;
    for (long long i = 0; i < N; i++) {
      value_t temp = A[i];
      if (273 <= temp && temp <= 273 + 60)
        continue;
      success = 0;
      break;
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");

    printf("[");
    for(int i=0; i<N; i++){
      printf("%f, ", A[i]);
    }
    printf("]\n");


  }

  // finalize by freeing vectors
  releaseVector(sub_array);
  MPI_Finalize();
  releaseVector(A);

  return 0;
}

Vector createVector(int N) {
  // create data and index vector
     return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) { free(m); }


void printTemperature(Vector m, int N) {
  const char *colors = " .-:=+*^X#%@";
  const int numColors = 12;

  // boundaries for temperature (for simplicity hard-coded)
  const value_t max = 273 + 30;
  const value_t min = 273 + 0;
  // set the 'render' resolution
  int W = RESOLUTION;

  // step size in each dimension
  int sW = N / W;
 
  // room
  // left wall
  printf("X");

  // actual room
  for (int i = 0; i < W; i++) {
    // get max temperature in this tile
    value_t max_t = 0;

    for (int x = sW * i; x < sW * i + sW; x++) {
      max_t = (max_t < m[x]) ? m[x] : max_t;
    }

    value_t temp = max_t;

    // pick the 'color'
    int c = ((temp - min) / (max - min)) * numColors;
    c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

    // print the average temperature
    printf("%c", colors[c]);
  }

  // right wall
  printf("X");
}


