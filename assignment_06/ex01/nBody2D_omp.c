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


// computing force the effective force on a particle for a given tree
vector compute_force(particle* current_particle, quadtree_node* tree);
// compute new position of a particle for a given force
void update_position(vector force, particle * particle, double max_x, double max_y);
// printing particle position to stdout
void print_particle(particle* particle);
// print whole array of particles
void print_particle_array(particle * particle_array, int number_of_particles);
// creating a new tree node 
quadtree_node* create_tree_node(particle* particle, double min_x, double max_x, double min_y, double max_y);
// free complete tree again
void destroy_tree(quadtree_node* root);
// add a particle to a tree
void add_particle(quadtree_node* tree, particle* particle);
// recursively walk through tree and set masses of nodes based on children
particle* set_node_mass(quadtree_node* tree);
// compute threshold theta to decide if node can be used for force computation 
double compute_theta(quadtree_node* current_node, particle* particle);
// check whether a node is a leaf without a particle (=2), leaf with particle (=1) or internal node (=0)
int is_leaf(quadtree_node* node);

#ifdef PLOT

#include "pbPlots.h"
#include "supportLib.h"
void plot_particle_array(particle * particle_array, int number_of_particles, int p);

#endif


int main(int argc, char **argv) {

  // runtime variables
  int number_of_particles = 10000;
  int number_of_timesteps = 100;

  // size of world
  const double max_x = 100.0;
  const double max_y = 100.0;
  const double max_mass = 50.0;

  // get commandline arguments
  if (argc > 3) {
    printf("USAGE: ./nBody2D\nOR: ./nBody2D <number-of-inputs>\nOR: ./nBody2D <number-of-inputs> <number-of-timesteps>\n");
    return(EXIT_FAILURE);    
  } 
  else if(argc == 3) {
    number_of_timesteps = atoi(argv[2]);
    number_of_particles = atoi(argv[1]);
  }
  else if(argc == 2) {
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
    // create quadtree
    quadtree_node *tree;
    tree = create_tree_node(NULL, 0, max_x, 0, max_y);

    // parallelized for loop
    #pragma omp parallel 
    for(int i = 0; i < number_of_particles; i++)
      add_particle(tree, &particle_array_a[i]);
    
    // set node weights
    set_node_mass(tree);
    
    // for each particle
    #pragma omp parallel for
    for(int i = 0; i < number_of_particles; i++){
      // compute force
      vector force = compute_force(&particle_array_a[i], tree);
      
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

    // swap working particle arrays
    particle_array_temp = particle_array_b;
    particle_array_b = particle_array_a;
    particle_array_a = particle_array_temp; 
    // free tree of this iteration
    destroy_tree(tree);
  }

  // free particle arrays
  free(particle_array_a);
  free(particle_array_b);

  // finish time measuring
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
  
  // compute theta to decide whether it is necessary to go one step down
  double theta = compute_theta(tree, current_particle);
  if(theta < 1){
    particle* node_particle = tree->particle;

    double calculated_force;
    vector direction_vector;
    force.x=0.0;
    force.y=0.0;

    // direction vector a -> b = b - a
    direction_vector.x = current_particle->position_x - node_particle->position_x;
    direction_vector.y = current_particle->position_y - node_particle->position_y;

    
    double distance = compute_vector_length(direction_vector);
    double d_eps = distance + EPS;
    calculated_force = G*(current_particle->mass*node_particle->mass)/sqrt(d_eps*d_eps*d_eps);
    
    force.x -= calculated_force/distance * direction_vector.x;
    force.y -= calculated_force/distance * direction_vector.y;
    
  }
  else{
    // sum forces of all child-nodes
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
  return force;
}


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

// create tree node - if particle = NULL it does not hold any particle :: other values are coordinate range of node
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
  if((is_leaf(root) == 0) && (root->particle != NULL)){
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
  if(tree == NULL){
    return;
  }
  
  double x_half = (tree->max_x-tree->min_x)/2;
  double y_half = (tree->max_y-tree->min_y)/2;
  int node_type = is_leaf(tree); 
  
  // case of node split (node_type = -1)
  node_type = (tree->particle == new_particle) ? -1 : node_type;
  
  // empty leaf can be filled
  if(node_type == 2){
    tree->particle = new_particle;
  }
  else if(node_type == 1){
    // if filled leaf -> split it
    add_particle(tree, tree->particle);    
    add_particle(tree, new_particle);
    tree->particle = NULL;
  }  
  else{
    // as long as no leaf - call recursively
    if(new_particle->position_x <= tree->min_x+x_half && new_particle->position_y <= tree->min_y+y_half){
      // if node split = -1 then create node
      if(node_type == -1)
        tree->nw = create_tree_node(NULL, tree->min_x, tree->min_x+x_half, tree->min_y, tree->min_y+y_half);
        
      add_particle(tree->nw, new_particle);
    }
    else if(new_particle->position_x > tree->min_x+x_half && new_particle->position_y <= tree->min_y+y_half){
      if(node_type == -1)
        tree->ne = create_tree_node(NULL, tree->min_x+x_half, tree->max_x, tree->min_y, tree->min_y+y_half);
        
      add_particle(tree->ne, new_particle);
    }
    else if(new_particle->position_x <= tree->min_x+x_half && new_particle->position_y > tree->min_y+y_half){
      if(node_type == -1)
        tree->sw = create_tree_node(NULL, tree->min_x, tree->min_x+x_half, tree->min_y+y_half, tree->max_y);
        
      add_particle(tree->sw, new_particle);
    }
    else{
      if(node_type == -1)
        tree->se = create_tree_node(NULL, tree->min_x+x_half, tree->max_x, tree->min_y+y_half, tree->max_y);
        
      add_particle(tree->se, new_particle);
    }
  }
}

// function to add an particle to the quadtree
particle* set_node_mass(quadtree_node* tree){
  particle* new_mass = NULL;

  // check whether it is leaf
  int node_type = is_leaf(tree);
  if((tree == NULL) || node_type == 2){
    // empty leafs have no mass
    return NULL;
  }
  else if(node_type == 1){
    // if leaf mass equals particle
    new_mass = tree->particle;
  }
  else{
    // otherwise take weighted masses
    new_mass = (particle*) malloc(sizeof(particle));
    new_mass->mass = 0.0;
    new_mass->position_x = 0.0;
    new_mass->position_y = 0.0;
      
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
      
      if(tmp_node_mass == NULL)
        continue;

      new_mass->mass += tmp_node_mass->mass;
      new_mass->position_x += tmp_node_mass->position_x * tmp_node_mass->mass;
      new_mass->position_y += tmp_node_mass->position_y * tmp_node_mass->mass;

    }  
  
    new_mass->position_x = new_mass->position_x / new_mass->mass;
    new_mass->position_y = new_mass->position_y / new_mass->mass;

    tree->particle = new_mass;
  }  
  
  return new_mass;  
}


// check whether a node is a empty leaf = no particle (2) or a filled leaf (1) or no leaf at all (0)
int is_leaf(quadtree_node* node){
  // if no node then it is no leaf
  if(node == NULL){
    return 0;
  }
  else if(node->nw != NULL 
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
