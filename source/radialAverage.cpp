/*
 *  radialaverage.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 23/11/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ConfigV3.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad/CspadTemp.hh"
#include "cspad/CspadCorrector.hh"
#include "cspad/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "pixelDetector.h"
#include "setup.h"
#include "worker.h"
#include "median.h"




/*
 *	Add radial average to stack
 */
 void addToRadialAverageStack(tThreadInfo *threadInfo, cGlobal *global, int powderClass, int detID){
 
	 long	radial_nn = global->detector[detID].radial_nn;
	 
     pthread_mutex_lock(&global->radialStack_mutex[powderClass]);

     // Offset to current position in stack
     long stackoffset = global->radialStackCounter[powderClass] % global->radialStackSize;
     long dataoffset = stackoffset*radial_nn;
     
     // Copy data and increment counter
     for(long i=0; i<radial_nn; i++) {
         global->radialAverageStack[powderClass][dataoffset+i] = (float) threadInfo->detector[detID].radialAverage[i];
     }
     global->radialStackCounter[powderClass] += 1;
     
     // Save out once stack is full
     if((global->radialStackCounter[powderClass] % global->radialStackSize) == 0) {
         saveRadialAverageStack(global, powderClass);
         
         // Zero array
         for(long j=0; j<radial_nn*global->radialStackSize; j++) {
             global->radialAverageStack[powderClass][j] = 0;
         }
     }
     
     pthread_mutex_unlock(&global->radialStack_mutex[powderClass]);			
 
 }


/*
 *  Save radial average stack
 */
void saveRadialAverageStack(cGlobal *global, int powderClass) {

    // Create filename
    char	filename[1024];
    int     frameNum = global->radialStackCounter[powderClass];
    sprintf(filename,"r%04u-radialstack-class%i-%06i.h5", global->runNumber, powderClass, frameNum);
    printf("Saving radial stack: %s\n", filename);

    writeSimpleHDF5(filename, global->radialAverageStack[powderClass], global->detector[0].radial_nn, global->radialStackSize, H5T_NATIVE_FLOAT);
	fflush(global->powderlogfp[powderClass]);
}


void saveRadialStacks(cGlobal *global) {
    for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
        saveRadialAverageStack(global, powderType);
    }
}


/*
 *  Calculate radial averages
 *  (repeated once for each different data type)
 */
//template <class tData>
void calculateRadialAverage(float *data, float *radialAverage, float *radialAverageCounter, cGlobal *global, int detID){

	
	long	radial_nn = global->detector[detID].radial_nn;
	long	pix_nn = global->detector[detID].pix_nn;

	
	// Zero arrays
	for(long i=0; i<radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<pix_nn; i++){
        
        // Don't count bad pixels in radial average
        if(global->detector[detID].badpixelmask[i] == 0)
            continue;

        // Radius of this pixel
		rbin = lrint(global->detector[detID].pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) 
            rbin = 0;
		
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

	
	// Zero arrays
	for(long i=0; i<radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<pix_nn; i++){
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




