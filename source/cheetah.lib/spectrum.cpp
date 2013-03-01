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
#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
//#include "median.h"


// Called by worker @ line 245

// proceed if event is a 'hit', spectrum is desired & camera sucesfully outputs data
void integrateSpectrum(cEventData *eventData, cGlobal *global) {
    
    int hit = eventData->hit;
    int camfail = eventData->pulnixFail;
    int pulnixWidth = eventData->pulnixWidth;
    int pulnixHeight = eventData->pulnixHeight;
//    unsigned short *camdata = eventData->pulnixImage;
    
    int spectra = global->espectrum1D;
    
    if(hit && !camfail && spectra && pulnixWidth == 900 && pulnixHeight == 1080){
        eventData->energySpectrumExist = 1;
        printf("======================================================\n");
        printf("event spectrum exists\n");
        printf("======================================================\n");
        
        integrateSpectrum(eventData,global,pulnixWidth,pulnixHeight);
        return;
    }
}


// integrate region of spectrum into single line and output
void integrateSpectrum(cEventData *eventData, cGlobal *global, int pulnixWidth,int pulnixHeight) {
    
    float PIE = 3.141;
    float ttilt = tanf(global->espectiltang*PIE/180);
    int pulindex;
    int newind;

    for (long i=0; i<pulnixHeight; i++) {
        for (long j=0; j<pulnixWidth; j++) {
            newind = i - ceilf(j*ttilt);        // index of the integrated array, must be integer
            if (newind >= 0 && newind < pulnixHeight) {
                pulindex = i*pulnixWidth + j;   // index of the 2D camera array
                eventData->energySpectrum1D[newind]+= eventData->pulnixImage[pulindex];
            }
        }
    }
    printf("======================================================\n");
    printf("event spectrum out\n");
    printf("======================================================\n");
    return;
}

void integrateRunSpectrum(cEventData *eventData, cGlobal *global) {

    // Update integrated run spectrum
    if(eventData->energySpectrumExist) {
        pthread_mutex_lock(&global->espectrumRun_mutex);
        for (long i=0; i<global->espectrumLength; i++) {
            global->espectrumRun[i] += eventData->energySpectrum1D[i];
        }
        pthread_mutex_unlock(&global->espectrumRun_mutex);
    }
    
    // Update spectrum hit counter
	if(eventData->energySpectrumExist) {
		pthread_mutex_lock(&global->nespechits_mutex);
		global->nespechits++;
		pthread_mutex_unlock(&global->nespechits_mutex);
	}
    printf("======================================================\n");
    printf("integrated run spectrum updated\n");
    printf("======================================================\n");
    return;
}

