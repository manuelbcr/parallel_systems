#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


typedef double value_t;

#define RESOLUTION 100

// -- Matrix utilities --

typedef value_t ***Matrix;

Matrix create3DMatrix(int N);

void releaseMatrix(Matrix m, int N);

void printTemperature(Matrix m, int N);

// -- simulation code ---

int main(int argc, char **argv){
  // 'parsing' optional input parameter = problem size
  int N = 200; // rows x columns

  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 100;
  printf("Computing heat-distribution for room size N=%dx%dx%d, for T=%d timesteps\n", N, N, N, T);

  // ---------- setup ----------

  // create a buffer for storing temperature fields
  Matrix A = create3DMatrix(N);

  // set up initial conditions in A
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      for(int k = 0; k < N; k++){
        //A[i][j][k] = 273; // temperature is 0° C everywhere (273 K)
        //A[i][j][k] = 1000*i+j+k/1000.0; // temperature is 0° C everywhere (273 K)
        A[i][j][k] = i*N*N+j*N+k; // temperature is 0° C everywhere (273 K)
      }
    }
  }


  MPI_Init(&argc, &argv);
  int rank, rank_size;
  MPI_Comm_size(MPI_COMM_WORLD, &rank_size);

  if(N % rank_size != 0){
    printf("The problem of size N=%d is not divisable by the number of ranks=%d \n", N, rank_size);
    MPI_Finalize(); // cleanup
    return EXIT_FAILURE;
  }

  int root_proc = 0;
  MPI_Comm stencil_comm;
  int dim[1] = {rank_size};
  int period[1] = {0};
  int reorder = 1;
  MPI_Cart_create(MPI_COMM_WORLD, 1, dim, period, reorder, &stencil_comm);
  MPI_Comm_rank(stencil_comm, &rank);

  if(rank == root_proc){
    printTemperature(A, N);
  }
  /* 
  MPI_Datatype heat_stencil_edge_top_type;

  int dimensions_full_array[3] = {N, N, N};
  int dimensions_subarray[3] = { 1, 5, 5 };
  int start_coordinates_top[3] = { 0, 0, 0 };
  MPI_Type_create_subarray(2,  dimensions_full_array, dimensions_subarray, start_coordinates_top, MPI_ORDER_C, MPI_DOUBLE, &heat_stencil_edge_top_type);

  MPI_Type_commit(&heat_stencil_edge_top_type);
  */

  MPI_Datatype heat_stencil_edge_type;
  
  MPI_Type_vector(N, N, N+2, MPI_DOUBLE, &heat_stencil_edge_type);
  MPI_Type_commit(&heat_stencil_edge_type);

  if(rank == 0){
    MPI_Send(&A[0][0][0], 1, heat_stencil_edge_type, 1, 0, stencil_comm);
    MPI_Send(&A[1][0][0], 1, heat_stencil_edge_type, 1, 0, stencil_comm);
  }

  if(rank == 1){
    value_t test_rec[N*N];
    printf("test_rec dim: %d\n", N*N);
    //MPI_Recv(&test_rec, 1, heat_stencil_edge_top_type, 0, 0, stencil_comm, MPI_STATUS_IGNORE); 
    MPI_Recv(&test_rec, N*N, MPI_DOUBLE, 0, 0, stencil_comm, MPI_STATUS_IGNORE); 
    printf("values top are: \n");
    for(int i=0;i<N;i++){
      for(int j=0;j<N;j++){
        printf("%f ", test_rec[i*N+j]);
      }
      printf("\n");
    }
    printf("\n\n");

    MPI_Recv(&test_rec, N*N, MPI_DOUBLE, 0, 0, stencil_comm, MPI_STATUS_IGNORE); 
    printf("values top are: \n");
    for(int i=0;i<N;i++){
      for(int j=0;j<N;j++){
        printf("%f ", test_rec[i*N+j]);
      }
      printf("\n");
    }
    printf("\n\n");


  }

  MPI_Finalize();
  releaseMatrix(A, N);
  return EXIT_SUCCESS;
  // and there is a heat source in one corner
  int source_x = N / 4;
  int source_y = N / 4;
  int source_z = N / 4;
  A[source_z][source_y][source_x] = 273 + 60;

  if(rank == root_proc){
    printf("Initial:\t");
    printTemperature(A, N);
    printf("\n");
  }
  // ---------- compute ----------

  // create a second buffer for the computation
  Matrix B = create3DMatrix(N);

  // for each time step ..
  for (int t = 0; t < T; t++) {
    // .. we propagate the temperature
    for (long long i = 0; i < N; i++) {
      for (long long j = 0; j < N; j++) {
        for (long long k = 0; k < N; k++) {
          // center stays constant (the heat is still on)
          if (i == source_z && j == source_y && k == source_z) {
            B[i][j][k] = A[i][j][k];
            continue;
          }

          // get temperature at current position
          value_t tc = A[i][j][k];

          //value_t = ((i > 0) && (j > 0)) ? A[i-1][j-1][k] : tc;
          //value_t = ((i > 0) && (j > 0) && (k > 0)) ? A[i-1][j-1][k-1] : tc;
          value_t t_up = (i > 0) ? A[i-1][j][k] : tc;
          value_t t_left = (j > 0) ? A[i][j-1][k] : tc;
          value_t t_front = (k > 0) ? A[i][j][k-1] : tc;
          value_t t_down = (i < N-1) ? A[i+1][j][k] : tc;
          value_t t_right = (j < N-1) ? A[i][j+1][k] : tc;
          value_t t_back = (k < N-1) ? A[i][j][k+1] : tc;

          // compute new temperature at current position
          B[i][j][k] = (1.0/6.0) * (t_up + t_down + t_left + t_right + t_front + t_back);
        }
      }
    }

    // swap matrices (just pointers, not content)
    Matrix H = A;
    A = B;
    B = H;

  }

  releaseMatrix(B, N);

  int success = 1;
  // ---------- check ----------
  if(rank == root_proc){
    printf("Final:\t\t\n");
    printTemperature(A, N);
    printf("\n");

    FILE *fp;

    fp = fopen("3D-output-seq.dat", "w");
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
  MPI_Finalize();
  releaseMatrix(A, N);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
  // */
  //releaseMatrix(A, N);
  //return 0;
}

Matrix create3DMatrix(int N) {
  // create data and index Matrix
  value_t ***mat = (value_t***) malloc(sizeof(value_t**)*N);
  for (int i = 0; i < N; i++) {
    mat[i] = (value_t **) malloc(sizeof(value_t*)*N);
    for (int j = 0; j < N; j++) {
      mat[i][j] = (value_t *) malloc(sizeof(value_t)*N);
    }
  }
  return mat;
}

void releaseMatrix(Matrix m, int N) { 
  for(int i=0; i < N; i++){
    for(int j=0; j < N; j++){
      free(m[i][j]);
    }
    free(m[i]); 
  }
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
