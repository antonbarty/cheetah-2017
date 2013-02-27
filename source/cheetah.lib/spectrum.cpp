//
//  spectrum.cpp
//  cheetah
//
//  Created by Richard Bean on 2/27/13.
//
//


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

//#include "spectrum.h"
//#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
//#include "median.h"


//NOT YET CALLED BY ANYTHING

// proceed if event is a 'hit', spectrum is desired & camera sucesfully outputs data
void integrateSpectrum(cEventData *eventData, cGlobal *global) {
    
    int hit = eventData->hit;
    int camfail = eventData->pulnixFail;
    int pulnixWidth = eventData->pulnixWidth;
    int pulnixHeight = eventData->pulnixHeight;
    
    int spectra = global->espectrum1D;
    
    if(hit && !camfail && spectra && pulnixWidth == 900 && pulnixHeight == 1080){
        printf("======================================================\n");
        printf("spectrum out\n");
        printf("======================================================\n");
        return;
    }
}
