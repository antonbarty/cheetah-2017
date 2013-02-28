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
        integrateSpectrum(eventData,global,pulnixWidth,pulnixHeight);
        eventData->energySpectrumExist = 1;
        //for(long i=0; i<pulnixHeight; i++){
        //    eventData->energySpectrum1D[i] = eventData->pulnixImage[i];
        //}
        printf("======================================================\n");
        printf("spectrum exists\n");
        printf("======================================================\n");
        return;
    }
}


// integrate region of spectrum into single line and output
void integrateSpectrum(cEventData *eventData, cGlobal *global, int pulnixWidth,int pulnixHeight) {
    
    for(long i=0; i<pulnixHeight; i++){
        eventData->energySpectrum1D[i] = eventData->pulnixImage[i];
    }
    printf("======================================================\n");
    printf("spectrum out\n");
    printf("======================================================\n");
    return;
}
//    uint16_t* pulnixImage = eventData->pulnixImage;
//    int tilt = global->spectiltang;
//    //int sint = global->specintstartx;
//    //int eint = global->specintendx;
//    
//    float PIE = 3.142;
//    float ttilt = tanf(tilt*PIE/180);
//    int pulindex;
//    int newind;
//    
//    
//    for (long i=0; i<pulnixHeight; i++) {
//        for (long j=0; j<pulnixWidth; j++) {
//            newind = i - j*ceilf(ttilt);  // index of the integrated array, must be integer
//            if (newind >= 0 && newind < pulnixHeight) {
//                pulindex = i*pulnixWidth + j;   // index of the '2D
//                specInt[newind]+=pulnixImage[pulindex];
//            }
//        }
//    }
//    
//}

//integrateSpectrum(eventData,global,pulnixWidth,pulnixHeight);

