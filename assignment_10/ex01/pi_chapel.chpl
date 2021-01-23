use Time;
use Random;

var stopwatch: Timer;

config var N = 10;
const seed = (getCurrentTime()+here.id) : int;


writeln("Computing PI with Montecarlo method!");
writeln("Number of iterations: ",N);

var random_num_generator = new RandomStream(real, seed);

stopwatch.start();
var inlier_counter : int = 0;

forall i in 1..N with (+ reduce inlier_counter){
    var random_pair = random_num_generator.iterate({0..1}, resultType=real); 
    if((random_pair[0]**2 + random_pair[1]**2) <= 1.0){
        inlier_counter += 1;
    }    
}


var approximated_pi = (inlier_counter*4.0)/N;
stopwatch.stop();
writeln("PI = ",approximated_pi," (",N," iterations computed)");
writeln("The computation took ",stopwatch.elapsed(),"s");


