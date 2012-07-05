#include <stdio.h>
#include <stdlib.h>

#include "aoc.h"
#include "cvrp_instance.h"

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
void getFeasibleComponents(int * visited, int currentCustomer, int Rduration, int Rdemand,
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

/*void runAOC(){
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
            int * tour = &(P[Psize][indexOfLastRoute]); // PUEDE HABER UN GRAN BUG EN ESTA LINEA
            ncities = currentPosition - indexOfLastRoute; // Actualizo ncities para aplicar el three_opt_first
            three_opt_first(tour);
            Sduration += calculateTourDuration(tour) + ncities * dropTime;
            indexOfLastRoute = currentPosition;
            routeDuration = 0;
            routeCapacity = 0;
            ++Rnumber[Psize];
            } else {
              // Si el componente no es el deposito, entonces ya cubri una ciudad
              // mas. Faltan numCities - citiesInS.
              ++citiesInS;
              if ( citiesInS == numCities ) {
                // Falta terminar la ruta, xq la siguiente iteracion no va a entrar
              }
              // Actualizar duracion y capacidad de la ruta y la duracion de la solucion
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
