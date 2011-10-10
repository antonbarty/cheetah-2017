/*
 * Algorithm from N. Wirth's book, implementation by N. Devillard.
 * This code in public domain.
 */

/*
 *	Find kth smallest element of a data array
 *	See: http://ndevilla.free.fr/median/median.pdf
 *	http://ndevilla.free.fr/median/median/src/wirth.c
 *	Reference: N. Wirth: "Algorithms + data structures = programs" Englewood Cliffs: Prentice-Hall, 1976, p. 366
 */

#include <stdint.h>
#include "median.h"

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

