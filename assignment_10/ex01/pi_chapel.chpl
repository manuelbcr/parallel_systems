use Time;
use Random;

var stopwatch: Timer;

config var N = 10;
const seed = (getCurrentTime()+here.id) : int;


writeln("Computing PI with Montecarlo method!");
writeln("Number of iterations: ",N);



stopwatch.start();

var thread_count : int = here.maxTaskPar;
var inlier_counts: [1..thread_count] int;
coforall i in 1..(thread_count){
    var inlier_counter_task : int = 0;
    var random_num_generator = createRandomStream(real, seed+i, false);
    for i in ((i-1)*N/thread_count+1)..(i*N/thread_count){
        if((random_num_generator.getNext()**2 + random_num_generator.getNext()**2) <= 1.0){
            inlier_counter_task += 1;
        }   
    }
    inlier_counts[i] = inlier_counter_task;
    
}


var inliers : int = 0;
for i in 1..thread_count do
  inliers += inlier_counts[i];

var approximated_pi = (inliers*4.0)/N;
stopwatch.stop();
writeln("PI = ",approximated_pi," (",N," iterations computed)");
writeln("The computation took ",stopwatch.elapsed(),"s");


