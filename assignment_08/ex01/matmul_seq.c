#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double value_t;
typedef value_t **Matrix;


Matrix createMatrix(int rows, int columns) {
  // create data and index Matrix
  value_t **mat = malloc(sizeof(value_t) * rows);
  for (int i = 0; i < columns; i++) {
    mat[i] = malloc(sizeof(value_t) *columns);
  }
  return mat;
}

void releaseMatrix(Matrix m, int rows) { 
  for(int i=0; i < rows; i++){
    free(m[i]); 
  }
  free(m); 
}

void printMatrix(Matrix m, int rows, int columns)
{
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      printf("%.2lf\t", m[i][j]);
    }
    printf("\n");
  }
}




int main(int argc, char **argv)
{
 
  int N = 5; // rows
  int M = 10; // columns

  //---------- create matrices ----------
  Matrix A = createMatrix(N, M);
  //identity matrix
  Matrix B = createMatrix(M, M); //identiy always MxM
  Matrix res = createMatrix(N, M);

  // initialize matrix A
  for(int i=0; i<N;i++){
      for(int j=0; j< M; j++){
          A[i][j] = i * j;
      }
  }
  printf("==============================");
  printf("MAT A");
  printf("==============================\n");
  printMatrix(A,N,M);

  //initialize matrix B (identity matrix)
  for(int i=0; i<M;i++){
      for(int j=0; j< M; j++){
          B[i][j] = (i == j) ? 1 : 0;
      }
  }
  printf("==============================");
  printf("MAT B");
  printf("==============================\n");
  printMatrix(B,M,M);


  // ---------- compute ----------
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < M; k++) {
        res[i][j] += A[i][k]*B[k][j];
      }
    }
  }
printf("==============================");
printf("RES");
printf("==============================\n");
printMatrix(res,N,M);

  // ---------- cleanup ----------
  releaseMatrix(A,N);
  releaseMatrix(B,M);
  releaseMatrix(res,N);

  return EXIT_SUCCESS;
}




