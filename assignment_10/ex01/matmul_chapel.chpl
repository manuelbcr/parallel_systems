use Time;
use LinearAlgebra;

var stopwatch: Timer;

config var N = 10;


writeln("Computing matrix multiplication!");

var M_lhs: [0..N-1, 0..N-1] real;
var M_rhs: [0..N-1, 0..N-1] real;

// initialize M_lhs with i*j and M_rhs as identity matrix
forall i in 0..N-1 {
    forall j in 0..N-1 {
        M_lhs[i,j] = (i*j) : real;
        if(i == j){
            M_rhs[i, j] = 1;
        }
    }
}

var M_rhs_T = transpose(M_rhs);

var M_result: [0..N-1, 0..N-1] real;
stopwatch.start();

// do the matrix multiplication
forall i in 0..N-1 {
    forall j in 0..N-1 {
        for k in 0..N-1{
            M_result[i, j] += M_lhs[i,k]*M_rhs_T[j,k];
        }
    }
}


stopwatch.stop();

//writeln("Resulting matrix:");
//writeln(M_result);

// verification
var success_matmul: bool = true;

// reduction necessary in order to check it in parallel
forall i in 0..N-1 with (&& reduce success_matmul) {    
    forall j in 0..N-1 with (&& reduce success_matmul) {         
        if(M_lhs[i,j] != M_result[i,j]){
            success_matmul = false;
        }
    }
}

writeln("Verification: ",success_matmul);

writeln("The computation took ",stopwatch.elapsed(),"s");


