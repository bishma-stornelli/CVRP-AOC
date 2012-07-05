/*

######################################################
###    simple LS and ILS algorithms for the TSP    ###
######################################################

      Version: 0.9
      File:    instance.h
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

#define LINE_BUF_LEN     100
#define TRACE( x ) x

extern int  ncities;                 /* number of cities in the instance */

extern char     name[LINE_BUF_LEN];  /* instance name */

extern int  (*distance)( int, int);  /* pointer to function returning distance */

extern int  **distMat;               /* distance matrix */

extern int  **nnMat;                 /* table of nearest neighbor list for all cities */

extern void read_instance(const char *tsp_file_name);

extern int  ** compute_distances(void);

extern int  compute_length ( int *t );

extern int  **compute_NNLists( void );

extern FILE *report, *comp_report;

extern int     iteration_best_found;

extern double  time_best_found;

#define MAX(x,y)        ((x)>=(y)?(x):(y))
#define MIN(x,y)        ((x)<=(y)?(x):(y))

