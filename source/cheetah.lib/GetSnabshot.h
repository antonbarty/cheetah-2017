#ifndef GETSNABSHOT_H
#define GETSNABSHOT_H

#include <stdint.h>
#include <stdio.h>

/*template <class T>
void getSnapshot( T src,const int oriWidth, const int scale , const int width, T dst)
{
	int16_t ox, oy, dx1, dy1, dx2, dy2;
	int16_t ox1, oy1,ox2, oy2;
	T tp1, tp2;
	T p1,p2,p3,p4;
	double factor = (double)width/oriWidth;

	int max = oriWidth -1;
	for (int y = 0; y< width; y++){
		oy  = (double)y * factor;
		oy1 = (int) oy;
		oy2 = ( oy1 == max ) ? oy1 : oy1 + 1;
		dy1 = oy - (double) oy1;
		dy2 = 1.0 - (double)dy1; 
		// get temp pointers
		tp1 = src + oy1 * oriWidth;
		tp2 = src + oy2 * oriWidth;


		// for each pixel
		for ( int x = 0; x < width; x++ )
		{
			// X coordinates
			ox  = (double) x * factor;
			ox1 = (int) ox;
			ox2 = ( ox1 == max ) ? ox1 : ox1 + 1;
			dx1 = ox - (double) ox1;
			dx2 = 1.0 - dx1;

			// get four points
			p1 = tp1 + ox1 ;
			p2 = tp1 + ox2 ;
			p3 = tp2 + ox1 ;
			p4 = tp2 + ox2 ;

			// interpolate using 4 points

			*dst =  ((int16_t)
				dy2 * ( dx2 * ( *p1 ) + dx1 * ( *p2 ) ) +
				dy1 * ( dx2 * ( *p3 ) + dx1 * ( *p4 ) ) );

			
			dst++;
			p1++; 
			p2++;
			p3++; 
			p4++;

		}
		//dst += dstOffset;
	}
//	fclose(filePtr);
//	exit(1);
}*/



template <class T>
void getSnapshot( T src,const int oriWidth, const int scale , const int width, T dst)
{
	int16_t image[oriWidth][oriWidth];
	int16_t re[width][width];
	int count = 0;
	for(int i =0; i< oriWidth; i++){
		for (int j =0; j<oriWidth; j++){
			image[i][j] = src[count];
			count++;
			
		}
	}

	double res=0;
	for(int i = 0; i <width; i++)
	{
		for(int j = 0; j<width; j++){
			for (int x = i*scale; x <i*scale+scale; x++){
				for(int y = j*scale; y <j*scale+scale; y++){
					res = res + image[x][y];
				}

			}
		re[i][j] = (int16_t)(res/(double)scale/(double)scale +0.5);
		res = 0;
		}
	}

	for(int i = 0; i< width ; i++){
		for(int j =0; j<width; j++){
			*dst = re[i][j];
			dst ++;					
		}
		
	}
}

#endif // GETSNABSHOT_H

