#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#include "TSP-TEST.V0.9/utilities.h"
#include "TSP-TEST.V0.9/instance.h"

#include "aoc.h"
#include "cvrp_instance.h"

int aoc_iteration_best_found;

// Numero de iteraciones totales
int aoc_total_iterations;

// Segundo en el que se encontro la mejor solucion
double aoc_time_best_found;

// Segundos totales que duro corriendo el algoritmo
double aoc_total_time;

// Tamano de la poblacion que va a construir en cada iteracion
int aoc_total_ants;

// Arreglo que contiene la mejor solucion encontrada
// Por ejemplo 0 2 4 0 1 0 3 5 0 0 0 es la representacion de una solucion
// con 3 rutas. Es importante que luego de la ultima ruta hayan al menos dos 0 seguidos
// o fin del arreglo.
int * aoc_best;

// Numero de rutas en la mejor solucion
int aoc_best_size;

// Duracion de la mejor solucion
int aoc_best_duration;

double aoc_evaporation_rate;

double aoc_component_selection_rate = 0.1;

// Corre la metaheuristica usando como informacion de entrada las variables
// declaradas en el archivo cvrp_instance.h
void run_aoc_metaheuristic();

int worst_solution_duration = -1;

int getComponent(int * components, int components_size, int current_customer, const double ** pheromones);

void getFeasibleComponents(const int*,int,int,int,int*,int*);

// Evapora cada feromona en (1 - aoc_evaporation_rate)
void evaporatePheromones(double **pheromones);

// Aumenta las feromonas de los componentes j usados para cada solucion i
// en g(i) donde g(.) es una funcion tal que durations(x) < durations(y) => g(x) > g(y)
void updatePheromones(const int ** P, const int * durations, double ** pheromones);

void imprimir_arreglo(int *v, int n );

/** Funcion auxiliar que construye una lista de componentes factibles a partir
* de los parametros recibidos.
*
* @param visited arreglo que indica si la i-esima ciudad ya esta en la solucion o no
* @param currentCustomer cliente en el que se encuentra la solucion actual (puede ser el deposito)
* @param Rduration duracion de la ruta que se esta construyendo
* @param Rdemand demanda actual de la ruta que se esta construyendo
* @param C Parametro de salida: arreglo de tamano cvrp_num_cities que contendra las ciudades que son factibles agregar a la solucion
* @param Csize Parametro de salida: el numero de componentes factibles encontrados
*/
void getFeasibleComponents(const int * visited, int currentCustomer, int Rduration, int Rdemand,
                           int * C, int * Csize){
  *Csize = 0;
  // Veo a que ciudades puedo ir
  int i ;
  for( i = 1 ; i <= cvrp_num_cities ; ++i){
    if(!visited[i] &&
      (Rduration + cvrp_distMat[currentCustomer][i] + cvrp_distMat[i][0] + cvrp_drop_time <=  cvrp_max_route_duration &&
      Rdemand + cvrp_demand[i] <= cvrp_truck_capacity)){
      // La ciudad a la que voy no se ha visitado, puedo agregarla a la ruta sin
      // exceder la capacidad actual y ademas puedo ir y volver al deposito sin exceder la duracion
      C[*Csize] = i;
    *Csize += 1;
    }
  }
  if (currentCustomer != 0){
    // Si no estoy en el deposito, el deposito tambien puede ser factible
    if (Rduration + cvrp_distMat[currentCustomer][0] <=  cvrp_max_route_duration) {
      C[*Csize] = 0;
      *Csize += 1;
    } else {
      printf("The last route built is unfeasible.");
      exit(1);
    }
  }
}

// Tomado y adaptado de wikipedia
int colocar(const double **pheromones, int current_customer, int *v, int b, int t)
{
  int i;
  int pivote;
  double valor_pivote;
  int f_pivote;
  int temp;
  
  pivote = b;
  valor_pivote = pheromones[current_customer][v[pivote]];
  f_pivote = cvrp_distMat[current_customer][v[pivote]];
  for (i=b+1; i<=t; i++){
    if (pheromones[current_customer][v[i]] < valor_pivote ||
      (pheromones[current_customer][v[i]] == valor_pivote && cvrp_distMat[current_customer][v[i]] < f_pivote)){
      pivote++;
      temp=v[i];
      v[i]=v[pivote];
      v[pivote]=temp;
    }
  }
  temp=v[b];
  v[b]=v[pivote];
  v[pivote]=temp;
  return pivote;
}

// Tomado y adaptado de wikipedia
void Quicksort(const double **pheromones, int current_customer, int* v, int b, int t)
{
  int pivote;
  if(b < t){
    pivote=colocar(pheromones, current_customer, v, b, t);
    Quicksort(pheromones,current_customer, v, b, pivote-1);
    Quicksort(pheromones,current_customer, v, pivote+1, t);
  }
}

int getComponent(int * components, int components_size, int current_customer, const double ** pheromones){
  printf("Empezando a aplicar quicksort en los componentes: "); imprimir_arreglo(components, components_size);
  // Aplico quicksort para ordenar los componentes
  Quicksort(pheromones, current_customer, components, 0 , components_size - 1);
  printf("Luego del quicksort los componentes quedaron asi: "); imprimir_arreglo(components, components_size);
  // Una vez arreglado elijo uno aleatorio de entre los primeros
  int temp = random_number(&seed) % (int)(1 + aoc_component_selection_rate * components_size);
  printf("seed: %ld\n", seed);
  printf("random_number: %d\n", temp);
  return components[ temp ];
}

// Evapora cada feromona en (1 - aoc_evaporation_rate)
void evaporatePheromones(double **pheromones){
  int i,j;
  for(i = 0 ; i < 1 + cvrp_num_cities ; ++i)
    for(j = 0 ; j < 1 + cvrp_num_cities ; ++j)
      pheromones[i][j] *= (1 - aoc_evaporation_rate);
}

void updatePheromones(const int ** P, const int * durations, double ** pheromones){
  int i,j;
  if (worst_solution_duration == -1 ){
    worst_solution_duration = 0;
    for( i = 0 ; i < cvrp_num_cities ; ++i)
        worst_solution_duration += 2 * cvrp_distMat[0][i]; 
  }
  for( i = 0 ; i < aoc_total_ants ; ++i){
    for( j = 0 ; j < 2 * cvrp_num_cities ; ++j){
      if (P[i][j] == 0 && P[i][j + 1] == 0){
        // La solucion termina cuando hay dos 0s seguidos indicando una ruta vacia
        break;
      }
      pheromones[P[i][j]][P[i][j + 1]] += worst_solution_duration - durations[i];
      pheromones[P[i][j + 1]][P[i][j]] += worst_solution_duration - durations[i];
    }
  }
}

void copy(int * from, int * to){
  int i;
  for ( i = 0 ; i < 2 * cvrp_num_cities + 1 ; ++i )
    to[i] = from[i];
}

double timeval_diff(struct timeval *a, struct timeval *b)
{
  return
  (double)(a->tv_sec + (double)a->tv_usec/1000000) -
  (double)(b->tv_sec + (double)b->tv_usec/1000000);
}

// returns 1 if its time to finish or 0 in other case
int terminar(){
  ++aoc_total_iterations;
  if (aoc_total_iterations == 100)
    return 1;
  return 0;
}

void imprimir_matriz_reales(double **v, int n , int m){
  int i , j ;
  for ( i = 0 ; i < n ; printf("\n"), ++i)
    for( j = 0 ; j < m ; printf("%f ", v[i][j]) , ++j);
}

void imprimir_matriz(int **v, int n , int m){
  int i;
  for ( i = 0 ; i < n ; ++i)
    imprimir_arreglo(v[i], m);
}

void imprimir_arreglo(int *v, int n ){
  int i;
  for ( i = 0 ; i < n ; printf("%d " , v[i] ) , ++i);
  printf("\n");
}

void run_aoc_metaheuristic(){
  
  
  // Inicializo variables locales
  struct timeval t_ini, t_fin, t_best;
  // Taking the initial time
  gettimeofday(&t_ini, 0);
  
  aoc_total_ants = cvrp_num_cities;
  int max_solution_size = (2 * cvrp_num_cities + 1 ) * sizeof(int);
  // Psize contiene el numero de soluciones construidas para cada iteracion
  int Psize = 0;
  //   P contiene todas las soluciones que van a ser construidas
  int ** P = (int**) malloc( aoc_total_ants * sizeof(int*));
  // Auxiliar para iteraciones cortass
  int i,j;
  for(i = 0 ; i < aoc_total_ants ; ++i)
    P[i] = (int*) malloc( max_solution_size );
  //  Route number: mantiene el numero de rutas para cada solucion P[i]
//   int * Rnumber = (int*) malloc( aoc_total_ants * sizeof(int));
  //  Durations: duracion de cada solucion
  int * durations = (int*) malloc( aoc_total_ants * sizeof(int));
  // C contiene el conjunto de componentes disponibles para cada iteracion mas interna
  int * C, Csize;
  C = (int*) malloc(cvrp_num_cities * sizeof(int));
  // visited contiene 1 si la ciudad visited[i] fue visitada o 0 en caso contrario
  int * visited = (int*) malloc(( 1 + cvrp_num_cities) * sizeof(int));
  // matriz de feromonas
  double ** pheromones = (double**) malloc( (1 + cvrp_num_cities) * sizeof(double*));
  for(i = 0 ; i <= cvrp_num_cities ; ++i) pheromones[i] = (double*) calloc( 1 + cvrp_num_cities, sizeof(double));

  printf("La matriz de feromonas es: \n");
  imprimir_matriz_reales(pheromones,1+cvrp_num_cities, 1 + cvrp_num_cities );

  // Inicializo variables globales
  aoc_best = (int*) malloc( max_solution_size );
  aoc_best_size = 0;
  aoc_best_duration = INT_MAX;
  aoc_iteration_best_found = 0;
  aoc_total_iterations = 0;
  
  do {
    // Inicializo todo para construir todo de nuevo
    //memset(Rnumber, 0, aoc_total_ants * sizeof(int));
    memset(durations, 0, aoc_total_ants * sizeof(int));
    Psize = 0;
    while (Psize < aoc_total_ants){
      int indexOfLastRoute = 0, currentPosition = 1;
      int Sduration = 0, Rduration = 0 , Rdemand = 0;
      P[Psize][0] = 0; // La solucion empieza en el deposito
      memset(visited, 0 , ( 1 + cvrp_num_cities) * sizeof(int));
      
      while (1){
        printf("La solucion %d construida hasta ahora es: ", Psize); imprimir_arreglo(P[Psize], currentPosition);
        // Deberia considerar no solo el arco de llegada al nuevo componente sino tambien el regreso al deposito
        // Y ademas no deberia regresar 0 si la posicion actual es 0 (el deposito)
        getFeasibleComponents(visited, P[Psize][currentPosition - 1], Rduration, Rdemand, C, &Csize);
        printf("Los componentes factibles son: "); imprimir_arreglo( C, Csize);
        if (Csize == 0){
          // Si no hay componentes factibles es porque ya se acabo de construir la solucion
          if ( currentPosition < max_solution_size ){
            P[Psize][currentPosition] = 0; // Agrego 0 para que queden dos 0s consecutivos indicando fin de solucion
          }
          break;
        } else {
          int component = getComponent(C, Csize, P[Psize][currentPosition - 1], pheromones);
          printf("El componente elegido es: %d\n", component); 
          
          P[Psize][currentPosition] = component;
          visited[component] = 1;
          
          if( component == 0 ){
            // Si el componente es el deposito, acabo de terminar de construir
            // una ruta y puedo aplicarle el 3-opt
            // Falta verificar que el tamano de la ruta sea el minimo requerido para 3-opt
            /*int * tour = &(P[Psize][indexOfLastRoute]); // PUEDE HABER UN GRAN BUG EN ESTA LINEA
            ncities = currentPosition - indexOfLastRoute; // Actualizo ncities para aplicar el three_opt_first
            three_opt_first(tour);
            Sduration += calculateTourDuration(tour) + ncities * dropTime;*/
	    Rduration += cvrp_distMat[P[Psize][currentPosition - 1]][component];
            Sduration += Rduration;
            indexOfLastRoute = currentPosition;
            Rduration = 0;
            Rdemand = 0;
	    //++Rnumber[Psize];
          } else {
            // Actualizar duracion y capacidad de la ruta y la duracion de la solucion
            Rduration += cvrp_distMat[P[Psize][currentPosition - 1]][component] + cvrp_drop_time;
            Rdemand += cvrp_demand[component];
          }
          ++currentPosition;
        }
      }
      durations[Psize] = Sduration;
      if ( durations[Psize] < aoc_best_duration ){
        gettimeofday(&t_best, 0);
        aoc_iteration_best_found = aoc_total_iterations;
        copy(P[Psize], aoc_best);
        aoc_best_duration = durations[Psize];
      }
      ++Psize;
    }
    evaporatePheromones(pheromones);
    updatePheromones(P,durations, pheromones);
    ++aoc_total_iterations;
  } while( !terminar() );
  gettimeofday(&t_fin, 0);
  aoc_time_best_found = timeval_diff(&t_best, &t_ini);
  aoc_total_time = timeval_diff(&t_fin, &t_ini);
}

void print_results(){
  printf("Cost: %d\n", aoc_best_duration - cvrp_num_cities * cvrp_drop_time);
  printf("Cost with drop time: %d\n", aoc_best_duration);
  printf("Iteration until best found: %d\n", aoc_iteration_best_found);
  printf("Total iterations: %d\n", aoc_total_iterations);
  printf("Time until best found: %f\n", aoc_time_best_found);
  printf("Total time elapsed: %f\n", aoc_total_time);
  printf("Routes:\n");
  int i,j;
  for (i = 0 ; i < 2 * cvrp_num_cities ; ++i ){
    printf("%d " , aoc_best[i]);
    if (aoc_best[i] == 0 && aoc_best[i + 1] == 0){
      printf("\n");
      break;
    }
    if( i > 0 && aoc_best[i] == 0 )
      printf("\n%d ", 0);
  }
  verified_solution();
}

void verified_solution() {
  
  int totalDuration = 0;
  int routeDuration = 0;
  int routeDemand = 0;
  int routeNum = 1;
  int edgeCost = 0;
  int numClient = 1;
  int i;
  
  for(i = 0; i < 2*cvrp_num_cities; ++i) {
    
    if (aoc_best[i] == 0 && aoc_best[i + 1] == 0) {
      break;
    }
    edgeCost = cvrp_distMat[aoc_best[i]][aoc_best[i + 1]];
    routeDuration += edgeCost;
    totalDuration += edgeCost;
    if(aoc_best[i] != 0) {
      ++numClient;
      routeDuration += cvrp_drop_time;
      routeDemand += cvrp_demand[i];
    }
    
    if(aoc_best[i+1] == 0) {
      printf("the duration for route %d is %d\n",routeNum,routeDuration);
      printf("the demand for route %d is %d\n",routeNum,routeDemand);
      if(routeDuration > cvrp_max_route_duration)
	printf("the route exceeds the max route duration\n");
      if(routeDemand > cvrp_truck_capacity)
	printf("the route exceeds the max truck capacity\n");
      routeDuration = 0;
      routeDemand = 0;
      ++routeNum;
    }

  }
  
  if(totalDuration != aoc_best_duration) {
    printf("the tour duration doesnt correspond to the compute duration\n");
  }
    
}
