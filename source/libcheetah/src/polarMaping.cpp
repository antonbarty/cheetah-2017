//
//  polarMaping.cpp
//  cheetah
//
//  Created by Anton Barty on 30/04/13.
//  Copyright (c) 2013 Anton Barty. All rights reserved.
//

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

/*
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

*/