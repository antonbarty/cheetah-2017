/*
 *  radialaverage.cpp
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
 *  Calculate radial averages
 *  (repeated once for each different data type)
 */

void calculateRadialAverage(cEventData *eventData, cGlobal *global) {
 	DETECTOR_LOOP {
		if (isBitOptionSet(global->detector[detIndex].saveFormat, DATA_FORMAT_RADIAL_AVERAGE)) {
			cDataVersion dataV_2d(&eventData->detector[detIndex], &global->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_NON_ASSEMBLED);
			cDataVersion dataV_r(&eventData->detector[detIndex], &global->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_RADIAL_AVERAGE);
			while (dataV_2d.next() && dataV_r.next()) {
				long	 radial_nn = global->detector[detIndex].radial_nn;
				long     pix_nn = global->detector[detIndex].pix_nn;
				float    *pix_r = global->detector[detIndex].pix_r;
				calculateRadialAverage(dataV_2d.data, dataV_2d.pixelmask, dataV_r.data, dataV_r.pixelmask, pix_r, radial_nn, pix_nn);
			}
		}
	}
}

template <class T>
void calculateRadialAverage(T *data2d, uint16_t *pixelmask2d, T *dataRadial, uint16_t *pixelmaskRadial, float * pix_r, long radial_nn, long pix_nn) {

	// Alloc temporary arrays
	int *      tempRadialAverageCounter = (int*) calloc(radial_nn, sizeof(int));
	uint16_t * tempBadBins              = (uint16_t*) calloc(radial_nn, sizeof(uint16_t));
	uint16_t maskOutBits = PIXEL_IS_INVALID | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_DEAD | PIXEL_IS_SHADOWED | PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD | PIXEL_IS_MISSING | PIXEL_IS_IN_HALO;
	
	// Init arrays
	for(long i=0; i<radial_nn; i++) {
		dataRadial[i] = 0.;
		pixelmaskRadial[i] = 0;
		tempBadBins[i] = PIXEL_IS_MISSING;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<pix_nn; i++){
        // Radius of this pixel
		rbin = lrint(pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0){
			rbin = 0;
		}

		// Don't count bad pixels in radial average
        if (isAnyOfBitOptionsSet(pixelmask2d[i],maskOutBits)) {
			tempBadBins[rbin] |= pixelmaskRadial[rbin];
			continue;
		}

        // Add to average
		dataRadial[rbin] += data2d[i];
		tempRadialAverageCounter[rbin] += 1;
	    pixelmaskRadial[rbin] |= pixelmask2d[i];
	}

	// Divide by number of actual pixels in ring to get the average
	for(long rbin=0; rbin<radial_nn; rbin++) {
        // Check if radial bin did receive any values
		if (tempRadialAverageCounter[rbin] == 0) {
			pixelmaskRadial[rbin] |= tempBadBins[rbin];
		} else {
			dataRadial[rbin] /= tempRadialAverageCounter[rbin];
		}
	}

	// Free temporary arrays
	free(tempRadialAverageCounter);
	free(tempBadBins);
}


/*
 * Calculate radial average of powder data
 */
void calculateRadialAveragePowder(cGlobal *global) {
 	DETECTOR_LOOP {
		if (isBitOptionSet(global->detector[detIndex].powderFormat, DATA_FORMAT_RADIAL_AVERAGE)) {
			long	 radial_nn = global->detector[detIndex].radial_nn;
			long     pix_nn = global->detector[detIndex].pix_nn;
			float    *pix_r = global->detector[detIndex].pix_r;
			cDataVersion dataV_2d(NULL, &global->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_NON_ASSEMBLED);
			cDataVersion dataV_r(NULL, &global->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_RADIAL_AVERAGE);
			while (dataV_2d.next() == 0) {
				dataV_r.next();
				for (long powderClass=0; powderClass < global->detector[detIndex].nPowderClasses; powderClass++) {
					uint16_t *buffer_2d = (uint16_t *) calloc(pix_nn,sizeof(uint16_t));
					uint16_t *buffer_radial = (uint16_t *) calloc(radial_nn,sizeof(uint16_t));
					calculateRadialAverage(dataV_2d.powder[powderClass], buffer_2d , dataV_r.powder[powderClass], buffer_radial, pix_r, radial_nn, pix_nn);
					// Currently we do not save any mask for the powders
					free(buffer_radial);
					free(buffer_2d);
				}
			}
		}
	}
}


// RADIAL-AVERAGE-STACKS

/*
 *	Add radial average to stack
 */
void addToRadialAverageStack(cEventData *eventData, cGlobal *global){
    
    // If not keeping stacks, simply return now
    if(!global->saveRadialStacks)
        return;
    
    // Sorting parameter
    int powderClass = eventData->powderClass;
    
    // Loop over all detectors
    DETECTOR_LOOP {
        addToRadialAverageStack(eventData, global, powderClass, detIndex);
    }
    
}


void addToRadialAverageStack(cEventData *eventData, cGlobal *global, int powderClass, int detIndex){
    
    cPixelDetectorCommon     *detector = &global->detector[detIndex];
    
    float   *stack = detector->radialAverageStack[powderClass];
    float   *radialAverage = eventData->detector[detIndex].radialAverage_detPhotCorr;
    long	radial_nn = detector->radial_nn;
    long    stackCounter = detector->radialStackCounter[powderClass];
    long    stackSize = detector->radialStackSize;
    
    pthread_mutex_t mutex = detector->radialStack_mutex[powderClass];
    pthread_mutex_lock(&mutex);
    
    
    // Data offsets
    long stackoffset = stackCounter % stackSize;
    long dataoffset = stackoffset*radial_nn;
    
    
    // Copy data and increment counter
    for(long i=0; i<radial_nn; i++) {
        stack[dataoffset+i] = (float) radialAverage[i];
    }
    
    // Increment counter
    detector->radialStackCounter[powderClass] += 1;
    
    
    // Save data once stack is full
    if((stackCounter % stackSize) == 0) {
        
        printf("Saving radial stack: %i %i\n", powderClass, detIndex);
        saveRadialAverageStack(global, powderClass, detIndex);
        
        for(long j=0; j<radial_nn*global->radialStackSize; j++)
            detector->radialAverageStack[powderClass][j] = 0;
    }
    
    pthread_mutex_unlock(&mutex);
    
}



/*
 *	Wrapper for saving all radial stacks
 */
void saveRadialStacks(cGlobal *global) {
    if(!global->saveRadialStacks)
        return;
    
    printf("Saving radial average stacks\n");
    
    DETECTOR_LOOP {
        for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
            saveRadialAverageStack(global, powderType, detIndex);
        }
    }
}




/*
 *  Save radial average stack
 */
void saveRadialAverageStack(cGlobal *global, int powderClass, int detIndex) {
    
    cPixelDetectorCommon     *detector = &global->detector[detIndex];
    
    pthread_mutex_lock(&detector->radialStack_mutex[powderClass]);
    
    char	filename[1024];
	long    stackCounter = detector->radialStackCounter[powderClass];
    long    stackSize = detector->radialStackSize;
	
	// We re-use stacks, what is this number?
	long	stackNum = stackCounter / stackSize;
	//if(stackNum == 0) stackNum =1;
	
	// If stack is not full, how many rows are full?
    long    nRows = stackSize;
    if(stackCounter % stackSize != 0)
        nRows = (stackCounter % stackSize);
	
	
    sprintf(filename,"r%04u-radialstack-detector%d-class%i-stack%li.h5", global->runNumber, detIndex, powderClass, stackNum);
    //sprintf(filename,"r%04u-radialstack-detector%d-class%i-%06ld.h5", global->runNumber, detIndex, powderClass, frameNum);
    printf("Saving radial stack: %s\n", filename);
    
    
    writeSimpleHDF5(filename, detector->radialAverageStack[powderClass], detector->radial_nn, nRows, H5T_NATIVE_FLOAT);
    for(long i=0; i<global->nPowderClasses; i++) {
        fflush(global->powderlogfp[i]);
    }
    
    pthread_mutex_unlock(&detector->radialStack_mutex[powderClass]);
    
}

