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
		cDataVersion dataV_2d(eventData->detector[detIndex],DATA_FORMAT_NON_ASSEMBLED);
		cDataVersion dataV_r(eventData->detector[detIndex],DATA_FORMAT_RADIAL_AVERAGE);
		while (dataV_2d.next() && dataV_r.next()) {
			long	 radial_nn = global->detector[detIndex].radial_nn;
			long     pix_nn = global->detector[detIndex].pix_nn;
			float    *pix_r = global->detector[detIndex].pix_r;
			calculateRadialAverage(eventData, global, detIndex, dataV_2d.data, dataV_2d.pixelmask, dataV_r.data, dataV_r.pixelmask, pix_r, radial_nn, pix_nn);
		}
    }
}


void calculateRadialAverage(float *data2d, uint16_t *pixelmask2d, float *dataRadial, uint16_t *pixelmaskRadial, float * pix_r, long radial_nn, long pix_nn, uint16_t maskOutBits) {

	// Alloc temporary arrays
	int *      tempRadialAverageCounter = calloc(radial_nn, sizeof(int));
	uint16_t * tempBadBins              = calloc(radial_nn, sizeof(uint16_t));
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
			tempBadBins[rbin] |= pixelmaskRadial[i];
			continue;
		}

        // Add to average
		dataRadial[rbin] += data2d[i];
		tempRadialAverageCounter[rbin] += 1;
	    pixelmaskRadial[rbin] |= mask[i] 
	}

	// Divide by number of actual pixels in ring to get the average
	for(long rbin=0; rbin<radial_nn; i++) {
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

