#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


typedef double value_t;

#define RESOLUTION 100

// -- Matrix utilities --

typedef value_t ***Matrix;

Matrix create3DMatrix(int N_x, int N_y, int N_z);

void releaseMatrix(Matrix m, int N_x, int N_y);

void printTemperature(Matrix m, int N);

void linearizeAsub(value_t* plane, value_t*** A_sub_tmp, int N, int window_size);

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
  Matrix A = create3DMatrix(N, N, N);

  // set up initial conditions in A
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      for(int k = 0; k < N; k++){
        A[i][j][k] = 273; // temperature is 0° C everywhere (273 K)
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

  // Determine top and bottom rank for each rank
  int top_rank;
  int bottom_rank;
  MPI_Cart_shift(stencil_comm, 0, 1, &bottom_rank, &top_rank);
 
  // create datatype for sending edge elements
  MPI_Datatype heat_stencil_edge_type;
  // edge plane with 2 ghost cells at the end  
  MPI_Type_vector(N, N, N+2, MPI_DOUBLE, &heat_stencil_edge_type);
  MPI_Type_commit(&heat_stencil_edge_type);

  // and there is a heat source in one corner
  int source_x = N / 8;
  int source_y = N / 8;
  int source_z = N / 8;
  A[source_z][source_y][source_x] = 273 + 60;

  if(rank == root_proc){
    printf("Initial - heatsource is at [%d][%d][%d]:\t", source_z, source_y, source_x);
    printTemperature(A, N);
    printf("\n");
  }
  // ---------- compute ----------

  // create a second buffer for the computation
  int window_size = N/rank_size;
  Matrix B_sub = create3DMatrix(N, N, window_size);
  Matrix A_sub = create3DMatrix(N, N, window_size);

  for(int i=0; i<window_size; i++){
    for(int j=0; j<N; j++){
      for(int k=0; k<N; k++){
        A_sub[i][j][k] = A[i+rank*window_size][j][k];
      }
    }
  }
  
  // top and bottom border edges that came from other ranks 
  value_t* edge_plane_bottom;// = &A_sub[0][0][0];
  value_t* edge_plane_top;// = &A_sub[window_size-1][0][0];
  
  // if there has to be stored a new edge plane from another rank allocate memory
  //if(bottom_rank >= 0){
    edge_plane_bottom = malloc(sizeof(value_t)*N*N);
  //}
  //if(top_rank >= 0){
    edge_plane_top = malloc(sizeof(value_t)*N*N);
  //}

  // for each time step ..
  for (int t = 0; t < T; t++) {
    
    // first all even ranks are sending than all odd ones    
    if(rank%2 == 0){
      if(bottom_rank >= 0){
        MPI_Send(&A_sub[0][0][0], 1, heat_stencil_edge_type, bottom_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_bottom, N*N, MPI_DOUBLE, bottom_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
        if(rank==2 && t<=2){
          printf("rank=%d get bottom from %d:\n", rank, bottom_rank);
          for(int a=0; a<N; a++){
            for(int b=0; b<N; b++){
              printf("%f ", edge_plane_bottom[a*N+b]);
            }
            printf("\n");
          }
          printf("#########END DEBUG\n");
        }
      }
      else{
        for(int b=0; b<N; b++){
          for(int c=0; c<N; c++){
            edge_plane_bottom[b*N+c] = A_sub[0][b][c];
          }
        }
      }
      if(top_rank >= 0){
        MPI_Send(&A_sub[window_size-1][0][0], 1, heat_stencil_edge_type, top_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_top, N*N, MPI_DOUBLE, top_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        for(int b=0; b<N; b++){
          for(int c=0; c<N; c++){
            edge_plane_top[b*N+c] = A_sub[window_size-1][b][c];
          }
        }
      }
    }
    else{
      if(bottom_rank >= 0){
        MPI_Send(&A_sub[0][0][0], 1, heat_stencil_edge_type, bottom_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_bottom, N*N, MPI_DOUBLE, bottom_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        for(int b=0; b<N; b++){
          for(int c=0; c<N; c++){
            edge_plane_bottom[b*N+c] = A_sub[0][b][c];
          }
        }
      }
      if(top_rank >= 0){
        MPI_Send(&A_sub[window_size-1][0][0], 1, heat_stencil_edge_type, top_rank, 0, stencil_comm);
        MPI_Recv(edge_plane_top, N*N, MPI_DOUBLE, top_rank, 0, stencil_comm, MPI_STATUS_IGNORE);
      }
      else{
        for(int b=0; b<N; b++){
          for(int c=0; c<N; c++){
            edge_plane_top[b*N+c] = A_sub[window_size-1][b][c];
          }
        }
      }
    }
     
    // .. we propagate the temperature
    for (long long i = 0; i < window_size; i++) {
      for (long long j = 0; j < N; j++) {
        for (long long k = 0; k < N; k++) {
          // center stays constant (the heat is still on)
          if (i+rank*window_size == source_z && j == source_y && k == source_x) {
            B_sub[i][j][k] = A_sub[i][j][k];
            continue;
          }

          // get temperature at current position
          value_t tc = A_sub[i][j][k];

          value_t t_back = (i > 0) ? A_sub[i-1][j][k] : edge_plane_bottom[j*N+k];
          value_t t_up = (j > 0) ? A_sub[i][j-1][k] : tc;
          value_t t_left = (k > 0) ? A_sub[i][j][k-1] : tc;
          value_t t_front = (i < window_size-1) ? A_sub[i+1][j][k] : edge_plane_top[j*N+k];
          value_t t_down = (j < N-1) ? A_sub[i][j+1][k] : tc;
          value_t t_right = (k < N-1) ? A_sub[i][j][k+1] : tc;

          // compute new temperature at current position
          B_sub[i][j][k] = (1.0/6.0) * (t_up + t_down + t_left + t_right + t_front + t_back);
        }
      }
    }

    if((abs(rank-1)<=1 || rank == 7 || rank == 0) && rank && t < 3){
      printf("PRINT HEATSOURCE SURROUNDING - rank#%d at t=%d:\n", rank, t);
      for(int i=0; i<window_size; i++){
        printf("####%d:\n", i);
        for(int j=0; j<N; j++){
          for(int k=0; k<N; k++){
            printf("%f ", B_sub[i][j][k]);
          }
          printf("\n");
        } 
      } 
    }


    // swap matrices (just pointers, not content)
    Matrix H_sub = A_sub;
    A_sub = B_sub;
    B_sub = H_sub;

  }

  // only free memory if it was allocated previously for this rank
  /*if(bottom_rank >= 0){
    free(edge_plane_bottom);
  }
  if(top_rank >= 0){
    free(edge_plane_top);
  }
  value_t buf[N*N*N];
  */

  value_t serialized_A_sub[window_size*N*N];
  for(int i=0; i<window_size;i++){
    for(int j=0; j<N; j++){
      for(int k=0; k<N; k++){
        serialized_A_sub[i*N*N+j*N+k] = A_sub[i][j][k];
      }
    }
  }
  
  value_t A_gather[N*N*N];

  MPI_Gather(serialized_A_sub, window_size*N*N, MPI_DOUBLE, A_gather, window_size*N*N, MPI_DOUBLE, root_proc, stencil_comm);
  int success = 1;
  // ---------- check ----------
  if(rank == root_proc){
    printf("GATHERED A:\n");
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
  MPI_Type_free(&heat_stencil_edge_type);
  releaseMatrix(A_sub, N, N);
  MPI_Finalize();
  releaseMatrix(A, N, N);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
  // */
  //MPI_Finalize();
  //releaseMatrix(A, N);
  //return 0;
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

void releaseMatrix(Matrix m, int N_x, int N_y) { 
  for(int i=0; i < N_y; i++){
    for(int j=0; j < N_x; j++){
      //free(m[i][j]);
    }
    //free(m[i]); 
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


void linearizeAsub(value_t* plane, value_t*** A_sub_tmp, int N, int window_size){
  for(int i=0; i<window_size; i++){
    for(int j=0; j<N; j++){
      for(int k=0; k<N; k++){
        plane[i*N*N+j*N+k] = A_sub_tmp[i][j][k];
      }
    }
  }
}


