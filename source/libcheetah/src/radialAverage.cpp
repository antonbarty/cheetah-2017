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
        float   *corrected_data = eventData->detector[detID].corrected_data;
        float   *pix_r = global->detector[detID].pix_r;
       	long	pix_nn = global->detector[detID].pix_nn;
        float   *radial_average = eventData->detector[detID].radialAverage;
        float   *radial_average_counter = eventData->detector[detID].radialAverageCounter;
        long	radial_nn = global->detector[detID].radial_nn;

        // Mask for where to calculate average
        int     *mask = (int *) calloc(pix_nn, sizeof(int));
        for(long i=0; i<pix_nn; i++){
			mask[i] = isNoneOfBitOptionsSet(eventData->detector[detID].pixelmask[i],(PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD));
        }
        
        calculateRadialAverage(corrected_data, pix_r, pix_nn, radial_average, radial_average_counter, radial_nn, mask);
        
        // Remember to free the mask
        free(mask); 
    }
   
}
    

void calculateRadialAverage(float *data, float *pix_r, long pix_nn, float *radialAverage, float *radialAverageCounter, long radial_nn, int *mask){
    
	// Zero arrays
	for(long i=0; i<radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<pix_nn; i++){
        
        // Don't count bad pixels in radial average
        if(mask[i] == 0)
            continue;
        
        // Radius of this pixel
		rbin = lrint(pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) rbin = 0;
		
        // Add to average
		radialAverage[rbin] += data[i];
		radialAverageCounter[rbin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<radial_nn; i++) {
		if (radialAverageCounter[i] != 0)
			radialAverage[i] /= radialAverageCounter[i];
	}
	
}


void calculateRadialAverage(double *data, double *radialAverage, double *radialAverageCounter, cGlobal *global, int detID){	
	
	long	radial_nn = global->detector[detID].radial_nn;
	long	pix_nn = global->detector[detID].pix_nn;
	uint16_t *mask = global->detector[detID].pixelmask_shared;
	
	// Zero arrays
	for(long i=0; i<radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<pix_nn; i++){

	// Don't count bad pixels in radial average
	  if( isAnyOfBitOptionsSet(mask[i],(PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD)) )
            continue;
        
	  rbin = lrint(global->detector[detID].pix_r[i]);
		
	  // Array bounds check (paranoia)
	  if(rbin < 0) rbin = 0;
	  
	  radialAverage[rbin] += data[i];
	  radialAverageCounter[rbin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<radial_nn; i++) {
		if (radialAverageCounter[i] != 0)
			radialAverage[i] /= radialAverageCounter[i];
	}
	
}





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
      addToRadialAverageStack(eventData, global, powderClass, detID);
    }

}


void addToRadialAverageStack(cEventData *eventData, cGlobal *global, int powderClass, int detID){
 
    cPixelDetectorCommon     *detector = &global->detector[detID];

    float   *stack = detector->radialAverageStack[powderClass];
    float   *radialAverage = eventData->detector[detID].radialAverage;
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
        
        printf("Saving radial stack: %i %i\n", powderClass, detID);
        saveRadialAverageStack(global, powderClass, detID);
        
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
            saveRadialAverageStack(global, powderType, detID);
        }
    }
}




/*
 *  Save radial average stack
 */
void saveRadialAverageStack(cGlobal *global, int powderClass, int detID) {

    cPixelDetectorCommon     *detector = &global->detector[detID];

    pthread_mutex_lock(&detector->radialStack_mutex[powderClass]);

    char	filename[1024];
    long    frameNum = detector->radialStackCounter[powderClass];
	long    stackCounter = detector->radialStackCounter[powderClass];
    long    stackSize = detector->radialStackSize;
	
	// We re-use stacks, what is this number?
	long	stackNum = stackCounter / stackSize;
	//if(stackNum == 0) stackNum =1;
	
	// If stack is not full, how many rows are full?
    long    nRows = stackSize;
    if(stackCounter % stackSize != 0)
        nRows = (stackCounter % stackSize);
	
	
    sprintf(filename,"r%04u-radialstack-detector%d-class%i-stack%li.h5", global->runNumber, detID, powderClass, stackNum);
    //sprintf(filename,"r%04u-radialstack-detector%d-class%i-%06ld.h5", global->runNumber, detID, powderClass, frameNum);
    printf("Saving radial stack: %s\n", filename);


    writeSimpleHDF5(filename, detector->radialAverageStack[powderClass], detector->radial_nn, nRows, H5T_NATIVE_FLOAT);
    for(long i=0; i<global->nPowderClasses; i++) {
        fflush(global->powderlogfp[i]);
    }

    pthread_mutex_unlock(&detector->radialStack_mutex[powderClass]);

}



