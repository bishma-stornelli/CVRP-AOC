#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "TSP-TEST.V0.9/utilities.h"
#include "TSP-TEST.V0.9/instance.h"
#include "TSP-TEST.V0.9/ls.h"

#include "aoc.h"
#include "cvrp_instance.h"

double aoc_evaporation_rate = -1;


// Tamano de la poblacion que va a construir en cada iteracion
int aoc_total_ants = -1;

// Iteracion en la que se encontro la mejor solucion
int aoc_iteration_best_found;

int finish_param = -1;

double aoc_pheromone_amplification = -1;

double aoc_heuristic_amplification = -1;

// Numero de iteraciones totales
int aoc_total_iterations = 0;

// Apuntador a funcion usada para terminar
int (*finish_function)() = NULL;

int aoc_iteration_best_found;

// Numero de iteraciones totales
int aoc_total_iterations;

// Segundo en el que se encontro la mejor solucion
double aoc_time_best_found;

// Segundos totales que duro corriendo el algoritmo
double aoc_total_time;

// Arreglo que contiene la mejor solucion encontrada
// Por ejemplo 0 2 4 0 1 0 3 5 0 0 0 es la representacion de una solucion
// con 3 rutas. Es importante que luego de la ultima ruta hayan al menos dos 0 seguidos
// o fin del arreglo.
int * aoc_best;

// Numero de rutas en la mejor solucion
int aoc_best_size;

// Duracion de la mejor solucion
int aoc_best_duration;

struct timeval t_ini, t_fin, t_best;
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

int finish_not_improvement();
// Terminar luego de un # fijo de iteraciones. Requiere el numero maximo de iteraciones
int finish_fixed_iterations();
// Terminar luego de cierto tiempo. Requiere el tiempo maximo
int finish_time();

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
  // Para cada cliente
  for( i = 1 ; i <= cvrp_num_cities ; ++i){
    if(!visited[i] && // Si no se ha visitado
      ( Rduration + // y la duracion actual
        cvrp_distMat[currentCustomer][i] + // mas la duracion para llegar al cliente
        cvrp_distMat[i][0] + cvrp_drop_time // y regresarme al deposito mas el tiempo de atender un cliente mas
        <=  cvrp_max_route_duration  // Es menor que la duracion maxima
        && (Rdemand + cvrp_demand[i] <= cvrp_truck_capacity)) // Verifico que no exceda la capacidad

      ){
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
    }
    else {
      printf("The last route built is unfeasible.");
      exit(1);
    }
  }
}

int getComponent(int * components, int components_size, int current_customer, const double ** pheromones){
  int l;
  double b = 0.0;
  for ( l = 0 ; l < components_size ; ++l ){
    b += pow( pheromones[current_customer][components[l]] , aoc_pheromone_amplification ) *
        pow( 1.0 / (double)cvrp_distMat[current_customer][components[l]] , aoc_heuristic_amplification );
  }
  int i = 5;
  while(--i >= 0){
    for ( l = 0 ; l < components_size ; ++l ){
      double p = (pow( pheromones[current_customer][components[l]] , aoc_pheromone_amplification ) *
                  pow( 1.0 / (double)cvrp_distMat[current_customer][components[l]] , aoc_heuristic_amplification )) / b;
      double r = random_number(&seed);
//       printf("%f < %f ... %f\n", r , p * INT_MAX , p);
      if (  r < p * INT_MAX  ){
        return components[l];
      }
    }
  }
  return components[0];
}

// Evapora cada feromona en (1 - aoc_evaporation_rate)
void evaporatePheromones(double **pheromones){
  int i,j;
  for(i = 0 ; i < 1 + cvrp_num_cities ; ++i)
    for(j = 0 ; j < 1 + cvrp_num_cities ; ++j)
      pheromones[i][j] *= (1 - aoc_evaporation_rate);
}
double update_function(double d, double aditional_weight){
  return aditional_weight / d;
}

void updatePheromones(const int ** P, const int * durations, double ** pheromones){
  int i,j;
//   if (worst_solution_duration == -1 ){
//     worst_solution_duration = 0;
//     for( i = 0 ; i < cvrp_num_cities ; ++i)
//         worst_solution_duration += 2 * cvrp_distMat[0][i];
//   }
  for( i = 0 ; i < aoc_total_ants ; ++i){
    for( j = 0 ; j < 2 * cvrp_num_cities ; ++j){
      if (P[i][j] == 0 && P[i][j + 1] == 0){
        // La solucion termina cuando hay dos 0s seguidos indicando una ruta vacia
        break;
      }
      pheromones[P[i][j]][P[i][j + 1]] += update_function(durations[i],1);//worst_solution_duration - durations[i];
      pheromones[P[i][j + 1]][P[i][j]] += update_function(durations[i],1);//worst_solution_duration - durations[i];
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

void elitistStrategy(double **p){
  int i;
  for ( i = 0 ; i < 2 * cvrp_num_cities ; ++i){
    if ( aoc_best[i] == 0 && aoc_best[i+1] == 0 ) break;
    p[aoc_best[i]][aoc_best[i+1]] = update_function(aoc_best_duration, 3);
    p[aoc_best[i+1]][aoc_best[i]] = update_function(aoc_best_duration, 3);
  }
}

/** map the tour in tour_route to variables used for the TSP library
 * to compute the two_opt_best local search
 */
void map(int length_route, int * tour_route) {

  ncities = length_route - 1;
  int **matrix;
  if((matrix = (int **)malloc(ncities * sizeof(int*))) == NULL ) {
    printf("Out of memory, exit.");
    exit(1);
  }
  int i, j;
  for(i = 0; i < ncities; ++i) {
    matrix[i] = (int *) malloc (ncities * sizeof(int));
    if(matrix[i] == NULL) {
      printf("Out of memory, exit.");
      exit(1);
    }
    for(j = 0; j < ncities; ++j)
      matrix[i][j] = cvrp_distMat[tour_route[i]][tour_route[j]];
  }
  distMat = matrix;
  nn_ls = MIN (ncities - 1, 40);
  nnMat = compute_NNLists();

}

int local_search(int length_route, int **P, int Psize, int indexOfLastRoute){
  int * real_tour_route = (int*) malloc(length_route * sizeof(int));
  int * tour_route = (int*) malloc(length_route * sizeof(int)); // 
  int k;
//   printf("Before two_opt_best: ");
  for(k = 0; k < length_route; ++k) {
//     printf("%d ", P[Psize][indexOfLastRoute + k]);
    real_tour_route[k] = P[Psize][indexOfLastRoute + k];
    tour_route[k] = k;
  }
//   printf("\n");
  tour_route[length_route - 1] = 0;
  map(length_route, real_tour_route);
  int m,n;

  // dlb is used for three_opt and two_opt_first. Not needed when using two_opt_best
  dlb = calloc(ncities, sizeof(int));
//   if(length_route > 12) {
    two_opt_best(tour_route);
//   } else {
//     two_opt_best(tour_route); // three_opt_first(tour_route);
//   }
  
  //               printf("\n");
  //               printf("Tour after opt: ");
  //               for(k = 0; k < length_route; ++k)
  //              printf("%d ", tour_route[k]);
  
  
  if(tour_route[0] == 0) {
    for(k = 1; k < length_route - 1; ++k) {
      P[Psize][indexOfLastRoute + k] = real_tour_route[tour_route[k]];
    }
  } else {
    int depot_index = 0;
    for(k = 1; k < length_route - 1; ++k) {
      if(tour_route[k] == 0)
        depot_index = k;
    }
    int l = 1;
    for(k = depot_index + 1; k < length_route - 1; ++k) {
      P[Psize][indexOfLastRoute + l] = real_tour_route[tour_route[k]];
      ++l;
    }
    for(k = 0; k < depot_index; ++k) {
      P[Psize][indexOfLastRoute + l] = real_tour_route[tour_route[k]];
      ++l;
    }
  }
  int Rduration = 0;

//   printf("After two_opt_best: ");
  for(k = 0; k < length_route - 1; ++k) {
//     printf("%d ", P[Psize][indexOfLastRoute + k]);
    Rduration += cvrp_distMat[P[Psize][indexOfLastRoute + k]][P[Psize][indexOfLastRoute + k + 1]];
  }
//   printf("\n");
  Rduration += cvrp_drop_time * (length_route - 2);
  
  // printf("Route Duration After: %d\n", Rduration);
  free(real_tour_route);
  free(tour_route);
  
  free(distMat);
  free(nnMat);
  return Rduration;
}

void verifyVariables(int * P, int indexLastRoute, int currentPosition, int Rduration, int  Rdemand, int * visited){
  int i;
  int rdu = 0, rde = 0;
  for ( i = indexLastRoute ; i < currentPosition - 1 ; ++i){
    rdu += cvrp_distMat[P[i]][P[i+1]];
    if(P[i] != 0 ){
      rdu += cvrp_drop_time;
      rde += cvrp_demand[P[i]];
    }
  }
  if(P[i] != 0 ){
    rdu += cvrp_drop_time;
    rde += cvrp_demand[P[i]];
  }
  if( rdu != Rduration ){
    printf("Duracion de la ruta esta mala\n");
  }
  if ( rde != Rdemand ){
    printf("Demanda de la ruta esta mala\n");
  }
}

void run_aoc_metaheuristic(){


  // Inicializo variables locales

  // Taking the initial time
  gettimeofday(&t_ini, 0);
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
  for(i = 0 ; i <= cvrp_num_cities ; ++i){
    pheromones[i] = (double*) malloc( (1 + cvrp_num_cities)* sizeof(double));
    for ( j = 0 ; j <= cvrp_num_cities ; ++j )
      pheromones[i][j] = 1.0;
  }

//   printf("La matriz de feromonas es: \n");
//   imprimir_matriz_reales(pheromones,1+cvrp_num_cities, 1 + cvrp_num_cities );

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
//         printf("La solucion %d construida hasta ahora es: ", Psize); imprimir_arreglo(P[Psize], currentPosition);
        // Deberia considerar no solo el arco de llegada al nuevo componente sino tambien el regreso al deposito
        // Y ademas no deberia regresar 0 si la posicion actual es 0 (el deposito)
        verifyVariables(P[Psize], indexOfLastRoute, currentPosition, Rduration, Rdemand, visited);
        getFeasibleComponents(visited, P[Psize][currentPosition - 1], Rduration, Rdemand, C, &Csize);
#ifdef DEBUG
        printf("Los componentes factibles son: "); imprimir_arreglo( C, Csize);
#endif
        if (Csize == 0){
          // Si no hay componentes factibles es porque ya se acabo de construir la solucion
          if ( currentPosition < max_solution_size ){
            P[Psize][currentPosition] = 0; // Agrego 0 para que queden dos 0s consecutivos indicando fin de solucion
          }
          break;
        } else {
          int component = getComponent(C, Csize, P[Psize][currentPosition - 1], pheromones);
#ifdef DEBUG
          printf("El componente elegido es: %d\n", component);
#endif

          P[Psize][currentPosition] = component;
          visited[component] = 1;

          if( component == 0 ) {
            // Si el componente es el deposito, acabo de terminar de construir
            // una ruta y puedo aplicarle el 3-opt
            // Falta verificar que el tamano de la ruta sea el minimo requerido para 3-opt
//             int * tour = &(P[Psize][indexOfLastRoute]); // PUEDE HABER UN GRAN BUG EN ESTA LINEA
//             ncities = currentPosition - indexOfLastRoute; // Actualizo ncities para aplicar el three_opt_first

            int length_route = currentPosition - indexOfLastRoute + 1;
            
//             printf("\nindexOfLastRoute: %d",indexOfLastRoute);
//             printf("\ncurrentPosition:  %d",currentPosition);
//             printf("\nlength_route: %d",length_route);
            
            if(length_route > 5) {
                Rduration = local_search(length_route, P, Psize, indexOfLastRoute);
            } else {
      //         Sduration += calculateTourDuration(tour) + ncities * dropTime;
              Rduration += cvrp_distMat[P[Psize][currentPosition - 1]][component];
              //++Rnumber[Psize];
            }
      
            Sduration += Rduration;
            indexOfLastRoute = currentPosition;
            Rduration = 0;
            Rdemand = 0;
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
#ifdef DEBUG
        printf("Mejor solucion encontrada. Duracion: %d, iteracion actual: %d\n", aoc_best_duration - cvrp_drop_time * cvrp_num_cities, aoc_total_iterations);
#endif
      }
      ++Psize;
    }
    evaporatePheromones(pheromones);
    updatePheromones(P,durations, pheromones);
    elitistStrategy(pheromones);
//     printf("############## FEROMONAS ####################\n");
//     imprimir_matriz_reales(pheromones,1+cvrp_num_cities, 1 + cvrp_num_cities );
  // Actualizar dinamicamente el factor de seleccion de componentes
//     if (aoc_iteration_best_found != aoc_total_iterations){
//       aoc_component_selection_rate *= 1.1;
//       if ( aoc_component_selection_rate > 1.0 ) aoc_component_selection_rate = 1;
//     } else {
//       aoc_component_selection_rate *= 0.95;
//       if (aoc_component_selection_rate <= 0.0 ) aoc_component_selection_rate = 0.1;
//     }
    ++aoc_total_iterations;
  } while( !finish_function() );
  gettimeofday(&t_fin, 0);
  aoc_time_best_found = timeval_diff(&t_best, &t_ini);
  aoc_total_time = timeval_diff(&t_fin, &t_ini);
}

void print_results(){
  printf("%d %d %d %f %f\n", aoc_best_duration - cvrp_num_cities * cvrp_drop_time,
         aoc_iteration_best_found,
         aoc_total_iterations,
         aoc_time_best_found,
         aoc_total_time);
  /*printf("Cost: %d\n", aoc_best_duration - cvrp_num_cities * cvrp_drop_time);
  printf("Cost with drop time: %d\n", aoc_best_duration);
  printf("Iteration until best found: %d\n", aoc_iteration_best_found);
  printf("Total iterations: %d\n", aoc_total_iterations);
  printf("Time until best found: %f\n", aoc_time_best_found);
  printf("Total time elapsed: %f\n", aoc_total_time);
  printf("Routes:\n");
  int i,j;
  double real_cost = 0.0;
  for (i = 0 ; i < 2 * cvrp_num_cities ; ++i ){
    printf("%d " , aoc_best[i]);
    if (aoc_best[i] == 0 && aoc_best[i + 1] == 0){
      printf("\n");
      break;
    }
    if( i > 0 && aoc_best[i] == 0 )
      printf("\n%d ", 0);
    real_cost += cvrp_real_distMat[aoc_best[i]][aoc_best[i+1]];
  }
//   for ( i = 0 ; i <= 2 * cvrp_num_cities ; ++i )
//     printf("%d ", aoc_best[i]);
//   printf("\n");
  printf("Costo en reales: %f\n", real_cost);*/
  verified_solution();
}

void verified_solution() {

  double totalDuration = 0;
  double routeDuration = 0;
  int routeDemand = 0;
  int routeNum = 1;
  int edgeCost = 0;
  int numClient = 1;
  int i;
  int * vis = calloc(cvrp_num_cities + 1, sizeof(int));

  for(i = 0; i < 2*cvrp_num_cities; ++i) {
    if (aoc_best[i] != 0 && vis[aoc_best[i]] == 1){
      printf("Hay un cliente repetido\n");
    }
    vis[aoc_best[i]] = 1;
    if (aoc_best[i] == 0 && aoc_best[i + 1] == 0) {
      break;
    }
    edgeCost = cvrp_real_distMat[aoc_best[i]][aoc_best[i + 1]];
    routeDuration += edgeCost;
    if(aoc_best[i] != 0) {
      ++numClient;
      routeDuration += cvrp_drop_time;
      routeDemand += cvrp_demand[aoc_best[i]];
    }

    if(aoc_best[i+1] == 0) {
//       printf("the duration for route %d is %d\n",routeNum,routeDuration);
//       printf("the demand for route %d is %d\n",routeNum,routeDemand);
      if(routeDuration > cvrp_max_route_duration)
        printf("the route %d exceeds the max route duration\n" , routeNum);
      if(routeDemand > cvrp_truck_capacity)
        printf("the route %d exceeds the max truck capacity. Expect %d <= %d\n" , routeNum, routeDemand, cvrp_truck_capacity);
      totalDuration += routeDuration;
      routeDuration = 0;
      routeDemand = 0;
      ++routeNum;
    }

  }
  for(i = 0; i <= cvrp_num_cities; ++i) {
    if (vis[i] != 1){
      printf("La solucion no incluye a la ciudad %d\n", i);
    }
  }
  free(vis);
//   if(totalDuration != aoc_best_duration) {
//     printf("the tour duration doesnt correspond to the computed duration\n");
//   }

}

void print_usage(){
  printf("Opciones:\n\n");

  printf("\t-a <total_ants>\n");
  printf("\t\tEspecifica el numero total de soluciones que se construyen por iteracion.\n");
  printf("\t\tEl numero por defecto es igual al numero de ciudades.\n\n");

  printf("\t-e <evaporation_rate>\n");
  printf("\t\tEspecifica el factor de evaporacion de las feromonas.\n");
  printf("\t\tDebe estar entre [0.0, 1.0]. Por defecto: 0.2\n\n");

  printf("\t-h <heuristic_amplification>\n");
  printf("\t\tEspecifica el factor de amplificacion de la informacion especifica del CVRP usada para seleccionar los componentes.\n");
  printf("\t\tDebe ser >= 0. Por defecto: 1.0\n\n");

  printf("\t-p <pheromone_amplification>\n");
  printf("\t\tEspecifica el factor de amplificacion de las feromonas usadas para seleccionar los componentes.\n");
  printf("\t\tDebe ser >= 0. Por defecto: 1.0\n\n");

  printf("\t-t <finish_function> <finish_param>\n");
  printf("\t\tEspecifica la funcion para terminar la metaheuristicas. Los posibles valores son:\n");
  printf("\t\t\timprovement en cuyo caso finish_param es el numero de iteraciones que pasan sin mejorar la solucion\n");
  printf("\t\t\tfixed en cuyo caso finish_param especifica el numero de iteraciones maximas\n");
  printf("\t\t\ttime en cuyo caso finish_param es el numero de segundos maximo que debe correr la solucion\n");
  printf("\t\tCuando no se especifica, la funcion usada por defecto es por tiempo con 60 segundos.\n");
  exit(1);

}

void initialize_aoc(char ** args, int argc){
  int i;
  for ( i = 0 ; i < argc ; ++i){
    if ( strcmp("-a", args[i]) == 0 ){
      // Total de hormigas
      if ( i + 1 == argc ) print_usage();
      aoc_total_ants = atoi(args[++i]);
    } else if ( strcmp("-e", args[i]) == 0 ){
      // Rango de evaporacion
      if ( i + 1 == argc ) print_usage();
      aoc_evaporation_rate = atof(args[++i]);
    }  else if ( strcmp("-p", args[i]) == 0 ){
      // Rango de evaporacion
      if ( i + 1 == argc ) print_usage();
      aoc_pheromone_amplification = atof(args[++i]);
    } else if ( strcmp("-h", args[i]) == 0 ){
      // Rango de evaporacion
      if ( i + 1 == argc ) print_usage();
      aoc_heuristic_amplification = atof(args[++i]);
    }else if ( strcmp("-t", args[i]) == 0 ){
      // -t <finish_function> <finish_param>
      if ( i + 2 >= argc ) print_usage();
      ++i;
      if ( strcmp("improvement", args[i]) == 0 ){
        finish_function = finish_not_improvement;
      } else if ( strcmp("fixed", args[i]) == 0 ){
        finish_function = finish_fixed_iterations;
      } else if ( strcmp("time", args[i]) == 0 ){
        finish_function = finish_time;
      } else {
        print_usage();
      }
      finish_param = atoi(args[++i]);
    }
  }

  if ( aoc_evaporation_rate == 0 ||
    (aoc_evaporation_rate != -1 && aoc_evaporation_rate < 0.0) ||
    aoc_evaporation_rate > 1.0 ||
       aoc_total_ants == 0 ||
       finish_param == 0 ||
       (aoc_heuristic_amplification != -1 && aoc_heuristic_amplification < 0.0) ||
       (aoc_pheromone_amplification != -1 && aoc_pheromone_amplification < 0.0)) {
    print_usage();
  }
  if ( aoc_evaporation_rate == -1 ) aoc_evaporation_rate = 0.3;
  if ( aoc_total_ants == -1 ) aoc_total_ants = 20;
  if ( finish_param == -1 ) {
    finish_function = finish_time;
    finish_param = 60;
  }
  if ( aoc_heuristic_amplification == -1 ) aoc_heuristic_amplification = 5.0;
  if ( aoc_pheromone_amplification == -1 ) aoc_pheromone_amplification = 0.5;
  aoc_iteration_best_found = 0;
  aoc_total_iterations = 0;
  aoc_time_best_found = 0.0;;
  aoc_total_time = 0.0;
  aoc_best = (int*) malloc( ( cvrp_num_cities * 2 + 1 ) * sizeof(int));
  aoc_best_duration = INT_MAX;
  aoc_best_size = 0;
#ifdef DEBUG
  print_aoc();
#endif
}

int finish_not_improvement(){
  if ( aoc_total_iterations - aoc_iteration_best_found  >= finish_param)
    return 1;
  return 0;
}
// Terminar luego de un # fijo de iteraciones. Requiere el numero maximo de iteraciones
int finish_fixed_iterations(){
  if ( aoc_total_iterations  >= finish_param)
    return 1;
  return 0;
}
// Terminar luego de cierto tiempo. Requiere el tiempo maximo
int finish_time(){
  gettimeofday(&t_fin, 0);
  aoc_total_time = timeval_diff(&t_fin, &t_ini);
  if ( aoc_total_time  >= finish_param)
    return 1;
  return 0;
}

void print_aoc(){
  printf("\n/****************** AOC configuration ***************/\naoc_evaporation_rate: %f\n", aoc_evaporation_rate);
  printf("aoc_total_ants: %d\n", aoc_total_ants);
  printf("finish_param: %d\n\n", finish_param);
}