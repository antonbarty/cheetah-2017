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
#include "cheetahmodules.h"
#include "median.h"

/*
 *	Subtract persistent background 
 */
void subtractPersistentBackground(cEventData *eventData, cGlobal *global){
	float	*frameData;
	DETECTOR_LOOP {
		if(global->detector[detIndex].useSubtractPersistentBackground) {
			DEBUG3("Subtract persistent background. (detectorID=%ld)",global->detector[detIndex].detectorID);
			// Running background subtraction to suppress the photon background after dark subtraction
			if (eventData->detector[detIndex].pedSubtracted && global->detector[detIndex].useDarkcalSubtraction) {
				frameData = eventData->detector[detIndex].data_detPhotCorr;
			// Running background subtraction to suppress both electronic and photon background (without using dark subtraction)
			} else if (!eventData->detector[detIndex].pedSubtracted && !global->detector[detIndex].useDarkcalSubtraction) {
				frameData = eventData->detector[detIndex].data_detCorr;
				eventData->detector[detIndex].pedSubtracted = 1;
			}
			// Subtract median
			global->detector[detIndex].frameBufferBlanks->subtractMedian(frameData,global->detector[detIndex].scaleBackground,global->detector[detIndex].subtractPersistentBackgroundMinAbsMedianOverStdRatio);
		}
	}	
}

/*
 *	Update background buffer
 */
void updateBackgroundBuffer(cEventData *eventData, cGlobal *global, int hit) {
	
	DETECTOR_LOOP {
		if (global->detector[detIndex].useSubtractPersistentBackground && (hit==0 || global->detector[detIndex].bgIncludeHits)) {
			long	recalc = global->detector[detIndex].bgRecalc;
			long	memory = global->detector[detIndex].bgMemory;
			float	medianPoint = global->detector[detIndex].bgMedian;

			// Select data type to add
			float* data;
			if (global->detector[detIndex].useDarkcalSubtraction){
				data = eventData->detector[detIndex].data_detCorr;
			} else {
				data = eventData->detector[detIndex].data_raw;
			}
			// Adding frame to buffer
			DEBUG3("Add a new frame to the persistent background buffer. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long counter = global->detector[detIndex].frameBufferBlanks->writeNextFrame(data);
			long bufferDepth = global->detector[detIndex].frameBufferBlanks->depth;
			if (counter < bufferDepth)
				printf("Calibrating persistent background: Ring buffer fill status %li/%li.\n",counter+1,bufferDepth);
			
			// Do we have to update the persistent background (median from the buffer)
			DEBUG3("Check wheter or not we need to calculate a persistent background from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			pthread_mutex_lock(&global->detector[detIndex].bg_update_mutex);
			long lastUpdate = global->detector[detIndex].bgLastUpdate;
			
			if( /* has processed recalc events since last update?  */ ((eventData->threadNum == lastUpdate+recalc) && (lastUpdate != 0)) || 
				/* uninitialised and buffer filled? */ ((counter >= memory) && (lastUpdate == 0)) ) {

				bool  keepThreadsLocked = global->detector[detIndex].bgCalibrated || global->threadSafetyLevel < 1;
				global->detector[detIndex].bgLastUpdate = eventData->threadNum;

				// Keep the lock during calculation of median either
				// - if we run at high thread safety level or
				// - if we are not calibrated yet (we do not want to loose frames unnecessarily during calibration)
				if(!keepThreadsLocked) pthread_mutex_unlock(&global->detector[detIndex].bg_update_mutex);

				DEBUG3("Actually calculate a persistent background from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);
				printf("Detector %li: Start calculation of persistent background.\n",detIndex);			
				
				global->detector[detIndex].frameBufferBlanks->updateMedian(medianPoint);
				
				printf("Detector %li: Persistent background calculated.\n",detIndex);      
				global->detector[detIndex].bgCalibrated = 1;

				if(keepThreadsLocked)	pthread_mutex_unlock(&global->detector[detIndex].bg_update_mutex);		   

			} else {
				pthread_mutex_unlock(&global->detector[detIndex].bg_update_mutex);			
			}
		}		
	}
}

/*
 *	Radial average background subtraction
 */
void subtractRadialBackground(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detIndex].useRadialBackgroundSubtraction) {
			DEBUG3("Subtract radial background. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long		pix_nn = global->detector[detIndex].pix_nn;
			float		*pix_r = global->detector[detIndex].pix_r;
			float		*data = eventData->detector[detIndex].data_detPhotCorr;
			float		sigmaThresh = 5;
			
			//	Masks for bad regions  (mask=0 to ignore regions)
			char		*mask = (char*) calloc(pix_nn, sizeof(char));
			uint16_t	combined_pixel_options = PIXEL_IS_IN_PEAKMASK|PIXEL_IS_BAD|PIXEL_IS_HOT|PIXEL_IS_BAD|PIXEL_IS_SATURATED;
			for(long i=0;i<pix_nn; i++)
				mask[i] = isNoneOfBitOptionsSet(eventData->detector[detIndex].pixelmask[i], combined_pixel_options);
			
			subtractRadialBackground(data, pix_r, mask, pix_nn, sigmaThresh);
			
			free(mask);
		}
	}
}


void subtractRadialBackground(float *data, float *pix_r, char *mask, long pix_nn, float sigmaThresh) {
	
	
	/*
	 *	Determine noise and offset as a funciton of radius
	 *	Be more sophisticated than a simple radial average:
	 *	Exclude things that look like they might be peaks from the background calculations (pixels > 5 sigma)
	 *	Code copied from peakfinder8 where it was originally tested
	 */
	float	fminr, fmaxr;
	long	lminr, lmaxr;
	fminr = 1e9;
	fmaxr = -1e9;
	
	// Figure out radius bounds
	for(long i=0;i<pix_nn;i++){
		if (pix_r[i] > fmaxr)
			fmaxr = pix_r[i];
		if (pix_r[i] < fminr)
			fminr = pix_r[i];
	}
	lmaxr = (long)ceil(fmaxr)+1;
	lminr = 0;
	
	// Allocate and zero arrays
	float	*rsigma = (float*) calloc(lmaxr, sizeof(float));
	float	*roffset = (float*) calloc(lmaxr, sizeof(float));
	long	*rcount = (long*) calloc(lmaxr, sizeof(long));
	float	*rthreshold = (float*) calloc(lmaxr, sizeof(float));
	for(long i=0; i<lmaxr; i++) {
		rthreshold[i] = 1e9;
	}
	
	// Compute sigma and average of data values at each radius
	// From this, compute the ADC threshold to be applied at each radius
	// Iterate a few times to reduce the effect of positive outliers (ie: peaks)
	long	thisr;
	float	thisoffset, thissigma;
	for(long counter=0; counter<5; counter++) {
		for(long i=0; i<lmaxr; i++) {
			roffset[i] = 0;
			rsigma[i] = 0;
			rcount[i] = 0;
		}
		for(long i=0;i<pix_nn;i++){
			if(mask[i] != 0) {
				thisr = lrint(pix_r[i]);
				if(data[i] < rthreshold[thisr]) {
					roffset[thisr] += data[i];
					rsigma[thisr] += (data[i]*data[i]);
					rcount[thisr] += 1;
				}
			}
		}
		for(long i=0; i<lmaxr; i++) {
			if(rcount[i] == 0) {
				roffset[i] = 0;
				rsigma[i] = 0;
				rthreshold[i] = 1e9;
				//rthreshold[i] = ADCthresh;		// For testing
			}
			else {
				thisoffset = roffset[i]/rcount[i];
				thissigma = sqrt(rsigma[i]/rcount[i] - ((roffset[i]/rcount[i])*(roffset[i]/rcount[i])));
				roffset[i] = thisoffset;
				rsigma[i] = thissigma;
				rthreshold[i] = roffset[i] + sigmaThresh*rsigma[i];
				//rthreshold[i] = ADCthresh;		// For testing
			}
		}
	}

	
	/*
	 *	Now do the actual background subtraction
	 *	(a trivial operation once we know the radial background profile)
	 */
	for(long i=0; i<pix_nn; i++) {
		thisr = lrint(pix_r[i]);
		data[i] -= roffset[thisr];
	}
	
	
	free(roffset);
	free(rsigma);
	free(rcount);
	free(rthreshold);

}



/*
 *	Local background subtraction
 */
void subtractLocalBackground(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detIndex].useLocalBackgroundSubtraction) {
			DEBUG3("Subtract local background. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long		asic_nx = global->detector[detIndex].asic_nx;
			long		asic_ny = global->detector[detIndex].asic_ny;
			long		nasics_x = global->detector[detIndex].nasics_x;
			long		nasics_y = global->detector[detIndex].nasics_y;
			long		radius = global->detector[detIndex].localBackgroundRadius;
			float		*data = eventData->detector[detIndex].data_detCorr;
			
			subtractLocalBackground(data, radius, asic_nx, asic_ny, nasics_x, nasics_y);
		}
	}
	
}


void subtractLocalBackground(float *data, long radius, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	// Tank for silly radius values
	if(radius <= 0 || radius >= asic_ny/2 )
		return;
	
	// Raw data array size can be calculated from asic modules
	long	pix_nx = asic_nx*nasics_x;
	long	pix_ny = asic_ny*nasics_y;
	long	pix_nn = pix_nx*pix_ny;
	long	asic_nn = asic_nx*asic_ny;
	
	
	// Median window size
	long nn = (2*radius+1);
	nn=nn*nn;
	
	float	*asic_buffer = (float*) calloc(asic_nn, sizeof(float));
	float	*median_buffer = (float*) calloc(nn, sizeof(float));
	float	*localBg = (float*) calloc(pix_nn, sizeof(float)); 
	
	
	/*
	 *	Determine local background
	 *	(median over window width either side of current pixel)
	 */
	long		e = 0;
	long        ee;
	long		counter;
	
	
	// Loop over ASIC modules 
	for(long mj=0; mj<nasics_y; mj++){
		for(long mi=0; mi<nasics_x; mi++){
			
			// Extract buffer of ASIC values (small array is cache friendly)
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					
					// Element in ASIC buffer
					e = i+j*asic_nx;
					
					// Element in data array
					ee = (j+mj*asic_ny)*pix_nx;
					ee += i+mi*asic_nx;
					
					asic_buffer[e] = data[ee];
				}
			}
			
			// Loop over pixels within a module
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					
					counter = 0;
					
					// Loop over median window
					for(long jj=-radius; jj<=radius; jj++){
						for(long ii=-radius; ii<=radius; ii++){
							
							// Quick array bounds check
							if((i+ii) < 0)
								continue;
							if((i+ii) >= asic_nx)
								continue;
							if((j+jj) < 0)
								continue;
							if((j+jj) >= asic_ny)
								continue;
							
							// Element in asic buffer
							e = (j+jj)*asic_nx;
							e += i+ii;
							if(e < 0 || e >= asic_nn){
								printf("Error: Array bounds error: e = %li > %li\n",e,asic_nn);
								continue;
							}
							median_buffer[counter] = asic_buffer[e];
							
							counter++;
						}
					}
					
					// Element in data array
					ee = (j+mj*asic_ny)*pix_nx;
					ee += i+mi*asic_nx;

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
					localBg[ee] = kth_smallest(median_buffer, counter, counter/2);
				}
			}
		}
	}
	
	
	/*
	 *	Do the background subtraction
	 */
	for(long i=0;i<pix_nn;i++) {
		data[i] -= localBg[i];	
	}	
	
	
	// Cleanup
	free(localBg);
	free(asic_buffer);
	free(median_buffer);
}


