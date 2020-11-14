#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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
        A[i][j][k] = 273; // temperature is 0° C everywhere (273 K)
      }
    }
  }

  printTemperature(A, N);
  
  // and there is a heat source in one corner
  int source_x = N / 4;
  int source_y = N / 4;
  int source_z = N / 4;
  A[source_z][source_y][source_x] = 273 + 60;

  printf("Initial:\t");
  printTemperature(A, N);
  printf("\n");

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

    // show intermediate step
    if (!(t % 1000)) {
      printf("Step t=%d:\n", t);
      printTemperature(A, N);
      printf("\n");
    }
  }

  releaseMatrix(B, N);

  // ---------- check ----------
  printf("Final:\t\t\n");
  printTemperature(A, N);
  printf("\n");

  FILE *fp;

  fp = fopen("3D-output-seq.dat", "w");
  fprintf(fp, "%d\n", N);


  int success = 1;
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

  // ---------- cleanup ----------

  releaseMatrix(A, N);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
  // */
  //releaseMatrix(A, N);
  //return 0;
}

Matrix create3DMatrix(int N) {
  // create data and index Matrix
  value_t ***mat = malloc(sizeof(value_t) * N);
  for (int i = 0; i < N; i++) {
    mat[i] = malloc(sizeof(value_t*) * N);
    for (int j = 0; j < N; j++) {
      mat[i][j] = malloc(sizeof(value_t) * N);
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
