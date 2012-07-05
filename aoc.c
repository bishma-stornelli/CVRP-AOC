#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "TSP-TEST.V0.9/utilities.h"

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
  // Aplico quicksort para ordenar los componentes
  Quicksort(pheromones, current_customer, components, 0 , components_size - 1);
  // Una vez arreglado elijo uno aleatorio de entre los primeros
  return components[ random_number(seed) % (int)(aoc_component_selection_rate * components_size) ];
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

int * copy(int * from, int * to){
  int i;
  for ( i = 0 ; i < 2 * cvrp_num_cities + 1 ; ++i )
    to[i] = from[i];
}

void run_aoc_metaheuristic(){
  
  
  // Inicializo variables locales
  int max_solution_size = (2 * cvrp_num_cities + 1 ) * sizeof(int);
  // Psize contiene el numero de soluciones construidas para cada iteracion
  int Psize = 0;
  //   P contiene todas las soluciones que van a ser construidas
  int ** P = (int**) malloc( aoc_total_ants * sizeof(int));
  // Auxiliar para iteraciones cortass
  int i;
  for(i = 0 ; i < aoc_total_ants ; ++i)
    P[i] = (int*) malloc( max_solution_size );
  //  Route number: mantiene el numero de rutas para cada solucion P[i]
  int * Rnumber = (int*) malloc( aoc_total_ants * sizeof(int));
  //  Durations: duracion de cada solucion
  int * durations = (int*) malloc( aoc_total_ants * sizeof(int));
  // C contiene el conjunto de componentes disponibles para cada iteracion mas interna
  int * C, Csize;
  C = (int*) malloc(cvrp_num_cities * sizeof(int));
  // visited contiene 1 si la ciudad visited[i] fue visitada o 0 en caso contrario
  int * visited = (int*) malloc(( 1 + cvrp_num_cities) * sizeof(int));
  // matriz de feromonas
  double ** pheromones = (double**) malloc((1 + cvrp_num_cities) * sizeof(double*));
  for(i = 0 ; i < aoc_total_ants ; calloc( &(pheromones[i]),(1 + cvrp_num_cities) * sizeof(double)), ++i);

  // Inicializo variables globales
  aoc_best = (int*) malloc( max_solution_size );
  aoc_best_size = 0;
  aoc_best_duration = INT_MAX;
  
  do {
    // Inicializo todo para construir todo de nuevo
    memset(Rnumber, 0, aoc_total_ants * sizeof(int));
    memset(durations, 0, aoc_total_ants * sizeof(int));
    Psize = 0;
    while (Psize < aoc_total_ants){
      int indexOfLastRoute = 0, currentPosition = 1;
      int Sduration = 0, Rduration = 0 , Rdemand = 0;
      P[Psize][0] = 0; // La solucion empieza en el deposito
      memset(visited, 0 , ( 1 + cvrp_num_cities) * sizeof(int));
      while (1){
        // Deberia considerar no solo el arco de llegada al nuevo componente sino tambien el regreso al deposito
        // Y ademas no deberia regresar 0 si la posicion actual es 0 (el deposito)
        getFeasibleComponents(visited, P[Psize][currentPosition - 1], Rduration, Rdemand, C, &Csize);
        if (Csize == 0){
          // Si no hay componentes factibles es porque ya se acabo de construir la solucion
          if ( currentPosition < max_solution_size ){
            P[Psize][currentPosition] = 0; // Agrego 0 para que queden dos 0s consecutivos indicando fin de solucion
          }
          break;
        } else {
          int component = getComponent(C, Csize, P[Psize][currentPosition - 1], pheromones);
          P[Psize][currentPosition] = component;
          
          if( component == 0 ){
            // Si el componente es el deposito, acabo de terminar de construir
            // una ruta y puedo aplicarle el 3-opt
            // Falta verificar que el tamano de la ruta sea el minimo requerido para 3-opt
            /*int * tour = &(P[Psize][indexOfLastRoute]); // PUEDE HABER UN GRAN BUG EN ESTA LINEA
            ncities = currentPosition - indexOfLastRoute; // Actualizo ncities para aplicar el three_opt_first
            three_opt_first(tour);
            Sduration += calculateTourDuration(tour) + ncities * dropTime;*/
            Sduration += Rduration;
            indexOfLastRoute = currentPosition;
            Rduration = 0;
            Rdemand = 0;
            ++Rnumber[Psize];
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
        copy(P[Psize], aoc_best);
        aoc_best_duration = durations[Psize];
      }
      ++Psize;
    }
    evaporatePheromones(pheromones);
    updatePheromones(P,durations, pheromones);
  } while( 1 );
  
}
