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

int main(int argc, char **argv) {
  // 'parsing' optional input parameter = problem size
  int N = 2000;
  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 100;
  printf("Computing heat-distribution for room size N=%d for T=%d timesteps\n", N, T);

  // ---------- setup ----------

  // create a buffer for storing temperature fields
  Vector A = createVector(N);

  // set up initial conditions in A
  for (int i = 0; i < N; i++) {
    A[i] = 273; // temperature is 0° C everywhere (273 K)
  }

  // and there is a heat source in one corner
  int source_x = N / 4;
  A[source_x] = 273 + 60;

  // setup of parallel part
  int rank, rank_size;
  int root_proc = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &rank_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // calculate window size and initialize subarray
  int window_size = N/rank_size;
  Vector A_sub = createVector(window_size);

  // copy values into sub array for each rank
  for(int i=0; i < window_size; i++){
    A_sub[i] = A[i+rank*window_size];
  }

  printf("(RANK#%d): I am responsible for subpart: [%d, %d]\n", rank, rank*window_size, rank*window_size+window_size-1);

  // intial print of heat environment
  if(rank == root_proc){
    printf("Initial:\t");
    printTemperature(A, N);
    printf("\n");
  }

  // ---------- compute ----------
  // create a second buffer for the computation
  Vector B_sub = createVector(window_size);
  int min_index_in_A = rank*window_size;

  // for each time step ..
  for (int t = 0; t < T; t++) {

    // left and right border cell that came from other ranks 
    value_t edge_cell_l = A_sub[0];
    value_t edge_cell_r = A_sub[window_size-1];
    
    // first all even ranks are sending than all odd ones
    if(rank%2 == 0){
      if(rank > 0){
        MPI_Send(&A_sub[0], 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
        MPI_Recv(&edge_cell_l, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
      if(rank < rank_size-1){
        MPI_Send(&A_sub[window_size-1], 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
        MPI_Recv(&edge_cell_r, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }
    else{
      if(rank < rank_size-1){
        MPI_Recv(&edge_cell_r, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&A_sub[window_size-1], 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
      }
      if(rank > 0){
        MPI_Recv(&edge_cell_l, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&A_sub[0], 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
      }
    }

    // .. we propagate the temperature
    for (long long i = 0; i < window_size; i++) {
      // center stays constant (the heat is still on)
      if (i+min_index_in_A == source_x) {
        B_sub[i] = A_sub[i];
        continue;
      }

      // get temperature at current position
      value_t tc = A_sub[i];

      // get temperatures of adjacent cells
      value_t tl = (i != 0) ? A_sub[i - 1] : edge_cell_l;
      value_t tr = (i != window_size - 1) ? A_sub[i + 1] : edge_cell_r;

      // compute new temperature at current position
      B_sub[i] = tc + 0.2 * (tl + tr + (-2 * tc));
    }

    // swap matrices (just pointers, not content)
    Vector H_sub = A_sub;
    A_sub = B_sub;
    B_sub = H_sub;
  }

  // initialize displacements of sub arrays for gathering subresults
  int* displs = (int *) malloc(sizeof(int)*rank_size);
  // initialize array containing number of elements of each subresult array
  int* receive_counts = (int *) malloc(sizeof(int)*rank_size);
  
  // set displacement and receive-counts for each rank
  for(int i=0; i<rank_size; i++){
    int min = (i*N)/rank_size;
    int max = (i == rank_size-1) ? N : (i+1)*N/rank_size;
    displs[i] = min;
    receive_counts[i] = max-min+1;
  }
  
  // gather all end-sub-results for final output
  MPI_Gatherv(A_sub, window_size, MPI_DOUBLE, 
               A, receive_counts, displs, MPI_DOUBLE, 
               0, MPI_COMM_WORLD);

  releaseVector(B_sub);
  releaseVector(A_sub);

  // ---------- check ----------
  int success = 1;
  if(rank == root_proc){
    printf("Final:\t\t");
    printTemperature(A, N);
    printf("\n");
   
    for (long long i = 0; i < N; i++) {
      value_t temp = A[i];
      if (273 <= temp && temp <= 273 + 60)
        continue;
      success = 0;
      break;
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");
  }
  // ---------- cleanup ----------
  MPI_Finalize();
  releaseVector(A);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
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
