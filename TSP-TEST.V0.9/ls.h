/*

######################################################
###    simple LS and ILS algorithms for the TSP    ###
######################################################

      Version: 0.1
      File:    ls.h
      Author:  Thomas Stuetzle
      Purpose: implementation of local search and ils routines
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

#define TRUE 1
#define FALSE 0

extern int dlb_flag; 

extern int *dlb;               /* don't look bit vector */

extern int  nn_ls;             /* length of nearest neighbor list */

int * construct_nn_tour( void );

void two_opt_f( int *tour );

void two_opt_first( int *tour );

void two_opt_best( int *tour );

int * doublebridge_window( int *tour );

void accept( int *t, int *l, int *s, int i );

long int three_opt_first( int *tour );
