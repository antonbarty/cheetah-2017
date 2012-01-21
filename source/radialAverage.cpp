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

#include "setup.h"
#include "worker.h"
#include "median.h"




/*
 *	Maintain running powder patterns
 *	(currently supports both old and new ways of writing out powder data)
 */
 void addToRadialAverageStack(tThreadInfo *threadInfo, cGlobal *global, int powderClass){
 
     pthread_mutex_lock(&global->radialStack_mutex[powderClass]);

     // Offset to current position in stack
     long stackoffset = global->radialStackCounter[powderClass]%global->radialStackSize;
     long dataoffset = global->radial_nn*stackoffset;
     
     // Copy data and increment counter
     for(long i=0; i<global->radial_nn; i++) {
         global->radialAverageStack[powderClass][dataoffset+i] = threadInfo->radialAverage[i];
     }
     global->radialStackCounter[powderClass] += 1;
     
     // Save out once stack is full
     if((global->radialStackCounter[powderClass] % global->radialStackSize) == 0) {
         saveRadialAverageStack(global, powderClass);
         
         // Zero array
         for(long j=0; j<global->radial_nn*global->radialStackSize; j++) {
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
    sprintf(filename,"r%04u-radialstack-class%i-%i", global->runNumber, powderClass, frameNum);

    writeSimpleHDF5(filename, global->radialAverageStack, global->radial_nn, global->radialStackSize, H5T_NATIVE_FLOAT);	
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
void calculateRadialAverage(float *data, float *radialAverage, float *radialAverageCounter, cGlobal *global){
	// Zero arrays
	for(long i=0; i<global->radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<global->pix_nn; i++){
		rbin = lrint(global->pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) rbin = 0;
		
		radialAverage[rbin] += data[i];
		radialAverageCounter[rbin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<global->radial_nn; i++) {
		if (radialAverageCounter[i] != 0)
			radialAverage[i] /= radialAverageCounter[i];
	}
	
}


void calculateRadialAverage(double *data, double *radialAverage, double *radialAverageCounter, cGlobal *global){	
	// Zero arrays
	for(long i=0; i<global->radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<global->pix_nn; i++){
		rbin = lrint(global->pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) rbin = 0;
		
		radialAverage[rbin] += data[i];
		radialAverageCounter[rbin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<global->radial_nn; i++) {
		if (radialAverageCounter[i] != 0)
			radialAverage[i] /= radialAverageCounter[i];
	}
	
}




