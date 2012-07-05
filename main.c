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

int main(int argc, char **argv) {
  cvrp_load_instance(argv[1]);
//   runAOC();
//   printResults();
  return 0;
}