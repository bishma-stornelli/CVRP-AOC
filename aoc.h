#ifndef CVRP_AOC_AOC_H_
#define CVRP_AOC_AOC_H_

/*********** PARAMETROS RECIBIDOS POR CONSOLA ***************/
extern double aoc_evaporation_rate;


// Tamano de la poblacion que va a construir en cada iteracion
extern int aoc_total_ants;


extern int finish_param;

// Apuntador a funcion usada para terminar
extern int (*finish_function)();

extern double aoc_pheromone_amplification;

extern double aoc_heuristic_amplification;

/************** OTROS PARAMETROS INTERNOS ****************/

// Terminar luego de # iteraciones sin mejorar. Requiere el numero de iteraciones sin mejorar
extern int finish_not_improvement();
// Terminar luego de un # fijo de iteraciones. Requiere el numero maximo de iteraciones
extern int finish_fixed_iterations();
// Terminar luego de cierto tiempo. Requiere el tiempo maximo
extern int finish_time();

// Segundo en el que se encontro la mejor solucion
extern double aoc_time_best_found;

// Segundos totales que duro corriendo el algoritmo
extern double aoc_total_time;


// Iteracion en la que se encontro la mejor solucion
extern int aoc_iteration_best_found;

// Numero de iteraciones totales
extern int aoc_total_iterations;


// Arreglo que contiene la mejor solucion encontrada
// Por ejemplo 0 2 4 0 1 0 3 5 0 0 0 es la representacion de una solucion
// con 3 rutas. Es importante que luego de la ultima ruta hayan al menos dos 0 seguidos
// o fin del arreglo.
extern int * aoc_best;

// Numero de rutas en la mejor solucion
extern int aoc_best_size;

// Duracion de la mejor solucion
extern int aoc_best_duration;


// Corre la metaheuristica usando como informacion de entrada las variables
// declaradas en el archivo cvrp_instance.h
extern void run_aoc_metaheuristic();

extern void print_results();

extern void initialize_aoc(char ** args, int argc);

extern void print_aoc();

#endif