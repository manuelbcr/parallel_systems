#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]){

    unsigned long nr_samples = 0;
    double pi = 0.0;

    if(argc == 2){

        // string to long(string, endpointer, base)
        nr_samples = strtoul(argv[1], NULL, 10);

    } else {

        printf("USAGE: ./pi_seq <Number of samples>\n");
        return 1;

    }

    // parallel part
    int rank, size;

    MPI_Init(&argc, &argv); //initialize MPI library
    MPI_Comm_size(MPI_COMM_WORLD, &size); //get number of MPI processes/ranks
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get the rank of the caller

    // current time as seed for random generator 
    srand(time(NULL)+rank);

    // number of points in the circle of each rank
    unsigned long local_nr_circle_pts = 0;

    for(unsigned long i = 0; i < (nr_samples/size); i++){

        // random number between 0 and 1
        double random_x = (double)rand() / RAND_MAX; 
        double random_y = (double)rand() / RAND_MAX;

        // distance from origin -> pythagoras
        double dist = random_x * random_x + random_y * random_y;

        if(dist <= 1){
            local_nr_circle_pts = local_nr_circle_pts + 1;
        }
    }

    printf("Number of points in circle of rank %d: [%ld/%ld]\n", rank, local_nr_circle_pts, (nr_samples/size));

    unsigned long global_nr_circle_pts = 0;
    // int MPI_Reduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
    // be careful with floating point types -> therefore number of points in circle
    MPI_Reduce(&local_nr_circle_pts, &global_nr_circle_pts, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank == 0){

        pi = 4 * ((double)global_nr_circle_pts / (double)nr_samples); 
        printf("Callculated pi with %ld samples: %lf\n", nr_samples, pi);

    }
    
    MPI_Finalize(); // MPI cleanup


    return 0;
    
}
