#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define G 1.0
#define DT 0.05

// particle struct
typedef struct {
	double position_x;
  double position_y;
  double velocity_x;
  double velocity_y;
  double mass;
} particle;

particle * init_particles(int number_of_particles, double max_x, double max_y, double max_mass);
double compute_vector_length(double x, double y);
double compute_force(particle particle_1, particle particle_2);
void update_position(double unit_vector_x, double unit_vector_y, double force, particle * particle);
void print_particle(particle particle);
void print_particle_array(particle * particle_array, int number_of_particles);

int main(int argc, char **argv) {

  // runtime variables
  int number_of_particles = 10000;
  const int number_of_timesteps = 1000;

  const double max_x = 100.0;
  const double max_y = 100.0;
  const double max_mass = 50.0;

  if (argc > 1) {
    number_of_particles = atoi(argv[1]);
  } else if(argc > 2) {
    printf("USAGE: please provide an input for the number of particles\n");
    return(EXIT_FAILURE);
  }

  // particle array of length number_of_particles (temp array to switch them later)
  particle * particle_array_a = malloc(sizeof(particle) * number_of_particles);
  particle * particle_array_b = malloc(sizeof(particle) * number_of_particles);
  particle * particle_array_temp = NULL;

  // current time as seed for random generator 
  srand(time(NULL));

  // initialize arrays
  for(int i = 0; i < number_of_particles; i++){

    particle_array_a[i].position_x = particle_array_b[i].position_x = ((double)rand() / RAND_MAX) * max_x; 
    particle_array_a[i].position_y = particle_array_b[i].position_y = ((double)rand() / RAND_MAX) * max_y; 
    particle_array_a[i].velocity_x = particle_array_b[i].velocity_x = 0.0;
    particle_array_a[i].velocity_y = particle_array_b[i].velocity_y = 0.0;
    particle_array_a[i].mass = particle_array_b[i].mass = ((double)rand() / RAND_MAX) * max_mass; 
  
  }
  
  for(int t = 0; t < number_of_timesteps; t++){

    printf("######## Timestep %d###########\n",t);

    for(int i = 0; i < number_of_particles; i++){

      double force = 0.0;
      double direction_vector_x = 0.0;
      double direction_vector_y = 0.0;

      for(int j = 0; j < number_of_particles; j++){

        if(i != j){

          force += compute_force(particle_array_a[i], particle_array_a[j]); 

          // direction vector a -> b = b - a  (sum the direction vectors to get one resulting direction vector)
          direction_vector_x = direction_vector_x + (particle_array_a[j].position_x - particle_array_a[i].position_x);
          direction_vector_y = direction_vector_y + (particle_array_a[j].position_y - particle_array_a[i].position_y);

        }

      }

      double vector_length = compute_vector_length(direction_vector_x, direction_vector_y);

      // calculate unit vector of direction vector
      double unit_vector_x = direction_vector_x/vector_length;
      double unit_vector_y = direction_vector_y/vector_length;

      // update in array_b to make all calculations be based on the same timestep
      update_position(unit_vector_x, unit_vector_y, force, &particle_array_b[i]);

    }

    //print_particle_array(particle_array_b, number_of_particles);

    particle_array_temp = particle_array_b;
    particle_array_b = particle_array_a;
    particle_array_a = particle_array_temp; 

  }
}

double compute_vector_length(double x, double y){

  return sqrt(pow(x,2) + pow(y,2));

}

double compute_force(particle particle_1, particle particle_2){

  double direction_vector_x = 0.0;
  double direction_vector_y = 0.0;

  // direction vector a -> b = b - a
  direction_vector_x = direction_vector_x + (particle_2.position_x - particle_1.position_x);
  direction_vector_y = direction_vector_y + (particle_2.position_y - particle_1.position_y);

  // pythagoras: distance between two points
  double distance = compute_vector_length(direction_vector_x, direction_vector_y);
  double force = G * (particle_1.mass * particle_2.mass) / distance;

  return force;
}

void update_position(double unit_vector_x, double unit_vector_y, double force, particle * particle){

  particle->velocity_x = particle->velocity_x + (unit_vector_x * (force / particle->mass));
  particle->velocity_y = particle->velocity_y + (unit_vector_y * (force / particle->mass));

  particle->position_x = particle->position_x + (particle->velocity_x * DT);
  particle->position_y = particle->position_y + (particle->velocity_y * DT);

}

void print_particle(particle particle){

  printf("(%lf, %lf)", particle.position_x, particle.position_y);

}

void print_particle_array(particle * particle_array, int number_of_particles){

  for(int i = 0; i < number_of_particles; i++){

    print_particle(particle_array[i]);

    if(((i % 5) == 0) && (i != 0)){

      printf("\n");

    }
  }
}

