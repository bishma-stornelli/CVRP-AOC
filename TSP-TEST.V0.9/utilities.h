/*

######################################################
###    simple LS and ILS algorithms for the TSP    ###
######################################################

      Version: 0.9
      File:    utilities.h
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

extern double ran01( long *idum );

extern long int random_number( long *idum );

extern int * generate_random_vector( void );


/* constants for a pseudo-random number generator, taken from
   Numerical Recipes in C book --- never trust the standard C random
   number generator */

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836

extern long int seed; /* seed for the random number generator */

void swap( int v[], int v2[], int i, int j);

void sort( int v[], int v2[], int left, int right);

