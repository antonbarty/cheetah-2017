/*
 *  median.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 4/8/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdint.h>
#include "median.h"

/*
 *	Find kth smallest element of a data array
 *	Algorithm from Wirth "Algorithms + data structures = programs" 
 *	Englewood Cliffs: Prentice-Hall, 1976, p. 366
 *	See: http://ndevilla.free.fr/median/median.pdf
 */
#define SWAP(a,b) { int16_t t=(a);(a)=(b);(b)=t; }
int16_t kth_smallest(int16_t *a, long n, long k) {
	register long i,j,l,m;
	register int16_t x;
	l=0; 
	m=n-1; 
	while (l<m) {
		x=a[k]; 
		i=l; 
		j=m; 
		do {
			while (a[i]<x) i++ ;
			while (x<a[j]) j-- ;
			if (i<=j) {
				SWAP(a[i],a[j]); 
				i++; 
				j--;
			} 
		} while (i<=j);
		if (j<k) l=i; 
		if (k<i) m=j ;
	} 			
	return a[k] ;
}
#undef SWAP

