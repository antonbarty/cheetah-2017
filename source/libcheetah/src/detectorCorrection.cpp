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

#include <mmintrin.h>
#include <emmintrin.h>


#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"
#include "pnCcdWorkarounds.h"


/*
 *	Subtract pre-loaded darkcal file
 */
void subtractDarkcal(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if(global->detector[detIndex].useDarkcalSubtraction) {
			/*
			 *	Subtract darkcal (from calibration file)
			 */
			DEBUG3("Subtract darkcal. (detectorID=%ld)",global->detector[detIndex].detectorID);
			long	pix_nn = global->detector[detIndex].pix_nn;
			float	*data = eventData->detector[detIndex].data_detCorr;
			float	*darkcal = global->detector[detIndex].darkcal;
			subtractDarkcal(data, darkcal, pix_nn);
			eventData->detector[detIndex].pedSubtracted = 1;
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
		if(global->detector[detIndex].useGaincal) {
			long	pix_nn = global->detector[detIndex].pix_nn;
			float	*data = eventData->detector[detIndex].data_detCorr;
			float	*gaincal = global->detector[detIndex].gaincal;
			DEBUG3("Apply gain correction. (detectorID=%ld)",global->detector[detIndex].detectorID);			
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
 *	Set bad pixels to zero
 */
void setBadPixelsToZero(cEventData *eventData, cGlobal *global){	
	
	DETECTOR_LOOP {
		if(global->detector[detIndex].applyBadPixelMask) {
			DEBUG3("Set bad pixels to zero. (detectorID=%ld)",global->detector[detIndex].detectorID);			
			long	 pix_nn = global->detector[detIndex].pix_nn;
			float	 *data = eventData->detector[detIndex].data_detCorr;
			uint16_t *mask = eventData->detector[detIndex].pixelmask;

			setBadPixelsToZero(data, mask, pix_nn);
		}
	} 
}

void setBadPixelsToZero(float *data, uint16_t *mask, long pix_nn) {
	for(long i=0; i<pix_nn; i++) {
		data[i] *= isBitOptionUnset(mask[i],PIXEL_IS_BAD); 
	}
}



/*
 *	Photon counting
 *	Convert ADU to photon counts, given a photon to a single ADU calibration value
 */
void photonCount(cEventData *eventData, cGlobal *global){

	DETECTOR_LOOP {
		if(global->detector[detIndex].photonCount) {
			DEBUG3("Photon count conversion (detectorID=%ld)",global->detector[detIndex].detectorID);
			long	 pix_nn = global->detector[detIndex].pix_nn;
			float	 *data = eventData->detector[detIndex].data_detPhotCorr;
			uint16_t *mask = eventData->detector[detIndex].pixelmask;
			
			float	photconv_adu = global->detector[detIndex].photconv_adu;
			float	photconv_ev = global->detector[detIndex].photconv_ev;
			
			float	adu_per_photon;
			if (photconv_ev > 0){
				adu_per_photon = photconv_adu * (eventData->photonEnergyeV/photconv_ev);
			} else {
				adu_per_photon = photconv_adu;
			}

			photonCount(data, mask, pix_nn, adu_per_photon);
		}
	}
}

void photonCount(float *data, uint16_t *mask, long pix_nn, float adu_per_photon) {

	float	adu;
	float	temp;
	
	for(long i=0; i<pix_nn; i++) {
		adu = data[i];
		if( isBitOptionSet(mask[i],PIXEL_IS_BAD) ) {
			data[i] = 0;
		}
		else {
			data[i] = 0;
			if(adu >= 0.7*adu_per_photon && adu <= 1.5*adu_per_photon) {
				data[i] = 1;
			}
			else if (adu > 1.5*adu_per_photon){
				temp = (adu - 0.5*adu_per_photon)/adu_per_photon;
				data[i] = floorf(temp);
			}
		}
	}
}





/*
 *	Subtract common mode on each module
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cspadModuleSubtractMedian(cEventData *eventData, cGlobal *global){
    cspadModuleSubtract(eventData, global, 1);
}
void cspadModuleSubtractHistogram(cEventData *eventData, cGlobal *global){
	cspadModuleSubtract(eventData, global, 3);
}

void cspadModuleSubtract2(cEventData *eventData, cGlobal *global){
    cspadModuleSubtract(eventData, global, 2);
}

void cspadModuleSubtract(cEventData *eventData, cGlobal *global, int flag){
	
	DETECTOR_LOOP {
		if((strcmp(global->detector[detIndex].detectorType, "cspad") == 0) || (strcmp(global->detector[detIndex].detectorType, "cspad2x2") == 0)) {
			if(global->detector[detIndex].cmModule == flag) {
				DEBUG3("CSPAD module subtraction. (detectorID=%ld)",global->detector[detIndex].detectorID);			
				// Dereference datector arrays
				float		threshold = global->detector[detIndex].cmFloor;
				float		*data = eventData->detector[detIndex].data_detCorr;
				uint16_t	*mask = eventData->detector[detIndex].pixelmask;
				long		asic_nx = global->detector[detIndex].asic_nx;
				long		asic_ny = global->detector[detIndex].asic_ny;
				long		nasics_x = global->detector[detIndex].nasics_x;
				long		nasics_y = global->detector[detIndex].nasics_y;
			
				if(flag==1 || flag==2) {
					cspadModuleSubtractMedian(data, mask, threshold, asic_nx, asic_ny, nasics_x, nasics_y);
				}
				else if(flag==3) {
					long span = 16384;
					cspadModuleSubtractHistogram(data, mask, span, asic_nx, asic_ny, nasics_x, nasics_y);
				}
			
			}
		}
	}
}

/*
 *	Subtract the median value on each ASIC
 */
void cspadModuleSubtractMedian(float *data, uint16_t *mask, float threshold, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
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
					if( isBitOptionUnset(mask[e],PIXEL_IS_BAD) ) {
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
 *	Subtract the location of the maximum of a histogram of ASIC values
 *	(the LCLS/psana apporach)
 */
void cspadModuleSubtractHistogram(float *data, uint16_t *mask, long hist_span, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	long		e;
	long		counter;
	
	
	// Create histogram array
	long	hist_offset = hist_span;
	long	hist_index;
	long	*histogram = (long*) calloc(2*hist_span, sizeof(long));
	
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			// Zero histogram array
			for(long i=0; i<2*hist_span; i++)
				histogram[i] = 0;
			
			// Loop over pixels within a module and populate the histogram
			// Ignoring bad pixels avoids being dominated by those pixels being set to 0
			// Remember to do an array bounds check
			counter = 0;
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					if( isBitOptionUnset(mask[e],PIXEL_IS_BAD) ) {
						hist_index = lrint(data[e]) + hist_offset;
						if(hist_index >= 0 && hist_index < 2*hist_span)
							histogram[hist_index] += 1;
					}
				}
			}
			
			// Maximum of the histogram is the common mode offset
			long	hist_max_value = -1;
			long	hist_max_position = -1;
			
			for(long i=0; i<2*hist_span; i++) {
				if(histogram[i] > hist_max_value) {
					hist_max_value = histogram[i];
					hist_max_position = i;
				}
			}
			if(hist_max_position != -1)
				hist_max_position -= hist_offset;
			else
				hist_max_position = 0;
			
			
			// Subtract median value
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					e = (j + mj*asic_ny) * (asic_nx*nasics_x);
					e += i + mi*asic_nx;
					data[e] -= hist_max_position;
				}
			}
		}
	}
	free(histogram);
}


/*
 *	Subtract offset estimated from unbonded pixels
 *	In the upstream detector, the unbonded pixels are in Q0:0-3 and Q2:4-5 and are at the 
 *	corners of each asic and at row=col (row<194) or row-194==col (row>194) for col%10=0.  
 */
void cspadSubtractUnbondedPixels(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
		if((strcmp(global->detector[detIndex].detectorType, "cspad") == 0) || (strcmp(global->detector[detIndex].detectorType, "cspad2x2") == 0)) {
			if(global->detector[detIndex].cspadSubtractUnbondedPixels) { 
				DEBUG3("CSPAD subtraction of background measured in unbonded pixels. (detectorID=%ld)",global->detector[detIndex].detectorID);							
				// Dereference datector arrays
				float		*data = eventData->detector[detIndex].data_detCorr;
				long		asic_nx = global->detector[detIndex].asic_nx;
				long		asic_ny = global->detector[detIndex].asic_ny;
				long		nasics_x = global->detector[detIndex].nasics_x;
				long		nasics_y = global->detector[detIndex].nasics_y;
				
				cspadSubtractUnbondedPixels(data, asic_nx, asic_ny, nasics_x, nasics_y);
				
			}
		}
	}
}

void cspadSubtractUnbondedPixels(float *data, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		e;
	float		counter;
	float		background;
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_x; mi++){
		for(long mj=0; mj<nasics_y; mj++){
			
			
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
void cspadSubtractBehindWires(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
		if((strcmp(global->detector[detIndex].detectorType, "cspad") == 0) || (strcmp(global->detector[detIndex].detectorType, "cspad2x2") == 0)) {
			if(global->detector[detIndex].cspadSubtractBehindWires) {
				DEBUG3("CSPAD subtraction of background measured in behind wires. (detectorID=%ld)",global->detector[detIndex].detectorID);							
				float		threshold = global->detector[detIndex].cmFloor;
				float		*data = eventData->detector[detIndex].data_detCorr;
				uint16_t      	*mask = eventData->detector[detIndex].pixelmask;
				long		asic_nx = global->detector[detIndex].asic_nx;
				long		asic_ny = global->detector[detIndex].asic_ny;
				long		nasics_x = global->detector[detIndex].nasics_x;
				long		nasics_y = global->detector[detIndex].nasics_y;
				
				cspadSubtractBehindWires(data, mask, threshold, asic_nx, asic_ny, nasics_x, nasics_y);
				
			}
		}
	}		
}

void cspadSubtractBehindWires(float *data, uint16_t *mask, float threshold, long asic_nx, long asic_ny, long nasics_x, long nasics_y) {
	
	long		p;
	long		counter;
	long		mval;
	float		median;
	
	// Create median buffer
	float	*buffer; 
	buffer = (float*) calloc(asic_ny*asic_nx, sizeof(float));
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<nasics_y; mi++){
		for(long mj=0; mj<nasics_x; mj++){
			
			
			// Loop over pixels within a module, remembering signal behind wires
			counter = 0;
			for(long j=0; j<asic_ny; j++){
				for(long i=0; i<asic_nx; i++){
					p = (j + mj*asic_ny) * (asic_nx*nasics_x);
					p += i + mi*asic_nx;
					if( isBitOptionSet(mask[p],PIXEL_IS_SHADOWED) ){
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
 *	Set hot pixels to zero
 */
void setHotPixelsToZero(cEventData *eventData, cGlobal *global){

	DETECTOR_LOOP {
		if (global->detector[detIndex].useAutoHotPixel && global->detector[detIndex].applyAutoHotPixel){
			DEBUG3("Set hot pixels to zero. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long	pix_nn = global->detector[detIndex].pix_nn;
			float	*frameData = eventData->detector[detIndex].data_detCorr;
			uint16_t *mask = eventData->detector[detIndex].pixelmask;
	
			for(long i=0; i<pix_nn; i++)
				frameData[i] *= isBitOptionUnset(mask[i],PIXEL_IS_HOT);
	
		}
	}    
}	


/*
 *	Update hot pixel buffer
 */
void updateHotPixelBuffer(cEventData *eventData, cGlobal *global) {
	int threadSafetyLevel = global->threadSafetyLevel;
	DETECTOR_LOOP {
		if (global->detector[detIndex].useAutoHotPixel) {
			long	recalc = global->detector[detIndex].hotPixRecalc;
			long	memory = global->detector[detIndex].hotPixMemory;
			cFrameBuffer *frameBuffer = global->detector[detIndex].frameBufferHotPix;
			long bufferDepth = frameBuffer->depth;

			// Select data
			float * data = eventData->detector[detIndex].data_detCorr;
			// Adding frame to buffer
			DEBUG3("Add a new frame to the hot pixel frame buffer. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			long counter = frameBuffer->writeNextFrame(data);
			if (counter < bufferDepth)
				printf("Calibrating hot pixel map: Ring buffer fill status %li/%li.\n",counter+1,bufferDepth);
			
			// Do we have to update the hot pixel map
			DEBUG3("Check wheter or not we need to calculate a new hot pixel map from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			pthread_mutex_lock(&global->detector[detIndex].hotPix_update_mutex);
			long lastUpdate = global->detector[detIndex].hotPixLastUpdate;
			
			if( /* has processed recalc events since last update?  */ ((eventData->threadNum == lastUpdate+recalc) && (lastUpdate != 0)) || 
				/* uninitialised and buffer filled? */ ((counter >= memory) && (lastUpdate == 0)) ) {

				bool  keepThreadsLocked = global->detector[detIndex].hotPixCalibrated || threadSafetyLevel < 1;
				global->detector[detIndex].hotPixLastUpdate = eventData->threadNum;

				// Keep the lock during calculation of median either
				// - if we run at high thread safety level or
				// - if we are not calibrated yet (we do not want to loose frames unnecessarily during calibration)
				if(!keepThreadsLocked) pthread_mutex_unlock(&global->detector[detIndex].hotPix_update_mutex);

				DEBUG3("Actually calculate a new hot pixel mask from the ringbuffer now. (detectorID=%ld)",global->detector[detIndex].detectorID);
				printf("Detector %li: Start calculation of hot pixel mask.\n",detIndex);			
				

				// Update map of absolute values above thrheshold
				float threshold = global->detector[detIndex].hotPixADC;
				frameBuffer->updateAbsAboveThresh(threshold);

				// Apply to shared mask
				float frequency = global->detector[detIndex].hotPixFreq;
				long pix_nn = global->detector[detIndex].pix_nn;
				float * absAboveThreshold = (float *) malloc(pix_nn*sizeof(float)); 
				frameBuffer->copyAbsAboveThresh(absAboveThreshold);
				if (threadSafetyLevel > 1) pthread_mutex_lock(&global->detector[detIndex].pixelmask_shared_mutex);					
				uint16_t * mask = global->detector[detIndex].pixelmask_shared;
				long	nHot = 0;
				for(long i=0; i<pix_nn; i++) {
					// Apply threshold
					if(absAboveThreshold[i] < frequency) {
						mask[i] &= ~(PIXEL_IS_HOT);
					}
					else {
						mask[i] |= PIXEL_IS_HOT;
						nHot++;				
					}		
				}
				if (threadSafetyLevel > 1) pthread_mutex_unlock(&global->detector[detIndex].pixelmask_shared_mutex);
				free(absAboveThreshold);
				global->detector[detIndex].nHot = nHot;
				printf("Detector %li: New hot pixel mask calculated - %li hot pixels identified.\n",detIndex,nHot);      
				global->detector[detIndex].hotPixCalibrated = 1;

				if(keepThreadsLocked)	pthread_mutex_unlock(&global->detector[detIndex].hotPix_update_mutex);		   

			} else {
				pthread_mutex_unlock(&global->detector[detIndex].hotPix_update_mutex);			
			}
		}		
	}
}



/*
 *	Apply polarization correction
 *	The polarization correction is calculated using classical electrodynamics (expression from Hura et al JCP 2000)
 */
void applyPolarizationCorrection(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if (global->detector[detIndex].usePolarizationCorrection) {
			DEBUG3("Apply polarization correction. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			float	*data = eventData->detector[detIndex].data_detPhotCorr;
            float   *pix_x = global->detector[detIndex].pix_x;
            float   *pix_y = global->detector[detIndex].pix_y;
            float   *pix_z = global->detector[detIndex].pix_z;
			long	pix_nn = global->detector[detIndex].pix_nn;
            float   pixelSize = global->detector[detIndex].pixelSize;
            double  detectorZ = global->detector[detIndex].detectorZ;
            float   cameraLengthScale = global->detector[detIndex].cameraLengthScale;
            double  horizontalFraction = global->detector[detIndex].horizontalFractionOfPolarization;
            
			applyPolarizationCorrection(data, pix_x, pix_y, pix_z, pixelSize, detectorZ, cameraLengthScale, horizontalFraction, pix_nn);
		}
	}
}


void applyPolarizationCorrection(float *data, float *pix_x, float *pix_y, float *pix_z, float pixelSize, double detectorZ, float detectorZScale, double horizontalFraction, long pix_nn) {
	for (long i=0; i<pix_nn; i++) {
        double pix_dist = sqrt(pix_x[i]*pix_x[i]*pixelSize*pixelSize + pix_y[i]*pix_y[i]*pixelSize*pixelSize + (pix_z[i]*pixelSize + detectorZ*detectorZScale)*(pix_z[i]*pixelSize + detectorZ*detectorZScale));
		data[i] /= horizontalFraction*(1 - pix_x[i]*pix_x[i]*pixelSize*pixelSize/(pix_dist*pix_dist)) + (1 - horizontalFraction)*(1 - pix_y[i]*pix_y[i]*pixelSize*pixelSize/(pix_dist*pix_dist));
	}
}



/*
 *	Apply solid angle correction, two algorithms are available:
 *  1. Assume pixels are azimuthally symmetric
 *  2. Rigorous correction from solid angle of a plane triangle
 *  Both algorithms divides by the constant term of the solid angle so that
 *  the pixel scale is still comparable to ADU for hitfinding. The constant
 *  term of the solid angle is saved as an individual value in the HDF5 files.
 */
void applySolidAngleCorrection(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if (global->detector[detIndex].useSolidAngleCorrection) {
			DEBUG3("Apply solid angle correction. (detectorID=%ld)",global->detector[detIndex].detectorID);										
            float	*data = eventData->detector[detIndex].data_detPhotCorr;
            float   *pix_x = global->detector[detIndex].pix_x;
            float   *pix_y = global->detector[detIndex].pix_y;
            float   *pix_z = global->detector[detIndex].pix_z;
            long	pix_nn = global->detector[detIndex].pix_nn;
            float   pixelSize = global->detector[detIndex].pixelSize;
            double  detectorZ = global->detector[detIndex].detectorZ;
            float   cameraLengthScale = global->detector[detIndex].cameraLengthScale;
            double  solidAngleConst = global->detector[detIndex].solidAngleConst;
            
            if (global->detector[detIndex].solidAngleAlgorithm == 1) {
                applyAzimuthallySymmetricSolidAngleCorrection(data, pix_x, pix_y, pix_z, pixelSize, detectorZ, cameraLengthScale, solidAngleConst, pix_nn);
            } else {                
                applyRigorousSolidAngleCorrection(data, pix_x, pix_y, pix_z, pixelSize, detectorZ, cameraLengthScale, solidAngleConst, pix_nn);
            }
        }
	}
}


void applyAzimuthallySymmetricSolidAngleCorrection(float *data, float *pix_x, float *pix_y, float *pix_z, float pixelSize, double detectorZ, float detectorZScale, double solidAngleConst, long pix_nn) {
    
    // Azimuthally symmetrical (cos(theta)^3) correction
    for (int i = 0; i < pix_nn; i++) {
        double z = pix_z[i]*pixelSize + detectorZ*detectorZScale;
        double pix_dist = sqrt(pix_x[i]*pix_x[i]*pixelSize*pixelSize + pix_y[i]*pix_y[i]*pixelSize*pixelSize + z*z);
        data[i] /= (z*pixelSize*pixelSize)/(pix_dist*pix_dist*pix_dist)/solidAngleConst; // remove constant term to only get theta/phi dependent part of solid angle correction for 2D pattern
    }
}


void applyRigorousSolidAngleCorrection(float *data, float *pix_x, float *pix_y, float *pix_z, float pixelSize, double detectorZ, float detectorZScale, double solidAngleConst, long pix_nn) {            
    
    // Rigorous correction from solid angle of a plane triangle
    for (int i = 0; i < pix_nn; i++) {
        
        // allocate local arrays
        double corner_coordinates[4][3]; // array of vector coordinates of pixel corners, first index starts from upper left corner and goes around clock-wise, second index determines X=0/Y=1/Z=2 coordinate
        double corner_distances[4]; // array of distances of pixel corners, index starts from upper left corner and goes around clock-wise
        double determinant;
        double denominator;
        double solid_angle[2]; // array of solid angles of the two plane triangles that form the pixel
        double total_solid_angle;
        
        // upper left corner
        corner_coordinates[0][0] = pix_x[i]*pixelSize + pixelSize/2;
        corner_coordinates[0][1] = pix_y[i]*pixelSize + pixelSize/2;
        // upper right corner
        corner_coordinates[1][0] = pix_x[i]*pixelSize - pixelSize/2;
        corner_coordinates[1][1] = pix_y[i]*pixelSize + pixelSize/2;
        // lower right corner
        corner_coordinates[2][0] = pix_x[i]*pixelSize - pixelSize/2;
        corner_coordinates[2][1] = pix_y[i]*pixelSize - pixelSize/2;
        // lower left corner
        corner_coordinates[3][0] = pix_x[i]*pixelSize + pixelSize/2;
        corner_coordinates[3][1] = pix_y[i]*pixelSize - pixelSize/2;
        // assign Z coordinate as detector distance and calculate length of the vectors to the pixel coordinates
        for (int j = 0; j < 4; j++) {
            corner_coordinates[j][2] = pix_z[i]*pixelSize + detectorZ*detectorZScale;
            corner_distances[j] = sqrt(corner_coordinates[j][0]*corner_coordinates[j][0] + corner_coordinates[j][1]*corner_coordinates[j][1] + corner_coordinates[j][2]*corner_coordinates[j][2]);
        }
        
        // first triangle made up of upper left, upper right, and lower right corner
        // nominator in expression for solid angle of a plane triangle - magnitude of triple product of first 3 corners
        determinant = fabs( corner_coordinates[0][0]*(corner_coordinates[1][1]*corner_coordinates[2][2] - corner_coordinates[1][2]*corner_coordinates[2][1])
                           - corner_coordinates[0][1]*(corner_coordinates[1][0]*corner_coordinates[2][2] - corner_coordinates[1][2]*corner_coordinates[2][0])
                           + corner_coordinates[0][2]*(corner_coordinates[1][0]*corner_coordinates[2][1] - corner_coordinates[1][1]*corner_coordinates[2][0]) );
        denominator = corner_distances[0]*corner_distances[1]*corner_distances[2] + corner_distances[2]*(corner_coordinates[0][0]*corner_coordinates[1][0] + corner_coordinates[0][1]*corner_coordinates[1][1] + corner_coordinates[0][2]*corner_coordinates[1][2])
        + corner_distances[1]*(corner_coordinates[0][0]*corner_coordinates[2][0] + corner_coordinates[0][1]*corner_coordinates[2][1] + corner_coordinates[0][2]*corner_coordinates[2][2])
        + corner_distances[0]*(corner_coordinates[1][0]*corner_coordinates[2][0] + corner_coordinates[1][1]*corner_coordinates[2][1] + corner_coordinates[1][2]*corner_coordinates[2][2]);
        solid_angle[0] = atan2(determinant, denominator);
        if (solid_angle[0] < 0)
            solid_angle[0] += M_PI; // If det > 0 and denom < 0 arctan2 returns < 0, so add PI
        
        // second triangle made up of lower right, lower left, and upper left corner
        // nominator in expression for solid angle of a plane triangle - magnitude of triple product of last 3 corners
        determinant = fabs( corner_coordinates[0][0]*(corner_coordinates[3][1]*corner_coordinates[2][2] - corner_coordinates[3][2]*corner_coordinates[2][1])
                           - corner_coordinates[0][1]*(corner_coordinates[3][0]*corner_coordinates[2][2] - corner_coordinates[3][2]*corner_coordinates[2][0])
                           + corner_coordinates[0][2]*(corner_coordinates[3][0]*corner_coordinates[2][1] - corner_coordinates[3][1]*corner_coordinates[2][0]) );
        denominator = corner_distances[2]*corner_distances[3]*corner_distances[0] + corner_distances[2]*(corner_coordinates[0][0]*corner_coordinates[3][0] + corner_coordinates[0][1]*corner_coordinates[3][1] + corner_coordinates[0][2]*corner_coordinates[3][2])
        + corner_distances[3]*(corner_coordinates[0][0]*corner_coordinates[2][0] + corner_coordinates[0][1]*corner_coordinates[2][1] + corner_coordinates[0][2]*corner_coordinates[2][2])
        + corner_distances[0]*(corner_coordinates[3][0]*corner_coordinates[2][0] + corner_coordinates[3][1]*corner_coordinates[2][1] + corner_coordinates[3][2]*corner_coordinates[2][2]);
        solid_angle[1] = atan2(determinant, denominator);
        if (solid_angle[1] < 0)
            solid_angle[1] += M_PI; // If det > 0 and denom < 0 arctan2 returns < 0, so add PI
        
        total_solid_angle = 2*(solid_angle[0] + solid_angle[1]);
        
        data[i] /= total_solid_angle/solidAngleConst; // remove constant term to only get theta/phi dependent part of solid angle correction for 2D pattern
    }
}



/*
 *	Subtract common mode on each read-out line for the pnCCD detector
 *  The common mode is currently implemented as the position of the zero-photon peak
 *  in the intensity histogram of each line. The histogram uses integer limits and bins (could be changed later).
 *  Sanity check makes sure that the zero-photon peak lies within the max/min of
 *  the insensitive pixels (12 pixels closest to the edge), see detector map below:
 *
 */
void pnccdModuleSubtract(cEventData *eventData, cGlobal *global) {
    
    DETECTOR_LOOP {
        if(strcmp(global->detector[detIndex].detectorType, "pnccd") == 0  && global->detector[detIndex].cmModule == 1) {
			DEBUG3("Apply PNCCD module subtraction. (detectorID=%ld)",global->detector[detIndex].detectorID);										
            float    *data = eventData->detector[detIndex].data_detCorr;
            uint16_t *mask = eventData->detector[detIndex].pixelmask;
            int      start = global->detector[detIndex].cmStart;
            int      stop = global->detector[detIndex].cmStop;
            float    delta = global->detector[detIndex].cmThreshold;
            float    nstdev = global->detector[detIndex].cmRange;
            
            pnccdModuleSubtract(data, mask, start, stop, delta, nstdev, global->debugLevel);
        }
    }
}


void pnccdModuleSubtract(float *data, uint16_t *mask, int start, int stop, float delta, float nstdev, int verbose) {
    float m, st, min_border, max_border;
    int i, n, q, x, y, mx, my, cm;
    // pnCCD geometry
    int asic_nx = PNCCD_ASIC_NX;
    int asic_ny = PNCCD_ASIC_NY;
    int nasics_x = PNCCD_nASICS_X;
    int nasics_y = PNCCD_nASICS_Y;
    // histogram length
    int nhist = stop - start + 1;
    
    // loop over quadrants
    for (my = 0; my < nasics_y; my++) {
        
        for (mx = 0; mx < nasics_x; mx++) {
            q = mx + my*nasics_x;
            
            for (y = 0; y < asic_ny; y++) {
                // allocate intensity histogram of line
                uint16_t *line_histogram = (uint16_t*) calloc(nhist, sizeof(uint16_t));
                int *line_histogram_x = (int*) calloc(nhist, sizeof(int));
                
                // set x-scale
                for (x = 0; x < nhist; x++)
                    line_histogram_x[x] = start + x;
                
                // fill intensity histogram with data for line
                min_border = 65536;
                max_border = -65536;
                m = 0;
                n = 0;
                for (x = 0; x < asic_nx; x++) {
                    i = my*asic_ny*asic_nx*nasics_x + y*asic_nx*nasics_x + mx*asic_nx + x;
                    // criteria for picking pixels for intensity histogram
                    //if (round(data[i] - start) >= 0 && round(data[i] - stop) <= 0 && (isNoneOfBitOptionsSet(mask[i], (PIXEL_IS_DEAD | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_BAD)) || isBitOptionSet(mask[i], PIXEL_IS_SHADOWED)))
                    //if (round(data[i] - start) >= 0 && round(data[i] - stop) <= 0 && (isNoneOfBitOptionsSet(mask[i], (PIXEL_IS_DEAD | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SHADOWED))))
                    if (round(data[i] - start) >= 0 && round(data[i] - stop) <= 0 && (isNoneOfBitOptionsSet(mask[i], (PIXEL_IS_DEAD | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_BAD))))
                        line_histogram[int(round(data[i] - start))]++;
                    if (isBitOptionSet(mask[i], PIXEL_IS_SHADOWED)) {
                        // calculate max/min/mean value of shadowed pixels
                        m += data[i];
                        if (data[i] < min_border)
                            min_border = data[i];
                        if (data[i] > max_border)
                            max_border = data[i];
                        n++;
                    }
                }
                m /= float(n);
                
                // calculate corrected sample standard deviation of shadowed pixels at the edges of lines
                st = 0;
                for (x = 0; x < asic_nx; x++) {
                    i = my*asic_ny*asic_nx*nasics_x + y*asic_nx*nasics_x + mx*asic_nx + x;
                    if (isBitOptionSet(mask[i], PIXEL_IS_SHADOWED)) {
                        st += (data[i] - m)*(data[i] - m);
                    }
                }
                st /= float(n) - 1;
                st = sqrt(st);
                
                // find peaks in histogram using PeakDetect class
                PeakDetect peakfinder(line_histogram_x, line_histogram, nhist);
                peakfinder.findAll(delta);
                
                Point *min_point, *max_point;
                cm = 0;
                if (peakfinder.maxima->size() > 0) {
                    // step through peaks in the histogram from low to high intensity, stop if peak fulfills common-mode criteria
                    for (unsigned k = 0; k < peakfinder.maxima->size(); k++) {
                        min_point = peakfinder.minima->get(k);
                        max_point = peakfinder.maxima->get(k);
                        if (verbose >= 5) {
                            printf("Peak[%u]_min = %d, Peak[%u]_max = %d, Border_min = %f, Border_mean = %f, Border_max = %f, Border_std = %f\n", k, min_point->getX(), k, max_point->getX(), min_border, m, max_border, st);
                        }
                        // sanity checks for common-mode correction
                        //if ((max_point->getX() - min_point->getX() > 4) && (max_point->getY() - line_histogram[max_point->getX() - start - 1] < delta) && (max_point->getY() - line_histogram[max_point->getX() - start + 1] < delta)) {
                        // max/min sanity check
                        //if (max_point->getX() - min_point->getX() > 2 && max_point->getX() <= ceil(max_border) && max_point->getX() >= floor(min_border)) {
                        // stdev sanity check (1 stdev = 68 % probability, 2 stdev = 95 % probability)
                        if (max_point->getX() - min_point->getX() > 2 && max_point->getX() <= ceil(m + nstdev*st) && max_point->getX() >= floor(m - nstdev*st)) {
                            cm = max_point->getX();
                            for (x = 0; x < asic_nx; x++) {
                                i = my*asic_ny*asic_nx*nasics_x + y*asic_nx*nasics_x + mx*asic_nx + x;
                                data[i] -= cm;
                                mask[i] |= PIXEL_IS_ARTIFACT_CORRECTED;
                            }
                            if (verbose >= 5) {
                                printf("Common-mode[%d][%d]: %d (peak)\n", q, y, cm);
                            }
                            break;
                        } else if (k == peakfinder.maxima->size() - 1) {
                            // if no peaks fulfill common-mode criteria, correct with mean value of insensitive pixels
                            for (x = 0; x < asic_nx; x++) {
                                i = my*asic_ny*asic_nx*nasics_x + y*asic_nx*nasics_x + mx*asic_nx + x;
                                data[i] -= m;
                                mask[i] |= PIXEL_IS_ARTIFACT_CORRECTED;
                                if (nstdev > 0)
                                    mask[i] |= PIXEL_FAILED_ARTIFACT_CORRECTION;
                            }
                            if (verbose >= 5) {
                                printf("Common-mode[%d][%d]: %f (mean)\n", q, y, m);
                            }
                        }
                    }
                } else {
                    // if no peaks are found in intensity histogram, correct with mean value of insensitive pixels
                    for (x = 0; x < asic_nx; x++) {
                        i = my*asic_ny*asic_nx*nasics_x + y*asic_nx*nasics_x + mx*asic_nx + x;
                        data[i] -= m;
                        mask[i] |= PIXEL_IS_ARTIFACT_CORRECTED;
                        if (nstdev > 0)
                            mask[i] |= PIXEL_FAILED_ARTIFACT_CORRECTION;
                    }
                    if (verbose >= 5) {
                        printf("Common-mode[%d][%d]: %f (mean)\n", q, y, m);
                    }
                }
                
                free(line_histogram);
                free(line_histogram_x);
            }
        }
    }
}



// Read out artifact compensation for pnCCD back detector
/*
  Effect: Negative offset in lines orthogonal to the read out direction. Occurs if integrated signal in line is high.
  Correction formula: O_i(x) = M(x) + ( M_i(x) * m_i + c_i ) * x
  O_i(x): offset that is applied to line x in quadrant i
  M_i(x): mean value of insensitive pixels (12 pixels closest to the edge) in line x in quadrant i
  m_a1 = 0.055 1/px ; m_a2 = 0.0050  1/px ; m_b2 = 0.0056 1/px ; m_b1 = 0.0049 1/px
  c_a1 = 0.0047 adu/px ; c_a2 = 0.0078 adu/px ; c_b2 = 0.0007 adu/px ; c_b1 = 0.0043 adu/px
  Apply correction only if integrated signal in line is above certain threshold (50000 ADU).

  This is what the detector map looks like:
 
  ---> x
  |
  v y
  
                insensitive pixels at the edge
                |                         |
                v                         v
                |A |A |A |A | |A |A |A |A |
                ------------- -------------
                |           | |           |
                |           | |           |
                |  q=0      | |  q=1      |
                |           | |           |
                |           | |           |
                | - - - - - |x| - - - - - |
                |           | |           |
                |           | |           |
                |  q=2      | |  q=3      |
                |           | |           |
                |           | |           |
                ------------- -------------
                |A |A |A |A | |A |A |A |A |
                ^                         ^
                |                         |
                insensitive pixels at the edge
  

	
*/
void pnccdOffsetCorrection(cEventData *eventData, cGlobal *global){

	DETECTOR_LOOP {
		if(strcmp(global->detector[detIndex].detectorType, "pnccd") == 0  && global->detector[detIndex].usePnccdOffsetCorrection == 1) {
			DEBUG3("Apply PNCCD offset correction. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			float	*data = eventData->detector[detIndex].data_detCorr;
			uint16_t *mask = eventData->detector[detIndex].pixelmask;
			pnccdOffsetCorrection(data,mask);
		}
	}
}		


void pnccdOffsetCorrection(float *data,uint16_t *mask) {
    float sum,m;
    int i,j,x,y,mx,my,x_;
    int q;
    int asic_nx = PNCCD_ASIC_NX;
    int asic_ny = PNCCD_ASIC_NY;
    int nasics_x = PNCCD_nASICS_X;
    int nasics_y = PNCCD_nASICS_Y;
    int x_insens_start[4] = {11,1012,11,1012};
    int x_sens_start[4] = {511,512,511,512};
    int insensitve_border_width = 12;
    int Nxsens = 500;
    float sumThreshold = 50000.;
    float offset_m[4] = {0.0055,0.0056,0.0050,0.0049};
    float offset_c[4] = {0.0047,0.0007,0.0078,0.0043};
    int read_out_direction[4] = {-1,1,-1,1};

    // Loop over quadrants
    for(my=0; my<nasics_y; my++){
        for(mx=0; mx<nasics_x; mx++){
            q = mx+my*nasics_x;
            for(y=0; y<asic_ny; y++){
                // sum up values in line
                sum = 0.;
                for(x=0; x<asic_nx; x++){
                    i = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + mx*asic_nx + x;
                    sum += data[i];
                }
                
                // only do corrections for line if sum exceeds threshold
                if (sum > sumThreshold){
                    
                    // calculate mean value of insensitve pixels
                    m = 0.;
                    for(x_=0; x_<insensitve_border_width; x_++){
                        x = x_insens_start[q]+x_*read_out_direction[q];
                        j = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + x;
                        m += data[j];
                    }
                    m /= float(insensitve_border_width);
                    
                    // do offset correction
                    for(x_=0; x_<Nxsens; x_++){
                        x = x_sens_start[q]+x_*read_out_direction[q];
                        j = my * (asic_ny*asic_nx*nasics_x) + y * asic_nx*nasics_x + x;
                        data[j] -= m;
                        data[j] -= (m*offset_m[q]+offset_c[q])*float(500-x_);
						mask[j] |= PIXEL_IS_ARTIFACT_CORRECTED;
                    }
                }
            }
        }
    }
}



void pnccdLineInterpolation(cEventData *eventData,cGlobal *global){
	DETECTOR_LOOP {
		if((strcmp(global->detector[detIndex].detectorType, "pnccd") == 0) && (global->detector[detIndex].usePnccdLineInterpolation == 1)) {
			DEBUG3("Apply PNCCD line interpolation. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			// lines in direction of the slowly changing dimension 
			long nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
			long ny = PNCCD_ASIC_NY * PNCCD_nASICS_Y;
			long x,y,i,i0,i1;
			long x_min = 1;
			long x_max = nx-1;
			float *data = eventData->detector[detIndex].data_detCorr;
			uint16_t *mask = eventData->detector[detIndex].pixelmask;
			uint16_t mask_out_bits = PIXEL_IS_INVALID | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_DEAD |
				PIXEL_IS_SHADOWED | PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD  | PIXEL_IS_MISSING;
			for(y=0; y<ny; y++){
				for(x=x_min;x<=x_max;x=x+2){
					i = nx*y+x;
					i0 = nx*y+x-1;
					i1 = nx*y+x+1;
					if (isNoneOfBitOptionsSet(mask[i0],mask_out_bits) && isNoneOfBitOptionsSet(mask[i1],mask_out_bits)){
						data[i] = (data[i0]+data[i1])/2.;
					}
				}
			}	    
		}
	}
}

void pnccdLineMasking(cEventData *eventData,cGlobal *global){
	DETECTOR_LOOP {
		if((strcmp(global->detector[detIndex].detectorType, "pnccd") == 0) && (global->detector[detIndex].usePnccdLineMasking == 1)) {
			DEBUG3("Apply PNCCD mask erroneous lines. (detectorID=%ld)",global->detector[detIndex].detectorID);										
			// lines in direction of the slowly changing dimension 
			long nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
			long ny = PNCCD_ASIC_NY * PNCCD_nASICS_Y;
			long x,y,i,i0,i1;
			long x_min = 1;
			long x_max = nx-1;
			uint16_t *mask = eventData->detector[detIndex].pixelmask;
			for(y=0; y<ny; y++){
				for(x=x_min;x<=x_max;x=x+2){
					i = nx*y+x;
					i0 = nx*y+x-1;
					i1 = nx*y+x+1;
					mask[i] |= PIXEL_IS_BAD;
					if (global->detector[detIndex].usePnccdLineInterpolation == 1){
						mask[i] |= PIXEL_IS_ARTIFACT_CORRECTED;
					}
				}
			}	    
		}
	}
}

void pnCcdModuleWiseOrderFilterBackgroundSubtraction(cEventData *eventData, cGlobal *global)
{
    DETECTOR_LOOP
    {
        if (strcmp(global->detector[detIndex].detectorType, "pnccd") == 0 && global->detector[detIndex].usePnccdOffsetCorrection == 1) {
            DEBUG3("Apply PNCCD module wise order filter background subtraction. (detectorID=%ld)", global->detector[detIndex].detectorID);
            float *data = eventData->detector[detIndex].data_detCorr;
            uint16_t *mask = eventData->detector[detIndex].pixelmask;

            //  Masks for bad regions  (mask=0 to ignore regions)
            char *mask_u8 = (char*) calloc(1024 * 1024, sizeof(char));
            uint16_t combined_pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_IN_JET;
            for (long i = 0; i < pix_nn; i++)
                mask_u8[i] = isNoneOfBitOptionsSet(mask, combined_pixel_options);

            pnCcdModuleWiseOrderFilterBackgroundSubtraction(data, mask_u8);
            free(mask_u8);
        }
    }
}


// The following can be fixed also with an adequate geometry as well.
/*
 * Fix pnccd wiring error
 *
 * For each CCD, there are two CAMEX channels which may require special attention for scientific analysis. 
 * When numbering the 1024 CAMEX channels of a CCD from 1 to 1024, these are channels 513 and 1024. 
 * CAMEX channel 513 is not bonded to a physical detector channel. It herefore does not contain any photon data, 
 * but only CAMEX noise, and should be excluded from scientific analysis. The data of the physical detector channel n 
 * for 512 ≤ n ≤ 1022 is in CAMEX channel n + 1. CAMEX channel 1024 is bonded to both detector channels 1023 and 1024. 
 * This channel therefore contains the summed signal of detector channels 1023 and 1024. Depending on the analysis goal, 
 * this channel may be excluded from further analysis, treated as the sum that it actually is, or may be even split into two 
 * channels under some assumptions.
 *	
 *	In IDL and using the 1st version of CASS this becomes:
 *	;; Re-align top right
 *	data[512:1023, 512:1023] = shift(data[512:1023,512:1023], 0, -1)
 *	data[512:1023, 1022:1023] = 0
 *
 *	;; Re-align bottom left
 *	data[0:511, 0:511] = shift(data[0:511, 0:511], 0, 1)
 *	data[0:511, 0:1] = 0
 *
 *  Everything in cheetah is rotated 90 degrees CW compared to CASS 
 *  (and this change is reflected in the code below)
 */

void pnccdFixWiringError(cEventData *eventData, cGlobal *global) {
    DETECTOR_LOOP {
        if(strcmp(global->detector[detIndex].detectorType, "pnccd") == 0 ) {
            if(global->detector[detIndex].usePnccdFixWiringError == 1) {
				DEBUG3("Fix PNCCD wiring error. (detectorID=%ld)",global->detector[detIndex].detectorID);										
                float	*data = eventData->detector[detIndex].data_detCorr;
                pnccdFixWiringError(data);
            }
        }
    }
}


void pnccdFixWiringError(float *data) {
	long	i,j;
	long    nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
    

	// Fix top left quadrant 
	// (shift all pixels right by one and zero first column)
	for(j=512; j<1023; j++)		
		for(i=511; i>0; i--)
			data[i+j*nx] = data[(i-1)+j*nx];            
	for(j=512; j<1024; j++) 
		data[0+j*nx] = 0;
    

	// Fix bottom right quadrant 
	//  (shift all pixels left by one and zero last column)
	for(j=0; j<512; j++)
		for(i=512; i<1024; i++)
			data[i+j*nx] =  data[(i+1)+j*nx];            
	for(j=0; j<512; j++) 
		data[1023+j*nx] = 0;
    
}

