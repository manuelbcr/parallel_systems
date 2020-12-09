#include <stdio.h>
#include <stdlib.h>

typedef double value_t;

#define RESOLUTION 120

// -- vector utilities --

typedef value_t *Vector;

Vector createVector(int N);

void releaseVector(Vector m);

void printTemperature(Vector m, int N);

// -- simulation code ---

int main(int argc, char **argv) {
  int success = EXIT_SUCCESS;
  // 'parsing' optional input parameter = problem size
  char* file_seq;
  char* file_mpi; 
  if (argc == 3) {
    file_seq = argv[1];
    file_mpi = argv[2];
  }
  else{
    printf("Wrong program call...\nPlease call it as: \n./heat_stencil_checker <sequential-output-file> <mpi-output-file>\n");
    return success;
  }
  printf("Check sequential file: %s and mpi: %s\n", file_seq, file_mpi);

  // get data from sequential file
  FILE *fp;

  fp = fopen(file_seq, "r");

  int N_seq = 0;
  fscanf(fp, "%d", &N_seq);
  printf("Dimensions of %s: %d\n", file_seq, N_seq);
  
  Vector seq_output = createVector(N_seq);
  int current_val = 0;
  for(int i=0; i<N_seq; i++){
    fscanf(fp, "%d", &current_val);
    seq_output[i] = current_val; 
  }

  fclose(fp);
  
  // get data from mpi file
  fp = fopen(file_mpi, "r");

  int N_mpi = 0;
  fscanf(fp, "%d", &N_mpi);
  printf("Dimensions of %s: %d\n", file_mpi, N_mpi);
  
  Vector mpi_output = createVector(N_mpi);
  current_val = 0;
  for(int i=0; i<N_mpi; i++){
    fscanf(fp, "%d", &current_val);
    mpi_output[i] = current_val; 
  }

  fclose(fp);
  

  // check files
  if(N_seq != N_mpi){
    printf("You are not checking the same dimensions: N_seq=%d <=> N_mpi=%d\n", N_seq, N_mpi);
    return EXIT_SUCCESS;
  }

  value_t abs_error = 0;
  for(int i=0; i < N_seq; i++){
    abs_error += abs(seq_output[i] - mpi_output[i]);
  }

  printf("Error value: %f\n", abs_error);

  // done
  releaseVector(seq_output);
  releaseVector(mpi_output);

  printf("Check was successful!\n");

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
