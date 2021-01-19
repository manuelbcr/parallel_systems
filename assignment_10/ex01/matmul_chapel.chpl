use Time;
use Random;

var stopwatch: Timer;

config var N = 10;


writeln("Computing matrix multiplication!");

stopwatch.start();
stopwatch.stop();
writeln("The computation took ",stopwatch.elapsed(),"s");


