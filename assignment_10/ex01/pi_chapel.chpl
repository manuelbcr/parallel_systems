use Time;
use Random;

config var N = 10;
const seed = (getCurrentTime()+here.id) : int;


writeln("Hello World!");
writeln(N);
writeln(seed);

var random_num_generator = new RandomStream(real, seed);


writeln(random_num_generator.getNext());
writeln(random_num_generator.getNext());
writeln(random_num_generator.getNext());
writeln(random_num_generator.getNext());