use Time;
use Random;
use CyclicDist;

var stopwatch: Timer;

config var N = 10;
const seed = (getCurrentTime()+here.id) : int;


writeln("Computing PI with Montecarlo method!");
writeln("Number of iterations: ",N);

var random_num_generator = new RandomStream(real, seed);

stopwatch.start();
var inlier_counter : sync int = 0;

const IndexSpace = {1..N} dmapped Cyclic(startIdx=1);

forall i in IndexSpace {
    if((random_num_generator.getNext()**2 + random_num_generator.getNext()**2) <= 1.0){
        inlier_counter += 1;
    }    
}

var approximated_pi = (inlier_counter*4.0)/N;
stopwatch.stop();
writeln("PI = ",approximated_pi," (",N," iterations computed)");
writeln("The computation took ",stopwatch.elapsed(),"s");
