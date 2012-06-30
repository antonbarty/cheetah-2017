/*
 *  detectorCorrection.cpp
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
#include "cheetahmodules.h"
#include "median.h"



/*
 *	Subtract pre-loaded darkcal file
 */
void subtractDarkcal(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detID].useDarkcalSubtraction) {
			long	pix_nn = global->detector[detID].pix_nn;
			float	*data = eventData->detector[detID].corrected_data;
			float	*darkcal = global->detector[detID].darkcal;

			subtractDarkcal(data, darkcal, pix_nn);
		}			
	}
}

void subtractDarkcal(float *data, float *darkcal, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] -= darkcal[i]; 
	}
}



/*
 *	Apply gain correction
 *	Assumes the gaincal array is appropriately 'prepared' when loaded so that all we do is a multiplication.
 *	All that checking for division by zero (and inverting when required) needs only be done once, right? 
 */
void applyGainCorrection(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detID].useGaincal) {
			long	pix_nn = global->detector[detID].pix_nn;
			float	*data = eventData->detector[detID].corrected_data;
			float	*gaincal = global->detector[detID].gaincal;
			
			applyGainCorrection(data, gaincal, pix_nn);
		}
	}
}

void applyGainCorrection(float *data, float *gaincal, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] *= gaincal[i]; 
	}
}



/*
 *	Apply bad pixel mask
 *	Assumes that all we have to do here is a multiplication.
 */

void applyBadPixelMask(cEventData *eventData, cGlobal *global){	
	
	DETECTOR_LOOP {
		if(global->detector[detID].useBadPixelMask) {
			long	pix_nn = global->detector[detID].pix_nn;
			float	*data = eventData->detector[detID].corrected_data;
			int16_t	*badpixelmask = global->detector[detID].badpixelmask;

			applyBadPixelMask(data, badpixelmask, pix_nn);
		}
	} 
}

void applyBadPixelMask(float *data, int16_t *badpixelmask, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] *= badpixelmask[i]; 
	}
}



/*
 *	Subtract common mode on each module
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cmModuleSubtract(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detID].cmModule) { 
		
			// Dereference datector arrays
			float		threshold = global->detector[detID].cmFloor;
			float		*data = eventData->detector[detID].corrected_data;
			int16_t		*mask = global->detector[detID].badpixelmask;
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;
			
			cmModuleSubtract(data, mask, threshold, asic_nx, asic_ny, nasics_x, nasics_y);
			
		}
	}
}

void cmModuleSubtract(float *data, int16_t *mask, float threshold, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		e;
	long		mval;
	long		counter;
	float		median;
					  
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
                    if(mask[e] != 0) {           // badpixelmask[e]==0 are the bad pixels
						buffer[counter++] = data[e];
					}
				}
			}
			
			
            // Calculate background using median value 
			//median = kth_smallest(buffer, global->asic_nx*global->asic_ny, mval);
			if(counter>0) {
				mval = lrint(counter*threshold);
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
					data[e] -= median;
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
void cmSubtractUnbondedPixels(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detID].cmSubtractUnbondedPixels) { 
			
			// Dereference datector arrays
			float		*data = eventData->detector[detID].corrected_data;
			int16_t		*mask = global->detector[detID].badpixelmask;
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;
			
			cmSubtractUnbondedPixels(data, mask, asic_nx, asic_ny, nasics_x, nasics_y);
			
		}
	}
}

void cmSubtractUnbondedPixels(float *data, int16_t *mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		e;
	double		counter;
	double		background;
	
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
				background += data[e];
				counter += 1;
			}
			background /= counter;
			
			
			// Subtract background from entire ASIC
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					data[e] -= background;
					
				}
			}
		}
	}
	
}


/*
 *	Subtract common mode estimated from signal behind wires
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cmSubtractBehindWires(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detID].cmSubtractBehindWires) {
			float		threshold = global->detector[detID].cmFloor;
			float		*data = eventData->detector[detID].corrected_data;
			int16_t		*mask = global->detector[detID].badpixelmask;
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;

			cmSubtractBehindWires(data, mask, threshold, asic_nx, asic_ny, nasics_x, nasics_y);

		}
	}
}		

void cmSubtractBehindWires(float *data, int16_t *mask, float threshold, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		p;
	long		counter;
	long		mval;
	float		median;
	
	// Create median buffer
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
					if(mask[i]) {
						buffer[counter] = data[p];
						counter++;
					}
				}
			}
			
			// Median value of pixels behind wires
			if(counter>0) {
				mval = lrint(counter*threshold);
				median = kth_smallest(buffer, counter, mval);
			}
			else 
				median = 0;
			
			
			// Subtract median value
			for(long i=0; i<asic_nx; i++){
				for(long j=0; j<asic_ny; j++){
					p = (j + mj*asic_ny) * (asic_nx*nasics_x);
					p += i + mi*asic_nx;
					data[p] -= median;
				}
			}
		}
	}
	free(buffer);
}



/*
 *	Identify and kill hot pixels
 */

void applyHotPixelMask(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
		if(global->detector[detID].useAutoHotpixel) {
			
			int		lockThreads = global->detector[detID].useBackgroundBufferMutex;
			long	pix_nn = global->detector[detID].pix_nn;
			long	hotpixADC = global->detector[detID].hotpixADC;
			long	bufferDepth = global->detector[detID].hotpixMemory;
			long	hotpixCounter = global->detector[detID].hotpixCounter;
			float	*frameData = eventData->detector[detID].corrected_data;
			int16_t	*frameBuffer = global->detector[detID].hotpix_buffer;
			int16_t	*mask = global->detector[detID].hotpixelmask;


			/*
			 *	First update global hot pixel buffer
			 */
			int16_t	*buffer = (int16_t *) calloc(pix_nn,sizeof(int16_t));
			for(long i=0; i<pix_nn; i++){
				buffer[i] = (fabs(frameData[i])>hotpixADC)?(1):(0);
			}
			
			if(lockThreads)
				pthread_mutex_lock(&global->hotpixel_mutex);
			
			global->detector[detID].hotpixCounter += 1;
			long frameID = hotpixCounter%bufferDepth;
			
			memcpy(frameBuffer+pix_nn*frameID, buffer, pix_nn*sizeof(int16_t));
			free(buffer);
			
			if(lockThreads)
				pthread_mutex_unlock(&global->hotpixel_mutex);
			
			
			/*
			 *	Then apply the current hot pixel mask 
			 */
			for(long i=0; i<pix_nn; i++){
				frameData[i] *= mask[i];
			}
			eventData->nHot = global->detector[detID].nhot;


		}

	}	
	
}


/* 
 *	Recalculate hot pixel masks using frame buffer
 */
void calculateHotPixelMask(cGlobal *global){

	
	DETECTOR_LOOP {
        if(global->detector[detID].useAutoHotpixel) {
			float	hotpixFrequency = global->detector[detID].hotpixFreq;
			long	bufferDepth = global->detector[detID].hotpixMemory;
			long	hotpixCounter = global->detector[detID].hotpixCounter;
			long	hotpixRecalc = global->detector[detID].hotpixRecalc;
			long	lastUpdate = global->detector[detID].last_hotpix_update;
			
			
            if( ( (hotpixCounter % hotpixRecalc) == 0 || hotpixCounter == bufferDepth) && hotpixCounter != lastUpdate ) {
				
				global->detector[detID].last_hotpix_update = hotpixCounter;
				
				long	nhot;
				int		lockThreads = global->detector[detID].useBackgroundBufferMutex;
				long	threshold = lrint(bufferDepth*hotpixFrequency);
				long	pix_nn = global->detector[detID].pix_nn;
				int16_t	*mask = global->detector[detID].hotpixelmask;
				int16_t	*frameBuffer = global->detector[detID].hotpix_buffer;
				

				if(lockThreads)
					pthread_mutex_lock(&global->hotpixel_mutex);

				printf("Recalculating detector %li hot pixel mask at %li/%i\n",detID, threshold, bufferDepth);	
				nhot = calculateHotPixelMask(mask, frameBuffer, threshold, bufferDepth, pix_nn);

                if(lockThreads)
					pthread_mutex_unlock(&global->hotpixel_mutex);

				global->detector[detID].nhot = nhot;
			}
        }	
	}
}


long calculateHotPixelMask(int16_t *mask, int16_t *frameBuffer, long threshold, long bufferDepth, long pix_nn){

	// Loop over all pixels 
	long	counter;
	long	nhot;
	for(long i=0; i<pix_nn; i++) {
		
		counter = 0;
		for(long j=0; j< bufferDepth; j++) {
			counter += frameBuffer[j*pix_nn+i]; 
		}
		
		// Apply threshold
		if(counter < threshold) {
			mask[i] = 1;
		}
		else {
			mask[i] = 0;
			nhot++;				
		}		
	}	
    
	return nhot;

}




