#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <omp.h>



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
vector compute_force(particle* particle_1, particle* particle_2);
void update_position(vector force, particle * particle, double max_x, double max_y);
void print_particle(particle* particle);
void print_particle_array(particle * particle_array, int number_of_particles);

#ifdef PLOT

#include "pbPlots.h"
#include "supportLib.h"
void plot_particle_array(particle * particle_array, int number_of_particles, int p);

#endif


int main(int argc, char **argv) {

  // runtime variables
  int number_of_particles = 10000;
  const int number_of_timesteps = 100;

  const double max_x = 100.0;
  const double max_y = 100.0;
  const double max_mass = 50.0;

  if (argc > 2) {
    printf("USAGE: ./nBody2D\nOR: ./nBody2D <number-of-inputs>\n");
    return(EXIT_FAILURE);    
  } 
  else if(argc > 1) {
    number_of_particles = atoi(argv[1]);
  }
  
  // start measuring time
  struct timeval tval_start, tval_end;
  gettimeofday(&tval_start, NULL);

  // particle array of length number_of_particles (temp array to switch them later)
  particle * particle_array_a = malloc(sizeof(particle) * number_of_particles);
  particle * particle_array_b = malloc(sizeof(particle) * number_of_particles);
  particle * particle_array_temp = NULL;

  // current time as seed for random generator 
  srand(time(NULL));

  // initialize arrays
  // TODO: test it if gets faster if we parallelize also this step...
  for(int i = 0; i < number_of_particles; i++){

    particle_array_a[i].position_x = particle_array_b[i].position_x = ((double)rand() / RAND_MAX) * max_x; 
    particle_array_a[i].position_y = particle_array_b[i].position_y = ((double)rand() / RAND_MAX) * max_y; 
    particle_array_a[i].velocity_x = particle_array_b[i].velocity_x = 0.0;
    particle_array_a[i].velocity_y = particle_array_b[i].velocity_y = 0.0;
    particle_array_a[i].mass = particle_array_b[i].mass = ((double)rand() / RAND_MAX) * max_mass; 
  
  }
  
  // simulate for number_of_timesteps timesteps
  for(int t = 0; t < number_of_timesteps; t++){

    printf("######## Timestep %d###########\n",t);

    // for each particle
    for(int i = 0; i < number_of_particles; i++){

      vector force;
      force.x = 0;
      force.y = 0;
     
      // compute force with each other particle
      for(int j = 0; j < number_of_particles; j++){

        if(i != j){
          vector computed = compute_force(&particle_array_a[i], &particle_array_a[j]); 
          force.x += computed.x;
          force.y += computed.y;          
        }

      }

      // update in array_b instead of array_a to make all calculations be based on the same timestep      
      update_position(force, &particle_array_b[i], max_x, max_y);

    }

    // print each 10-th state
    if(t % 10 == 0){
      print_particle_array(particle_array_b, number_of_particles);
    }
    
    #ifdef PLOT
    // plot current state
    plot_particle_array(particle_array_b,number_of_particles,t);
    #endif


    particle_array_temp = particle_array_b;
    particle_array_b = particle_array_a;
    particle_array_a = particle_array_temp; 

  }

  gettimeofday(&tval_end, NULL);
  long timediff_s = tval_end.tv_sec - tval_start.tv_sec;
  long timediff_ms = ((timediff_s*1000000) + tval_end.tv_usec) - tval_start.tv_usec;
  printf("Time elapsed: %ld.%06ld\n", timediff_s, timediff_ms);  
}

double compute_vector_length(vector vector){

  return (double) sqrt((vector.x*vector.x) + (vector.y*vector.y));

}

vector compute_force(particle* particle_1, particle* particle_2){

  double calculated_force;
  vector force;
  vector direction_vector;
  force.x=0.0;
  force.y=0.0;

   // direction vector a -> b = b - a
  direction_vector.x = particle_1->position_x - particle_2->position_x;
  direction_vector.y = particle_1->position_y - particle_2->position_y;

  
  double distance = compute_vector_length(direction_vector);
  double d_eps = distance + EPS;
  calculated_force = G*(particle_1->mass*particle_2->mass)/sqrt(d_eps*d_eps*d_eps);
  
  force.x -= calculated_force/distance * direction_vector.x;
  force.y -= calculated_force/distance * direction_vector.y;
  return force;
}

void update_position(vector force, particle * particle, double max_x, double max_y){

  particle->velocity_x = particle->velocity_x + (force.x / particle->mass * DT);
  particle->velocity_y = particle->velocity_y + (force.y / particle->mass * DT);

  double position_x = particle->position_x + (particle->velocity_x * DT);
  double position_y = particle->position_y + (particle->velocity_y * DT);



  if(position_x > max_x){

    // bounce of wall (invert velocity_x)
    particle->velocity_x = particle->velocity_x * -1;
    position_x = max_x + (particle->velocity_x * DT);

  }

   if(position_x < 0.0){

    // bounce of wall (invert velocity_x)
    particle->velocity_x = particle->velocity_x * -1;
    position_x = 0.0 + (particle->velocity_x * DT);

  }

  if(position_y > max_y){

    // bounce of wall (invert velocity_y)
    particle->velocity_y = particle->velocity_y * -1;
    position_y = max_y + (particle->velocity_y * DT);

  }

  if(position_y < 0.0){

    // bounce of wall (invert velocity_y)
    particle->velocity_y = particle->velocity_y * -1;
    position_y = 0.0 + (particle->velocity_y * DT);

  }
  
  particle->position_x = position_x;
  particle->position_y = position_y;

}

void print_particle(particle* particle){

  printf("%lf, %lf;", particle->position_x, particle->position_y);

}

void print_particle_array(particle * particle_array, int number_of_particles){

  for(int i = 0; i < number_of_particles; i++){

    print_particle(&particle_array[i]);
    
  }

  printf("\n");

}


#ifdef PLOT

void plot_particle_array(particle * particle_array, int number_of_particles, int p){
  double xs[number_of_particles];
  double ys[number_of_particles];

  for(int i= 0; i< number_of_particles; i++){
    xs[i]= particle_array[i].position_x;
    ys[i]= particle_array[i].position_y;
  }

RGBABitmapImageReference *imageReference = CreateRGBABitmapImageReference();

	ScatterPlotSeries *series = GetDefaultScatterPlotSeriesSettings();
	series->xs = xs;
	series->xsLength = sizeof(xs)/sizeof(double);
	series->ys = ys;
	series->ysLength = sizeof(ys)/sizeof(double);
	series->linearInterpolation = false;
	series->pointType = L"dots";
	series->pointTypeLength = wcslen(series->pointType);
	series->color = CreateRGBColor(1, 0, 0);


	ScatterPlotSettings *settings = GetDefaultScatterPlotSettings();
	settings->width = 600;
	settings->height = 400;
	settings->autoBoundaries = true;
	settings->autoPadding = true;
	settings->title = L"";
	settings->titleLength = wcslen(settings->title);
	settings->xLabel = L"";
	settings->xLabelLength = wcslen(settings->xLabel);
	settings->yLabel = L"";
	settings->yLabelLength = wcslen(settings->yLabel);
	ScatterPlotSeries *s [] = {series};
	settings->scatterPlotSeries = s;
	settings->scatterPlotSeriesLength = 1;

	DrawScatterPlotFromSettings(imageReference, settings);

  char buffer[80];
  int output_len;
  output_len = snprintf( buffer, 80, "img%d.png",p);

	size_t length;
	double *pngdata = ConvertToPNG(&length, imageReference->image);
	WriteToFile(pngdata, length, buffer);
	DeleteImage(imageReference->image);
}

#endif
