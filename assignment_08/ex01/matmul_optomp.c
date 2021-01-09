#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

typedef double value_t;
typedef value_t **Matrix;


Matrix createMatrix(int rows, int columns) {
  // create data and index Matrix
  value_t **mat = malloc(sizeof(value_t*)*rows);
  for (int i = 0; i < columns; i++) {
    mat[i] = malloc(sizeof(value_t)*columns);
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

int N = 10; // rows
int M = 10; // columns
int block_size = 10;
double start;
double end;

 if (argc > 1) {
    N = M = atoi(argv[1]);
 }
 if (argc > 2) {
    block_size = atoi(argv[2]);
 }

  //---------- create matrices ----------
  Matrix A = createMatrix(N, M);
  //identity matrix
  Matrix B = createMatrix(M, M); //identiy always MxM
  Matrix transposedB= createMatrix(M, M);
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

 //transpose matrix b
  for(int x=0; x<N; x++) {
      for(int y=0; y<N; y++) {
          transposedB[x][y] = B[y][x];
      }
  }


start = omp_get_wtime();
// ---------- compute ----------
int i = 0, j = 0, k = 0, jj = 0, kk = 0;
double tmp;
int chunk = 1;

#pragma omp parallel shared(A, transposedB, res, N, chunk) private(i, j, k, jj, kk, tmp)
{
  #pragma omp for schedule (static, chunk)
  for (jj = 0; jj < N; jj += block_size)
  {
    for (kk = 0; kk < N; kk += block_size)
    {
      for (i = 0; i < N; i++)
      {
        for (j = jj; j < ((jj + block_size) > N ? N : (jj + block_size)); j++)
        {
          tmp = 0.0f;
          for (k = kk; k < ((kk + block_size) > N ? N : (kk + block_size)); k++)
          {
            tmp += A[i][k] * transposedB[j][k];
          }
          res[i][j] += tmp;
        }
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




