#include "aoc.h"

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
      (Rduration + distMat[currentCustomer][i] + cvrp_distMat[i][0] + dropTime <=  maximumRouteDuration &&
      Rdemand + demand[i] <= vehicleCapacity)){
      // La ciudad a la que voy no se ha visitado, puedo agregarla a la ruta sin
      // exceder la capacidad actual y ademas puedo ir y volver al deposito sin exceder la duracion
      C[*Csize] = i;
    *Csize += 1;
    }
  }
  if (currentCustomer != 0){
    // Si no estoy en el deposito, el deposito tambien puede ser factible
    if (Rduration + cvrp_distMat[currentCustomer][0] <=  maximumRouteDuration) {
      C[*Csize] = 0;
      *Csize += 1;
    } else {
      printf("The last route built is unfeasible.");
      exit(1);
    }
  }
}