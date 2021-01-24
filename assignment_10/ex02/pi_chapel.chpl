use Time;
use Random;

var stopwatch: Timer;

config var N = 10;
const seed = (getCurrentTime()+here.id) : int;


writeln("Computing PI with Montecarlo method!");
writeln("Number of iterations: ",N);



stopwatch.start();

var thread_count : int = here.maxTaskPar;
var inlier_counts_of_locales: [1..numLocales] int;

coforall l in 1..(numLocales){

    var N_loc = ((here.id+1)*N)/numLocales - ((here.id)*N)/numLocales;
    writeln(N_loc);
    var inlier_counts: [1..thread_count] int;
    inlier_counts_of_locales[l] = 0;
    // start task parallelism with coforall -> for each thread one portion of N_loc
    coforall i in 1..(thread_count){
        var inlier_counter_task : int = 0;
        // thread unsafe randomstream is much faster therefore parSafe=false
        var random_num_generator = createRandomStream(real, seed+here.id+i, false);
        for i in ((i-1)*N_loc/thread_count+1)..(i*N_loc/thread_count){
            if((random_num_generator.getNext()**2 + random_num_generator.getNext()**2) <= 1.0){
                inlier_counter_task += 1;
            }   
        }
        // when counter is written into array no locking is necessary
        inlier_counts[i] = inlier_counter_task;
        
    }

    for i in 1..thread_count do
        inlier_counts_of_locales[here.id] += inlier_counts[i];

    
}


// reduce results
var inliers : int = 0;
for i in 1..thread_count do
  inliers += inlier_counts[i];

var approximated_pi = (inliers*4.0)/N;
stopwatch.stop();
writeln("PI = ",approximated_pi," (",N," iterations computed)");
writeln("The computation took ",stopwatch.elapsed(),"s");


