//  integratePattern.cpp
//  cheetah
//
//  Created by Max Hantke on 15/06/13.
//  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
//


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include <math.h>
#include <hdf5.h>
#include <limits.h>

#include "cheetah.h"
#include "detectorObject.h"

void integratePattern(cEventData * eventData,cGlobal * global){
	DETECTOR_LOOP{
		long	pix_nn = global->detector[detIndex].pix_nn;
		uint16_t	*mask = eventData->detector[detIndex].pixelmask;
		float       *data = eventData->detector[detIndex].data_detPhotCorr;
		uint16_t  combined_pixel_options = PIXEL_IS_NOISY | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_MISSING;
		
		eventData->detector[detIndex].sum = 0.;
		for(long i=0;i<pix_nn;i++){
			eventData->detector[detIndex].sum += data[i] * isNoneOfBitOptionsSet(mask[i], combined_pixel_options);
		}
	}
}
