/*
 *  pixelmask.cpp
 *  cheetah
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"


void initPixelmask(cEventData *eventData, cGlobal *global){
	// Copy pixelmask_shared into pixelmask as a starting point for masking
	int threadSafetyLevel = global->threadSafetyLevel;
	DETECTOR_LOOP {
		DEBUG3("Initializing pixelmask with shared pixelmask. (detectorID=%ld)",global->detector[detIndex].detectorID);
		if (threadSafetyLevel > 1) pthread_mutex_lock(&global->detector[detIndex].pixelmask_shared_mutex);					
		memcpy(eventData->detector[detIndex].pixelmask,global->detector[detIndex].pixelmask_shared,global->detector[detIndex].pix_nn*sizeof(uint16_t));
		if (threadSafetyLevel > 1) pthread_mutex_unlock(&global->detector[detIndex].pixelmask_shared_mutex);
	}
}

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
			if ((strcmp(global->detector[detIndex].detectorType, "pnccd") == 0) && (global->detector[detIndex].maskPnccdSaturatedPixels)) {
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


/*
 *	Update noisy pixel buffer
 */
void updateNoisyPixelBuffer(cEventData *eventData, cGlobal *global, int hit) {
	int threadSafetyLevel = global->threadSafetyLevel;
	DETECTOR_LOOP {
		if (global->detector[detIndex].useAutoHotPixel && (hit == 0 || global->detector[detIndex].noisyPixIncludeHits)) {
			long	recalc = global->detector[detIndex].noisyPixRecalc;
			long	memory = global->detector[detIndex].noisyPixMemory;
			cFrameBuffer * frameBuffer = global->detector[detIndex].frameBufferNoisyPix;
			long bufferDepth = frameBuffer->depth;

			// Select data
			float * data = eventData->detector[detIndex].data_detCorr;
			// Adding frame to buffer
			DEBUG3("Add a new frame to the noisy pixel frame buffer. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long counter = frameBuffer->writeNextFrame(data);
			if (counter < bufferDepth)
				printf("Calibrating noisy pixel map: Ring buffer fill status %li/%li.\n",counter+1,bufferDepth);
			
			// Do we have to update?
			DEBUG3("Check wheter or not we need to calculate a new noisy pixel map from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			pthread_mutex_lock(&global->detector[detIndex].noisyPix_update_mutex);
			long lastUpdate = global->detector[detIndex].noisyPixLastUpdate;
			
			if( /* has processed recalc events since last update?  */ ((eventData->threadNum == lastUpdate+recalc) && (lastUpdate != 0)) || 
				/* uninitialised and buffer filled? */ ((counter >= memory) && (lastUpdate == 0)) ) {

				bool  keepThreadsLocked = global->detector[detIndex].noisyPixCalibrated || threadSafetyLevel < 1;
				global->detector[detIndex].noisyPixLastUpdate = eventData->threadNum;

				// Keep the lock during calculation of median either
				// - if we run at high thread safety level or
				// - if we are not calibrated yet (we do not want to loose frames unnecessarily during calibration)
				if(!keepThreadsLocked) pthread_mutex_unlock(&global->detector[detIndex].noisyPix_update_mutex);

				DEBUG3("Actually calculate a new noisy pixel mask from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);
				printf("Detector %li: Start calculation of persistent background.\n",detIndex);			

				// Update std
				frameBuffer->updateStd();

				// Apply to shared mask
				float minStd = global->detector[detIndex].noisyPixMinDeviation;
				long pix_nn = global->detector[detIndex].pix_nn;
				float * std = (float *) malloc(pix_nn*sizeof(float)); 
				frameBuffer->copyStd(std);
				if (threadSafetyLevel > 1) pthread_mutex_lock(&global->detector[detIndex].pixelmask_shared_mutex);
				uint16_t * mask = global->detector[detIndex].pixelmask_shared;
				long	nNoisy = 0;
				for(long i=0; i<pix_nn; i++) {
					// Apply threshold
					if(std[i] < minStd) {
						mask[i] &= ~(PIXEL_IS_NOISY);
					}
					else {
						mask[i] |= PIXEL_IS_NOISY;
						nNoisy++;				
					}		
				}
				if (threadSafetyLevel > 1) pthread_mutex_unlock(&global->detector[detIndex].pixelmask_shared_mutex);					
				free(std);
				global->detector[detIndex].nNoisy = nNoisy;
				printf("Detector %li: New noisy pixel mask calculated - %li noisy pixels identified.\n",detIndex,nNoisy);      
				global->detector[detIndex].noisyPixCalibrated = 1;

				if(keepThreadsLocked)	pthread_mutex_unlock(&global->detector[detIndex].noisyPix_update_mutex);		   

			} else {
				pthread_mutex_unlock(&global->detector[detIndex].noisyPix_update_mutex);			
			}
		}		
	}
}

