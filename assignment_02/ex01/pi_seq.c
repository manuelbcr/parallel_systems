#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]){

    unsigned long nr_samples = 0;
    long num_circle_pts = 0;
    long num_all_pts = 0;

    double random_x = 0.0;
    double random_y = 0.0;
    double dist = 0.0;
    double pi = 0.0;

    if(argc == 2){

        // string to long(string, endpointer, base)
        nr_samples = strtoul(argv[1], NULL, 10);

    } else {

        printf("USAGE: ./pi_seq <Number of samples>\n");
        return 1;

    }

    // current time as seed for random generator 
    srand(time(NULL));

    for(unsigned long i = 0; i < nr_samples; i++){

        // random number between 0 and 1
        random_x = (double)rand() / RAND_MAX; 
        random_y = (double)rand() / RAND_MAX;

        // distance from origin -> pythagoras
        dist = random_x * random_x + random_y * random_y;

        if(dist <= 1){

            num_circle_pts++;
            
        }

        num_all_pts = num_all_pts + 1;

        pi = 4 * ((double)num_circle_pts / (double)num_all_pts); 
    }

    printf("callculated pi with %ld iterations: %lf\n", iterations, pi);

    return 0;
    
}