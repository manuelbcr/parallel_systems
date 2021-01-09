// this implementation is based on an online example: https://www.geeksforgeeks.org/n-queen-problem-backtracking-3/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>


// ---------------- nQueens ---------------------

void printSolution(int queens_list[], int N);

void place_queen(int queens_list[], int row, int col, int N);
  
void solve(int N);

// ---------------- Main ---------------------

int solutions_counter = 0;

int main(int argc, char **argv){

  // number of queens and NxN chessboard
  int N = 8;

  if (argc == 2) {
    N = atoi(argv[1]);
  } else {
    printf("[USAGE]: ./nQueens <number of queens>\n");
    printf("[INFO]: ./nQueens 8 is executed now\n");
  }
  
  clock_t start = clock();
  solve(N);
  clock_t end = clock();
  
  printf("Number of solutions: %d\n", solutions_counter);
  printf("The process took %f seconds to finish. \n", ((double)(end - start)) / CLOCKS_PER_SEC);
}

// ---------------- Function Definitions ---------------------

void solve(int N){ 
  for(int i=0; i<N; i++) {
    
    int queens_list[N];

    place_queen(queens_list, 0, i, N);
  }
}

// function to place a queen if it is possible
void place_queen(int queens_list[], int row, int col, int N){
  // check for all rows/ queens if there is a conflict 
  for(int i=0; i<row; i++) {
    // check whether it is only queen in column
    if (queens_list[i]==col) {
      return;
    }
    // check diagonals
    if (abs(queens_list[i]-col) == (row-i) ) {
      return;
    }
  }

  // if there was a conflict the function already returned so we can assume that the queen can be placed here
  queens_list[row]=col;

  // if last row is reached = no conflicts = valid solution
  if(row == N-1) {
      // increment the number of solutions  
      solutions_counter++;
      // print solution
      printSolution(queens_list, N);
  }
  else {
    // if not last row is reached the next row has to be checked
    for(int i=0; i<N; i++) {
      place_queen(queens_list, row+1, i, N);
    }
  }
}

// function to print the solution
void printSolution(int queens_list[], int N){ 
    for (int i = 0; i < N; i++) { 
        for (int j = 0; j < N; j++) {
          if(queens_list[i] == j){
            printf(" X "); 
          }
          else{
            printf(" - ");
          }
        }

            
        printf("\n"); 
    }
    printf("\n");
    printf("#########################\n");
    printf("\n");
} 


  
