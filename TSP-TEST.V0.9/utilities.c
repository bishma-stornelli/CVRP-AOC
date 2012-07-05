/*

######################################################
###    simple LS and ILS algorithms for the TSP    ###
######################################################

      Version: 0.9
      File:    utilities.c
      Author:  Thomas Stuetzle
      Purpose: some utility routines (random numbers, sorting)
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

#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"
#include "instance.h"

long int seed;


double ran01( long *idum ) {
/* 
      FUNCTION:      returns a pseudo-random number
      INPUT:         a pointer to the seed variable 
      OUTPUT:        a pseudo-random number uniformly distributed in [0,1]
      (SIDE)EFFECTS: changes the value of seed
*/
  long k;
  double ans;

  k =(*idum)/IQ;
  *idum = IA * (*idum - k * IQ) - IR * k;
  if (*idum < 0 ) *idum += IM;
  ans = AM * (*idum);
  return ans;
}

long int random_number( long *idum )
/*    
      FUNCTION:       generate an integer random number
      INPUT:          pointer to variable containing random number seed
      OUTPUT:         integer random number uniformly distributed in {0,2147483647}
      (SIDE)EFFECTS:  random number seed is modified (important, needs to be so!)
      ORIGIN:         numerical recipes in C
*/
{
  long k;

  k =(*idum)/IQ;
  *idum = IA * (*idum - k * IQ) - IR * k;
  if (*idum < 0 ) *idum += IM;
  return *idum;
}


int * generate_random_vector()
/*    
      FUNCTION:       generate a random vector, quick and dirty
      INPUT:          none
      OUTPUT:         pointer to a "random permutation"
*/

{
   int     *random_vector;
   int     i, help, node, tot_assigned = 0;
   double  rnd;

   random_vector = malloc(ncities * sizeof(int));  

   for ( i = 0 ; i < ncities; i++) 
     random_vector[i] = i;

   for ( i = 0 ; i < ncities ; i++ ) {
     /* find (randomly) an index for a free unit */ 
     rnd  = ran01 ( &seed );
     node = (long int) (rnd  * (ncities - tot_assigned)); 
     help = random_vector[i];
     random_vector[i] = random_vector[i+node];
     random_vector[i+node] = help;
     tot_assigned++;
   }

   return random_vector;

}

void swap(int v[], int v2[], int i, int j)
/*    
      FUNCTION:       swaps two elements in a vector and a second vector; used in quicksort
      INPUT:          two arrays, positions to be swapped
      OUTPUT:         none
*/
{
  long int tmp;

  tmp = v[i];
  v[i] = v[j];
  v[j] = tmp;
  tmp = v2[i];
  v2[i] = v2[j];
  v2[j] = tmp;
  
}

void sort(int v[], int v2[], int left, int right)
/*    
      FUNCTION:       implementation of quicksort; second index vector is updated, too
      INPUT:          two arrays, left and right delimiter
      OUTPUT:         none
*/
{
  long int k, last;

  if (left >= right) 
    return;
  swap(v, v2, left, (left + right)/2);
  last = left;
  for (k=left+1; k <= right; k++)
    if (v[k] < v[left])
      swap(v, v2, ++last, k);
  swap(v, v2, left, last);
  sort(v, v2, left, last);
  sort(v, v2, last+1, right);
}
