#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "TSP-TEST.V0.9/instance.h"
#include "TSP-TEST.V0.9/utilities.h"
#include "TSP-TEST.V0.9/timer.h"
#include "TSP-TEST.V0.9/ls.h"
 
int maximumRouteTime;
int vehicleCapacity;
int dropTime;
int * demand; 
 
void loadInstance(char *cvrp_file_name) {
 
  FILE *tsp_file;
  FILE *cvrp_file;
  char buf[LINE_BUF_LEN];
  tsp_file  = fopen("tsp_file_name",  "w");
  cvrp_file = fopen(cvrp_file_name, "r");
  if ( cvrp_file == NULL ) {
    fprintf(stderr,"No instance file specified, abort\n");
  }
  /*assert(tsp_file != NULL);*/
  printf("\nconverting %s to tsp_file_name ... \n\n", cvrp_file_name);
  
  fprintf(tsp_file, "NAME : %s\n",cvrp_file_name);
  fprintf(tsp_file, "TYPE : TSP\n");
  fprintf(tsp_file, "COMMENT : %s (S.Eilon, C.D.T.Watson-Gandy and N.Christofides)\n",cvrp_file_name);
  fscanf(cvrp_file,"%s", buf);
  int numCities = atoi(buf);
  demand = malloc( (numCities+1) * sizeof(int));
  memset(demand, 0, (numCities + 1));
  fprintf(tsp_file, "DIMENSION : %d\n",numCities+1);
  fprintf(tsp_file, "EDGE_WEIGHT_TYPE : EUC_2D\n");
  fprintf(tsp_file, "NODE_COORD_SECTION\n");
  fscanf(cvrp_file,"%s", buf);
  maximumRouteTime = atoi(buf);
  fscanf(cvrp_file,"%s", buf);
  vehicleCapacity = atoi(buf);
  fscanf(cvrp_file,"%s", buf);
  dropTime = atoi(buf);
 
  /*
  printf("numCities: %d\n",numCities);
  printf("maximumRouteTime: %d\n",maximumRouteTime);
  printf("vehicleCapacity: %d\n",vehicleCapacity);
  printf("dropTime: %d\n",dropTime);
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
    demand[counter] = atoi(buf);
        
    counter++;
    
  }
  
  fprintf(tsp_file, "EOF\n");
  fclose(tsp_file);
  fclose(cvrp_file);
  
  read_instance("tsp_file_name");
  distMat = compute_distances();
  nn_ls = MIN (ncities - 1, 40);
  nnMat = compute_NNLists();
} 

/*
void runAOC(){
  int maxPossibleSize = (2 * numCities + 1 ) * sizeof(int);
  int * B = (int*) malloc( maxPossibleSize ), Bsize = 0, Bduration = MAX_INT;
  // Psize contiene el numero de soluciones construidas para cada iteracion
  int Psize = 0;
  //   P contiene todas las soluciones que van a ser construidas
  int ** P = (int**) malloc( numAnts * sizeof(int));
  for(i = 0 ; i < numAnts ; ++i)
    P[i] = (int*) malloc( maxPossibleSize );

//  Route number: mantiene el numero de rutas para cada solucion P[i]
  int * Rnumber = (int*) malloc( numAnts * sizeof(int));
//  Durations: duracion de cada solucion
  int * durations = (int*) malloc( numAnts * sizeof(int));
// C contiene el conjunto de componentes disponibles para cada iteracion mas interna
  int * C, Csize;
  C = (int*) malloc(numCities * sizeof(int));
  // visited contiene 1 si la ciudad visited[i] fue visitada o 0 en caso contrario
  int * visited = (int*) malloc(( 1 + numCities) * sizeof(int));
  do {
    // Inicializo todo para construir todo de nuevo
    memset(Rnumber, 0, numAnts * sizeof(int));
    memset(durations, 0, numAnts * sizeof(int));
    Psize = 0;
    while (Psize < numAnts){
      int citiesInS = 0 , indexOfLastRoute = 0, currentPosition = 0;
      int Sduration = 0, routeDuration = 0 , routeCapacity = 0;
      memset(visited, 0 , ( 1 + numCities) * sizeof(int));
      while (citiesInS < numCities){
        // Deberia considerar no solo el arco de llegada al nuevo componente sino tambien el regreso al deposito
        // Y ademas no deberia regresar 0 si la posicion actual es 0 (el deposito)
        getFeasibleComponents(C, &Csize); 
        if (Csize == 0){
          // ESTO NO DEBERIA PASAR NUNCA O SINO LA INSTANCIA NO TENDRIA SOLUCION FACTIBLE
          citiesInS = 0;
          currentPosition = 0;
          indexOfLastRoute = 0;
        } else {
          int component = getComponent(C, Csize);
          P[Psize][currentPosition] = component;
          
          if( component == 0 ){
            // Si el componente es el deposito, acabo de terminar de construir
            // una ruta y puedo aplicarle el 3-opt
            // Falta verificar que el tamano de la ruta sea el minimo requerido para 3-opt
            threeOpt(P[Psize], indexOfLastRoute , currentPosition);
            indexOfLastRoute = currentPosition;
            ++Rnumber[Psize];
            Sduration += distance[P[Psize][currentPosition - 1]][component];
            routeDuration = 0;
            routeCapacity = 0;
          } else {
            // Si el componente no es el deposito, entonces ya cubri una ciudad
            // mas. Faltan numCities - citiesInS.
            ++citiesInS;
            if ( citiesInS == numCities ) {
              // Falta terminar la ruta, xq la siguiente iteracion no va a entrar
            }
            // Actualizar duracion y capacidad de la ruta y de la solucion
            Sduration += distance[P[Psize][currentPosition - 1]][component] + dropTime;
            routeDuration += distance[P[Psize][currentPosition - 1]][component] + dropTime;
            routeCapacity += demand[component];
          }
          ++currentPosition;
        }
      }
      durations[Psize] = Sduration;
      if ( duration[Psize] < Bduration ){
        B = copy(P[Psize]);
        Bduration = duration[Psize];
      }
      ++Psize;
    }
    // Evaporo feromonas
    // Actualizo feromonas
  } while( !terminar() );
  
}*/

int main(int argc, char **argv) {
  loadInstance(argv[1]);
  /*runAOC();
    printResults();*/
  return 0;
}