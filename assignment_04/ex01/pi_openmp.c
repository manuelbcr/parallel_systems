#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

const char USAGE_MSG[] = "USAGE: ./pi_seq <Number of samples>\n";

int main(int argc, char *argv[]){

    // start timer
   double start_execution_time = omp_get_wtime();

    unsigned long nr_samples = 0;
    long num_circle_pts = 0;

    double random_x = 0.0;
    double random_y = 0.0;
    double dist = 0.0;
    double pi = 0.0;

    if(argc == 2){

        // string to long(string, endpointer, base)
        nr_samples = strtoul(argv[1], NULL, 10);
        
        // check for meaningful input
        if(nr_samples <= 0){
            printf("Number of samples has to be an integer and bigger than 0.\n");    
            printf(USAGE_MSG);   
            return 1; 
        }        

    } else {
        printf(USAGE_MSG);
        return 1;
    }

    unsigned int seed;

    #pragma omp parallel for reduction(+ : num_circle_pts) private(random_x,random_y,dist,seed)
        for(unsigned long i = 0; i < nr_samples; i++){
            seed = time(NULL)+omp_get_thread_num();
            // random number between 0 and 1
            random_x = (double) rand_r(&seed) / RAND_MAX;
            random_y = (double) rand_r(&seed) / RAND_MAX;


            // distance from origin -> pythagoras
            dist = random_x * random_x + random_y * random_y;

            if(dist <= 1){

                num_circle_pts++;
                
            }
        }
    
    pi = 4.0* ((double)num_circle_pts / (double)nr_samples); 
    double end_time = omp_get_wtime();

    printf("pi: %lf (%ld samples) (in %f)\n", pi,nr_samples, end_time-start_execution_time);

    return 0;
    
}
