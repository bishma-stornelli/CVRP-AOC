#ifndef CVRP_AOC_AOC_H_
#define CVRP_AOC_AOC_H_

// Iteracion en la que se encontro la mejor solucion
extern int aoc_iteration_best_found;

// Numero de iteraciones totales
extern int aoc_total_iterations;

// Segundo en el que se encontro la mejor solucion
extern double aoc_time_best_found;

// Segundos totales que duro corriendo el algoritmo
extern double aoc_total_time;

// Tamano de la poblacion que va a construir en cada iteracion
extern int aoc_total_ants;

// Porcentaje que determina cuantos de los componentes disponibles van a ser
// seleccionados para elegir aleatoriamente el componente final.
extern double aoc_component_selection_rate;

// Arreglo que contiene la mejor solucion encontrada
// Por ejemplo 0 2 4 0 1 0 3 5 0 0 0 es la representacion de una solucion
// con 3 rutas. Es importante que luego de la ultima ruta hayan al menos dos 0 seguidos
// o fin del arreglo.
extern int * aoc_best;

// Numero de rutas en la mejor solucion
extern int aoc_best_size;

// Duracion de la mejor solucion
extern int aoc_best_duration;

extern double aoc_evaporation_rate;


// Corre la metaheuristica usando como informacion de entrada las variables
// declaradas en el archivo cvrp_instance.h
extern void run_aoc_metaheuristic();

#endif