#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

typedef double value_t;
typedef value_t **Matrix;


Matrix createMatrix(int rows, int columns) {
  // create data and index Matrix
  value_t **mat = malloc(sizeof(value_t*) * rows);
  for (int i = 0; i < columns; i++) {
    mat[i] = malloc(sizeof(value_t) *columns);
  }
  return mat;
}

void releaseMatrix(Matrix m, int rows) { 
  for(int i=1; i < rows; i++){
    free(m[i]); 
  }
  free(m); 
}

void printMatrix(Matrix m, int rows, int columns) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      printf("%.2lf\t", m[i][j]);
    }
    printf("\n");
  }
}




int main(int argc, char **argv){

int N = 5; // rows
int M = 10; // columns

double start;
double end;
printf("%d\n", omp_get_num_threads());
 if (argc > 1) {
    N = M = atoi(argv[1]);
 }

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
  /*
  printf("==============================");
  printf("MAT A");
  printf("==============================\n");
  printMatrix(A,N,M);
  */

  //initialize matrix B (identity matrix)
  for(int i=0; i<M;i++){
      for(int j=0; j< M; j++){
          B[i][j] = (i == j) ? 1 : 0;
      }
  }
  /*
  printf("==============================");
  printf("MAT B");
  printf("==============================\n");
  printMatrix(B,M,M);
   */

int i,j,k;

printf("N=%d \n ", N);
start = omp_get_wtime();
// ---------- compute ----------
#pragma omp parallel  shared(A,B,res) private(i,j,k) 
{
  #pragma omp for schedule(static)
  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      for (k = 0; k < M; k++) {
        res[i][j] += A[i][k]*B[k][j];
      }
    }
  }
}

end = omp_get_wtime(); 
printf("Work took %f seconds\n", end - start);
/*
printf("==============================");
printf("RES");
printf("==============================\n");
printMatrix(res,N,M);
*/
  
  // ---------- cleanup ----------
releaseMatrix(A,N);
releaseMatrix(B,M);
releaseMatrix(res,N);

  return EXIT_SUCCESS;
}




