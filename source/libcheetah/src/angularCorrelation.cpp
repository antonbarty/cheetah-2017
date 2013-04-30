/*
 *  angularCorrelation.cpp
 *  cheetah
 *
 *  Created by Haiguang Liu on 04/22/13.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
// #include <fftw3.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"



/*
 *  Calculate angular correlations
 *  (repeated once for each different data type)
 */

void calculateAngularCorrelation(cEventData *eventData, cGlobal *global) {

   cPixelDetectorCommon     *detector;
   mapToPolar(eventData, global);

 	DETECTOR_LOOP {
        detector = &global->detector[detID];    
        float   *polarData = eventData->detector[detID].polarData;
		  long 	 polar_nn = global->detector[detID].polar_nn;
        float *this_angularcorrelation = (float*) calloc( polar_nn, sizeof(float) );

        // compute angular-correlation 
        calculateACviaFFT(polarData, this_angularcorrelation, polar_nn, detector);

	     pthread_mutex_lock(&detector->angularcorrelation_mutex);
        for(long ii=0;ii<polar_nn;ii++){
          detector->polarIntensities[ii] += polarData[ii]; 
//     detector->angularcorrelation[ii] += this_angularcorrelation[ii]; 
        }
   	  detector->angularcorrelationCounter++;
   	  pthread_mutex_unlock(&detector->angularcorrelation_mutex);
    }
   
}
    
void calculateACviaFFT(float *polarData, float* this_angularcorrelation, long polar_nn, cPixelDetectorCommon* detector) {
}

/*
 *  Save angular-correlation
 */

 void saveAngularCorrelation(cGlobal *global) {
    DETECTOR_LOOP {
        for(int powderType=0; powderType < global->nPowderClasses; powderType++) {
 			 saveAngularCorrelation(global, powderType, detID) ;
        }
    }
}

 void saveAngularCorrelation(cGlobal *global, int powderClass, int detID) {  // saved

    cPixelDetectorCommon     *detector = &global->detector[detID];

    pthread_mutex_lock(&detector->angularcorrelation_mutex);

    char	filename[1024];
    long    frameNum = detector->angularcorrelationCounter;

    sprintf(filename,"r%04u-angularcorrelation-detector%d-class%i-frame%ld.h5", global->runNumber, detID, powderClass,frameNum);
    printf("Saving Angular-correlation: %s\n", filename);
    writeSimpleHDF5(filename, detector->angularcorrelation, detector->nAngularBins, detector->nRadialBins, H5T_NATIVE_DOUBLE);

    sprintf(filename,"r%04u-polar-detector%d-class%i-frame%ld.h5", global->runNumber, detID, powderClass,frameNum);
    printf("Saving Polar pixel intensity: %s\n", filename);
    writeSimpleHDF5(filename, detector->polarIntensities, detector->nAngularBins, detector->nRadialBins, H5T_NATIVE_DOUBLE);

    pthread_mutex_unlock(&detector->angularcorrelation_mutex);			

}



