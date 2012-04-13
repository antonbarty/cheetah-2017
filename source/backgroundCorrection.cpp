/*
 *  backgroundCorrection.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 23/11/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "median.h"



/*
 *	Update background buffer
 */
void updateBackgroundBuffer(cEventData *eventData, cGlobal *global, int detID) {
	
    if(global->detector[detID].useBackgroundBufferMutex)
        pthread_mutex_lock(&global->bgbuffer_mutex);

	long frameID = global->detector[detID].bgCounter%global->detector[detID].bgMemory;	
	global->detector[detID].bgCounter += 1;
	memcpy(global->detector[detID].bg_buffer+global->detector[detID].pix_nn*frameID, eventData->detector[detID].corrected_data_int16, global->detector[detID].pix_nn*sizeof(int16_t));
	
	//for(long i=0;i<global->pix_nn;i++)
	//	global->bg_buffer[global->pix_nn*frameID + i] = eventData->corrected_data_int16[i];
	
    if(global->detector[detID].useBackgroundBufferMutex)
        pthread_mutex_unlock(&global->bgbuffer_mutex);
	
}




/*
 *	Calculate persistent background from stack of remembered frames
 */
void calculatePersistentBackground(cGlobal *global, int detID) {
	
	
	long	median_element = lrint((global->detector[detID].bgMemory*global->detector[detID].bgMedian));
	int16_t	*buffer = (int16_t*) calloc(global->detector[detID].bgMemory, sizeof(int16_t));
	printf("Finding %lith smallest element of buffer depth %li\n",median_element,global->detector[detID].bgMemory);	
	
	// Lock the global variables
    if(global->detector[detID].useBackgroundBufferMutex){
        //pthread_mutex_lock(&global->bgbuffer_mutex);
        pthread_mutex_lock(&global->selfdark_mutex);
    }
	
	// Loop over all pixels 
	for(long i=0; i<global->detector[detID].pix_nn; i++) {
		
		// Create a local array for sorting
		for(long j=0; j< global->detector[detID].bgMemory; j++) {
			buffer[j] = global->detector[detID].bg_buffer[j*global->detector[detID].pix_nn+i];
		}
		
		// Find median value of the temporary array
		global->detector[detID].selfdark[i] = kth_smallest(buffer, global->detector[detID].bgMemory, median_element);
	}	
	global->detector[detID].last_bg_update = global->detector[detID].bgCounter;

    if(global->detector[detID].useBackgroundBufferMutex){
        //pthread_mutex_unlock(&global->bgbuffer_mutex);
        pthread_mutex_unlock(&global->selfdark_mutex);
    }
	
	free (buffer);
}




/*
 *	Subtract persistent background 
 */
void subtractPersistentBackground(cEventData *eventData, cGlobal *global, int detID){
	
	float	top = 0;
	float	s1 = 0;
	float	s2 = 0;
	float	v1, v2;
	float	factor;
	float	gmd;
	
	
	// Add current (uncorrected) image to self darkcal
	pthread_mutex_lock(&global->selfdark_mutex);
	//for(long i=0;i<global->pix_nn;i++){
	//	global->selfdark[i] = ( eventData->corrected_data[i] + (global->bgMemory-1)*global->selfdark[i]) / global->bgMemory;
	//}
	gmd = (eventData->gmd21+eventData->gmd22)/2;
	global->avgGMD = ( gmd + (global->detector[0].bgMemory-1)*global->avgGMD) / global->detector[0].bgMemory;
	pthread_mutex_unlock(&global->selfdark_mutex);
	
	
	// Find appropriate scaling factor 
	if(global->detector[detID].scaleBackground) {
		for(long i=0;i<global->detector[detID].pix_nn;i++){
			//v1 = pow(global->selfdark[i], 0.25);
			//v2 = pow(eventData->corrected_data[i], 0.25);
			v1 = global->detector[detID].selfdark[i];
			v2 = eventData->detector[detID].corrected_data[i];
			if(v2 > global->hitfinderADC)
				continue;
			
			// Simple inner product gives cos(theta), which is always less than zero
			// Want ( (a.b)/|b| ) * (b/|b|)
			top += v1*v2;
			s1 += v1*v1;
			s2 += v2*v2;
		}
		factor = top/s1;
	}
	else 
		factor=1;
	
	
	// Do the weighted subtraction
	for(long i=0;i<global->detector[detID].pix_nn;i++) {
		eventData->detector[detID].corrected_data[i] -= (factor*global->detector[detID].selfdark[i]);	
	}	
	
}

/*
 *	Local background subtraction
 */
void subtractLocalBackground(cEventData *eventData, cGlobal *global, int detID){
	
	long		e,ee;
	long		counter;
	
	// Dereference datector arrays
	long		pix_nx = global->detector[detID].pix_nx;
	long		pix_ny = global->detector[detID].pix_ny;
	long		pix_nn = global->detector[detID].pix_nn;
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	long		nasics_x = global->detector[detID].nasics_x;
	long		nasics_y = global->detector[detID].nasics_y;
	float		*corrected_data = eventData->detector[detID].corrected_data;

	
	// Search subunits
	if(global->detector[detID].localBackgroundRadius <= 0 || global->detector[detID].localBackgroundRadius >= asic_ny/2 )
		return;
	long nn = (2*global->detector[detID].localBackgroundRadius+1);
	nn=nn*nn;
	
	
	// Create local arrays needed for background subtraction
	float	*localBg = (float*) calloc(pix_nn, sizeof(float)); 
	float	*buffer = (float*) calloc(nn, sizeof(float));
	
	
	
	// Loop over ASIC modules 
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			// Loop over pixels within a module
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					
					counter = 0;
					e = (j+mj*asic_ny)*pix_nx;
					e += i+mi*asic_nx;
					
					// Loop over median window
					for(long jj=-global->detector[detID].localBackgroundRadius; jj<=global->detector[detID].localBackgroundRadius; jj++){
						for(long ii=-global->detector[detID].localBackgroundRadius; ii<=global->detector[detID].localBackgroundRadius; ii++){
							
							// Quick array bounds check
							if((i+ii) < 0)
								continue;
							if((i+ii) >= asic_nx)
								continue;
							if((j+jj) < 0)
								continue;
							if((j+jj) >= asic_ny)
								continue;
							
							ee = (j+jj+mj*asic_ny)*pix_nx;
							ee += i+ii+mi*asic_nx;
							
							if(ee < 0 || ee >= pix_nn){
								printf("Error: Array bounds error: e = %li > %li\n",e,pix_nn);
								continue;
							}
							
							buffer[counter] = corrected_data[ee];
							counter++;
						}
					}
					
					// No elements -> trap an error
					if(counter == 0) {
						printf("Error: Local background counter == 0\n");
						localBg[e] = 0;
						continue;
					}
					if(counter > nn) {
						printf("Error: counter == %li > %li\n",counter,nn);
						continue;
					}
					
					// Find median value
					localBg[e] = kth_smallest(buffer, counter, counter/2);
				}
			}
		}
	}
	
	
	// Actually do the local background subtraction
	for(long i=0;i<pix_nn;i++) {
		corrected_data[i] -= localBg[i];	
	}	
	
	
	// Cleanup
	free(localBg);
	free(buffer);
}



/*
 * Make a saturated pixel mask
 */
void checkSaturatedPixels(cEventData *eventData, cGlobal *global, int detID){
	
	for(long i=0;i<global->detector[i].pix_nn;i++) { 
		if ( eventData->detector[detID].raw_data[i] >= global->detector[detID].pixelSaturationADC) 
			eventData->detector[detID].saturatedPixelMask[i] = 0;
		else
			eventData->detector[detID].saturatedPixelMask[i] = 1;
	}
	
}

