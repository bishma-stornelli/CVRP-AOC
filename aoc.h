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


// Corre la metaheuristica usando como informacion de entrada las variables
// declaradas en el archivo cvrp_instance.h
extern void run_aoc_metaheuristic();

#endif