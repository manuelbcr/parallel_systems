#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


typedef double value_t;

#define RESOLUTION 100

// -- Matrix utilities --

typedef value_t ***Matrix;

Matrix create3DMatrix(int N_x, int N_y, int N_z);

void releaseMatrix(Matrix m);

void printTemperature(Matrix m, int N);

void linearizeAsub(value_t* plane, value_t* A_sub_tmp, int N, int offset);

// -- simulation code ---

int main(int argc, char **argv){
  double start = MPI_Wtime();
  // 'parsing' optional input parameter = problem size
  int N = 200; // rows x columns

  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 100;
  printf("Computing heat-distribution for room size N=%dx%dx%d, for T=%d timesteps\n", N, N, N, T);

  // ---------- setup ----------

  // create a buffer for storing temperature fields
  Matrix A = create3DMatrix(N, N, N);

  // set up initial conditions in A
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      for(int k = 0; k < N; k++){
        A[i][j][k] = 273; // temperature is 0° C everywhere (273 K)
      }
    }
  }

  // start parallel execution
  MPI_Init(&argc, &argv);
  int rank, rank_size;
  // get number of ranks
  MPI_Comm_size(MPI_COMM_WORLD, &rank_size);

  // check if problem is dividable by 8
  if(N % rank_size != 0){
    printf("The problem of size N=%d is not divisable by the number of ranks=%d \n", N, rank_size);
    MPI_Finalize(); // cleanup
    return EXIT_FAILURE;
  }

  // define root process
  int root_proc = 0;
  // create virtual topology - 1D cartesian
  MPI_Comm stencil_comm;
  int dim[1] = {rank_size};
  int period[1] = {0};
  int reorder = 1;
  MPI_Cart_create(MPI_COMM_WORLD, 1, dim, period, reorder, &stencil_comm);
  MPI_Comm_rank(stencil_comm, &rank);

  // Determine top and bottom rank for each rank
  int top_rank;
  int bottom_rank;
  MPI_Cart_shift(stencil_comm, 0, 1, &bottom_rank, &top_rank);
 
  // and there is a heat source in one corner
  int source_x = 2;
  int source_y = 2;
  int source_z = 2;
  A[source_z][source_y][source_x] = 273 + 60;

  if(rank == root_proc){
    printf("Initial - heatsource is at [%d][%d][%d]:\t", source_z, source_y, source_x);
    printTemperature(A, N);
    printf("\n");
  }
  // ---------- compute ----------

  // create a second buffer for the computation
  int window_size = N/rank_size;
  value_t* A_sub = malloc(sizeof(value_t)*window_size*N*N);
  value_t* B_sub = malloc(sizeof(value_t)*window_size*N*N);

  // copy data into sub array for processing
  for(int i=0; i<window_size; i++){
    for(int j=0; j<N; j++){
      for(int k=0; k<N; k++){
        A_sub[i*N*N+j*N+k] = A[i+rank*window_size][j][k];
      }
    }
  }
  
  // if there has to be stored a new edge plane from another rank allocate memory
  value_t* edge_plane_bottom = malloc(sizeof(value_t)*N*N);
  value_t* edge_plane_top = malloc(sizeof(value_t)*N*N);

  // for each time step ..
  for (int t = 0; t < T; t++) {

    // first all even ranks are sending than all odd ones    
    // linearizeAsub brings the top/bottom plane of A_sub into the same format as sended/received
    if(rank%2 == 0){
      if(bottom_rank >= 0){
        MPI_Send(&A_sub[0], N*N, MPI_DOUBLE, bottom_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_bottom, N*N, MPI_DOUBLE, bottom_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        linearizeAsub(edge_plane_bottom, A_sub, N, 0);

      }
      if(top_rank >= 0){
        MPI_Send(&A_sub[(window_size-1)*N*N], N*N, MPI_DOUBLE, top_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_top, N*N, MPI_DOUBLE, top_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        linearizeAsub(edge_plane_top, A_sub, N, window_size-1);
      }
    }
    else{
      if(bottom_rank >= 0){
        MPI_Send(&A_sub[0], N*N, MPI_DOUBLE, bottom_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_bottom, N*N, MPI_DOUBLE, bottom_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        linearizeAsub(edge_plane_bottom, A_sub, N, 0);
      }
      if(top_rank >= 0){
        MPI_Send(&A_sub[(window_size-1)*N*N], N*N, MPI_DOUBLE, top_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_top, N*N, MPI_DOUBLE, top_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        linearizeAsub(edge_plane_top, A_sub, N, window_size-1);
      }
    }

    // .. we propagate the temperature
    for (long long i = 0; i < window_size; i++) {
      for (long long j = 0; j < N; j++) {
        for (long long k = 0; k < N; k++) {
          // center stays constant (the heat is still on)
          if (i+rank*window_size == source_z && j == source_y && k == source_x) {
            B_sub[i*N*N+j*N+k] = A_sub[i*N*N+j*N+k];
            continue;
          }

          // get temperature at current position
          value_t tc = A_sub[i*N*N+j*N+k];

          value_t t_back = (i > 0) ? A_sub[(i-1)*N*N+j*N+k] : edge_plane_bottom[j*N+k];
          value_t t_up = (j > 0) ? A_sub[i*N*N+(j-1)*N+k] : tc;
          value_t t_left = (k > 0) ? A_sub[i*N*N+j*N+k-1] : tc;
          value_t t_front = (i < window_size-1) ? A_sub[(i+1)*N*N+j*N+k] : edge_plane_top[j*N+k];
          value_t t_down = (j < N-1) ? A_sub[i*N*N+(j+1)*N+k] : tc;
          value_t t_right = (k < N-1) ? A_sub[i*N*N+j*N+k+1] : tc;

          // compute new temperature at current position
          B_sub[i*N*N+j*N+k] = (1.0/6.0) * (t_up + t_down + t_left + t_right + t_front + t_back);
        }
      }
    }

    // swap matrices (just pointers, not content)
    value_t* H_sub = A_sub;
    A_sub = B_sub;
    B_sub = H_sub;

  }

  // only free memory if it was allocated previously for this rank
  free(edge_plane_bottom);
  free(edge_plane_top);

  // has to linearize A_sub because we did not managed to get rid of ghost-cells
  /*value_t serialized_A_sub[window_size*N*N];
  for(int i=0; i<window_size;i++){
    for(int j=0; j<N; j++){
      for(int k=0; k<N; k++){
        serialized_A_sub[i*N*N+j*N+k] = A_sub[i][j][k];
      }
    }
  }
  */
  // linearized A where data is written to
  value_t A_gather[N*N*N];

  MPI_Gather(A_sub, window_size*N*N, MPI_DOUBLE, A_gather, window_size*N*N, MPI_DOUBLE, root_proc, stencil_comm);
  int success = 1;

  if (rank == 0)
  {
    double end = MPI_Wtime();
    printf("The process took %g seconds to finish. \n", end - start);
  }
  // ---------- check ----------
  if(rank == root_proc){
    // bring linearized A back to 3D format
    for(int i=0; i<N; i++){
      for(int j=0;j<N; j++){
        for(int k=0;k<N;k++){
          A[i][j][k] = A_gather[i*N*N+j*N+k];
        }
      }
    }
    
    printf("Final:\t\t\n");
    printTemperature(A, N);
    printf("\n");

    FILE *fp;

    fp = fopen("3D-output-mpi.dat", "w");
    fprintf(fp, "%d\n", N);


    for (long long i = 0; i < N; i++) {
      for (long long j = 0; j < N; j++) {
        for (long long k = 0; k < N; k++) {
          value_t temp = A[i][j][k];
          fprintf(fp, "%f\n", temp);
          if (273 <= temp && temp <= 273 + 60)
            continue;
          success = 0;
          break;
        }
      }
    }

    fclose(fp);

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");
  }
  // ---------- cleanup ----------
  free(A_sub);
  free(B_sub);
  MPI_Finalize();
  releaseMatrix(A);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

Matrix create3DMatrix(int N_x, int N_y, int N_z) {
  // create data and index Matrix
  value_t ***mat = (value_t***) malloc(sizeof(value_t**)*N_z);
  for (int i = 0; i < N_z; i++) {
    mat[i] = (value_t **) malloc(sizeof(value_t*)*N_y);
    for (int j = 0; j < N_y; j++) {
      mat[i][j] = (value_t *) malloc(sizeof(value_t)*N_x);
    }
  }
  return mat;
}

void releaseMatrix(Matrix m) { 
  free(m);
}

void printTemperature(Matrix m, int N) {

  printf("########################################----START 3D MATRIX\n");
  for(int i = 0; i < N; i++){
    if((i < 5) || (i >= N-5)){
      printf("#%d:\n", i);
      for(int j = 0; j < N; j++){
        if((j < 5) || (j >= N-5)){
          for(int k = 0; k < N; k++){
            // only print first and last 5 values of one line
   	    if(k < 5 || k >= N-5) {
              printf("%1.3f ", m[i][j][k]);
            }
            else if(k == 5){
              printf("... ");
            }
          }
          printf("\n");
        }
        else if(j == 5){
          printf("...\n");
        }
      }
      printf("\n\n");
    } 
  }

  printf("###########################################----END 3D MATRIX\n");
}


void linearizeAsub(value_t* plane, value_t* A_sub_tmp, int N, int offset){
  for(int i=0; i<N*N; i++){
    plane[i] = A_sub_tmp[offset+i];

    
  }
}


