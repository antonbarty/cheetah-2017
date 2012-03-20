/*
 *  detectorCorrection.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 23/11/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ConfigV3.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad/CspadTemp.hh"
#include "cspad/CspadCorrector.hh"
#include "cspad/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorClass.h"
#include "setup.h"
#include "worker.h"
#include "median.h"



/*
 *	Subtract pre-loaded darkcal file
 */
void subtractDarkcal(cPixelDetectorEvent detectorEvent, cPixelDetectorCommon detectorGlobal){
	
	// Do darkcal subtraction
	for(long i=0;i< detectorGlobal.pix_nn;i++) {
		detectorEvent.corrected_data[i] -= detectorGlobal.darkcal[i]; 
	}
	
}


/*
 *	Apply gain correction
 *	Assumes the gaincal array is appropriately 'prepared' when loaded so that all we do is a multiplication.
 *	All that checking for division by zero (and inverting when required) needs only be done once, right? 
 */
void applyGainCorrection(cPixelDetectorEvent detectorEvent, cPixelDetectorCommon detectorGlobal){
	
	for(long i=0;i<detectorGlobal.pix_nn;i++) 
		detectorEvent.corrected_data[i] *= detectorGlobal.gaincal[i];
	
}


/*
 *	Apply bad pixel mask
 *	Assumes that all we have to do here is a multiplication.
 */
void applyBadPixelMask(cPixelDetectorEvent detectorEvent, cPixelDetectorCommon detectorGlobal){
	
	for(long i=0;i<detectorGlobal.pix_nn;i++) 
		detectorEvent.corrected_data[i] *= detectorGlobal.badpixelmask[i];
	
}



/*
 *	Subtract common mode on each module
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cmModuleSubtract(tEventData *eventData, cGlobal *global, int detID){
	
	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		e;
	long		mval;
    long		counter;
	float		median;
	
	// Dereference datector arrays
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	long		nasics_x = global->detector[detID].nasics_x;
	long		nasics_y = global->detector[detID].nasics_y;
	float		*corrected_data = eventData->detector[detID].corrected_data;
	
	
	// Create median buffer
	float	*buffer; 
	buffer = (float*) calloc(asic_nx*asic_ny, sizeof(float));
	
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			// Zero array
			for(long i=0; i<asic_nx*asic_ny; i++)
				buffer[i] = 0;
			
			// Loop over pixels within a module
            counter = 0;
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
                    if(global->detector[detID].badpixelmask[e] != 0) {           // badpixelmask[e]==0 are the bad pixels
						buffer[counter++] = corrected_data[e];
					}
				}
			}
			
			
            // Calculate background using median value 
			//median = kth_smallest(buffer, global->asic_nx*global->asic_ny, mval);
			if(counter>0) {
				mval = lrint(counter*global->detector[detID].cmFloor);
                if(mval < 0) 
                    mval = 1;
				median = kth_smallest(buffer, counter, mval);
			}
			else 
				median = 0;

			// Subtract median value
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					corrected_data[e] -= median;
				}
			}
		}
	}
	free(buffer);
}



/*
 *	Subtract offset estimated from unbonded pixels
 *	In the upstream detector, the unbonded pixels are in Q0:0-3 and Q2:4-5 and are at the 
 *	corners of each asic and at row=col (row<194) or row-194==col (row>194) for col%10=0.  
 */
void cmSubtractUnbondedPixels(tEventData *eventData, cGlobal *global, int detID){
	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		e;
	double		counter;
	double		background;
	
	// Dereference datector arrays
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	long		nasics_x = global->detector[detID].nasics_x;
	long		nasics_y = global->detector[detID].nasics_y;
	float		*corrected_data = eventData->detector[detID].corrected_data;

	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			// Only asics in Q0:0-3 and Q2:4-5 are unbonded
			if( ! ((mi<=1 && mj<=3) || (mi >= 4 && mi<=5 && mj >= 4 && mj<=5)) )
				continue;
			
			
			// Loop over unbonded pixels within each ASIC
			background = 0.0;
			counter = 0.0;
			for(long j=0; j<asic_ny-1; j+=10){
				long i=j;
				e = (j + mj*asic_ny) * (asic_nx*nasics_x);
				e += i + mi*asic_nx;
				background += corrected_data[e];
				counter += 1;
			}
			background /= counter;
			
			
			// Subtract background from entire ASIC
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					corrected_data[e] -= background;
					
				}
			}
		}
	}
	
}

/*
 *	Subtract common mode estimated from signal behind wires
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cmSubtractBehindWires(tEventData *eventData, cGlobal *global, int detID){
	
	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		p;
	long		counter;
	long		mval;
	float		median;
	
	// Dereference datector arrays
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	long		nasics_x = global->detector[detID].nasics_x;
	long		nasics_y = global->detector[detID].nasics_y;
	float		*corrected_data = eventData->detector[detID].corrected_data;
	
	float	*buffer; 
	buffer = (float*) calloc(asic_ny*asic_nx, sizeof(float));
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_x; mj++){
			
			
			// Loop over pixels within a module, remembering signal behind wires
			counter = 0;
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					p = (j + mj*asic_ny) * (asic_nx*nasics_x);
					p += i + mi*asic_nx;
					if(global->detector[detID].wiremask[i] && global->detector[detID].badpixelmask[p] != 0) {
						buffer[counter] = corrected_data[p];
						counter++;
					}
				}
			}
			
			// Median value of pixels behind wires
			if(counter>0) {
				mval = lrint(counter*global->detector[detID].cmFloor);
				median = kth_smallest(buffer, counter, mval);
			}
			else 
				median = 0;
			
			
			// Subtract median value
			for(long i=0; i<asic_nx; i++){
				for(long j=0; j<asic_ny; j++){
					p = (j + mj*asic_ny) * (asic_nx*nasics_x);
					p += i + mi*asic_nx;
					corrected_data[p] -= median;
				}
			}
		}
	}
	free(buffer);
}


/*
 *	Identify and kill hot pixels
 */
void killHotpixels(tEventData *eventData, cGlobal *global, int detID){
	
	
	// First update global hot pixel buffer
	int16_t	*buffer = (int16_t *) calloc(global->detector[detID].pix_nn,sizeof(int16_t));
	for(long i=0;i<global->detector[detID].pix_nn;i++){
		buffer[i] = (fabs(eventData->detector[detID].corrected_data[i])>global->detector[detID].hotpixADC)?(1):(0);
	}

    if(global->detector[detID].useBackgroundBufferMutex)
        pthread_mutex_lock(&global->hotpixel_mutex);
	
    long frameID = global->detector[detID].hotpixCounter%global->detector[detID].hotpixMemory;
	memcpy(global->detector[detID].hotpix_buffer+global->detector[detID].pix_nn*frameID, buffer, global->detector[detID].pix_nn*sizeof(int16_t));
	global->detector[detID].hotpixCounter += 1;

    if(global->detector[detID].useBackgroundBufferMutex)
        pthread_mutex_unlock(&global->hotpixel_mutex);
	free(buffer);
	
	
	// Apply the current hot pixel mask 
	for(long i=0;i<global->detector[detID].pix_nn;i++){
		eventData->detector[detID].corrected_data[i] *= global->detector[detID].hotpixelmask[i];
	}
	eventData->nHot = global->detector[detID].nhot;
	
	
	
}


void calculateHotPixelMask(cGlobal *global, int detID){
	
	long	cutoff = lrint((global->detector[detID].hotpixMemory*global->detector[detID].hotpixFreq));
	printf("Recalculating hot pixel mask at %li/%i\n",cutoff,global->detector[detID].hotpixMemory);	
    
    if(global->detector[detID].useBackgroundBufferMutex)
        pthread_mutex_lock(&global->hotpixel_mutex);
	
	// Loop over all pixels 
	long	counter;
	long	nhot;
	for(long i=0; i<global->detector[detID].pix_nn; i++) {
		
		counter = 0;
		for(long j=0; j< global->detector[detID].hotpixMemory; j++) {
			counter += global->detector[detID].hotpix_buffer[j*global->detector[detID].pix_nn+i]; 
		}
		
		// Apply threshold
		if(counter < cutoff) {
			global->detector[detID].hotpixelmask[i] = 1;
		}
		else{
			global->detector[detID].hotpixelmask[i] = 0;
			nhot++;				
		}		
	}	
	global->detector[detID].nhot = nhot;
	global->detector[detID].last_hotpix_update = global->detector[detID].hotpixCounter;
    
    if(global->detector[detID].useBackgroundBufferMutex)
        pthread_mutex_unlock(&global->hotpixel_mutex);

}




