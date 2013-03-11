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

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "data2d.h"


void integrateSpectrum(cEventData *eventData, cGlobal *global) {
    // proceed if event is a 'hit', spectrum data exists & spectrum required    
    int hit = eventData->hit;
    int opalfail = eventData->specFail;
    int specWidth = eventData->specWidth;
    int specHeight = eventData->specHeight;
    
    int spectra = global->espectrum1D;
    
    if(global->generateDarkcal && !opalfail && spectra){
        eventData->energySpectrumExist = 1;
        genSpectrumBackground(eventData,global,specWidth,specHeight);
    }
    if(hit && !opalfail && spectra){
        eventData->energySpectrumExist = 1;
        integrateSpectrum(eventData,global,specWidth,specHeight);
    }
    return;
}


void integrateSpectrum(cEventData *eventData, cGlobal *global, int specWidth,int specHeight) {
    // integrate spectrum into single line and output to event data
    
    float PIE = 3.141;
    float ttilt = tanf(global->espectiltang*PIE/180);
    int opalindex;
    int newind;

    for (long i=0; i<specHeight; i++) {
        for (long j=0; j<specWidth; j++) {
            newind = i + (int) ceilf(j*ttilt);        // index of the integrated array, must be integer,!
            if (newind >= 0 && newind < specHeight) {
                opalindex = i*specWidth + j;   // index of the 2D camera array
                eventData->energySpectrum1D[newind]+=eventData->specImage[opalindex];
                if (global->espectrumDarkSubtract) {
                    eventData->energySpectrum1D[newind]-=global->espectrumDarkcal[opalindex];
                }
            }
        }
    }
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
    }
    
    // Update spectrum hit counter
	if(eventData->energySpectrumExist && !global->generateDarkcal) {
		pthread_mutex_lock(&global->nespechits_mutex);
		global->nespechits++;
		pthread_mutex_unlock(&global->nespechits_mutex);
	}
    return;
}


void genSpectrumBackground(cEventData *eventData, cGlobal *global, int specWidth, int specHeight) {
    // Generate background for spectrum detector
    int spectrumpix = specWidth*specHeight;
    
    pthread_mutex_lock(&global->espectrumBuffer_mutex);
    for (int i=0; i<spectrumpix; i++) {
        global->espectrumBuffer[i]+=eventData->specImage[i];
    }
    pthread_mutex_unlock(&global->espectrumBuffer_mutex);
    pthread_mutex_lock(&global->nespechits_mutex);
    global->nespechits++;
    pthread_mutex_unlock(&global->nespechits_mutex);
    return;
}


void saveIntegratedRunSpectrum(cGlobal *global) {

    int     spectrumpix = global->espectrumWidth*global->espectrumLength;
    double *espectrumDark = (double*) calloc(spectrumpix, sizeof(double));
    char	filename[1024];
    int     maxindex = 0;
    
    // compute spectrum camera darkcal and save to HDF5
    if(global->generateDarkcal){
        pthread_mutex_lock(&global->espectrumRun_mutex);
        pthread_mutex_lock(&global->nespechits_mutex);
        for(int i=0; i<spectrumpix; i++) {
            espectrumDark[i] = global->espectrumBuffer[i]/global->nespechits;
        }
        
        sprintf(filename,"r%04u-energySpectrum-darkcal.h5", global->runNumber);
        printf("Saving energy spectrum darkcal to file: %s\n", filename);
        
        writeSimpleHDF5(filename, espectrumDark, global->espectrumWidth, global->espectrumLength, H5T_NATIVE_DOUBLE);
        
        free(espectrumDark);
        return;
    }
    
    // find maximum of run integrated spectum array and save both to HDF5
    pthread_mutex_lock(&global->espectrumRun_mutex);
    pthread_mutex_lock(&global->nespechits_mutex);
    
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
	return;
    
}

void readSpectrumDarkcal(cGlobal *global, char *filename) {
    
    int spectrumpix = global->espectrumLength*global->espectrumWidth;
    
    // Do we need a darkcal file?
	if (global->espectrumDarkSubtract == 0){
		return;
	}
    
    // Check if a darkcal file has been specified
	if ( strcmp(filename,"") == 0 ){
		printf("spectrum camera Darkcal file path was not specified.\n");
		exit(1);
	}
    
    // Check whether file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tspecified energy spectrum Darkcal file does not exist: %s\n",filename);
		printf("\tAborting...\n");
		exit(1);
	}
    
	printf("Reading energy spectrum Darkcal file:\n");
	printf("\t%s\n",filename);
	
	// Read darkcal data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
    // Copy into darkcal array
	for(long i=0; i<spectrumpix; i++) {
		global->espectrumDarkcal[i] =  temp2d.data[i];
    }
    return;
}