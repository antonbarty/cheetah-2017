/*
 * Algorithm from N. Wirth's book, implementation by N. Devillard.
 * This code in public domain.
 *	See: http://ndevilla.free.fr/median/median.pdf
 *	http://ndevilla.free.fr/median/median/src/wirth.c
 */

/*---------------------------------------------------------------------------
  Function :   kth_smallest()
  In       :   array of elements, # of elements in the array, rank k
  Out      :   one element
  Job      :   find the kth smallest element in the array
  Notice   :   use the median() macro defined below to get the median.
 
  Reference:
 
  Author: Wirth, Niklaus
  Title: Algorithms + data structures = programs
  Publisher: Englewood Cliffs: Prentice-Hall, 1976
  Physical description: 366 p.
  Series: Prentice-Hall Series in Automatic Computation
 
  ---------------------------------------------------------------------------*/

#include <stdint.h>
#include "median.h"

#define median(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2)-1)))


// overloaded to int16_t
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

// overloaded to float
#define SWAP(a,b) { float t=(a);(a)=(b);(b)=t; }
float kth_smallest(float *a, long n, long k) {
	register long i,j,l,m;
	register float x;
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

