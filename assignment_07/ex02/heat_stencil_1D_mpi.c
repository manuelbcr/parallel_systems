#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define RESOLUTION 120

typedef double value_t;

// -- vector utilities --

typedef value_t *Vector;

Vector createVector(int N);

void releaseVector(Vector m);

void printTemperature(Vector m, int N);

void handleError(int error);

// -- simulation code ---

int main(int argc, char **argv) {
  double start = MPI_Wtime();
  
  int N = 100;
  if (argc > 1) {
    N = atoi(argv[1]);
  }

  int T = 500;
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
  int ierr;
  int rank, rank_size;
  int root_proc = 0;
  int provided = 0;
  ierr = MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  // error codes are returned to the user
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
  ierr = MPI_Comm_size(MPI_COMM_WORLD, &rank_size);
  handleError(ierr);
  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  handleError(ierr);

  // create new intra communicator
  MPI_Comm comm_intra;
  MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &comm_intra);

  int rank_intra, rank_size_intra;
  int root_proc_intra = 0;
  
  ierr = MPI_Comm_size(comm_intra, &rank_size_intra);
  handleError(ierr);
  ierr = MPI_Comm_rank(comm_intra, &rank_intra);
  handleError(ierr);

  if(rank_size%rank_size_intra != 0){
    printf("Ranks are not evenly distributed over nodes... (there is at least 1 node with more ranks)\n");
    return EXIT_FAILURE;
  }
  int num_of_nodes = rank_size/rank_size_intra;

  // calculate window size and initialize subarray
  // we have to deal the case when N is not divideable by the rank_size
  // window_size is the window_size of all ranks (except of last one maybe)
  // last_index is used for loop boundaries for(i<last_index) 
  // that is dependent on the rank and potentially different for last one
  int mod_N = N%num_of_nodes;
  int window_size = (mod_N != 0) ? (N-mod_N)/num_of_nodes : N/num_of_nodes;
  int last_index = window_size;
  if((rank_intra >= (num_of_nodes-1)*rank_size_intra) && (mod_N != 0)){
    last_index += mod_N;
  }
  Vector A_sub = createVector(last_index);
  int window_size_intra = last_index/rank_size_intra;

  printf("(RANK#%d::%d): num_of_nodes = %d, window_size = %d::%d, last_index = %d\n", rank, rank_intra, num_of_nodes, window_size, window_size_intra, last_index);
  // copy values into sub array for each rank
  for(int i=0; i < last_index; i++){
    A_sub[i] = A[i+(rank-rank_intra)*window_size];
  }

  printf("(RANK#%d::%d): I am responsible for subpart: [%d, %d] Num_ranks %d::%d\n", rank, rank_intra, (rank-rank_intra)*window_size, (rank-rank_intra)*window_size+last_index-1, rank_size, rank_size_intra);

  // intial print of heat environment
  if(rank == root_proc){
    printf("Initial:\t");
    printTemperature(A, N);
    printf("\n");
  }

  // ---------- compute ----------
  // create a second buffer for the computation
  Vector B_sub = createVector(last_index);
  int min_index_in_A = (rank-rank_intra)*window_size;

  MPI_Win win;
  // for each time step ..
  for (int t = 0; t < T; t++) {
    printf("(RANK#%d::%d): DEBUG1\n", rank, rank_intra);
    MPI_Win_allocate_shared((MPI_Aint)(sizeof(value_t)*N),sizeof(value_t), MPI_INFO_NULL, comm_intra, B_sub, &win);
    // left and right border cell that came from other ranks 
    value_t edge_cell_l = A_sub[0];
    value_t edge_cell_r = A_sub[last_index-1];
    printf("(RANK#%d::%d): DEBUG2\n", rank, rank_intra);
    // first all even ranks are sending than all odd ones
    // and only intra_root nodes should exchange data
    if(rank_intra == root_proc_intra){
      if((rank_size_intra)%2 == 0){
        if(rank > 0){
          ierr = MPI_Send(&A_sub[0], 1, MPI_DOUBLE, rank-rank_size_intra, 0, MPI_COMM_WORLD);
          handleError(ierr);
          ierr = MPI_Recv(&edge_cell_l, 1, MPI_DOUBLE, rank-rank_size_intra, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          handleError(ierr);
        }
        if(rank < rank_size-rank_size_intra){
          ierr = MPI_Send(&A_sub[last_index-1], 1, MPI_DOUBLE, rank+rank_size_intra, 0, MPI_COMM_WORLD);
          handleError(ierr);
          ierr = MPI_Recv(&edge_cell_r, 1, MPI_DOUBLE, rank+rank_size_intra, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          handleError(ierr);
        }
      }
      else{
        if(rank < rank_size-rank_size_intra){
          ierr = MPI_Recv(&edge_cell_r, 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          handleError(ierr);
          ierr = MPI_Send(&A_sub[last_index-1], 1, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD);
          handleError(ierr);
        }
        if(rank > 0){
          ierr = MPI_Recv(&edge_cell_l, 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          handleError(ierr);
          ierr = MPI_Send(&A_sub[0], 1, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD);
          handleError(ierr);
        }
      }
    }
    //printf("(RANK#%d): send and receive finished for timestep=%d\n", rank, t);
    // .. we propagate the temperature
    for (long long i = 0; i < last_index; i++) {
      if((i < rank_intra*window_size_intra) || (i >= rank_intra*window_size_intra+window_size_intra)){
        continue;
      }

      //printf("(RANK#%d): dealing with item %lld of timestep=%d by Thread=%d\n", rank, i, t, omp_get_thread_num());
      // center stays constant (the heat is still on)
      if (i+min_index_in_A == source_x) {
        B_sub[i] = A_sub[i];
        continue;
      }

      // get temperature at current position
      value_t tc = A_sub[i];

      // get temperatures of adjacent cells
      value_t tl = (i != 0) ? A_sub[i - 1] : edge_cell_l;
      value_t tr = (i != last_index - 1) ? A_sub[i + 1] : edge_cell_r;

      // compute new temperature at current position
      B_sub[i] = tc + 0.2 * (tl + tr + (-2 * tc));
    }

    MPI_Win_free(&win);
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
    displs[i] = i*window_size;
    receive_counts[i] = last_index;
  }
  
  // gather all end-sub-results for final output
  ierr = MPI_Gatherv(A_sub, window_size, MPI_DOUBLE, 
               A, receive_counts, displs, MPI_DOUBLE, 
               0, MPI_COMM_WORLD);
  handleError(ierr);

  releaseVector(B_sub);
  releaseVector(A_sub);

  if (rank == 0)
  {
    double end = MPI_Wtime();
    printf("The process took %g seconds to finish. \n", end - start);
  }


  // ---------- check ----------
  int success = 1;
  if(rank == root_proc){
    printf("Final:\t\t");
    printTemperature(A, N);
    printf("\n");

    FILE *fp;

    fp = fopen("1D-output-mpi.dat", "w");
    fprintf(fp, "%d\n", N);


    int success = 1;
    for (long long i = 0; i < N; i++) {
      value_t temp = A[i];
      fprintf(fp, "%f\n", temp);
      if (273 <= temp && temp <= 273 + 60)
        continue;
      success = 0;
      break;
    }
    fclose(fp);

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

void handleError(int error){

  if(error == MPI_SUCCESS){
    return;
  }

  int len, eclass;
  char estring[MPI_MAX_ERROR_STRING];
  MPI_Error_class(error, &eclass);
  MPI_Error_string(error, estring, &len);
  printf("Error %d: %s\n", eclass, estring);
  MPI_Abort(MPI_COMM_WORLD, error);

}
