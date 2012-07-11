#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "TSP-TEST.V0.9/instance.h"
#include "TSP-TEST.V0.9/utilities.h"
#include "TSP-TEST.V0.9/timer.h"
#include "TSP-TEST.V0.9/ls.h"

#include "cvrp_instance.h"
#include "aoc.h"


void print_instance(){
  printf("cvrp_num_cities: %d\n" , cvrp_num_cities);
  printf("cvrp_max_route_duration: %d\n" , cvrp_max_route_duration);
  printf("cvrp_truck_capacity: %d\n" , cvrp_truck_capacity);
  printf("cvrp_drop_time: %d\n" , cvrp_drop_time);
  
  int i,j;
  printf("cvrp_distMat: \n");
  for (i = 0 ; i <= cvrp_num_cities ; printf("\n"), ++i)
    for (j = 0 ; j <= cvrp_num_cities ; printf("%d ", cvrp_distMat[i][j]), ++j);
  printf("cvrp_demand: \n");
  for (i = 0 ; i <= cvrp_num_cities ; printf("%d ", cvrp_demand[i]), ++i);
  printf("\n");
  printf("seed: %d\n", seed);
}

int main(int argc, char **argv) {
  cvrp_load_instance(argv[1]);
#ifdef DEBUG
  print_instance();
#endif
  initialize_aoc(argv, argc);
#ifdef DEBUG
  print_aoc();
#endif
  run_aoc_metaheuristic();
  print_results();
  return 0;
}