#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <time.h>
typedef double value_t;

#define RESOLUTION 100
#define DEBUG 0

// -- Vector utilities --
typedef value_t *Vector;

Vector createVector(int N);

void releaseVector(Vector m);

void printTemperature(Vector m, int nx, int ny);

int calculateIndex(int x, int y, int Nx);

int main(int argc, char **argv){

  int Nx = 100;
  int Ny = 100; 

  if (argc == 2) {
    Nx = Ny = atoi(argv[1]);
  }

  int T = (Nx < Ny ? Ny : Nx) * 500; 

  // MPI init
  int number_of_ranks;
  int rank;
  MPI_Init(&argc, &argv);                          //initialize the MPI environment
  MPI_Comm_size(MPI_COMM_WORLD, &number_of_ranks); //get the number of ranks
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);            //get the rankof the caller

  if(Ny % number_of_ranks != 0){

    printf("The problem is not divisable by the number of ranks\n");
    MPI_Finalize(); // cleanup
    return EXIT_FAILURE;

  }

  int number_of_rows_per_process = Ny / number_of_ranks;

  // Cartesian topology setup 1 dimensional(number_of_ranks x 1), non periodic, no reorders
  MPI_Comm comm_cartesian;
  int ndims = 1;
  int dims[1] = {number_of_ranks};
  int periods[1] = {0};
  int reorder = 0;
  MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, reorder, &comm_cartesian);

  // Determine top and bottom rank for each rank
  int top_rank;
  int bottom_rank;
  MPI_Cart_shift(comm_cartesian, 0, 1, &top_rank, &bottom_rank);

  //printf("[Rank %d]: Top neighbour = rank %d, Bottom neighbour = rank %d\n", rank, top_rank, bottom_rank);


  // begin INIT line vector: represents whole matrix => line vector of size Ny*Nx =====================================================
  Vector column_vector = createVector(number_of_rows_per_process*Nx);
  
  for(int i = 0; i < number_of_rows_per_process*Nx; i++){
    column_vector[i] = 273; // Set 0 degree everywhere 
  }

  // and there is a heat source in one corner
  int source_x = 25;
  int source_y = 25;

  // set heat source by the corresponding rank
  if(((rank * number_of_rows_per_process) <= source_y) && (((rank+1) * number_of_rows_per_process) > source_y)){
    // source_y - (rank * number_of_rows_per_process) to 
    column_vector[calculateIndex(source_x, source_y - (rank * number_of_rows_per_process), Nx)] = 273 + 60;
  }

  Vector vector_b = createVector(number_of_rows_per_process*Nx);
  Vector gather_vector = createVector(Ny*Nx);

  // end INIT line vector: represents whole matrix => line vector of size Ny*Nx =====================================================

  Vector top_line = createVector(Nx);
  Vector bottom_line = createVector(Nx);

  // temperature calculation for each time step ..
  for (int t = 0; t < T; t++) {

    //printf("TIMESTEP %d\n", t);

    // send the uppermost and lowest layer to upper and lower slices
    // if there is a top neighbour
    if(top_rank >= 0){
      MPI_Send(column_vector, Nx, MPI_DOUBLE, top_rank, 0, comm_cartesian);
      //printf("[Rank %d]: MPI SEND TOP LINE\n", rank);
    }
    
    // if there is a bottom neighbour
    if(bottom_rank >= 0){
      MPI_Send(&column_vector[calculateIndex(0, number_of_rows_per_process-1, Nx)], Nx, MPI_DOUBLE, bottom_rank, 0, comm_cartesian);
      //printf("[Rank %d]: MPI SEND BOTTOM LINE\n", rank);
    }

    // iterate over all lines of a rank
    for (int y = 0; y < number_of_rows_per_process; y++){

      // if computation is in top line, receive line from top neighbour if exist
      if(y == 0){
        if(top_rank >= 0){
          MPI_Recv(top_line, Nx, MPI_DOUBLE, top_rank, 0, comm_cartesian, MPI_STATUS_IGNORE);
          //printf("[Rank %d]: MPI RECV TOP LINE\n", rank);
        }
      }
      // if computation is in bottom line, receive line from bottom neighbour if exist
      if (y == (number_of_rows_per_process - 1)){
        if(bottom_rank >= 0){
          MPI_Recv(bottom_line, Nx, MPI_DOUBLE, bottom_rank, 0, comm_cartesian, MPI_STATUS_IGNORE);
          //printf("[Rank %d]: MPI RECV BOTTOM LINE\n", rank);
        }
      }

      // computation of all elements in a line
      for (int x = 0; x < Nx; x++){

        value_t tt;
        value_t tb;
        value_t tl;
        value_t tr;

        // get temperature at current position
        value_t tc = column_vector[calculateIndex(x, y, Nx)];

        // if current index is index of heat source => do not change heat source
        if (calculateIndex(x, y, Nx) + rank * Nx * number_of_rows_per_process == calculateIndex(source_x, source_y, Nx)) {
          vector_b[calculateIndex(x, y, Nx)] = column_vector[calculateIndex(x, y, Nx)];
          continue;
        }

        // if the line is the overall top line (no top neighbour)
        if((y == 0) && (top_rank < 0)){

          tt = tc;

        } else {
            
          // if the line is the top line of a rank => get top element from neighbour
          tt = (y != 0) ? column_vector[calculateIndex(x, y-1, Nx)] : top_line[x];

        }

        // if the line is the overall bottom line (no bottom neighbour)
        if(y == ((number_of_rows_per_process - 1)) && (bottom_rank < 0)){

          tb = tc;

        } else {

          // if the line is the bottom line of a rank => get bottom element from neighbour
          tb = (y != (number_of_rows_per_process - 1)) ? column_vector[calculateIndex(x, y+1, Nx)] : bottom_line[x];

        }

        tl = (x != 0) ? column_vector[calculateIndex(x-1, y, Nx)] : tc;
        tr = (x != Nx - 1) ? column_vector[calculateIndex(x+1, y, Nx)] : tc;

        // compute new temperature at current position
        vector_b[calculateIndex(x, y, Nx)] = 0.25 * (tl + tr + tt + tb);
          
      }
    }

    // swap vectors
    Vector temp = column_vector;
    column_vector = vector_b;
    vector_b = temp;

    if(DEBUG){

      // show intermediate step
      if (!(t % 1000)){

        MPI_Gather(column_vector, Nx * number_of_rows_per_process, MPI_DOUBLE, gather_vector, Nx * number_of_rows_per_process, MPI_DOUBLE, 0, comm_cartesian);

        if (rank == 0){
          printf("Step t=%d:\n", t);
          printTemperature(gather_vector, Nx, Ny);
          printf("\n");
        }
      }
    }
  }

  releaseVector(top_line);
  releaseVector(bottom_line);
  releaseVector(vector_b);

  MPI_Gather(column_vector, Nx * number_of_rows_per_process, MPI_DOUBLE, gather_vector, Nx *number_of_rows_per_process, MPI_DOUBLE, 0, comm_cartesian);
  releaseVector(column_vector);

  if(rank == 0){

    printf("Final:\n");
    printTemperature(gather_vector, Nx, Ny);
    printf("\n");

      FILE *fp;

      fp = fopen("2D-output-mpi.dat", "w");
      fprintf(fp, "%d\n", Nx);


      int success = 1;
      for (long long i = 0; i < Nx; i++) {
        for (long long j = 0; j < Ny; j++) {
          value_t temp = gather_vector[calculateIndex(i, j, Nx)];
          fprintf(fp, "%f\n", temp);
          if (273 <= temp && temp <= 273 + 60)
            continue;
          success = 0;
          break;
        }
      }

  }
  

  MPI_Finalize(); //cleanup
  return 0;
}

int calculateIndex(int x, int y, int Nx){
  return (y * Nx) + x;
}

Vector createVector(int N) {
  // create data and index vector
  return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) { free(m); }

void printTemperature(Vector m, int nx, int ny){

  const char *colors = " .-:=+*^X#%@";
  const int numColors = 12;

  // boundaries for temperature (for simplicity hard-coded)
  const value_t max = 273 + 30;
  const value_t min = 273 + 0;

  // set the 'render' resolution
  int Wx = RESOLUTION;
  int Wy = RESOLUTION / 4;

  // step size in each dimension
  int sWx = nx / Wx;
  int sWy = ny / Wy;

  for (int j = 0; j < Wy; j++){
    // left wall
    printf("X");
    for (int i = 0; i < Wx; i++){
      // get max temperature in this tile
      value_t max_t = 0;
       
      for (int y = sWy * j; y < sWy * j + sWy; y++){
        for (int x = sWx * i; x < sWx * i + sWx; x++){
          max_t = (max_t < m[calculateIndex(x,y,nx)]) ? m[calculateIndex(x,y,nx)] : max_t;
        }
      }
        
      value_t temp = max_t;

      // pick the 'color'
       int c = ((temp - min) / (max - min)) * numColors;
      c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

      // print the average temperature
      // if numbers are desired use printf("%2.2f\t",temp-273);
      printf("%c", colors[c]);
    }
    // right wall
    printf("X\n");
  }
  
}