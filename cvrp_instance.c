#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "TSP-TEST.V0.9/instance.h"
#include "TSP-TEST.V0.9/ls.h"
#include "TSP-TEST.V0.9/utilities.h"

#include "cvrp_instance.h"

int ** cvrp_distMat;

// Numero de ciudades (no incluye el deposito)
int cvrp_num_cities;

// Arreglo de capacidades. En la posicion i esta la demanda de la ciudad i. Tamano: cvrp_num_cities + 1
int * cvrp_demand;

// Maxima duracion de la ruta
int cvrp_max_route_duration;

// Maxima capacidad de cada camion
int cvrp_truck_capacity;

// Tiempo para atender al cliente
int cvrp_drop_time;

// Carga la instancia a partir del archivo cvrp_file_name
void cvrp_load_instance(char *cvrp_file_name);

void cvrp_load_instance(char *cvrp_file_name) {
  seed = (long int) time(NULL);
  
  FILE *tsp_file;
  FILE *cvrp_file;
  char buf[LINE_BUF_LEN];
  tsp_file  = fopen("tsp_file_name",  "w");
  cvrp_file = fopen(cvrp_file_name, "r");
  if ( cvrp_file == NULL ) {
    fprintf(stderr,"No instance file specified, abort\n");
  }
  // assert(tsp_file != NULL);
  printf("\nconverting %s to tsp_file_name ... \n\n", cvrp_file_name);
  
  fprintf(tsp_file, "NAME : %s\n",cvrp_file_name);
  fprintf(tsp_file, "TYPE : TSP\n");
  fprintf(tsp_file, "COMMENT : %s (S.Eilon, C.D.T.Watson-Gandy and N.Christofides)\n",cvrp_file_name);
  fscanf(cvrp_file,"%s", buf);
  cvrp_num_cities = atoi(buf);
  cvrp_demand = (int*) calloc( (cvrp_num_cities+1) , sizeof(int));
  
  fprintf(tsp_file, "DIMENSION : %d\n",cvrp_num_cities+1);
  fprintf(tsp_file, "EDGE_WEIGHT_TYPE : EUC_2D\n");
  fprintf(tsp_file, "NODE_COORD_SECTION\n");
  fscanf(cvrp_file,"%s", buf);
  cvrp_max_route_duration = atoi(buf);
  fscanf(cvrp_file,"%s", buf);
  cvrp_truck_capacity = atoi(buf);
  fscanf(cvrp_file,"%s", buf);
  cvrp_drop_time = atoi(buf);
  
  /*
  printf("cvrp_num_cities: %d\n",cvrp_num_cities);
  printf("cvrp_max_route_duration: %d\n",cvrp_max_route_duration);
  printf("cvrp_truck_capacity: %d\n",cvrp_truck_capacity);
  printf("cvrp_drop_time: %d\n",cvrp_drop_time);
  */
  
  int counter = 0;
  
  fprintf(tsp_file, "%d ",counter);
  fscanf(cvrp_file,"%s", buf);
  fprintf(tsp_file, "%s ", buf);
  fscanf(cvrp_file,"%s", buf);
  fprintf(tsp_file, "%s\n", buf);
  
  counter++;
  
  while(fscanf(cvrp_file,"%s", buf) != EOF) {
    
    fprintf(tsp_file, "%d ",counter);
    fprintf(tsp_file, "%s ",buf);
    fscanf(cvrp_file,"%s", buf);
    fprintf(tsp_file, "%s\n",buf);
    fscanf(cvrp_file,"%s", buf);
    cvrp_demand[counter] = atoi(buf);
    
    counter++;
  }
  fprintf(tsp_file, "EOF\n");
  fclose(tsp_file);
  fclose(cvrp_file);
  
  read_instance("tsp_file_name");
  distMat = compute_distances();
  nn_ls = MIN (ncities - 1, 40);
  nnMat = compute_NNLists();
  cvrp_distMat = compute_distances();
}