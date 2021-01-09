// this implementation is based on an online example: https://www.geeksforgeeks.org/n-queen-problem-backtracking-3/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>

// ---------------- Chessboard ---------------------

typedef int **Chessboard;

Chessboard init_chessboard(int N);

void free_chessboard(Chessboard m, int N);

// ---------------- nQueens ---------------------

void printSolution(Chessboard chessboard, int N);

bool is_safe(Chessboard chessboard, int row, int column, int N);
 
void place_queen(int queens[], int row, int col, int N);
  
void solve(int N);

// ---------------- Main ---------------------

int nrOfSolutions=0;

int main(int argc, char **argv){

  // number of queens and NxN chessboard
  int N = 8;

  if (argc == 2) {
    N = atoi(argv[1]);
  } else {
    printf("[USAGE]: ./nQueens <number of queens>\n");
    printf("[INFO]: ./nQueens 8 is executed now\n");
  }
  
  double start = omp_get_wtime();
  solve(N);
  double end = omp_get_wtime(); 

  printf("Number of solutions: %d\n", nrOfSolutions);
  printf("Work took %f seconds\n", end - start);
}

// ---------------- Function Definitions ---------------------

void solve(int N){ 
  #pragma omp parallel
  #pragma omp single
  {
    for(int i=0; i<N; i++) {
      
      int queens[N];
      #pragma omp task
      place_queen(queens, 0, i, N);
    }
  }
  return;
}

void place_queen(int queens[], int row, int col, int N){
  for(int i=0; i<row; i++) {
    // vertical attacks
    if (queens[i]==col) {
      return;
    }
    // diagonal attacks
    if (abs(queens[i]-col) == (row-i) ) {
      return;
    }
  }
  // column is ok, set the queen
  queens[row]=col;

  if(row==N-1) {


    // only one thread should print allowed to print at a time
    #pragma omp critical
    {
      // increasing the solution counter is not atomic      
      nrOfSolutions++;
    }

  }
  else {
    // try to fill next row
    for(int i=0; i<N; i++) {
      place_queen(queens, row+1, i, N);
    }
  }
}


bool is_safe(Chessboard chessboard, int row, int column, int N) { 
    int i, j; 
  
    /* Check this row on left side */
    for (i = 0; i < column; i++) 
        if (chessboard[row][i]) 
            return false; 
  
    /* Check upper diagonal on left side */
    for (i = row, j = column; i >= 0 && j >= 0; i--, j--) 
        if (chessboard[i][j]) 
            return false; 
  
    /* Check lower diagonal on left side */
    for (i = row, j = column; j >= 0 && i < N; i++, j--) 
        if (chessboard[i][j]) 
            return false; 
      
    return true; 
} 

void printSolution(Chessboard chessboard, int N){ 
    for (int i = 0; i < N; i++) { 
        for (int j = 0; j < N; j++) 
            printf(" %d ", chessboard[i][j]); 
        printf("\n"); 
    }
    printf("\n");
} 

Chessboard init_chessboard(int N) {
  
  //malloc chessboard
  Chessboard chessboard = (int **) malloc(N * sizeof(int *)); 
  for (int i=0; i<N; i++) {
    chessboard[i] = (int *) malloc(N * sizeof(int)); 
  }

  // init chessboard with no queens (0)
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      chessboard[i][j] = 0; 
    }
  }
   
  return chessboard;
}

void free_chessboard(Chessboard chessboard, int N) { 
  for(int i=0; i < N; i++){
    free(chessboard[i]); 
  }
  free(chessboard); 
}


  
