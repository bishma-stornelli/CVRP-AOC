/*

######################################################
###    simple LS and ILS algorithms for the TSP    ###
######################################################

      Version: 0.9
      File:    instance.c
      Author:  Thomas Stuetzle
      Purpose: read instance and produce distance matrix, nearest neighbour lists
      Check:   README and gpl.txt
      Copyright (C) 2004  Thomas Stuetzle
*/

/***************************************************************************

    Program's name: tsp-test

    iterative improvement (2-opt, 3-opt) for the TSP and a
    simple ILS algorithm for the TSP

    Copyright (C) 2004  Thomas Stuetzle

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    email: stuetzle no@spam informatik.tu-darmstadt.de
    mail address: Universitaet Darmstadt
                  Fachbereich Informatik
                  Hochschulstr. 10
                  D-64283 Darmstadt
		  Germany

***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "instance.h"
#include "utilities.h"
#include "ls.h"


char     name[LINE_BUF_LEN];      	  /* instance name */
char     edge_weight_type[LINE_BUF_LEN];  /* selfexplanatory */

double   *xc, *yc;

int   (*distance)(int, int);  /* pointer to function returning distance */

int   round_distance (int i, int j);

int   ceil_distance (int i, int j);

int   geo_distance (int i, int j);

int   att_distance (int i, int j);

int   compute_tour_length( int *t );

int   **compute_distances(void);

int   **compute_nn_lists( void );

int   ncities;                /* number of cities in the instance */

int     **distMat;            /* distance matrix containing the distances
                                 between all pairs of cities */

int     **nnMat;              /* list of nearest neighbor lists for all cities */

FILE    *report, *comp_report;

int     iteration_best_found;
double  time_best_found;
/*************************************************************************/

#define M_PI 3.14159265358979323846264

static double dtrunc (double x)
{
    int k;

    k = (int) x;
    x = (double) k;
    return x;
}

int  (*distance)(int, int);  /* function pointer */

/*    
      FUNCTION: the following four functions implement different ways of 
                computing distances for TSPLIB instances
      INPUT:    two node indices
      OUTPUT:   distance between the two nodes
*/

int round_distance (int i, int j) 
/*    
      FUNCTION: compute Euclidean distances between two nodes rounded to next 
                integer for TSPLIB instances
      INPUT:    two node indices
      OUTPUT:   distance between the two nodes
      COMMENTS: for the definition of how to compute this distance see TSPLIB
*/
{
    double xd = xc[i] - xc[j];
    double yd = yc[i] - yc[j];
    double r  = sqrt(xd*xd + yd*yd) + 0.5;

    return (int) r;
}

int ceil_distance (int i, int j) 
/*    
      FUNCTION: compute ceiling distance between two nodes rounded to next 
                integer for TSPLIB instances
      INPUT:    two node indices
      OUTPUT:   distance between the two nodes
      COMMENTS: for the definition of how to compute this distance see TSPLIB
*/
{
    double xd = xc[i] - xc[j];
    double yd = yc[i] - yc[j];
    double r  = sqrt(xd*xd + yd*yd) + 0.000000001;

    return (int)r;
}

int geo_distance (int i, int j) 
/*    
      FUNCTION: compute geometric distance between two nodes rounded to next 
                integer for TSPLIB instances
      INPUT:    two node indices
      OUTPUT:   distance between the two nodes
      COMMENTS: adapted from concorde code
                for the definition of how to compute this distance see TSPLIB
*/
{
    double deg, min;
    double lati, latj, longi, longj;
    double q1, q2, q3;
    int dd;
    double x1 = xc[i], x2 = xc[j], 
	y1 = yc[i], y2 = yc[j];

    deg = dtrunc (x1);
    min = x1 - deg;
    lati = M_PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = dtrunc (x2);
    min = x2 - deg;
    latj = M_PI * (deg + 5.0 * min / 3.0) / 180.0;

    deg = dtrunc (y1);
    min = y1 - deg;
    longi = M_PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = dtrunc (y2);
    min = y2 - deg;
    longj = M_PI * (deg + 5.0 * min / 3.0) / 180.0;

    q1 = cos (longi - longj);
    q2 = cos (lati - latj);
    q3 = cos (lati + latj);
    dd = (int) (6378.388 * acos (0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
    return dd;

}

int att_distance (int i, int j) 
/*    
      FUNCTION: compute ATT distance between two nodes rounded to next 
                integer for TSPLIB instances
      INPUT:    two node indices
      OUTPUT:   distance between the two nodes
      COMMENTS: for the definition of how to compute this distance see TSPLIB
*/
{
    double xd = xc[i] - xc[j];
    double yd = yc[i] - yc[j];
    double rij = sqrt ((xd * xd + yd * yd) / 10.0);
    double tij = dtrunc (rij);
    int dij;

    if (tij < rij)
        dij = (int) tij + 1;
    else
        dij = (int) tij;
    return dij;
}

int ** compute_distances(void)
/*    
      FUNCTION: computes the matrix of all intercity distances
      INPUT:    none
      OUTPUT:   pointer to distance matrix, has to be freed when program stops
*/
{
  int     i, j;
  int     **matrix;

  if((matrix = malloc(sizeof( int) * ncities * ncities +
		      sizeof( int *) * ncities )) == NULL){
    printf("Out of memory, exit.");
    exit(1);
  }
  for ( i = 0 ; i < ncities ; i++ ) {
    matrix[i] = (int *)(matrix + ncities) + i*ncities;
    for ( j = 0  ; j < ncities ; j++ ) {
/*  	printf("%d %d  ",i,j); */
/*  	printf("%f %f  \n",xc[i],xc[j]); */
      matrix[i][j] = distance(i,j); 
    }
  }
  return matrix;
}

void read_instance(const char *tsp_file_name) 
/*    
      FUNCTION: parse and read instance file
      INPUT:    instance name
      OUTPUT:   list of coordinates for all nodes
      COMMENTS: Instance files have to be in TSPLIB format, otherwise procedure fails
*/
{
    FILE         *tsp_file;
    char         buf[LINE_BUF_LEN];
    int          i, j;

    tsp_file = fopen(tsp_file_name, "r");
    if ( tsp_file == NULL ) {
	fprintf(stderr,"No instance file specified, abort\n");
	exit(1);
    }
    assert(tsp_file != NULL);
#ifdef DEBUG
    printf("\nreading tsp-file %s ... \n\n", tsp_file_name);
#endif

    fscanf(tsp_file,"%s", buf);
    while ( strcmp("NODE_COORD_SECTION", buf) != 0 ) {
	if ( strcmp("NAME", buf) == 0 ) {
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s ", buf); )
	    fscanf(tsp_file, "%s", buf);
	    strcpy(name, buf);
	    // TRACE ( printf("%s \n", name); )
	    buf[0]=0;
	}
	else if ( strcmp("NAME:", buf) == 0 ) {
	    fscanf(tsp_file, "%s", buf);
	    strcpy(name, buf);
	    // TRACE ( printf("%s \n", name); )
	    buf[0]=0;
	}
	else if ( strcmp("COMMENT", buf) == 0 ){
	    fgets(buf, LINE_BUF_LEN, tsp_file);
	    // TRACE ( printf("%s", buf); )
	    buf[0]=0;
	}
	else if ( strcmp("COMMENT:", buf) == 0 ){
	    fgets(buf, LINE_BUF_LEN, tsp_file);
	    // TRACE ( printf("%s", buf); )
	    buf[0]=0;
	}
	else if ( strcmp("TYPE", buf) == 0 ) {
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s ", buf); )
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s\n", buf); )
	    if( strcmp("TSP", buf) != 0 ) {
		fprintf(stderr,"\n Not a TSP instance in TSPLIB format !!\n");
		exit(1);
	    }
	    buf[0]=0;
	}
	else if ( strcmp("TYPE:", buf) == 0 ) {
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s\n", buf); )
	    if( strcmp("TSP", buf) != 0 ) {
		fprintf(stderr,"\n Not a TSP instance in TSPLIB format !!\n");
		exit(1);
	    }
	    buf[0]=0;
	}
	else if( strcmp("DIMENSION", buf) == 0 ){
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s ", buf); );
	    fscanf(tsp_file, "%d", &ncities);
	    // TRACE ( printf("%d\n", ncities); );
	    assert ( ncities > 2 && ncities < 6000);
	    buf[0]=0;
	}
	else if ( strcmp("DIMENSION:", buf) == 0 ) {
	    fscanf(tsp_file, "%d", &ncities);
	    // TRACE ( printf("%d\n", ncities); );
	    assert ( ncities > 2 && ncities < 6000);
	    buf[0]=0;
	}
	else if( strcmp("DISPLAY_DATA_TYPE", buf) == 0 ){
	    fgets(buf, LINE_BUF_LEN, tsp_file);
	    // TRACE ( printf("%s", buf); );
	    buf[0]=0;
	}
	else if ( strcmp("DISPLAY_DATA_TYPE:", buf) == 0 ) {
	    fgets(buf, LINE_BUF_LEN, tsp_file);
	    // TRACE ( printf("%s", buf); );
	    buf[0]=0;
	}
	else if( strcmp("EDGE_WEIGHT_TYPE", buf) == 0 ){
	    buf[0]=0;
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s ", buf); );
	    buf[0]=0;
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s\n", buf); );
	    if ( strcmp("EUC_2D", buf) == 0 ) {
		distance = round_distance;
	    }
	    else if ( strcmp("CEIL_2D", buf) == 0 ) {
		distance = ceil_distance;
	    }
	    else if ( strcmp("GEO", buf) == 0 ) {
		distance = geo_distance;
	    }
	    else if ( strcmp("ATT", buf) == 0 ) {
		distance = att_distance;
	    }
	    else
		fprintf(stderr,"EDGE_WEIGHT_TYPE %s not implemented\n",buf);
	    strcpy(edge_weight_type, buf);
	    buf[0]=0;
	}
	else if( strcmp("EDGE_WEIGHT_TYPE:", buf) == 0 ){
	    /* set pointer to appropriate distance function; has to be one of 
	       EUC_2D, CEIL_2D, GEO, or ATT. Everything else fails */
	    buf[0]=0;
	    fscanf(tsp_file, "%s", buf);
	    // TRACE ( printf("%s\n", buf); )
		printf("%s\n", buf);
	    printf("%s\n", buf);
	    if ( strcmp("EUC_2D", buf) == 0 ) {
		distance = round_distance;
	    }
	    else if ( strcmp("CEIL_2D", buf) == 0 ) {
		distance = ceil_distance;
	    }
	    else if ( strcmp("GEO", buf) == 0 ) {
		distance = geo_distance;
	    }
	    else if ( strcmp("ATT", buf) == 0 ) {
		distance = att_distance;
	    }
	    else {
		fprintf(stderr,"EDGE_WEIGHT_TYPE %s not implemented\n",buf);
		exit(1);
	    }
	    strcpy(edge_weight_type, buf);
	    buf[0]=0;
	}
	buf[0]=0;
	fscanf(tsp_file,"%s", buf);
    }


    if( strcmp("NODE_COORD_SECTION", buf) == 0 ){
	// TRACE ( printf("found section contaning the node coordinates\n"); )
	    }
    else{
	fprintf(stderr,"\n\nSome error ocurred finding start of coordinates from tsp file !!\n");
	exit(1);
    }

    if( (xc = malloc(sizeof(double) * ncities)) == NULL )
	exit(EXIT_FAILURE);
    if( (yc = malloc(sizeof(double) * ncities)) == NULL )
	exit(EXIT_FAILURE);
	
    for ( i = 0 ; i < ncities ; i++ ) {
	fscanf(tsp_file,"%d %lf %lf", &j, &xc[i], &yc[i] );
    }

    // TRACE ( printf("number of cities is %d\n",ncities); )
    // TRACE ( printf("\n... done\n"); )
}

/*************************************************************************/

int compute_length ( int *t ) 
/*    
      FUNCTION: calculates the tour length of tour t
      INPUT:    pointer to the tour 
      OUTPUT:   tour length of the tour
      REMARK:   it is assumed that at position n the index of 
                the first city in the tour is repeated (see comment in main.c). 
*/
{
  int  i;
  int  t_length = 0;
  
  for ( i = 0 ; i < ncities ; i++ ) {
      t_length += distMat[t[i]][t[i+1]];
   }
   return t_length;
}

int **compute_NNLists( void )
/*    
      FUNCTION: computes nearest neighbor lists of size nn_ls for all cities
      INPUT:    none
      OUTPUT:   pointer to the nearest neighbor lists
      REMARKS:  rather inefficient way of generating the NN lists; better use geometry
*/
{
  int i, node;
  int *distance_vector;
  int *help_vector;
  int **m_nnear;
 
/*    printf("\n computing nearest neighbor lists, "); */


  nn_ls = MIN(nn_ls,ncities);

  if((m_nnear = malloc(sizeof(int) * ncities * nn_ls
		       + ncities * sizeof(int *))) == NULL){
    exit(EXIT_FAILURE);
  }
  distance_vector = calloc(ncities, sizeof(int));
  help_vector = calloc(ncities, sizeof(int));
 
  for ( node = 0 ; node < ncities ; node++ ) {  /* compute cnd-sets for all node */
    m_nnear[node] = (int *)(m_nnear + ncities) + node * nn_ls;

    for ( i = 0 ; i < ncities ; i++ ) {  /* Copy distances from nodes to the others */
      distance_vector[i] = distMat[node][i];
      help_vector[i] = i;
    }
    distance_vector[node] = INT_MAX;  /* city is not nearest neighbour */
    sort(distance_vector, help_vector, 0, ncities-1);
    for ( i = 0 ; i < nn_ls ; i++ ) {
      m_nnear[node][i] = help_vector[i];
    }
  }
  free(distance_vector);
  free(help_vector);
/*    printf("\n    .. done\n"); */
  return m_nnear;
}








