#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define G 1.0
#define DT 0.05
#define EPS 0.0001

// particle struct
typedef struct {
	double position_x;
  double position_y;
  double velocity_x;
  double velocity_y;
  double mass;
} particle;

// particle struct
typedef struct {
	double x;
  double y;
} vector;

particle * init_particles(int number_of_particles, double max_x, double max_y, double max_mass);
double compute_squared_vector_length(vector vector);
vector compute_force(particle particle_1, particle particle_2);
void update_position(vector force, particle * particle, double max_x, double max_y);
void print_particle(particle particle);
void print_particle_array(particle * particle_array, int number_of_particles);

int main(int argc, char **argv) {

  // runtime variables
  int number_of_particles = 3;
  const int number_of_timesteps = 10000;

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

    //printf("######## Timestep %d###########\n",t);

    for(int i = 0; i < number_of_particles; i++){

      vector force;
      force.x = 0;
      force.y = 0;
     

      for(int j = 0; j < number_of_particles; j++){

        if(i != j){
          vector computed = compute_force(particle_array_a[i], particle_array_a[j]); 
          force.x += computed.x;
          force.y += computed.y;
          
        }

      }
      // update in array_b to make all calculations be based on the same timestep
      
      update_position(force, &particle_array_b[i], max_x, max_y);

    }

    //if(t % 10 == 0){
    //  print_particle_array(particle_array_b, number_of_particles);
    //}
    

    particle_array_temp = particle_array_b;
    particle_array_b = particle_array_a;
    particle_array_a = particle_array_temp; 

  }
}

double compute_squared_vector_length(vector vector){

  return pow(vector.x,2) + pow(vector.y,2);

}

vector compute_force(particle particle_1, particle particle_2){

  vector force;
  vector direction_vector;
  vector unit_vector;

   // direction vector a -> b = b - a
  direction_vector.x = particle_2.position_x - particle_1.position_x;
  direction_vector.y = particle_2.position_y - particle_1.position_y;

  double squared_distance = compute_squared_vector_length(direction_vector);
  // pythagoras: distance between two points
  double distance = sqrt(squared_distance);

  unit_vector.x = direction_vector.x / distance;
  unit_vector.y = direction_vector.y / distance;

  force.x = (G * (particle_1.mass * particle_2.mass) / pow(squared_distance + EPS, 3.0/2.0)) * unit_vector.x;
  force.y = (G * (particle_1.mass * particle_2.mass) / pow(squared_distance + EPS, 3.0/2.0)) * unit_vector.y;

  return force;
}

void update_position(vector force, particle * particle, double max_x, double max_y){

  particle->velocity_x = particle->velocity_x + (force.x / particle->mass * DT);
  particle->velocity_y = particle->velocity_y + (force.y / particle->mass * DT);

  double position_x = particle->position_x + (particle->velocity_x * DT);
  double position_y = particle->position_y + (particle->velocity_y * DT);

  if((particle->velocity_x > 100) || (particle->velocity_y > 100)){

    printf("Force (%lf, %lf)\n", force.x, force.y);

  }

  if(position_x > max_x){

    //printf("[positon_x > max]: \n");
    //printf("position before: %lf\n", position_x);

    // bounce of wall (invert velocity_x)
    particle->velocity_x = particle->velocity_x * -1;
    position_x = max_x + (particle->velocity_x * DT);

    //printf("velocity after: %lf\n", particle->velocity_x);
    //printf("position after: %lf\n", position_x);
  }

   if(position_x < 0.0){

    //printf("[positon_x > max]: \n");
    //printf("position before: %lf\n", position_x);

    // bounce of wall (invert velocity_x)
    particle->velocity_x = particle->velocity_x * -1;
    position_x = 0.0 + (particle->velocity_x * DT);

    //printf("velocity after: %lf\n", particle->velocity_x);
    //printf("position after: %lf\n", position_x);

  }

  if(position_y > max_y){

    //printf("[positon_y > max]: \n");
    //printf("position before: %lf\n", position_y);

    // bounce of wall (invert velocity_y)
    particle->velocity_y = particle->velocity_y * -1;
    position_y = max_y + (particle->velocity_y * DT);

    //printf("velocity after: %lf\n", particle->velocity_y);
    //printf("position after: %lf\n", position_y);

  }

  if(position_y < 0.0){

    //printf("[positon_y < 0]: \n");
    //printf("position before: %lf\n", position_y);

    // bounce of wall (invert velocity_y)
    particle->velocity_y = particle->velocity_y * -1;
    position_y = 0.0 + (particle->velocity_y * DT);

    //printf("velocity after: %lf\n", particle->velocity_y);
    //printf("position after: %lf\n", position_y);

  }
  
  particle->position_x = position_x;
  particle->position_y = position_y;

}

void print_particle(particle particle){

  printf("%lf, %lf;", particle.position_x, particle.position_y);

}

void print_particle_array(particle * particle_array, int number_of_particles){

  for(int i = 0; i < number_of_particles; i++){

    print_particle(particle_array[i]);
    
  }

  printf("\n");

}

