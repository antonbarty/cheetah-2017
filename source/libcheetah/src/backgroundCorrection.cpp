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


void initPhotonCorrection(cEventData *eventData, cGlobal *global){
	// Copy detector corrected data into photon corrected array as starting point for photon corrections
	DETECTOR_LOOP {
		DEBUG3("Initialise photon corrected data with detector corrected data. (detectorID=%ld)",global->detector[detIndex].detectorID);										
		for(long i=0;i<global->detector[detIndex].pix_nn;i++){
			eventData->detector[detIndex].data_detPhotCorr[i] = eventData->detector[detIndex].data_detCorr[i];
		}
	}
}



/*
 *	Subtract persistent background 
 */
void subtractPersistentBackground(cEventData *eventData, cGlobal *global){
	float	*frameData;
	DETECTOR_LOOP {
		if(global->detector[detIndex].useSubtractPersistentBackground) {
			DEBUG3("Subtract persistent background. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			int	scaleBg = global->detector[detIndex].scaleBackground;
			long	pix_nn = global->detector[detIndex].pix_nn;
			float	*background = global->detector[detIndex].persistentBackground;
			// Running background subtraction to suppress the photon background after dark subtraction
			if (eventData->detector[detIndex].pedSubtracted && global->detector[detIndex].useDarkcalSubtraction) {
				frameData = eventData->detector[detIndex].data_detPhotCorr;
			// Running background subtraction to suppress both electronic and photon background (without using dark subtraction)
			} else if (!eventData->detector[detIndex].pedSubtracted && !global->detector[detIndex].useDarkcalSubtraction) {
				frameData = eventData->detector[detIndex].data_detCorr;
				eventData->detector[detIndex].pedSubtracted = 1;
			}
			subtractPersistentBackground(frameData, background, scaleBg, pix_nn);
		}
	}	
}


/*
 *	Subtract pre-calculated persistent background value
 */
void subtractPersistentBackground(float *data, float *background, int scaleBg, long pix_nn) {
	
	float	top = 0;
	float	s1 = 0;
	float	s2 = 0;
	float	v1, v2;
	float	factor;
	
	
	/*
	 *	Find appropriate scaling factor to match background with current image
	 *	Use with care: this assumes background vector is orthogonal to the image vector (which is often not true)
	 */
	factor = 1;
	if(scaleBg) {
		for(long i=0; i<pix_nn; i++){
			v1 = background[i];
			v2 = data[i];
			
			// Simple inner product gives cos(theta), which is always less than zero
			// Want ( (a.b)/|b| ) * (b/|b|)
			top += v1*v2;
			s1 += v1*v1;
			s2 += v2*v2;
		}
		factor = top/s1;
	}
	// Do the weighted subtraction
	for(long i=0; i<pix_nn; i++) {
		data[i] -= (factor*background[i]);	
	}	
}


void calculatePersistentBackground(cEventData *eventData, cGlobal *global){
	DETECTOR_LOOP {
		if(global->detector[detIndex].useSubtractPersistentBackground){
			/*
			 *	Recalculate background from time to time
			 */
			DEBUG3("Check wheter or not we need to calculate a persistent background from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long	pix_nn = global->detector[detIndex].pix_nn;
			int	lockThreads = global->detector[detIndex].useBackgroundBufferMutex;
			long	bufferDepth = global->detector[detIndex].bgMemory;
			long	bgRecalc = global->detector[detIndex].bgRecalc;
			long	bgMemory = global->detector[detIndex].bgMemory;
			float	medianPoint = global->detector[detIndex].bgMedian;
			long	threshold = lrint(bufferDepth*medianPoint);
			float   *background = global->detector[detIndex].persistentBackground;

			if(lockThreads)	pthread_mutex_lock(&global->detector[detIndex].bgCounter_mutex);
			long lastUpdate = global->detector[detIndex].bgLastUpdate;

			if( (eventData->threadNum == bgRecalc+lastUpdate) || ((eventData->threadNum == (bgMemory-1)) && (lastUpdate == 0)) ) {

				global->detector[detIndex].bgLastUpdate = eventData->threadNum;
				if(lockThreads)	pthread_mutex_unlock(&global->detector[detIndex].bgCounter_mutex);

				DEBUG3("Actually calculate a persistent background from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);										

				printf("Detector %li: Start calculation of persistent background.\n",detIndex);

				// Copy frame buffer
				int16_t   *frameBuffer = (int16_t *) calloc(pix_nn*bufferDepth,sizeof(int16_t));
				long k;
				for(long i = 0;i<bufferDepth;i++){
					if(lockThreads) pthread_mutex_lock(&global->detector[detIndex].bg_mutexes[i]);
					k = pix_nn*i;
					for(long j = 0;j<pix_nn;j++){
						frameBuffer[k+j] = global->detector[detIndex].bg_buffer[k+j];
					}
					if(lockThreads) pthread_mutex_unlock(&global->detector[detIndex].bg_mutexes[i]);
				}
				
				// Calculate persistent background
				if(lockThreads) pthread_mutex_lock(&global->detector[detIndex].persistentBackground_mutex);
				calculatePersistentBackground(background, frameBuffer, threshold, bufferDepth, pix_nn);
				if(lockThreads) pthread_mutex_unlock(&global->detector[detIndex].persistentBackground_mutex);
				global->detector[detIndex].bgCalibrated = 1;
				printf("Detector %li: Persistent background calculated.\n",detIndex);      
		   
				free(frameBuffer);			
			} else {
				if(lockThreads)	pthread_mutex_unlock(&global->detector[detIndex].bgCounter_mutex);			
			}
		}
	}
}	


/*
 *  Set background to the first frame by default
 *  (possibly not needed)
 */
void initBackgroundBuffer(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detIndex].useSubtractPersistentBackground && ((eventData->detector[detIndex].pedSubtracted && global->detector[detIndex].useDarkcalSubtraction) || (!eventData->detector[detIndex].pedSubtracted && !global->detector[detIndex].useDarkcalSubtraction))){
			if (global->detector[detIndex].useSubtractPersistentBackground && global->detector[detIndex].bgCounter == 0){
				DEBUG3("Initialise the persistent background from the ringbuffer with the data of the first frame. (detectorID=%ld)",global->detector[detIndex].detectorID);										

				long	pix_nn = global->detector[detIndex].pix_nn;
				float	*background = global->detector[detIndex].persistentBackground;
                
				pthread_mutex_lock(&global->detector[detIndex].bg_mutexes[0]);
				if (global->detector[detIndex].useDarkcalSubtraction){
					for(long i = 0;i<pix_nn;i++){
						background[i] = eventData->detector[detIndex].data_detCorr[i];
					}
				} else {
					for(long i = 0;i<pix_nn;i++){
						background[i] = (float) eventData->detector[detIndex].data_raw16[i];
					}
				}
				pthread_mutex_unlock(&global->detector[detIndex].bg_mutexes[0]);
			}
		}
	}
}


/*
 *	Update background buffer
 */
void updateBackgroundBuffer(cEventData *eventData, cGlobal *global, int hit) {
	
	DETECTOR_LOOP {
		if (global->detector[detIndex].useSubtractPersistentBackground && (hit==0 || global->detector[detIndex].bgIncludeHits)) {
			DEBUG3("Add a new frame to the persistent background buffer. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long	bufferDepth = global->detector[detIndex].bgMemory;
			long	pix_nn = global->detector[detIndex].pix_nn;
			int16_t	*frameBuffer = global->detector[detIndex].bg_buffer;
			long frameID = eventData->threadNum%bufferDepth;
			pthread_mutex_lock(&global->detector[detIndex].bg_mutexes[frameID]);
			if (global->detector[detIndex].useDarkcalSubtraction){
				for(long i = 0;i<pix_nn;i++){
					frameBuffer[i+pix_nn*frameID] = (int16_t) lrint(eventData->detector[detIndex].data_detCorr[i]);
				}
			} else {
				for(long i = 0;i<pix_nn;i++){
					frameBuffer[i+pix_nn*frameID] = eventData->detector[detIndex].data_raw16[i];
				}
			}
			pthread_mutex_unlock(&global->detector[detIndex].bg_mutexes[frameID]);
#ifdef __GNUC__
			__sync_fetch_and_add(&(global->detector[detIndex].bgCounter),1);
#else
			int lockThreads = global->detector[detIndex].useBackgroundBufferMutex;
			if(lockThreads){pthread_mutex_lock(&global->detector[detIndex].bgCounter_mutex);}
			global->detector[detIndex].bgCounter += 1;
			if(lockThreads){pthread_mutex_unlock(&global->detector[detIndex].bgCounter_mutex);}
#endif
		}		
	}
}
		   


/*
 *	Calculate persistent background from stack of remembered frames
 */
void calculatePersistentBackground(float *background, int16_t *frameBuffer, long threshold, long bufferDepth, long pix_nn) {

	// Buffer for median calculation
	int16_t	*buffer = (int16_t*) calloc(bufferDepth, sizeof(int16_t));
		
	// Loop over all pixels 
	for(long i=0; i<pix_nn; i++) {
		
		// Create a local array for sorting
		for(long j=0; j< bufferDepth; j++) {
			buffer[j] = frameBuffer[j*pix_nn+i];
		}
		
		// Find median value of the temporary array
		background[i] = (float) kth_smallest(buffer, bufferDepth, threshold);
	}
	free (buffer);
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


/*
 * Make a saturated pixel mask
 */
void checkSaturatedPixels(uint16_t *data_raw16, uint16_t *mask, long pix_nn, long pixelSaturationADC) {
	for(long i=0; i<pix_nn; i++) { 
		if ( data_raw16[i] >= pixelSaturationADC)
			mask[i] |= PIXEL_IS_SATURATED;
		else
			mask[i] &= ~PIXEL_IS_SATURATED;
	}
}

void checkSaturatedPixelsPnccd(uint16_t *data_raw16, uint16_t *mask){
	long i,x,y,mx,my,q;
	long asic_nx = PNCCD_ASIC_NX;
	long asic_ny = PNCCD_ASIC_NY;
	long nasics_x = PNCCD_nASICS_X;
	long nasics_y = PNCCD_nASICS_Y;
	uint16_t saturation_threshold[4] = {8500,5600,10000,5600};
	// Loop over quadrants
	for(my=0; my<nasics_y; my++){
		for(mx=0; mx<nasics_x; mx++){
			q = mx+my*nasics_x;
			for(y=0; y<asic_ny; y++){
				for(x=0; x<asic_nx; x++){
					i = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + mx*asic_nx + x;
					if (data_raw16[i] > saturation_threshold[q]){
						mask[i] |= PIXEL_IS_SATURATED; 
					}
				}
			}
		}
	}
}

void checkSaturatedPixels(cEventData *eventData, cGlobal *global){
	DETECTOR_LOOP {
		if (global->detector[detIndex].maskSaturatedPixels) {
			uint16_t	*raw_data = eventData->detector[detIndex].data_raw16;
			uint16_t	*mask = eventData->detector[detIndex].pixelmask;
			if (strcmp(global->detector[detIndex].detectorType, "pnccd") == 0) {
				DEBUG3("Check for saturated pixels (PNCCD). (detectorID=%ld)",global->detector[detIndex].detectorID);										
				checkSaturatedPixelsPnccd(raw_data,mask);
			} else {
				DEBUG3("Check for saturated pixels (other than PNCCD). (detectorID=%ld)",global->detector[detIndex].detectorID);										
				long		nn = global->detector[detIndex].pix_nn;
				long		pixelSaturationADC = global->detector[detIndex].pixelSaturationADC;			
				checkSaturatedPixels(raw_data, mask, nn, pixelSaturationADC);
			}
		}
	}
}	






void updateHaloBuffer(cEventData *eventData, cGlobal *global,int hit){
	DETECTOR_LOOP{
		/* FOR TESTING
		   printf("updateHaloBuffer\n");
		   printf("global->detector[%i].useAutoHalopixel=%i\n",detIndex,global->detector[detIndex].useAutoHalopixel);
		   printf("hit=%i\n",hit);
		   printf("global->detector[%i].halopixIncludeHits=%i\n",detIndex,global->detector[detIndex].halopixIncludeHits);
		   printf("global->detector[%i].useSubtractPersistentBackground=%i\n",detIndex,global->detector[detIndex].useSubtractPersistentBackground);
		   printf("global->detector[%i].bgCalibrated=%i\n",detIndex,global->detector[detIndex].bgCalibrated);
		*/
		if(global->detector[detIndex].useAutoHalopixel && (!hit || global->detector[detIndex].halopixIncludeHits) && (!global->detector[detIndex].useSubtractPersistentBackground || global->detector[detIndex].bgCalibrated)){
			DEBUG3("Add frame to halo buffer. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			float	*frameData = eventData->detector[detIndex].data_detCorr;
			float     *frameBuffer = global->detector[detIndex].halopix_buffer;
			long	pix_nn = global->detector[detIndex].pix_nn;
			long	bufferDepth = global->detector[detIndex].halopixMemory;
			long	frameID = eventData->threadNum%bufferDepth;

			//puts("Update halo buffer");
			float	*buffer = (float *) malloc(pix_nn*sizeof(float));
			for(long i=0; i<pix_nn; i++){
				buffer[i] = fabs(frameData[i]);
			}

			// Update buffer slice
			pthread_mutex_lock(&global->detector[detIndex].halopix_mutexes[frameID]);
			memcpy(frameBuffer+pix_nn*frameID, buffer, pix_nn*sizeof(float));
			pthread_mutex_unlock(&global->detector[detIndex].halopix_mutexes[frameID]);
			// Update counter
#ifdef __GNUC__
			__sync_fetch_and_add(&(global->detector[detIndex].halopixCounter),1);
#else
			pthread_mutex_lock(&global->detector[detIndex].halopixCounter_mutex);
			global->detector[detIndex].halopixCounter += 1;
			pthread_mutex_unlock(&global->detector[detIndex].halopixCounter_mutex);
#endif      
			free(buffer);
		}
	}
}
     
/* 
 *	Recalculate halo pixel masks using frame buffer
 */
void calculateHaloPixelMask(cEventData *eventData,cGlobal *global){
	DETECTOR_LOOP {
		if(global->detector[detIndex].useAutoHalopixel) {
			DEBUG3("Check wheter or not we need to calculate a new halo pixel mask yet. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			float	halopixMinDeviation = global->detector[detIndex].halopixMinDeviation;
			long	bufferDepth = global->detector[detIndex].halopixMemory;
			long	halopixRecalc = global->detector[detIndex].halopixRecalc;
			long	halopixMemory = global->detector[detIndex].halopixMemory;
			long	halopixCounter = global->detector[detIndex].halopixCounter;
			int	halopixCalibrated = global->detector[detIndex].halopixCalibrated;
			long	nhalo;
			float	threshold = bufferDepth*halopixMinDeviation;
			long	pix_nn = global->detector[detIndex].pix_nn;
			uint16_t  *mask = global->detector[detIndex].pixelmask_shared;
			uint16_t  *maskMinExtent = global->detector[detIndex].pixelmask_shared_min;
			uint16_t  *maskMaxExtent = global->detector[detIndex].pixelmask_shared_max;
			//pthread_mutex_t maskMutex = global->detector[detIndex].pixelmask_shared_mutex;
			//pthread_mutex_t maskMinMutex = global->detector[detIndex].pixelmask_shared_min_mutex;
			//pthread_mutex_t maskMaxMutex = global->detector[detIndex].pixelmask_shared_max_mutex;

			pthread_mutex_lock(&global->detector[detIndex].halopixCounter_mutex);
			long	lastUpdate = global->detector[detIndex].halopixLastUpdate;

			// here the condition (eventData->threadNum%50 == 0) made multiple initial calibrations unlikely
			if( (eventData->threadNum == halopixRecalc+lastUpdate && halopixCalibrated) || ( (halopixCounter >= halopixMemory)  /* && (eventData->threadNum%50 == 0) */ && !halopixCalibrated) ) { 
				DEBUG3("Actually calculate a new halo pixel mask now. (detectorID=%ld)",global->detector[detIndex].detectorID);										
				global->detector[detIndex].halopixLastUpdate = eventData->threadNum;				
				pthread_mutex_unlock(&global->detector[detIndex].halopixCounter_mutex);

				pthread_mutex_lock(&global->detector[detIndex].pixelmask_shared_mutex);					
				printf("Detector %li: Calculating halo pixel mask.\n",detIndex);
				nhalo = calculateHaloPixelMask(mask,maskMinExtent,maskMaxExtent,global->detector[detIndex].halopix_buffer,threshold, bufferDepth, pix_nn);//, maskMinMutex, maskMaxMutex);
				if (nhalo == pix_nn){
					printf("Warning: Detector %li: All pixels are in halo pixel mask.\n",detIndex);
				}
				global->detector[detIndex].nhalo = nhalo;
				global->detector[detIndex].halopixCalibrated = 1;
				printf("Detector %li: Identified %li halo pixels.\n",detIndex,nhalo);	
				pthread_mutex_unlock(&global->detector[detIndex].pixelmask_shared_mutex);					


			} else {
				pthread_mutex_unlock(&global->detector[detIndex].halopixCounter_mutex);

			}

		}	
	}
}
  

long calculateHaloPixelMask(uint16_t *mask, uint16_t *maskMinExtent, uint16_t *maskMaxExtent, float *frameBuffer, float threshold, long bufferDepth, long pix_nn) { //,  pthread_mutex_t maskMin_mutex, pthread_mutex_t maskMax_mutex){

	// Loop over all pixels 
	float sum;
	long	nhalo = 0;

	bool* buffer = (bool *) calloc(pix_nn,sizeof(bool));

	for(long i=0; i<pix_nn; i++) {
		sum = 0.;
		for(long j=0; j< bufferDepth; j++) {
			sum += frameBuffer[j*pix_nn+i]; 
		}
		if(sum >= threshold) {
			buffer[i] = true;
			nhalo += 1;
		} else {
			buffer[i] = false;
		}
	}

#ifndef __GNUC__
	//pthread_mutex_lock(&mask_min_mutex);
	//pthread_mutex_lock(&mask_max_mutex);
#endif
	for(long i=0; i<pix_nn; i++) {
		if (buffer[i]){
#ifdef __GNUC__
			__sync_fetch_and_or(&(mask[i]),PIXEL_IS_IN_HALO);
			__sync_fetch_and_or(&(maskMaxExtent[i]),PIXEL_IS_IN_HALO);
#else
			mask[i] |= PIXEL_IS_IN_HALO; 
			maskMaxExtent[i] |= PIXEL_IS_IN_HALO;
#endif
		} else {
#ifdef __GNUC__
			__sync_fetch_and_and(&(mask[i]),~PIXEL_IS_IN_HALO);
			__sync_fetch_and_and(&(maskMinExtent[i]),~PIXEL_IS_IN_HALO);
#else
			mask[i] &= ~PIXEL_IS_IN_HALO; 
			maskMinExtent[i] &= ~PIXEL_IS_IN_HALO;
#endif
		}
	}
#ifndef __GNUC__
    //pthread_mutex_unlock(&mask_min_mutex);
    //pthread_mutex_unlock(&mask_max_mutex);
#endif

	return nhalo;

}


			
