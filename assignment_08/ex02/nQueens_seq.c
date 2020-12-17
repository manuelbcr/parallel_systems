// this implementation is based on an online example: https://www.geeksforgeeks.org/n-queen-problem-backtracking-3/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ---------------- Chessboard ---------------------

typedef int **Chessboard;

Chessboard init_chessboard(int N);

void free_chessboard(Chessboard m, int N);

// ---------------- nQueens ---------------------

void printSolution(Chessboard chessboard, int N);

bool is_safe(Chessboard chessboard, int row, int column, int N);
 
bool place_queen(Chessboard chessboard, int column, int N, int * number_of_solutions);
  
bool solve(int N);

// ---------------- Main ---------------------

int main(int argc, char **argv){

  // number of queens and NxN chessboard
  int N = 8;

  if (argc == 2) {
    N = atoi(argv[1]);
  } else {
    printf("[USAGE]: ./nQueens <number of queens>\n");
    printf("[INFO]: ./nQueens 8 is executed now\n");
  }

  solve(N);

}

// ---------------- Function Definitions ---------------------

bool solve(int N){ 

  Chessboard chessboard = init_chessboard(N);

  int number_of_solutions = 0;
  
  if(place_queen(chessboard, 0, N, &number_of_solutions) == false){
    printf("Solution does not exist"); 
    return false; 
  }else{
    printf("%d\n", number_of_solutions);
  }

  free_chessboard(chessboard, N);
  
  return true; 
}

bool place_queen(Chessboard chessboard, int column, int N, int * number_of_solutions){
  
  bool result = false;

  if (column == N){
    printSolution(chessboard, N);
    *number_of_solutions = *number_of_solutions +1;
    result = true;
  }
  
  // iterate over all elements in a column
  for (int i = 0; i < N; i++) { 

    /* Check if the queen can be placed on 
    board[i][col] */
    if (is_safe(chessboard, i, column, N)) { 
      
      //if save place queen
      chessboard[i][column] = 1; 
  
      // if save go to next column
      if (place_queen(chessboard, column + 1, N, number_of_solutions)) {
        result = true;
      } 
  
      chessboard[i][column] = 0; // BACKTRACK 
    } 
  } 
  
    return result; 
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


  
