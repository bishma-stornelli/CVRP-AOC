#ifndef CVRP_AOC_CVRP_INSTANCE_H_
#define CVRP_AOC_CVRP_INSTANCE_H_


// Matriz de distancias entre ciudades y el deposito
extern int ** cvrp_distMat;

// Numero de ciudades (no incluye el deposito)
extern int cvrp_num_cities;

// Arreglo de capacidades. En la posicion i esta la demanda de la ciudad i. Tamano: cvrp_num_cities + 1
extern int * cvrp_demand;

// Maxima duracion de la ruta
extern int cvrp_max_route_duration;

// Maxima capacidad de cada camion
extern int cvrp_truck_capacity;

// Tiempo para atender al cliente
extern int cvrp_drop_time;

// Carga la instancia a partir del archivo cvrp_file_name
extern void cvrp_load_instance(char *cvrp_file_name);

extern double ** cvrp_real_distMat;


#endif