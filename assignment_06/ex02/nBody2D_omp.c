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


// quadtree node struct (north-west, north-east, south-west, south-east)
typedef struct quadtree_node{
  struct quadtree_node* nw;
  struct quadtree_node* ne;
  struct quadtree_node* sw;
  struct quadtree_node* se;
  particle* particle;
  double min_x;
  double max_x;
  double min_y;
  double max_y;  
} quadtree_node;



particle * init_particles(int number_of_particles, double max_x, double max_y, double max_mass);
double compute_squared_vector_length(vector vector);
vector compute_force(particle* current_particle, quadtree_node* tree);
void update_position(vector force, particle * particle, double max_x, double max_y);
void print_particle(particle* particle);
void print_particle_array(particle * particle_array, int number_of_particles);
quadtree_node* create_tree_node(particle* particle, double min_x, double max_x, double min_y, double max_y);
void destroy_tree(quadtree_node* root);
void add_particle(quadtree_node* tree, particle* particle);
particle* set_node_mass(quadtree_node* tree);
double compute_theta(quadtree_node* current_node, particle* particle);
int is_leaf(quadtree_node* node);

#ifdef PLOT

#include "pbPlots.h"
#include "supportLib.h"
void plot_particle_array(particle * particle_array, int number_of_particles, int p);

#endif


int main(int argc, char **argv) {

  const double max_xi = 100.0;
  const double max_yi = 100.0;
  const double max_massi = 50.0;
  int number_of_particlesi = 100;

  particle * particle_array = malloc(sizeof(particle) * number_of_particlesi);
  
  quadtree_node *tree;
  tree = create_tree_node(NULL, 0, max_xi, 0, max_yi);
  double pm = 0.0;
  double px = 0.0;
  double py = 0.0;
  for(int i = 0; i < number_of_particlesi; i++){

    particle_array[i].position_x = ((double)rand() / RAND_MAX) * max_xi; 
    particle_array[i].position_y = ((double)rand() / RAND_MAX) * max_yi; 
    particle_array[i].velocity_x = 0.0;
    particle_array[i].velocity_y = 0.0;
    particle_array[i].mass = ((double)rand() / RAND_MAX) * max_massi; 
    pm += particle_array[i].mass;
    px += particle_array[i].position_x*particle_array[i].mass;
    py += particle_array[i].position_y*particle_array[i].mass;
    add_particle(tree, &particle_array[i]);
  }

  particle* nm = set_node_mass(tree);

  
  printf("OVERALL-mass: %f at [%f, %f]\n", nm->mass, nm->position_x, nm->position_y);
  printf("OVERALL-mass-groundtruth: %f at [%f, %f]\n", pm, px/pm, py/pm);
  
  vector F = compute_force(&particle_array[0], tree);
  

  printf("F = [%f, %f]\n", F.x, F.y);
  printf("BEFORE destroy\n");
  destroy_tree(tree);
  printf("AFTER destroy\n");
  free(particle_array);
  printf("AFTER particles free\n");
  return 0; 

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
          //vector computed = compute_force(&particle_array_a[i], &particle_array_a[j]); 
          //force.x += computed.x;
          //force.y += computed.y;          
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

vector compute_force(particle* current_particle, quadtree_node* tree){
  
  vector force, tmp_force;
  force.x = 0.0;
  force.y = 0.0;
  if(tree == NULL || tree->particle == NULL){
    return force;
  }
    

  double theta = compute_theta(tree, current_particle);
  if(theta < 1){
    particle* node_particle = tree->particle;
    double diff_x = node_particle->position_x - current_particle->position_x;
    double diff_y = node_particle->position_y - current_particle->position_y;
    double denom_xy = sqrt(diff_x*diff_x + diff_y*diff_y);
    force.x = G * node_particle->mass * current_particle->mass * (1/(denom_xy*denom_xy*denom_xy)) * diff_x;
    force.y = G * node_particle->mass * current_particle->mass * (1/(denom_xy*denom_xy*denom_xy)) * diff_y; 
    
  }
  else{
    printf("THeta bigger 1...\n");
    for(int i = 0; i < 4; i++){
      if(i == 0)
        tmp_force = compute_force(current_particle, tree->nw);
      else if(i == 1)
        tmp_force = compute_force(current_particle, tree->ne);
      else if(i == 2)
        tmp_force = compute_force(current_particle, tree->sw);
      else
        tmp_force = compute_force(current_particle, tree->se);

      force.x += tmp_force.x;
      force.y += tmp_force.y;
    }    
  }
  printf("COMPUTE_FORCE: return (%f, %f)\n", force.x, force.y);
  return force;
}


// compute theta to determine if node is suitable
double compute_theta(quadtree_node* current_node, particle* particle){
  double dx = current_node->particle->position_x - particle->position_x;
  double dy = current_node->particle->position_y - particle->position_y;
  double r = sqrt(dx*dx + dy*dy);
  return (current_node->max_x - current_node->min_x)/r;
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

// create tree node - if particle = NULL it does not hold any particle
quadtree_node* create_tree_node(particle* particle, double min_x, double max_x, double min_y, double max_y){
  quadtree_node* new_node = (quadtree_node*) malloc(sizeof(quadtree_node));
  new_node->nw = NULL;
  new_node->ne = NULL;
  new_node->sw = NULL;
  new_node->se = NULL;
  new_node->particle = particle;
  new_node->min_x = min_x;
  new_node->max_x = max_x;
  new_node->min_y = min_y;
  new_node->max_y = max_y;

  return new_node;
}

void destroy_tree(quadtree_node* root){
  // free particle = centered mass only if it was set and is an internal node 
  if((is_leaf(root) == 2) && (root->particle != NULL)){
    free(root->particle);
  }

  if(root->nw != NULL)
    destroy_tree(root->nw);

  if(root->ne != NULL)
    destroy_tree(root->ne);

  if(root->sw != NULL)
    destroy_tree(root->sw);

  if(root->se != NULL)
    destroy_tree(root->se);


  free(root);
}

// function to add an particle to the quadtree
void add_particle(quadtree_node* tree, particle* new_particle){
  double x_half = (tree->max_x-tree->min_x)/2;
  double y_half = (tree->max_y-tree->min_y)/2;
  
  int node_type = is_leaf(tree); 
  
  //printf("%p (type=%d): range[%f-%f, %f-%f] - particle=%p :: add=%p(%f, %f)\n", (void*)tree, node_type, tree->min_x, tree->max_x, tree->min_y, tree->max_y, (void*)tree->particle, (void*)new_particle, new_particle->position_x, new_particle->position_y);
  
  // empty leaf can be filled
  if(node_type == 2){
    tree->particle = new_particle;
  }
  // if filled leaf -> split it
  else if(node_type == 1){
    tree->nw = create_tree_node(NULL, tree->min_x, tree->min_x+x_half, tree->min_y, tree->min_y+y_half);
    tree->ne = create_tree_node(NULL, tree->min_x+x_half, tree->max_x, tree->min_y, tree->min_y+y_half);
    tree->sw = create_tree_node(NULL, tree->min_x, tree->min_x+x_half, tree->min_y+y_half, tree->max_y);
    tree->se = create_tree_node(NULL, tree->min_x+x_half, tree->max_x, tree->min_y+y_half, tree->max_y);
    
    add_particle(tree, tree->particle);    
    add_particle(tree, new_particle);
    tree->particle = NULL;
  }
  // as long as no leaf - call recursively
  else{
    if(new_particle->position_x <= tree->min_x+x_half && new_particle->position_y <= tree->min_y+y_half){
      add_particle(tree->nw, new_particle);
    }
    else if(new_particle->position_x > tree->min_x+x_half && new_particle->position_y <= tree->min_y+y_half){
      add_particle(tree->ne, new_particle);
    }
    else if(new_particle->position_x <= tree->min_x+x_half && new_particle->position_y > tree->min_y+y_half){
      add_particle(tree->sw, new_particle);
    }
    else{
      add_particle(tree->se, new_particle);
    }
  }
  
}

// function to add an particle to the quadtree
particle* set_node_mass(quadtree_node* tree){
  particle* new_mass = (particle*) malloc(sizeof(particle));
  new_mass->mass = 0.0;
  new_mass->position_x = 0.0;
  new_mass->position_y = 0.0;
  
  int node_type = is_leaf(tree);
  if((tree == NULL) || node_type == 2){
    tree->particle = new_mass;
    return new_mass;
  }
    

  
  if(node_type == 1){
    new_mass->mass = tree->particle->mass;
    new_mass->position_x = tree->particle->position_x;
    new_mass->position_y = tree->particle->position_y;
  }
  else{
    particle* tmp_node_mass;
    for(int i=0; i<4; i++){
      if(i == 0)
        tmp_node_mass = set_node_mass(tree->nw);
      else if(i == 1)
        tmp_node_mass = set_node_mass(tree->ne);
      else if(i == 2)
        tmp_node_mass = set_node_mass(tree->sw);
      else
        tmp_node_mass = set_node_mass(tree->se);
      

      new_mass->mass += tmp_node_mass->mass;
      new_mass->position_x += tmp_node_mass->position_x * tmp_node_mass->mass;
      new_mass->position_y += tmp_node_mass->position_y * tmp_node_mass->mass;

    }  
  
    new_mass->position_x = new_mass->position_x / new_mass->mass;
    new_mass->position_y = new_mass->position_y / new_mass->mass;
  }

  tree->particle = new_mass;
  
  return new_mass;  
}


// check whether a node is a empty leaf = no particle (2) or a filled leaf (1) or no leaf at all (0)
int is_leaf(quadtree_node* node){
  if(node->nw != NULL 
    || node->ne != NULL
    || node->sw != NULL
    || node->se != NULL
  ){
    return 0;
  }

  if(node->particle == NULL){
    return 2;
  }

  return 1;
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
