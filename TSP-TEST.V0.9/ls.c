/*

######################################################
###    simple LS and ILS algorithms for the TSP    ###
######################################################

      Version: 0.1
      File:    ls.c
      Author:  Thomas Stuetzle
      Purpose: implementation of local search and ils routines
      Check:   README and gpl.txt
      Remark:  though reasonably fast, the code is definitely the most efficient possible
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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "utilities.h"
#include "instance.h"
#include "ls.h"
#include "timer.h"

#define  WINDOW_SIZE 500

#define  RADIUS      10

int dlb_flag = TRUE;  /* flag indicating whether don't look bits are
			 used. I recommend to always use it if ILS is run */

int *dlb;             /* don't look bit vector */

int  nn_ls;           /* length of nearest neighbor list */

int find_closest( int pos, int *v, int *t )
{ 
    int i, last, next, b;
    
    next = 0;
    last = t[pos];
    b = INT_MAX;             /* search for closest city */    
    for ( i = 0 ; i < ncities ; i++ ) {
	if ( v[i] == TRUE )      /* if already visited */ 
	    ;
	else if (i == last)
	    printf("Strange to reach this position %d %d\n",i,last);
	else {
	    if ( distMat[last][i] < b) {
		next = i;
		b = distMat[last][i];
	    }
	} 
    }
    return next;
}

int * construct_nn_tour( void )
/*    
      FUNCTION: generates an initial tour, here NN
      INPUT:    none
      OUTPUT:   pointer to the initial tour
*/
{
    int i, next;
    int *visited;
    int *nn_tour;
    
    
    if ( (visited = calloc( ncities, sizeof(int))) == NULL)
	exit(EXIT_FAILURE);
    
    if ( (nn_tour = malloc( sizeof(int) * (ncities+1))) == NULL)
	exit(EXIT_FAILURE);
    
    next = random_number( &seed ) % ncities;
    nn_tour[0] = next;
    visited[next] = TRUE;
    for ( i = 1 ; i < ncities ; i++) {
	next = find_closest( i-1, visited, nn_tour );
	nn_tour[i] = next;
	visited[next] = TRUE;
    }
    
    nn_tour[ncities] = nn_tour[0];
    
    return nn_tour;
    free (visited);
}

void printTour( int *tour ) 
{

    int i;

    for ( i = 0 ; i <= ncities ; i++ ) {
	printf(" %d",tour[i]);
    }
    printf("\n"); 
}


void two_opt_f( int *tour ) {
/* 
   FUNCTION:      very simple 2-opt first improvement without speed-up techniques
   INPUT:         a pointer to a tour
   OUTPUT:        none
   (SIDE)EFFECTS: tour is 2-opted
   REMARKS:       I did not really test this one
*/

    int i, j, h, l, temp;
    int n_inner_loop;
    int improvement_flag = TRUE;
    int num_exchanges = 0, num_scans = 0;
    int remove, add, improvement;
/*      printTour( tour ); */
    printf("%d ",compute_length(tour)); 
    while( improvement_flag ) {
	improvement_flag = FALSE;
	num_scans++;
	for( i = 0 ; i < ncities-2 ; i++ ) {
	    if ( i )
		n_inner_loop = ncities;
	    else
		n_inner_loop = ncities - 1;
	    for( j = i+2 ; j < n_inner_loop ; j++ ) {

		remove = distMat[tour[i]][tour[i+1]] + distMat[tour[j]][tour[j+1]];
		add   = distMat[tour[i]][tour[j]] + distMat[tour[i+1]][tour[j+1]];

		improvement = - remove + add;
		if( improvement < 0 ) {
		    improvement_flag = TRUE;
		    num_exchanges++;
		    h = i + 1;
		    l = j;
		    while( l >= h) {
			temp=tour[h];				
			tour[h]=tour[l];
			tour[l]=temp;
			h++;
			l--;
		    }
		}
	    }
	}
    }
/*    printf("%d %d %d %f\n",compute_length(tour),num_exchanges,num_scans,fabs(elapsed_time(VIRTUAL)));  */
/*      printTour( tour ); */
}



void two_opt_b( int *tour ) {
/* 
   FUNCTION:      very simple 2-opt best improvement without speed-up techniques
   INPUT:         a pointer to a tour
   OUTPUT:        none
   (SIDE)EFFECTS: tour is 2-opted
   REMARKS:       I did not really test this one
*/

    int i, j, h, l, temp;
    h = 0;
    l = 0;
    int n_inner_loop;
    int improvement_flag = TRUE;
    int gain, max_gain;
    int num_exchanges = 0;
    int remove, add;
    
    printf("%d ",compute_length(tour)); 
    while( improvement_flag ) {
	improvement_flag = FALSE;
	max_gain = 0;
	for( i = 0 ; i < ncities-2 ; i++ ) {
	    if ( i )
		n_inner_loop = ncities;
	    else
		n_inner_loop = ncities - 1;
	    for( j = i+2 ; j < n_inner_loop ; j++ ) {
		
		remove = distMat[tour[i]][tour[i+1]] + distMat[tour[j]][tour[j+1]];
		add   = distMat[tour[i]][tour[j]] + distMat[tour[i+1]][tour[j+1]];

		gain = - remove + add;
		if( gain < max_gain ) {
		    improvement_flag = TRUE;
		    max_gain = gain;
		    h = i + 1;
		    l = j;
		}
	    }
	}
	if ( improvement_flag ) {
	    num_exchanges++;
	    while( l >= h) {
		temp=tour[h];				
		tour[h]=tour[l];
		tour[l]=temp;
		h++;
		l--;
	    }
	}
    }
/*    printf("%d %d %d %f\n",compute_length(tour),num_exchanges,num_exchanges+1,fabs(elapsed_time(VIRTUAL)));  */
}



void two_opt_first( int *tour ) {
/* 
   FUNCTION:      2-opt first improvement including speed-ups (dlbs, fixed radius near neighbour searches, neughbourhood lists)
   INPUT:         a pointer to a tour
   OUTPUT:        none
   (SIDE)EFFECTS: tour is 2-opted (modulo speed-ups)
   COMMENTS:      Concerning the speed-ups used 
   here consult, for example, Chapter 8 of
   Holger H. Hoos and Thomas Stuetzle, 
   Stochastic Local Search---Foundations and Applications, 
   Morgan Kaufmann Publishers, 2004.*/

    int c1, c2;         /* cities considered for an exchange */
    int suc_c1, suc_c2;     /* successors of c1 and c2           */
    int pred_c1, pred_c2;     /* predecessors of c1 and c2         */   
    int pos_c1, pos_c2;     /* positions of cities c1, c2        */
    int i, j, h, l;
    h = 0;
    l = 0;
    int improvement_flag, improve_node, help, n_improves = 0, n_exchanges=0;
    int h1=0, h2=0, h3=0, h4=0;
    int radius;             /* radius of nn-search */
    int gain = 0;
    int tour_length;
    int *pos;               /* positions of cities in tour */ 
/*    int *r;   */                /* random vector */

/*    tour_length = compute_length(tour); */
/*    printf("%d ",tour_length); */
    /* determine cities' positions, reset don't look bits to 0 */
    pos = calloc(ncities, sizeof(int));
    for ( i = 0 ; i < ncities ; i++ ) {
	pos[tour[i]] = i;
/*      dlb[i] = FALSE; */
    }
    improvement_flag = TRUE;
/*    r = generate_random_vector(); */

/*    printf("Before 2-opt first %d\n",compute_length(tour)); */

    while ( improvement_flag ) {

	improvement_flag = FALSE;

	for (l = 0 ; l < ncities; l++) {

	    c1 = l;
/*  	c1 = r[l]; */
	    if ( dlb_flag && dlb[c1] )
		continue; 
	    improve_node = FALSE;
	    pos_c1 = pos[c1];
	    suc_c1 = tour[pos_c1+1];
	    radius = distMat[c1][suc_c1];
	
	    /* First search for c1's nearest neighbours, use successor of c1 */
	
	    for ( h = 0 ; h < nn_ls ; h++ ) {
	    
		c2 = nnMat[c1][h]; /* exchange partner, determine its position */
		if ( radius > distMat[c1][c2] ) {
		    suc_c2 = tour[pos[c2]+1];
		    gain =  - radius + distMat[c1][c2] + 
			distMat[suc_c1][suc_c2] - distMat[c2][suc_c2];
		    if ( gain < 0 ) {
			h1 = c1; h2 = suc_c1; h3 = c2; h4 = suc_c2; 
			improve_node = TRUE;
			goto exchange2opt;
		    }
		}
		else 
		    break;
	    }      
	    /* Search one for next c1's h-nearest neighbours, use predecessor c1 */
	    if (pos_c1 > 0)
		pred_c1 = tour[pos_c1-1];
	    else 
		pred_c1 = tour[ncities-1];
	    radius = distMat[pred_c1][c1];
	    for ( h = 0 ; h < nn_ls ; h++ ) {
		c2 = nnMat[c1][h];  /* exchange partner, determine its position */
		if ( radius > distMat[c1][c2] ) {
		    pos_c2 = pos[c2];
		    if (pos_c2 > 0)
			pred_c2 = tour[pos_c2-1];
		    else 
			pred_c2 = tour[ncities-1];
		    if ( pred_c2 == c1 )
			continue;
		    if ( pred_c1 == c2 )
			continue;
		    gain =  - radius + distMat[c1][c2] + 
			distMat[pred_c1][pred_c2] - distMat[pred_c2][c2];
		    if ( gain < 0 ) {
			h1 = pred_c1; h2 = c1; h3 = pred_c2; h4 = c2; 
			improve_node = TRUE;
			goto exchange2opt;
		    }
		}
		else 
		    break;
	    }      
	    if (improve_node) {
	    exchange2opt:
		n_exchanges++;
		improvement_flag = TRUE;
		tour_length += gain;
		dlb[h1] = FALSE; dlb[h2] = FALSE;
		dlb[h3] = FALSE; dlb[h4] = FALSE;
		    /* Now perform move */
		    if ( pos[h3] < pos[h1] ) {
			help = h1; h1 = h3; h3 = help;
			help = h2; h2 = h4; h4 = help;
		    }
		if ( pos[h3] - pos[h2] < ncities / 2 + 1) {
		    /* reverse inner part from pos[h2] to pos[h3] */
		    i = pos[h2]; j = pos[h3];
		    while (i < j) {
			c1 = tour[i];
			c2 = tour[j];
			tour[i] = c2;
			tour[j] = c1;
			pos[c1] = j;
			pos[c2] = i;
			i++; j--;
		    }
		}
		else {
		    /* reverse outer part from pos[h4] to pos[h1] */
		    i = pos[h1]; j = pos[h4];
		    if ( j > i )
			help = ncities - (j - i) + 1;
		    else 
			help = (i - j) + 1;
		    help = help / 2;
		    for ( h = 0 ; h < help ; h++ ) {
			c1 = tour[i];
			c2 = tour[j];
			tour[i] = c2;
			tour[j] = c1;
			pos[c1] = j;
			pos[c2] = i;
			i--; j++;
			if ( i < 0 )
			    i = ncities-1;
			if ( j >= ncities )
			    j = 0;
		    }
		    tour[ncities] = tour[0];
		}
	    } else {
		dlb[c1] = TRUE; 
	    }
      
	}
	if ( improvement_flag ) {
	    n_improves++;
	}
    }
    free( pos );
/*    free( r ); */
/*    printf("%d %d %d %f\n",compute_length(tour),n_exchanges,n_improves,fabs(elapsed_time(VIRTUAL)));  */
}


void two_opt_best( int *tour ) {
/* 
      FUNCTION:      2-opt best improvement including speed-ups (dlbs, fixed radius near neighbour searches, neughbourhood lists)
      INPUT:         a pointer to a tour
      OUTPUT:        none
      (SIDE)EFFECTS: tour is 2-opted (modulo speed-ups)
*/

  int c1, c2;         /* cities considered for an exchange */
  int suc_c1, suc_c2;     /* successors of c1 and c2           */
  int pred_c1, pred_c2;     /* predecessors of c1 and c2         */   
  int pos_c1, pos_c2;     /* positions of cities c1, c2        */
  int i, j, h, l;
  int improvement_flag, help, n_exchanges=0;
  int h1=0, h2=0, h3=0, h4=0;
  int radius;             /* radius of nn-search */
  int gain = 0;
  int max_gain = 0;
  int tour_length;
  int *pos;               /* positions of cities in tour */ 

  tour_length = compute_length(tour);
/*    printf("%d ",tour_length);  */
  /* determine cities' positions, reset don't look bits to 0 */
  pos = calloc(ncities, sizeof(int));
  for ( i = 0 ; i < ncities ; i++ ) {
    pos[tour[i]] = i;
  }
  improvement_flag = TRUE;

/*    printf("Before veryfast 2-opt first %d\n",compute_length(tour)); */

  while ( improvement_flag ) {

    improvement_flag = FALSE;
    max_gain = 0;

    for (l = 0 ; l < ncities; l++) {

      c1 = l; 
      pos_c1 = pos[c1];
      suc_c1 = tour[pos_c1+1];
      radius = distMat[c1][suc_c1];

      /* First search for c1's nearest neighbours, use successor of c1 */

      for ( h = 0 ; h < nn_ls ; h++ ) {

	c2 = nnMat[c1][h]; /* exchange partner, determine its position */
	if ( radius > 0 /*  distMat[c1][c2] */ ) {
	  suc_c2 = tour[pos[c2]+1];
	  gain =  - radius + distMat[c1][c2] + 
	            distMat[suc_c1][suc_c2] - distMat[c2][suc_c2];
	  if ( gain < max_gain ) {
	    h1 = c1; h2 = suc_c1; h3 = c2; h4 = suc_c2; 
	    improvement_flag = TRUE;
	    max_gain = gain;
	  }
	}
	else 
	  break;
      }      
      /* Search one for next c1's h-nearest neighbours, use predecessor c1 */
      if (pos_c1 > 0)
	pred_c1 = tour[pos_c1-1];
      else 
	pred_c1 = tour[ncities-1];
      radius = distMat[pred_c1][c1];
      for ( h = 0 ; h < nn_ls ; h++ ) {
	c2 = nnMat[c1][h];  /* exchange partner, determine its position */
	if ( radius > 0 /*  distMat[c1][c2] */ ) {
	  pos_c2 = pos[c2];
	  if (pos_c2 > 0)
	    pred_c2 = tour[pos_c2-1];
	  else 
	    pred_c2 = tour[ncities-1];
	  if ( pred_c2 == c1 )
	    continue;
	  if ( pred_c1 == c2 )
	    continue;
	  gain =  - radius + distMat[c1][c2] + 
	            distMat[pred_c1][pred_c2] - distMat[pred_c2][c2];
	  if ( gain < max_gain ) {
	    h1 = pred_c1; h2 = c1; h3 = pred_c2; h4 = c2; 
	    improvement_flag = TRUE;
	    max_gain = gain;
	  }
	}
	else 
	  break;
      }
    }
    if (improvement_flag) {
	n_exchanges++;
	tour_length += max_gain;
	/* Now perform move */
	if ( pos[h3] < pos[h1] ) {
	  help = h1; h1 = h3; h3 = help;
	  help = h2; h2 = h4; h4 = help;
	}
	if ( pos[h3] - pos[h2] < ncities / 2 + 1) {
	  /* reverse inner part from pos[h2] to pos[h3] */
	  i = pos[h2]; j = pos[h3];
	  while (i < j) {
	    c1 = tour[i];
	    c2 = tour[j];
	    tour[i] = c2;
	    tour[j] = c1;
	    pos[c1] = j;
	    pos[c2] = i;
	    i++; j--;
	  }
	}
 	else {
	  /* reverse outer part from pos[h4] to pos[h1] */
 	  i = pos[h1]; j = pos[h4];
	  if ( j > i )
	    help = ncities - (j - i) + 1;
	  else 
	    help = (i - j) + 1;
	  help = help / 2;
 	  for ( h = 0 ; h < help ; h++ ) {
 	    c1 = tour[i];
 	    c2 = tour[j];
	    tour[i] = c2;
	    tour[j] = c1;
 	    pos[c1] = j;
 	    pos[c2] = i;
 	    i--; j++;
	    if ( i < 0 )
	      i = ncities-1;
	    if ( j >= ncities )
	      j = 0;
 	  }
	  tour[ncities] = tour[0];
	}
      }
  }  
  free( pos );
/*    printf("%d %d %d %f\n",compute_length(tour),n_exchanges,n_exchanges+1,fabs(elapsed_time(VIRTUAL)));  */
}


int * doublebridge_window( int *tour )
/* 
      FUNCTION:      introduces a biased random doublebridge move into tour
      INPUT:         a pointer to a tour
      OUTPUT:        none
      (SIDE)EFFECTS: doublebridge move is applied to tour; the four cut positions 
                     are restricted to be within a range of WINDOW_SIZE successive 
		     cities (see define above ) 
*/
{
  int     g, h, i, j;
  int     help1, help2;
  double  rnd_0;
  int     values[4], val[4];
  int     *s;


  rnd_0 = ran01 ( &seed );
  i     = (int) (ncities - 1) * rnd_0 + 1;
  val[0] = i;
  j = i;
  
  while ( j == i ) {
    rnd_0 = ran01 ( &seed );
    j     = (int) WINDOW_SIZE * rnd_0 + 1 + i;
    j = j % ncities;
    val[1] = j;
  }

  g = j;
  while ( (g == j) || (g == i) ) {
    rnd_0 = ran01 ( &seed );
    g = (int) WINDOW_SIZE * rnd_0 + 1 + i;
    g = g % ncities;
    val[2] = g;
  }

  h = j;
  while ( (h == j) || (h == i) || (h == g) ) {
    rnd_0 = ran01 ( &seed );
    h = (int) WINDOW_SIZE * rnd_0 + 1 + i;
    h = h % ncities;
    val[3] = h;
  }
/*  printf("Cut at positions: %ld %ld %ld %ld\n",i,j,g,h); */

  for ( i = 0; i <= 3; i++) {
    help1 = INT_MAX;
    j = 0;
    for ( g = 0; g <= 3; g++) {
      if ( help1 > val[g] ) {
	help1 = val[g];
	h = g;
      }
    }
    values[i] = help1;
    val[h] = INT_MAX;
  }
  i = values[0]; j = values[1]; g = values[2]; h = values[3];
/*    printf("Cut at positions %ld %ld %ld %ld\n",i,j,g,h); */

  /* Set don't look bits for the cities around the cut in a radius RADIUS */  
  if ( i <= RADIUS  )
    help1 = ncities - ( RADIUS - i);
  else
    help1 = i - RADIUS + 1;
  assert ( help1 <= ncities && help1 >= 0 );
  for ( help2 = 0; help2 < 2*RADIUS; help2++ ) {
    dlb[tour[help1]] = FALSE;
    help1 += 1;
    help1 %= ncities;
  }
  if ( j <= RADIUS  )
    help1 = ncities - ( RADIUS - j );
  else 
    help1 = j - RADIUS + 1;
  assert ( help1 <= ncities && help1 >= 0 );
  for ( help2 = 0; help2 < 2*RADIUS; help2++ ) {
    dlb[tour[help1]] = FALSE;
    help1 += 1;
    help1 %= ncities;
  }
  if ( g <= RADIUS  )
    help1 = ncities - ( RADIUS - g);
  else 
    help1 = g - RADIUS + 1;
  assert ( help1 <= ncities && help1 > 0 );
  for ( help2 = 0; help2 < 2*RADIUS; help2++ ) {
    dlb[tour[help1]] = FALSE;
    help1 += 1;
    help1 %= ncities;
  }
  if ( h <= RADIUS  )
    help1 = ncities - ( RADIUS - h);
  else
    help1 = h - RADIUS + 1;
  assert ( help1 <= ncities && help1 > 0 );
  for ( help2 = 0; help2 < 2*RADIUS; help2++ ) {
    dlb[tour[help1]] = FALSE;
    help1 += 1;
    help1 %= ncities;
  }
  

  /* finally generate the modified tour */
  s = malloc((ncities + 1 ) * sizeof(int));

  help1 = 0;
  help2 = g + 1;
  while (help2 <= h) {
    s[help1] = tour[help2];
    help2++;help1++;
  }
  help2 = j + 1;
/*   printf(" help2 = %ld, help1 = %ld\n",help1,help2); */
  while (help2 <= g) {
    s[help1] = tour[help2];
    help2++;help1++;
  }
  help2 = i + 1;
/*   printf(" help2 = %ld, help1 = %ld\n",help1,help2); */
  while (help2 <= j) {
    s[help1] = tour[help2];
    help2++;help1++;
  }
  help2 = h + 1;
/*   printf(" help2 = %ld, help1 = %ld\n",help1,help2); */
  while (help2 < ncities) {
    s[help1] = tour[help2];
    help2++;help1++;
  }
  help2 = 0;
/*   printf(" help2 = %ld, help1 = %ld\n",help1,help2); */
  while (help2 <= i) {
    s[help1] = tour[help2];
    help2++;help1++;
  }
/*   printf(" help2 = %ld, help1 = %ld\n",help1,help2); */
  s[ncities] = s[0];

  return s;
}

void accept( int *t, int *l, int *s, int i ) 
/* 
      FUNCTION:      copies the better of tour t and s into t
      INPUT:         pointers to tours t and s, length *l of best so far tour, iteration i
      OUTPUT:        none
      (SIDE)EFFECTS: see FUNCTION; additionally *l may be updated if tour s is shorter than t
*/
{

    int j, help;

    help = compute_length( s );
    if ( help < *l ) { 
	*l = help;
	for ( j = 0; j <= ncities ; j++ )
	    t[j] = s[j];
	iteration_best_found = i;
	time_best_found = fabs(elapsed_time( VIRTUAL ));
	printf("best %d\t time %f\t iteration %d\n",help,time_best_found,i);
	fprintf(comp_report,"best %d\t time %f\t iteration %d\n",help,time_best_found,i);
    }

}


long int three_opt_first( int *tour )
/*    
      FUNCTION:       3-opt the tour
      INPUT:          pointer to the tour that is to optimize
      OUTPUT:         none
      (SIDE)EFFECTS:  tour is 3-opt
      COMMENT:        this is certainly not the best possible implementation of a 3-opt 
                      local search algorithm. In addition, the procedure is very long; the main 
		      reason herefore is that awkward way of making an exchange, where 
		      it is tried to copy only the shortest possible part of a tour. 
		      Concerning the speed-ups used here consult, for example, Chapter 8 of
		      Holger H. Hoos and Thomas Stuetzle, 
		      Stochastic Local Search---Foundations and Applications, 
		      Morgan Kaufmann Publishers, 2004.
*/
{
  /* In case a 2-opt move should be performed, we only need to store opt2_move = TRUE,
     as h1, .. h4 are used in such a way that they store the correct move */

  int   c1, c2, c3;           /* cities considered for an exchange */
  int   s_c1, s_c2, s_c3;     /* successors of these cities        */
  int   p_c1, p_c2, p_c3;     /* predecessors of these cities      */   
  int   pos1, pos2, pos3;     /* positions of cities c1, c2, c3    */
  int   i, j, h, g, l;
  int   improvement_flag, help, num_improves = 0;
  int   h1=0, h2=0, h3=0, h4=0, h5=0, h6=0; /* memorize cities involved in a move */
  int   diffs, diffp;
  int   between; 
  int   opt2_flag;  /* = TRUE: perform 2-opt move, otherwise none or 3-opt move */
  int   move_flag;  /* move_flag = 0 --> no 3-opt move 
			 move_flag = 1 --> between_move (c3 between c1 and c1)
			 move_flag = 2 --> not_between with successors of c2 and c3
			 move_flag = 3 --> not_between with predecessors of c2 and c3
			 move_flag = 4 --> cyclic move 
		      */
  int gain, move_value, radius;
  int decrease_breaks;      /* Stores decrease by breaking two edges (a,b) (c,d) */
  int m1 = 0, m2 = 0, m3 = 0, m4 = 0, m5 = 0;
  int val[3];
  int tour_length;
  int n1, n2, n3;
  int *pos;               /* positions of cities in tour */ 
  int *h_tour;            /* help vector for making exchanges */ 
  int *hh_tour;            /* help vector for making exchanges */ 
  /*  int *r;   */             /* random vector */

/*    tour_length = compute_length(tour); */
/*    printf("%d ",tour_length); */
  pos = malloc(ncities* sizeof(int));
  h_tour = malloc(ncities* sizeof(int));
  hh_tour = malloc(ncities* sizeof(int));

  for ( i = 0 ; i < ncities; i++ ) {
    pos[tour[i]] = i;
/*      dlb[i] = FALSE; */
  }
  improvement_flag = TRUE;
/*    r = generate_random_vector(); */

/*    printf("Before 3-opt first %ld\n",tour_length); */

  while ( improvement_flag ) {
      move_value = 0;
      improvement_flag = FALSE;

      for ( l = 0 ; l < ncities; l++ ) {

	  c1 = l; /*  c1 = r[l]; */
	  if ( dlb_flag && dlb[c1] )
	      continue; 
	  opt2_flag = FALSE;

	  move_flag = 0;
	  pos1 = pos[c1];
	  s_c1 = tour[pos1+1];
	  if (pos1 > 0)
	      p_c1 = tour[pos1-1];
	  else 
	      p_c1 = tour[ncities-1];

	  h = 0;    /* Search for one of the h-nearest neighbours */
	  while ( h < nn_ls ) {

	      c2   = nnMat[c1][h];  /* second city, determine its position */
	      pos2 = pos[c2];
	      s_c2 = tour[pos2+1];
	      if (pos2 > 0)
		  p_c2 = tour[pos2-1];
	      else 
		  p_c2 = tour[ncities-1];
	  
	      diffs = 0; diffp = 0;

	      radius = distMat[c1][s_c1];

	      /* Here a fixed radius neighbour search is performed */
	      if ( radius > distMat[c1][c2] ) {
		  decrease_breaks = - radius - distMat[c2][s_c2];
		  diffs =  decrease_breaks + distMat[c1][c2] + distMat[s_c1][s_c2];
		  diffp =  - radius - distMat[c2][p_c2] + 
		      distMat[c1][p_c2] + distMat[s_c1][c2];
	      }
	      else 
		  break;
	      if ( p_c2 == c1 )  /* in case p_c2 == c1 no exchange is possible */
		  diffp = 0;
	      if ( (diffs < move_value) || (diffp < move_value) ) {
		  improvement_flag = TRUE; 
		  if (diffs <= diffp) { 
		      h1 = c1; h2 = s_c1; h3 = c2; h4 = s_c2; 
		      move_value = diffs; 
		      opt2_flag = TRUE; move_flag = 0;
                      /*     	    goto exchange; */
		  } else {
		      h1 = c1; h2 = s_c1; h3 = p_c2; h4 = c2; 
		      move_value = diffp;  
		      opt2_flag = TRUE; move_flag = 0;
                      /*     	    goto exchange; */
		  }
	      }
	      /* Now perform the innermost search */
	      g = 0;
	      while (g < nn_ls) {
	  
		  c3   = nnMat[s_c1][g];
		  pos3 = pos[c3];
		  s_c3 = tour[pos3+1];
		  if (pos3 > 0)
		      p_c3 = tour[pos3-1];
		  else 
		      p_c3 = tour[ncities-1];
		  
		  if ( c3 == c1 ) {
		      g++;
		      continue;
		  }
		  else {
		      /* Perform fixed radius neighbour search for innermost search */
		      if ( decrease_breaks + distMat[c1][c2] < distMat[s_c1][c3] ) {
			  
			  if ( pos2 > pos1 ) {
			      if ( pos3 <= pos2 && pos3 > pos1 )
				  between = TRUE;
			      else 
				  between = FALSE;
			  }
			  else if ( pos2 < pos1 )
			      if ( pos3 > pos1 || pos3 < pos2 )
				  between = TRUE;
			      else 
				  between = FALSE;
			  else {
			      printf(" Strange !!, pos_1 %d == pos_2 %d, \n",pos1,pos2);
			  }
			  
			  if ( between ) {
			      /* We have to add edges (c1,c2), (c3,s_c1), (p_c3,s_c2) to get 
				 valid tour; it's the only possibility */
			      
			      gain = decrease_breaks - distMat[c3][p_c3] +
				  distMat[c1][c2] + distMat[c3][s_c1] +
				  distMat[p_c3][s_c2];
			      
			      /* check for improvement by move */
			      if ( gain < move_value ) {
				  if ( p_c3 == c1 ) {
				      g++;
				      continue;
				  }
				  improvement_flag = TRUE; /* g = neigh_ls + 1; */
				  move_value = gain;
				  opt2_flag = FALSE;
				  move_flag = 1;
				  /* store nodes involved in move */
				  h1 = c1; h2 = s_c1; h3 = c2; h4 = s_c2; h5 = p_c3; h6 = c3;
				  goto exchange;
			      } 
			  }
			  else {   /* not between(pos1,pos2,pos3) */
			      
			      /* We have to add edges (c1,c2), (s_c1,c3), (s_c2,s_c3) */
			      
			      gain = decrease_breaks - distMat[c3][s_c3] +
				  distMat[c1][c2] + distMat[s_c1][c3] + 
				  distMat[s_c2][s_c3];
			      
			      if ( pos2 == pos3 ) {
				  gain = 20000;
			      }
			      
			      /* check for improvement by move */
			      if ( gain < move_value ) {
				  improvement_flag = TRUE; /* g = neigh_ls + 1; */
				  move_value = gain;
				  opt2_flag = FALSE;
				  move_flag = 2;
				  /* store nodes involved in move */
				  h1 = c1; h2 = s_c1; h3 = c2; h4 = s_c2; h5 = c3; h6 = s_c3;
				  goto exchange;
			      }
			      
			      /* or add edges (c1,c2), (s_c1,c3), (p_c2,p_c3) */
			      gain = - radius - distMat[p_c2][c2] 
				  - distMat[p_c3][c3] +
				  distMat[c1][c2] + distMat[s_c1][c3] + 
				  distMat[p_c2][p_c3];
			      
			      if ( c3 == c2 || c2 == c1 || c1 == c3 || p_c2 == c1 ) {
				  gain = 2000000;
			      }
			      
			      if ( gain < move_value ) {
				  improvement_flag = TRUE;
				  move_value = gain;
				  opt2_flag = FALSE;
				  move_flag = 3;
				  h1 = c1; h2 = s_c1; h3 = p_c2; h4 = c2; h5 = p_c3; h6 = c3;
				  goto exchange;
			      }
			      
			      /* Or perform the 3-opt move where no subtour inversion is necessary 
				 i.e. delete edges (c1,s_c1), (c2,p_c2), (c3,s_c3) and 
				 add edges (c1,c2), (c3,s_c1), (p_c2,s_c3) */
			      
			      gain = - radius - distMat[p_c2][c2] - distMat[c3][s_c3]
				  + distMat[c1][c2] + distMat[s_c1][c3] + 
				  distMat[p_c2][s_c3];
			      
			      /* check for improvement */
			      if ( gain < move_value ) {
				  improvement_flag = TRUE;
				  move_value = gain;
				  opt2_flag = FALSE;
				  move_flag = 4;
				  improvement_flag = TRUE;
				  /* store nodes involved in move */
				  h1 = c1; h2 = s_c1; h3 = p_c2; h4 = c2; h5 = c3; h6 = s_c3; 
				  goto exchange;
			      }
			  }
		      }
		      else
			  g = nn_ls + 1;
		  }
		  g++;
	      }
	      h++;
	  }
	  if ( move_flag || opt2_flag ) {
	  exchange:
	      num_improves++;
	      tour_length += move_value;
	      move_value = 0;
/*  	      printf("cities %ld %ld %ld %ld %ld %ld\n",h1,h2,h3,h4,h5,h6); */
/*  	      printf("positions %ld %ld %ld %ld %ld %ld\n",pos[h1],pos[h2],pos[h3],pos[h4],pos[h5],pos[h6]); */
	      
	      /* Now make the exchange */
	      if ( move_flag ) {
		  dlb[h1] = FALSE; 
		  pos1 = pos[h1]; 
		  dlb[tour[pos1+1]] = FALSE;
		  if (pos1 > 0) 
		      help = tour[pos1-1];
		  else 
		      help = tour[ncities-1];
		  dlb[help] = FALSE;

		  dlb[h2] = FALSE; 
		  pos1 = pos[h2]; 
		  dlb[tour[pos1+1]] = FALSE;
		  if (pos1 > 0) 
		      help = tour[pos1-1];
		  else 
		      help = tour[ncities-1];
		  dlb[help] = FALSE;

		  dlb[h3] = FALSE; 
		  pos1 = pos[h3]; 
		  dlb[tour[pos1+1]] = FALSE;
		  if (pos1 > 0) 
		      help = tour[pos1-1];
		  else 
		      help = tour[ncities-1];
		  dlb[help] = FALSE;

		  dlb[h4] = FALSE; 
		  pos1 = pos[h4]; 
		  dlb[tour[pos1+1]] = FALSE;
		  if (pos1 > 0) 
		      help = tour[pos1-1];
		  else 
		      help = tour[ncities-1];
		  dlb[help] = FALSE;

		  dlb[h5] = FALSE; 
		  pos1 = pos[h5]; 
		  dlb[tour[pos1+1]] = FALSE;
		  if (pos1 > 0) 
		      help = tour[pos1-1];
		  else 
		      help = tour[ncities-1];
		  dlb[help] = FALSE;

		  dlb[h6] = FALSE;
		  pos1 = pos[h6]; 
		  dlb[tour[pos1+1]] = FALSE;
		  if (pos1 > 0) 
		      help = tour[pos1-1];
		  else 
		      help = tour[ncities-1];
		  dlb[help] = FALSE;
		  
		  pos1 = pos[h1]; pos2 = pos[h3]; pos3 = pos[h5];
		  
		  if ( move_flag == 4 ) {

/*  		      printf("move flag %ld\n",move_flag); */
		      
		      m1++;
		      if ( pos2 > pos1 ) 
			  n1 = pos2 - pos1;
		      else
			  n1 = ncities- (pos1 - pos2);
		      if ( pos3 > pos2 ) 
			  n2 = pos3 - pos2;
		      else
			  n2 = ncities- (pos2 - pos3);
		      if ( pos1 > pos3 ) 
			  n3 = pos1 - pos3;
		      else
			  n3 = ncities- (pos3 - pos1);
		      
		      /* n1: Laenge h2 - h3, n2: Laenge h4 - h5, n3: Laenge h6 - h1 */
		      val[0] = n1; val[1] = n2; val[2] = n3; 
		      /* Now order the partial tours */
		      h = 0;
		      help = INT_MIN;
		      for ( g = 0; g <= 2; g++) {
			  if ( help < val[g] ) {
			      help = val[g];
			      h = g;
			  }
		      }
		      
		      /* order partial tours according length */
		      if ( h == 0 ) {
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* copy of the part from pos[h4] to pos[h5]
			     copied directly: part from pos[h6] to pos[h1], it remains
			     the part from pos[h2] to pos[h3] */
			  j = pos[h4];
			  h = pos[h5];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h) {
			      i++;
			      j++;
			      if ( j  >= ncities)
				  j = 0;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  /* First copy partial tour 3 in new position */
			  j = pos[h4];
			  i = pos[h6];
			  tour[j] = tour[i];
			  pos[tour[i]] = j; 
			  while ( i != pos1) {
			      i++;
			      if ( i >= ncities)
				  i = 0;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      tour[j] = tour[i];
			      pos[tour[i]] = j; 
			  }
			  
			  /* Now copy stored part from h_tour */
			  j++;
			  if ( j >= ncities)
			      j = 0;
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		      else if ( h == 1 ) {
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  
			  /* Hilfskopie des Teils von pos[h6] bis pos[h1]
			     direkt kopiert: Teil von pos[h2] bis pos[h3], stehen
			     bleibt der Teil von pos[h4] bis pos[h5] */
			  j = pos[h6];
			  h = pos[h1];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h) {
			      i++;
			      j++;
			      if ( j  >= ncities)
				  j = 0;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  /* First copy partial tour 3 in new position */
			  j = pos[h6];
			  i = pos[h2];
			  tour[j] = tour[i];
			  pos[tour[i]] = j; 
			  while ( i != pos2) {
			      i++;
			      if ( i >= ncities)
				  i = 0;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      tour[j] = tour[i];
			      pos[tour[i]] = j; 
			  }
			  
			  /* Now copy stored part from h_tour */
			  j++;
			  if ( j >= ncities)
			      j = 0;
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities )
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		      else if ( h == 2 ) {
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h2] bis pos[h3]
			     direkt kopiert: Teil von pos[h4] bis pos[h5], stehen
			     bleibt der Teil von pos[h6] bis pos[h1] */
			  j = pos[h2];
			  h = pos[h3];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h) {
			      i++;
			      j++;
			      if ( j  >= ncities)
				  j = 0;
			      h_tour[i] = tour[j];
			      n1++;
			  }
	      
			  /* First copy partial tour 3 in new position */
			  j = pos[h2];
			  i = pos[h4];
			  tour[j] = tour[i];
			  pos[tour[i]] = j; 
			  while ( i != pos3) {
			      i++;
			      if ( i >= ncities)
				  i = 0;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      tour[j] = tour[i];
			      pos[tour[i]] = j; 
			  }
			  
			  /* Now copy stored part from h_tour */
			  j++;
			  if ( j >= ncities)
			      j = 0;
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i]; 
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		  }
		  else if ( move_flag == 1 ) {
		      m2++;
		      
/*  		      printf("move flag %ld\n",move_flag); */

		      if ( pos3 < pos2 ) 
			  n1 = pos2 - pos3;
		      else
			  n1 = ncities- (pos3 - pos2);
		      if ( pos3 > pos1 ) 
			  n2 = pos3 - pos1 + 1;
		      else
			  n2 = ncities- (pos1 - pos3 + 1);
		      if ( pos2 > pos1 ) 
			  n3 = ncities- (pos2 - pos1 + 1);
		      else
			  n3 = pos1 - pos2 + 1;
		      
		      /* n1: Laenge h6 - h3, n2: Laenge h5 - h2, n2: Laenge h1 - h3 */
		      val[0] = n1; val[1] = n2; val[2] = n3; 
		      /* Now order the partial tours */
		      h = 0;
		      help = INT_MIN;
		      for ( g = 0; g <= 2; g++) {
			  if ( help < val[g] ) {
			      help = val[g];
			      h = g;
			  }
		      }
		      /* order partial tours according length */
		      
		      if ( h == 0 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h5] bis pos[h2]
			     (invertiert) und von pos[h4] bis pos[h1] (invertiert)
			     stehen bleibt der Teil von pos[h6] bis pos[h3] */
			  j = pos[h5];
			  h = pos[h2];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  j = pos[h1];
			  h = pos[h4];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      hh_tour[i] = tour[j];
			      n2++;
			  }
			  
			  j = pos[h4];
			  for ( i = 0; i< n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if (j >= ncities)
				  j = 0;
			  }
			  
			  /* Now copy stored part from h_tour */
			  for ( i = 0; i< n1 ; i++ ) {
			      tour[j] = h_tour[i]; 
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		      else if ( h == 1 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von h3 bis h6 (wird invertiert) erstellen : */
			  j = pos[h3];
			  h = pos[h6];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h) {
			      i++;
			      j--;
			      if ( j  < 0 )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  j = pos[h6];
			  i = pos[h4];
			  
			  tour[j] = tour[i];
			  pos[tour[i]] = j; 
			  while ( i != pos1) {
			      i++;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      if ( i >= ncities)
				  i = 0;
			      tour[j] = tour[i];
			      pos[tour[i]] = j; 
			  }
			  
			  /* Now copy stored part from h_tour */
			  j++;
			  if ( j >= ncities)
			      j = 0;
			  i = 0;
			  tour[j] = h_tour[i];
			  pos[h_tour[i]] = j; 
			  while ( j != pos1 ) {
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      i++;
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			  }
			  tour[ncities] = tour[0];
		      }
		      
		      else if ( h == 2 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h2] bis pos[h5] und von
			     pos[h3] bis pos[h6] (invertiert) stehen bleibt der
			     Teil von pos[h4] bis pos[h1] */
			  j = pos[h2];
			  h = pos[h5];
			  i = 0;
			  h_tour[i] =  tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  j = pos2;
			  h = pos[h6];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      hh_tour[i] = tour[j];
			      n2++;
			  }
			  
			  j = pos[h2];
			  for ( i = 0; i< n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  
			  /* Now copy stored part from h_tour */
			  for ( i = 0; i< n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		  }
		  else if ( move_flag == 2 ) {
/*  		      printf("move flag %ld\n",move_flag); */
		      
		      m3++;
		      if ( pos3 < pos1 ) 
			  n1 = pos1 - pos3;
		      else
			  n1 = ncities- (pos3 - pos1);
		      if ( pos3 > pos2 ) 
			  n2 = pos3 - pos2;
		      else
			  n2 = ncities- (pos2 - pos3);
		      if ( pos2 > pos1 ) 
			  n3 = pos2 - pos1;
		      else
			  n3 = ncities- (pos1 - pos2);
/*   	    printf("lengths of partial tours: %ld, %ld, %ld\n",n1,n2,n3); */
		      /* n1: Laenge h6 - h1, n2: Laenge h4 - h5, n2: Laenge h2 - h3 */
		      
		      val[0] = n1; val[1] = n2; val[2] = n3; 
		      /* Determine which is the longest part */
		      h = 0;
		      help = INT_MIN;
		      for ( g = 0; g <= 2; g++) {
			  if ( help < val[g] ) {
			      help = val[g];
			      h = g;
			  }
		      }
		      /* order partial tours according length */
		      
/*    	    printf(" longest part is in part %ld\n",h); */
		      
		      if ( h == 0 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h3] bis pos[h2]
			     (invertiert) und von pos[h5] bis pos[h4], stehen
			     bleibt der Teil von pos[h6] bis pos[h1] */
			  j = pos[h3];
			  h = pos[h2];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  j = pos[h5];
			  h = pos[h4];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      hh_tour[i] = tour[j];
			      n2++;
			  }
			  
			  j = pos[h2];
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i]; 
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
	      
			  for ( i = 0; i < n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
			  /*  	      getchar(); */
		      }
		      else if ( h == 1 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h2] bis pos[h3] und von
			     pos[h1] bis pos[h6] (invertiert), stehen bleibt der
			     Teil von pos[h4] bis pos[h5] */
			  j = pos[h2];
			  h = pos[h3];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j++;
			      if ( j >= ncities )
				  j = 0;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  j = pos[h1];
			  h = pos[h6];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j =  ncities-1;
			      hh_tour[i] = tour[j];
			      n2++;
			  }
			  j = pos[h6];
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i]; 
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  for ( i = 0; i < n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		      
		      else if ( h == 2 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h1] bis pos[h6]
			     (invertiert) und von pos[h4] bis pos[h5] stehen
			     bleibt der Teil von pos[h2] bis pos[h3] */
			  j = pos[h1];
			  h = pos[h6];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }

			  j = pos[h4];
			  h = pos[h5];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h ) {
			      i++;
			      j++;
			      if ( j >= ncities )
				  j = 0;
			      hh_tour[i] = tour[j];
			      n2++;
			  }

			  j = pos[h4];
			  /* Now copy stored part from h_tour */
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  
			  /* Now copy stored part from h_tour */
			  for ( i = 0; i < n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		  }
		  else if ( move_flag == 3 ) {
		      m4++;
/*  		      printf("move flag %ld\n",move_flag); */
		      
		      if ( pos3 < pos1 ) 
			  n1 = pos1 - pos3;
		      else
			  n1 = ncities- (pos3 - pos1);
		      if ( pos3 > pos2 ) 
			  n2 = pos3 - pos2;
		      else
			  n2 = ncities- (pos2 - pos3);
		      if ( pos2 > pos1 ) 
			  n3 = pos2 - pos1;
		      else
			  n3 = ncities- (pos1 - pos2);
		      /* n1: Laenge h6 - h1, n2: Laenge h4 - h5, n2: Laenge h2 - h3 */
		      
		      val[0] = n1; val[1] = n2; val[2] = n3; 
		      /* Determine which is the longest part */
		      h = 0;
		      help = INT_MIN;
		      for ( g = 0; g <= 2; g++) {
			  if ( help < val[g] ) {
			      help = val[g];
			      h = g;
			  }
		      }
		      /* order partial tours according length */
		      
		      if ( h == 0 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h2] bis pos[h3]
			     (invertiert) und von pos[h4] bis pos[h5], stehen
			     bleibt der Teil von pos[h6] bis pos[h1] */
			  j = pos[h3];
			  h = pos[h2];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }
			  
			  j = pos[h2];
			  h = pos[h5];
			  i = pos[h4];
			  tour[j] = h4;
			  pos[h4] = j;
			  while ( i != h ) {
			      i++;
			      if ( i >= ncities)
				  i = 0;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      tour[j] = tour[i];
			      pos[tour[i]] = j;
			  }
			  j++;
			  if ( j >= ncities)
			      j = 0;
			  for ( i = 0; i < n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		      else if ( h == 1 ) {

/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h3] bis pos[h2]
			     (invertiert) und von pos[h6] bis pos[h1], stehen
			     bleibt der Teil von pos[h4] bis pos[h5] */
			  j = pos[h3];
			  h = pos[h2];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0  )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }

			  j = pos[h6];
			  h = pos[h1];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h ) {
			      i++;
			      j++;
			      if ( j >= ncities)
				  j = 0;
			      hh_tour[i] = tour[j];
			      n2++;
			  }
			  
			  j = pos[h6];
			  for ( i = 0; i<n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }

			  for ( i = 0 ; i < n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		      
		      else if ( h == 2 ) {
			  
/*  			  printf("move_flag %ld, h %ld\n",move_flag,h); */
			  /* Hilfskopie des Teils von pos[h4] bis pos[h5]
			     (invertiert) und von pos[h6] bis pos[h1] (invertiert)
			     stehen bleibt der Teil von pos[h2] bis pos[h3] */
			  j = pos[h5];
			  h = pos[h4];
			  i = 0;
			  h_tour[i] = tour[j];
			  n1 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      h_tour[i] = tour[j];
			      n1++;
			  }

			  j = pos[h1];
			  h = pos[h6];
			  i = 0;
			  hh_tour[i] = tour[j];
			  n2 = 1;
			  while ( j != h ) {
			      i++;
			      j--;
			      if ( j < 0 )
				  j = ncities-1;
			      hh_tour[i] = tour[j];
			      n2++;
			  }

			  j = pos[h4];
			  /* Now copy stored part from h_tour */
			  for ( i = 0; i< n1 ; i++ ) {
			      tour[j] = h_tour[i];
			      pos[h_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  /* Now copy stored part from h_tour */
			  for ( i = 0; i< n2 ; i++ ) {
			      tour[j] = hh_tour[i];
			      pos[hh_tour[i]] = j; 
			      j++;
			      if ( j >= ncities)
				  j = 0;
			  }
			  tour[ncities] = tour[0];
		      }
		  }
		  else {
		      printf(" Some very strange error must have occurred !!!\n\n");
		      exit(0);
		  }
	      }
	      if (opt2_flag) {
		  m5++;
/*  		  printf("2-opt move\n"); */
		  /* Now perform move */
		  dlb[h1] = FALSE; dlb[h2] = FALSE;
		  dlb[h3] = FALSE; dlb[h4] = FALSE;
		  if ( pos[h3] < pos[h1] ) {
		      help = h1; h1 = h3; h3 = help;
		      help = h2; h2 = h4; h4 = help;
		  }
		  if ( pos[h3]-pos[h2] < ncities/ 2 + 1) {
		      /* reverse inner part from pos[h2] to pos[h3] */
		      i = pos[h2]; j = pos[h3];
		      while (i < j) {
			  c1 = tour[i];
			  c2 = tour[j];
			  tour[i] = c2;
			  tour[j] = c1;
			  pos[c1] = j;
			  pos[c2] = i;
			  i++; j--;
		      }
		  }
		  else {
		      /* reverse outer part from pos[h4] to pos[h1] */
		      i = pos[h1]; j = pos[h4];
		      if ( j > i )
			  help = ncities- (j - i) + 1;
		      else 
			  help = (i - j) + 1;
		      help = help / 2;
		      for ( h = 0 ; h < help ; h++ ) {
			  c1 = tour[i];
			  c2 = tour[j];
			  tour[i] = c2;
			  tour[j] = c1;
			  pos[c1] = j;
			  pos[c2] = i;
			  i--; j++;
			  if ( i < 0 )
			      i = ncities- 1;
			  if ( j >= ncities)
			      j = 0;
		      }
		      tour[ncities] = tour[0];
		  }
	      }
	  }
	  else {
	      dlb[c1] = TRUE;
	  }
      }
  }
/*    free( r ); */
  free( pos );
  free( h_tour );
  free( hh_tour );
/*    printf("%d %d %d %f\n",compute_length(tour),num_improves,num_improves,fabs(elapsed_time(VIRTUAL)));  */
  return num_improves + 1;
}






