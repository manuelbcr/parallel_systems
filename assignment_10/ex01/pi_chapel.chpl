use Time;
use Random;

config var N = 10;
const seed = (getCurrentTime()+here.id) : int;


writeln("Hello World!");
writeln(N);
writeln(seed);

var random_num_generator = new RandomStream(real, seed);


var inlier_counter : sync int = 0;

forall i in 1..N {
    if((random_num_generator.getNext()**2 + random_num_generator.getNext()**2) <= 1.0){
        inlier_counter += 1;
    }    
}

var approximated_pi = (inlier_counter*4.0)/N;

writeln("PI = ",approximated_pi," (",N," iterations computed)");


