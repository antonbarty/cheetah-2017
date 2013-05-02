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
#include <fftw3.h>

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
        detector = &(global->detector[detID]);    
        float   *polarData = eventData->detector[detID].polarData;
        long    nRadialBins = global->detector[detID].nRadialBins;
		  long    nAngularBins = global->detector[detID].nAngularBins;
		  long 	 polar_nn = global->detector[detID].polar_nn;
        double *this_angularcorrelation = (double*) calloc( polar_nn, sizeof(double) );

        // compute angular-correlation 
        // calculateACviaFFT(polarData, this_angularcorrelation, nRadialBins, nAngularBins);

	     pthread_mutex_lock(&detector->angularcorrelation_mutex);
        for(long ii=0;ii<polar_nn;ii++){
 //         detector->polarIntensities[ii] += polarData[ii]; 
          detector->angularcorrelation[ii] += this_angularcorrelation[ii]; 
        }
   	  detector->angularcorrelationCounter++;
   	  pthread_mutex_unlock(&detector->angularcorrelation_mutex);
        free( this_angularcorrelation );
    }
}
    
void calculateACviaFFT(float *polarData, double* this_angularcorrelation, long nRadialBins, long nAngularBins) {  
//cPixelDetectorCommon* detector) {
  fftw_plan p_forward, p_backward;  // if the array sizes are constant, these plans can be saved for future
  fftw_complex *out;
  fftw_complex *in;
  double nAngularBins2 = nAngularBins * nAngularBins;
  long offset;

  //may not be necessary to make this new input array, if converting to double is not required during FFT
  in = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * nAngularBins );  
  out = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * nAngularBins );

  p_forward = fftw_plan_dft_1d( nAngularBins, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  p_backward = fftw_plan_dft_1d( nAngularBins, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

  for( long ii=0;ii<nAngularBins;ii++) in[ii][1] = 0;

  for(int nr=0;nr<nRadialBins;nr++) {
    offset = nr*nAngularBins;
    for( long ii=0;ii<nAngularBins;ii++) in[ii][0] = polarData[offset+ii]; 
    fftw_execute( p_forward );

    for( long ii=0;ii<nAngularBins;ii++) {
      in[ii][0] = out[ii][0]*out[ii][0] + out[ii][1]*out[ii][1];
      in[ii][0] /= nAngularBins2;
    }
  
    fftw_execute( p_backward );
    for( long ii=0;ii<nAngularBins;ii++) 
      this_angularcorrelation[offset+ii] = out[ii][0]/nAngularBins;
  }
  fftw_destroy_plan( p_forward );
  fftw_destroy_plan( p_backward);
  fftw_free(in);
  fftw_free(out);
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

    cPixelDetectorCommon     *detector = &(global->detector[detID]);
    long    frameNum = detector->angularcorrelationCounter;
    double *this_angularcorrelation = (double*) calloc( detector->polar_nn, sizeof(double) );

    pthread_mutex_lock(&detector->angularcorrelation_mutex);

    detector->getGapCorrelation( );
    for(long ii=0;ii<detector->polar_nn;ii++) {
      if( detector->mask_angularcorrelation[ii] == 0 )
	     continue;
      this_angularcorrelation[ii] = detector->angularcorrelation[ii] / detector->mask_angularcorrelation[ii];
    }

    char	filename[1024];

    sprintf(filename,"r%04u-angularcorrelation-detector%d-class%i-frame%ld.h5", global->runNumber, detID, powderClass,frameNum);
    printf("Saving Angular-correlation: %s\n", filename);
    writeSimpleHDF5(filename, this_angularcorrelation, detector->nAngularBins, detector->nRadialBins, H5T_NATIVE_DOUBLE);

/*
    sprintf(filename,"r%04u-polar-detector%d-class%i-frame%ld.h5", global->runNumber, detID, powderClass,frameNum);
    printf("Saving Polar pixel intensity: %s\n", filename);
    writeSimpleHDF5(filename, detector->polarIntensities, detector->nAngularBins, detector->nRadialBins, H5T_NATIVE_DOUBLE);

*/
    pthread_mutex_unlock(&detector->angularcorrelation_mutex);			
    free(this_angularcorrelation);
}



