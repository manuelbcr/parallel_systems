#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>


typedef double value_t;

#define RESOLUTION 120


typedef value_t *Vector;

Vector createVector(int N);

void releaseVector(Vector m);


int main(int argc, char *argv[]) {
  int my_rank, numproc;
  int N = 80;

  Vector A = createVector(N);
 
  for(int i=0; i<N; i++){
    A[i] = i;
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  Vector B = createVector(N/8);
   
  for(int i=0; i<10; i++){
    B[i] = A[my_rank*10+i]*100+my_rank;
  }

  MPI_Comm comm_sm;
  MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &comm_sm);

  int num_proc_shared, my_rank_shared;
  MPI_Comm_size(comm_sm, &num_proc_shared);
  MPI_Comm_rank(comm_sm, &my_rank_shared);

  printf("(WRANK#%d/%d vs. SMRANK#%d/%d)\n", my_rank, numproc, my_rank_shared, num_proc_shared);


  MPI_Win win;
  MPI_Win_allocate_shared((MPI_Aint)(sizeof(value_t)*N),sizeof(value_t), MPI_INFO_NULL, comm_sm, B, &win);

  MPI_Win_fence(0, win);
  B[my_rank] = 1990+my_rank; // replaces the MPI_Put() call!
  MPI_Win_fence(0, win);
  MPI_Win_free(&win);

  printf("(RANK#%d of %d): my_num = %f\n", my_rank, numproc, A[my_rank*10]);
  for(int i = 0; i<10; i++){
    printf("%d-%d#%f ", my_rank, i, B[i]);
  }
  printf("\n");

  releaseVector(B);
  releaseVector(A);
  MPI_Finalize();
  return EXIT_SUCCESS;
}



Vector createVector(int N) {
  // create data and index vector
  return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) { 
  free(m); 
}


