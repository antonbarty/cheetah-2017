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

  DETECTOR_LOOP {
    if(global->detector[detID].useSubtractPersistentBackground && ((eventData->detector[detID].pedSubtracted && global->detector[detID].useDarkcalSubtraction) || (!eventData->detector[detID].pedSubtracted && !global->detector[detID].useDarkcalSubtraction))){
      /*
       *	Subtract persistent background
       */
      int	scaleBg = global->detector[detID].scaleBackground;
      long	pix_nn = global->detector[detID].pix_nn;
      float	*frameData = eventData->detector[detID].corrected_data;
      float	*background = global->detector[detID].selfdark;
      subtractPersistentBackground(frameData, background, scaleBg, pix_nn);
      eventData->detector[detID].pedSubtracted = 1;
    }
  }	
}

void calculatePersistentBackground(cEventData *eventData, cGlobal *global){
  DETECTOR_LOOP {
    if(global->detector[detID].useSubtractPersistentBackground){
      /*
       *	Recalculate background from time to time
       */
      long	pix_nn = global->detector[detID].pix_nn;
      int	lockThreads = global->detector[detID].useBackgroundBufferMutex;
      long	bufferDepth = global->detector[detID].bgMemory;
      long	bgRecalc = global->detector[detID].bgRecalc;
      long	bgMemory = global->detector[detID].bgMemory;
      float	medianPoint = global->detector[detID].bgMedian;
      long	threshold = lrint(bufferDepth*medianPoint);
      long	bgCounter,lastUpdate;
      float	*background = global->detector[detID].selfdark;

      if(lockThreads){
	pthread_mutex_lock(&global->bgbuffer_mutex);
      }
			
      bgCounter = global->detector[detID].bgCounter;
      lastUpdate = global->detector[detID].bgLastUpdate;
      if( (eventData->threadNum == bgRecalc+lastUpdate) || ((eventData->threadNum == (bgMemory-1)) && (lastUpdate == 0)) ) {
	int16_t   *frameBuffer = (int16_t *) calloc(pix_nn*bufferDepth,sizeof(int16_t));
	for(long i = 0;i<pix_nn*bufferDepth;i++){
	  frameBuffer[i] = global->detector[detID].bg_buffer[i];
	}
	if(lockThreads){
	  pthread_mutex_unlock(&global->bgbuffer_mutex);
	}

	printf("Detector %li: Start calculation of persistent background.\n",detID);

	pthread_mutex_lock(&global->selfdark_mutex);
	calculatePersistentBackground(background, frameBuffer, threshold, bufferDepth, pix_nn);
	pthread_mutex_unlock(&global->selfdark_mutex);
	global->detector[detID].bgLastUpdate = eventData->threadNum;
	global->detector[detID].bgCalibrated = 1;
	printf("Detector %li: Persistent background calculated.\n",detID);      

	free(frameBuffer);			
      } else {
	if(lockThreads){
	  pthread_mutex_unlock(&global->bgbuffer_mutex);			
	}
      }
    }
  }
}


/*
 *  Set background to the first frame by default
 *  (possibly not needed...)
 */
void initBackgroundBuffer(cEventData *eventData, cGlobal *global) {
  DETECTOR_LOOP {
    if(global->detector[detID].useSubtractPersistentBackground && ((eventData->detector[detID].pedSubtracted && global->detector[detID].useDarkcalSubtraction) || (!eventData->detector[detID].pedSubtracted && !global->detector[detID].useDarkcalSubtraction))){
      if (global->detector[detID].useSubtractPersistentBackground && global->detector[detID].bgCounter == 0){
	long	pix_nn = global->detector[detID].pix_nn;
	float	*background = global->detector[detID].selfdark;
                
	pthread_mutex_lock(&global->bgbuffer_mutex);
	if (global->detector[detID].useDarkcalSubtraction){
	  for(long i = 0;i<pix_nn;i++){
	    background[i] = eventData->detector[detID].detector_corrected_data[i];
	  }
	} else {
	  for(long i = 0;i<pix_nn;i++){
	    background[i] = (float) eventData->detector[detID].raw_data[i];
	  }
	}
   	pthread_mutex_unlock(&global->bgbuffer_mutex);
      }
    }
  }
}


/*
 *	Update background buffer
 *	Requires eventData->detector[detID].corrected_data_int16
 */
void updateBackgroundBuffer(cEventData *eventData, cGlobal *global, int hit) {
	
  DETECTOR_LOOP {
    if (global->detector[detID].useSubtractPersistentBackground && (hit==0 || global->detector[detID].bgIncludeHits)) {
      long	bufferDepth = global->detector[detID].bgMemory;
      long	pix_nn = global->detector[detID].pix_nn;
      int16_t	*frameBuffer = global->detector[detID].bg_buffer;
      long frameID = eventData->threadNum%bufferDepth;
      pthread_mutex_lock(&global->bgbuffer_mutex);
      if (global->detector[detID].useDarkcalSubtraction){
	for(long i = 0;i<pix_nn;i++){
	  frameBuffer[i+pix_nn*frameID] = (int16_t) lrint(eventData->detector[detID].detector_corrected_data[i]);
	}
      } else {
	for(long i = 0;i<pix_nn;i++){
	  frameBuffer[i+pix_nn*frameID] = eventData->detector[detID].raw_data[i];
	}
      }
      pthread_mutex_unlock(&global->bgbuffer_mutex);
#ifdef __GNUC__
      __sync_fetch_and_add(&(global->detector[detID].bgCounter),1);
#else
      int lockThreads = global->detector[detID].useBackgroundBufferMutex;
      if(lockThreads){pthread_mutex_lock(&global->bgbuffer_mutex);}
      long bgCounter = global->detector[detID].bgCounter;
      global->detector[detID].bgCounter += 1;
      if(lockThreads){pthread_mutex_unlock(&global->bgbuffer_mutex);}
#endif
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
 *	Local background subtraction
 */
void subtractLocalBackground(cEventData *eventData, cGlobal *global){
	
	DETECTOR_LOOP {
        if(global->detector[detID].useLocalBackgroundSubtraction) {
			long		asic_nx = global->detector[detID].asic_nx;
			long		asic_ny = global->detector[detID].asic_ny;
			long		nasics_x = global->detector[detID].nasics_x;
			long		nasics_y = global->detector[detID].nasics_y;
			long		radius = global->detector[detID].localBackgroundRadius;
			float		*data = eventData->detector[detID].corrected_data;
			
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
	long		e,ee;
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
							
							// Element in raw data array
							//ee = (j+jj+mj*asic_ny)*pix_nx;
							//ee += i+ii+mi*asic_nx;
							//if(ee < 0 || ee >= pix_nn){
							//	printf("Error: Array bounds error: e = %li > %li\n",e,pix_nn);
							//	continue;
							//}
							//median_buffer[counter] = data[ee];
							
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
void checkSaturatedPixels(uint16_t *raw_data, uint16_t *mask, long pix_nn, long pixelSaturationADC) {
	
	for(long i=0; i<pix_nn; i++) { 
		if ( raw_data[i] >= pixelSaturationADC)
		  mask[i] |= PIXEL_IS_SATURATED;
		else
		  mask[i] &= ~PIXEL_IS_SATURATED;
	}
}

void checkPnccdSaturatedPixels(cEventData *eventData, cGlobal *global){
  DETECTOR_LOOP {
    if((strcmp(global->detector[detID].detectorType, "pnccd") == 0) && (global->detector[detID].maskPnccdSaturatedPixels == 1)) {
      uint16_t	*data = eventData->detector[detID].raw_data;
      uint16_t	*mask = eventData->detector[detID].pixelmask;
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
	      if (data[i] > saturation_threshold[q]){
		mask[i] |= PIXEL_IS_SATURATED; 
	      }
	    }
	  }
	}
      }
    }
  }
}

void checkSaturatedPixels(cEventData *eventData, cGlobal *global){
  DETECTOR_LOOP {
    if (global->detector[detID].maskSaturatedPixels) {
      uint16_t	*raw_data = eventData->detector[detID].raw_data;
      uint16_t	*mask = eventData->detector[detID].pixelmask;
      long		nn = global->detector[detID].pix_nn;
      long		pixelSaturationADC = global->detector[detID].pixelSaturationADC;
      checkSaturatedPixels(raw_data, mask, nn, pixelSaturationADC);
    }
  }
}	
    





void updateHaloBuffer(cEventData *eventData, cGlobal *global,int hit){
  DETECTOR_LOOP{
    if(global->detector[detID].useAutoHalopixel && (!hit || global->detector[detID].halopixIncludeHits) && (!global->detector[detID].useSubtractPersistentBackground || global->detector[detID].bgCalibrated)){
      float	*frameData = eventData->detector[detID].corrected_data;
      float     *frameBuffer = global->detector[detID].halopix_buffer;
      long	pix_nn = global->detector[detID].pix_nn;
      long	bufferDepth = global->detector[detID].halopixMemory;
      long	frameID = eventData->threadNum%bufferDepth;

      float	*buffer = (float *) malloc(pix_nn*sizeof(float));
      for(long i=0; i<pix_nn; i++){
	buffer[i] = fabs(frameData[i]);
      }

      // Update buffer slice
      pthread_mutex_lock(&global->detector[detID].halopix_mutexes[frameID]);
      memcpy(frameBuffer+pix_nn*frameID, buffer, pix_nn*sizeof(float));
      pthread_mutex_unlock(&global->detector[detID].halopix_mutexes[frameID]);
      // Update counter
#ifdef __GNUC__
      __sync_fetch_and_add(&(global->detector[detID].halopixCounter),1);
#else
      pthread_mutex_lock(&global->halopixel_mutex);
      global->detector[detID].halopixCounter += 1;
      pthread_mutex_unlock(&global->halopixel_mutex);
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
      if(global->detector[detID].useAutoHalopixel) {
      float	halopixMinDeviation = global->detector[detID].halopixMinDeviation;
      long	bufferDepth = global->detector[detID].halopixMemory;
      long	halopixRecalc = global->detector[detID].halopixRecalc;
      long	halopixMemory = global->detector[detID].halopixMemory;
      long	halopixCounter = global->detector[detID].halopixCounter;
      int	halopixCalibrated = global->detector[detID].halopixCalibrated;
      long	lastUpdate = global->detector[detID].halopixLastUpdate;
      long	nhalo;
      float	threshold = bufferDepth*halopixMinDeviation;
      long	pix_nn = global->detector[detID].pix_nn;
      uint16_t  *mask = global->detector[detID].pixelmask_shared;
      uint16_t  *maskMinExtent = global->detector[detID].pixelmask_shared_min;
      uint16_t  *maskMaxExtent = global->detector[detID].pixelmask_shared_max;

      // here the condition (eventData->threadNum%50 == 0) made multiple initial calibrations unlikely
      if( (eventData->threadNum == halopixRecalc+lastUpdate && halopixCalibrated) || ( (halopixCounter >= halopixMemory)  /* && (eventData->threadNum%50 == 0) */ && !halopixCalibrated) ) { 
	
	pthread_mutex_lock(&global->halopixel_mutex);
			
	printf("Detector %li: Calculating halo pixel mask.\n",detID);
	nhalo = calculateHaloPixelMask(mask,maskMinExtent,maskMaxExtent,global->detector[detID].halopix_buffer,threshold, bufferDepth, pix_nn);
	global->detector[detID].nhalo = nhalo;
	global->detector[detID].halopixLastUpdate = eventData->threadNum;
	global->detector[detID].halopixCalibrated = 1;
	printf("Detector %li: Identified %li halo pixels.\n",detID,nhalo);	

	pthread_mutex_unlock(&global->halopixel_mutex);

      }
    }	
  }
}
  

long calculateHaloPixelMask(uint16_t *mask, uint16_t *maskMinExtent, uint16_t *maskMaxExtent, float *frameBuffer, float threshold, long bufferDepth, long pix_nn){

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

  for(long i=0; i<pix_nn; i++) {
    if (buffer[i]){
#ifdef __GNUC__
      __sync_fetch_and_or(&(mask[i]),PIXEL_IS_IN_HALO);
#else
      // Not threadsafe
      mask[i] |= PIXEL_IS_IN_HALO; 
#endif
      maskMaxExtent[i] |= PIXEL_IS_IN_HALO;
    } else {
#ifdef __GNUC__
      __sync_fetch_and_and(&(mask[i]),~PIXEL_IS_IN_HALO);
#else
      // Not threadsafe
      mask[i] &= ~PIXEL_IS_IN_HALO; 
      maskMinExtent[i] &= ~PIXEL_IS_IN_HALO;
#endif
    }
  }

  return nhalo;

}


			
