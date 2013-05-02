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
 *  Map to Polar 
 *  (repeated once for each different data type)
 */


void mapToPolar(cEventData *eventData, cGlobal *global) {
    
 	DETECTOR_LOOP {
       	long	pix_nn = global->detector[detID].pix_nn;
        float   *corrected_data = eventData->detector[detID].corrected_data;
        long    *cart2polar_map = global->detector[detID].cart2polar_map;
        
        float   *polarData = eventData->detector[detID].polarData;
        float   *mask_polar = global->detector[detID].mask_polar;
        float   *polarDataCounter = eventData->detector[detID].polarDataCounter;
        long	nRadialBins = global->detector[detID].nRadialBins;
        long	nAngularBins = global->detector[detID].nAngularBins;
        
        
        // Mask for where to calculate average
        int     *mask = (int *) calloc(pix_nn, sizeof(int));
        for(long i=0; i<pix_nn; i++){
            mask[i] = isNoneOfBitOptionsSet(eventData->detector[detID].pixelmask[i],(PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD));
        }
        
        mapToPolar(corrected_data, cart2polar_map, pix_nn, polarData, polarDataCounter, nRadialBins, nAngularBins, mask, mask_polar);
        
        // Remember to free the mask
        free(mask);
    }
    
}


void mapToPolar(float *data, long *polarMap, long pix_nn, float *polarData, float *polarDataCounter, long nRadialBins, long nAngularBins, int *mask, float *mask_polar){
    
    // polar array size
    long   polar_nn = nRadialBins * nAngularBins;

    // Zero arrays
	for(long i=0; i<polar_nn; i++) {
		polarData[i] = 0.;
		polarDataCounter[i] = 0.;
	}
	
	// Bin average
	long	bin;
	for(long i=0; i<pix_nn; i++){
        
        // Don't count bad pixels in radial average
        if(mask[i] == 0)
            continue;
        
        // Bin index of this pixel
		bin = polarMap[i];
		
        // Add to Sum 
		polarData[bin] += data[i];
		polarDataCounter[bin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<polar_nn; i++) {
		if (polarDataCounter[i] != 0) {
			polarData[i] /= polarDataCounter[i];
         mask_polar[i] = 1.0; // this might be saved IF we assume mask is not changing
      } else {
         mask_polar[i] = 0.0; // this might be saved IF we assume mask is not changing
      }
	}
	
}

