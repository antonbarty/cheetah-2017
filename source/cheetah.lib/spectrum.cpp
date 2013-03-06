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
    int opalfail = eventData->specFail;
    int specWidth = eventData->specWidth;
    int specHeight = eventData->specHeight;
    
    int spectra = global->espectrum1D;
    
    if(hit && !opalfail && spectra && specWidth == 900 && specHeight == 1080){
        eventData->energySpectrumExist = 1;
        //printf("======================================================\n");
        //printf("event spectrum exists\n");
        //printf("======================================================\n");
        
        integrateSpectrum(eventData,global,specWidth,specHeight);
        return;
    }
}


// integrate region of spectrum into single line and output
void integrateSpectrum(cEventData *eventData, cGlobal *global, int specWidth,int specHeight) {
    
    float PIE = 3.141;
    float ttilt = tanf(global->espectiltang*PIE/180);
    int opalindex;
    int newind;

    for (long i=0; i<specHeight; i++) {
        for (long j=0; j<specWidth; j++) {
            newind = i + ceilf(j*ttilt);        // index of the integrated array, must be integer,!
            if (newind >= 0 && newind < specHeight) {
                opalindex = i*specWidth + j;   // index of the 2D camera array
                eventData->energySpectrum1D[newind]+=eventData->specImage[opalindex];
            }
        }
    }
    //printf("======================================================\n");
    //printf("event spectrum out\n");
    //printf("======================================================\n");
    return;
}

void integrateRunSpectrum(cEventData *eventData, cGlobal *global) {

    // Update integrated run spectrum
    if(eventData->hit && eventData->energySpectrumExist) {
        pthread_mutex_lock(&global->espectrumRun_mutex);
        for (long i=0; i<global->espectrumLength; i++) {
            global->espectrumRun[i] += eventData->energySpectrum1D[i];
        }
        pthread_mutex_unlock(&global->espectrumRun_mutex);
        //printf("======================================================\n");
        //printf("integrated run spectrum updated\n");
        //printf("======================================================\n");
        return;
    }
    
    // Update spectrum hit counter
	if(eventData->energySpectrumExist) {
		pthread_mutex_lock(&global->nespechits_mutex);
		global->nespechits++;
		pthread_mutex_unlock(&global->nespechits_mutex);
	}
}

void saveIntegratedRunSpectrum(cGlobal *global) {
    
    int maxindex = 0;
    
    pthread_mutex_lock(&global->espectrumRun_mutex);
    pthread_mutex_lock(&global->nespechits_mutex);
    
    char	filename[1024];

    for (int i=0; i<global->espectrumLength; i++) {
        if (global->espectrumRun[i] > global->espectrumRun[maxindex]) {
                maxindex = i;
        }

    }
    
    sprintf(filename,"r%04u-integratedEnergySpectrum.h5", global->runNumber);
    printf("Saving run-integrated energy spectrum: %s\n", filename);
    
    writeSpectrumInfoHDF5(filename, global->espectrumRun, global->espectrumLength, H5T_NATIVE_DOUBLE, &maxindex, 1, H5T_NATIVE_INT);
    
    
    pthread_mutex_unlock(&global->espectrumRun_mutex);
    pthread_mutex_unlock(&global->nespechits_mutex);
    
    printf("======================================================\n");
    printf("integrated run spectrum output\n");
    printf("======================================================\n");
    return;
    
}

