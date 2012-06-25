//
//  libcheetah.cpp
//  cheetah
//
//  Created by Anton Barty on 11/04/12.
//  Copyright (c) 2012 CFEL. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <hdf5.h>
#include <math.h>
#include <pthread.h>
#include <limits>
#include <stdint.h>
#include <stdlib.h>
#include <fenv.h>
#include <unistd.h>

#include "cheetah.h"


/*
 *  libCheetah initialisation function
 */
void cheetahInit(cGlobal *global) {
    
    global->self = global;
	//global->defaultConfiguration();
	global->parseConfigFile(global->configFile);

	// Configure detectors
	for(long i=0; i<global->nDetectors; i++) {
		global->detector[i].readDetectorGeometry(global->detector[i].geometryFile);
		global->detector[i].readDarkcal(global, global->detector[i].darkcalFile);
		global->detector[i].readGaincal(global, global->detector[i].gaincalFile);
		global->detector[i].readPeakmask(global, global->peaksearchFile);
		global->detector[i].readBadpixelMask(global, global->detector[i].badpixelFile);
		global->detector[i].readBaddataMask(global, global->detector[i].baddataFile);
		global->detector[i].readWireMask(global, global->detector[i].wireMaskFile);
	}
	
	global->setup();
	global->writeInitialLog();
	global->writeConfigurationLog();
    printf("Cheetah clean initialisation\n");
    
}

/*
 *  libCheetah function for start of a new run
 */
void cheetahNewRun(cGlobal *global) {

    // Reset the powder log files
    if(global->runNumber > 0) {
        for(long i=0; i<global->nPowderClasses; i++) {
            char	filename[1024];
            if(global->powderlogfp[i] != NULL)
                fclose(global->powderlogfp[i]);
            sprintf(filename,"r%04u-class%ld-log.txt",global->runNumber,i);
            global->powderlogfp[i] = fopen(filename, "w");        
            fprintf(global->powderlogfp[i], "eventData->eventname, eventData->threadNum, eventData->photonEnergyeV, eventData->wavelengthA, eventData->detector[0].detectorZ, eventData->gmd1, eventData->gmd2, eventData->nPeaks, eventData->peakNpix, eventData->peakTotal, eventData->peakResolution, eventData->peakDensity, eventData->laserEventCodeOn, eventData->laserDelay\n");
        }
    }
}


/*
 *  libCheetah function to create structure for holding new event information
 *  Currently only a malloc() but set up as a function so that we have the option of 
 *  initialising variables without needing to change any top level code
 */
cEventData* cheetahNewEvent(cGlobal	*global) {
    
    /*
	 *	Create new event structure
	 */
    cEventData	*eventData;
    eventData = (cEventData*) malloc(sizeof(cEventData));
	eventData->pGlobal = global;

    /*
	 *	Initialise any common default values
	 */
    eventData->useThreads = 0;
    eventData->hit = 0;
	eventData->samplePumped = 0;   

	long		pix_nn1 = global->detector[0].pix_nn;
	long		asic_nx = global->detector[0].asic_nx;
	long		asic_ny = global->detector[0].asic_ny;	
	//printf("************>>> %li, %li, %li\n", asic_nx, asic_ny, pix_nn1);
	

	/*
	 *	Create arrays for intermediate detector data, etc 
	 */
	DETECTOR_LOOP {
		long	pix_nn = global->detector[detID].pix_nn;
		long	image_nn = global->detector[detID].image_nn;
		long	radial_nn = global->detector[detID].radial_nn;
		
		eventData->detector[detID].corrected_data = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detID].corrected_data_int16 = (int16_t*) calloc(pix_nn,sizeof(int16_t));
		eventData->detector[detID].detector_corrected_data = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detID].saturatedPixelMask = (int16_t *) calloc(pix_nn,sizeof(int16_t));
		eventData->detector[detID].image = (int16_t*) calloc(image_nn,sizeof(int16_t));
		
		eventData->detector[detID].radialAverage = (float *) calloc(radial_nn, sizeof(float));
		eventData->detector[detID].radialAverageCounter = (float *) calloc(radial_nn, sizeof(float));
	}	
	
	
	
	/*
	 *	Create arrays for remembering Bragg peak data
	 */
	long NpeaksMax = global->hitfinderNpeaksMax;
	eventData->peak_com_index = (long *) calloc(NpeaksMax, sizeof(long));
	eventData->peak_intensity = (float *) calloc(NpeaksMax, sizeof(float));	
	eventData->peak_npix = (float *) calloc(NpeaksMax, sizeof(float));	
	eventData->peak_snr = (float *) calloc(NpeaksMax, sizeof(float));
	eventData->peak_com_x = (float *) calloc(NpeaksMax, sizeof(float));
	eventData->peak_com_y = (float *) calloc(NpeaksMax, sizeof(float));
	eventData->peak_com_x_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	eventData->peak_com_y_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	eventData->peak_com_r_assembled = (float *) calloc(NpeaksMax, sizeof(float));
	eventData->good_peaks = (int *) calloc(NpeaksMax, sizeof(int));
	

	
    // Return
    return eventData;
}


/*
 *  libCheetah function to clean up all memory allocated in event struture
 */
void cheetahDestroyEvent(cEventData *eventData) {
    
    cGlobal	*global = eventData->pGlobal;;
    
    // Free memory
	DETECTOR_LOOP {
		//for(int quadrant=0; quadrant<4; quadrant++) 
		//	free(eventData->detector[detID].quad_data[quadrant]);	
		free(eventData->detector[detID].raw_data);
		free(eventData->detector[detID].corrected_data);
		free(eventData->detector[detID].detector_corrected_data);
		free(eventData->detector[detID].corrected_data_int16);
		free(eventData->detector[detID].image);
		free(eventData->detector[detID].radialAverage);
		free(eventData->detector[detID].radialAverageCounter);
		free(eventData->detector[detID].saturatedPixelMask);
	}
	free(eventData->peak_com_index);
	free(eventData->peak_com_x);
	free(eventData->peak_com_y);
	free(eventData->peak_com_x_assembled);
	free(eventData->peak_com_y_assembled);
	free(eventData->peak_com_r_assembled);
	free(eventData->peak_intensity);
	free(eventData->peak_npix);
	free(eventData->peak_snr);
	free(eventData->good_peaks);
	
	
	// Pulnix external camera
    if(eventData->pulnixFail == 0) 
        free(eventData->pulnixImage);
    
	//TOF stuff.
	if(eventData->TOFPresent==1){
		free(eventData->TOFTime);
		free(eventData->TOFVoltage); 
	}
    
	free(eventData);
}


/*
 *  libCheetah function to update global variables where needed from new event data
 */
void cheetahUpdateGlobal(cGlobal *global, cEventData *eventData){
      
    
    /*
     *  Fix up detector Z position, which can be flakey
	 *
	 *	Seems that this PV is updated at only about 1 Hz.  
	 *	The function getPvFloat seems to misbehave.  
	 *	Firstly, if you skip the first few XTC datagrams, you will likely
	 *	get error messages telling you that the EPICS PV is invalid.  
	 *	More worrysome is the fact that it occasionally gives a bogus value 
	 *	of detposnew=0, without a fail message.  Hardware problem? 
	 *
	 *  event->detector[detID].detectorZ holds the read-out value
     *  fail=std::numeric_limits<float>::quiet_NaN();
     */
    DETECTOR_LOOP {
        float detposnew;
        int update_camera_length;
        
        detposnew = eventData->detector[detID].detectorZ;
		
        if ( !isnan(detposnew) ) {
			
			// New detector position = 0 could be an error
            if ( detposnew == 0 ) {
                detposnew = global->detector[detID].detposprev;
                printf("WARNING: detector position is zero, which could be an error\n"
                       "         will use previous position (%s=%f) instead...\n",global->detector[detID].detectorZpvname, detposnew);
            }
			
            //	Apply offsets
			//	When encoder reads -500mm, detector is at its closest possible
			//	position to the specimen, and is 79mm from the centre of the 
			//	8" flange where the injector is mounted.  
			//	The injector itself is about 4mm further away from the detector than this. 
            global->detector[detID].detposprev = detposnew;
            global->detector[detID].detectorEncoderValue = detposnew;
            global->detector[detID].detectorZ = detposnew + global->detector[detID].cameraLengthOffset;

            //	Round to the nearest two decimal places 
			//	(10 micron, much less than a pixel size) 
            global->detector[detID].detectorZ = floorf(global->detector[detID].detectorZ*100+0.5)/100;
            update_camera_length = 1;
        }	 
        
		//	What to do if there is no camera length information?  
		//	Keep skipping frames until this info is found?  
		//	For now, set a (non-zero) default camera length.
        if ( global->detector[detID].detectorZ == 0 ) {
			printf("global->detector[%ld].detectorZ == 0\n", detID);

            if ( global->detector[detID].defaultCameraLengthMm == 0 ) {
                printf("======================================================\n");
                printf("WARNING: Camera length %s is zero!\n", global->detector[detID].detectorZpvname);
                printf("If the problem persists, try setting the keyword\n");
                printf("defaultCameraLengthMm in your ini file\n"); 
                printf("======================================================\n");
                return;
            } 
            else {
                printf("MESSAGE: Setting default camera length (%gmm).\n",global->detector[detID].defaultCameraLengthMm);
                global->detector[detID].detectorZ = global->detector[detID].defaultCameraLengthMm;	
                update_camera_length = 1;
            }
        }
        
		
        /*
         * Recalculate reciprocal space geometry if the camera length has changed, 
         */
        if ( update_camera_length && ( global->detector[detID].detectorZprevious != global->detector[detID].detectorZ ) ) {
            // don't tinker with cheetahGlobal geometry while there are active threads...
            while (global->nActiveThreads > 0) 
                usleep(10000);
            
            printf("MESSAGE: Camera length changed from %gmm to %gmm.\n", global->detector[detID].detectorZprevious,global->detector[detID].detectorZ);
            if ( isnan(eventData->wavelengthA ) ) {
                printf("MESSAGE: Bad wavelength data (NaN). Consider using defaultPhotonEnergyeV keyword.\n");
            }	
            global->detector[detID].detectorZprevious = global->detector[detID].detectorZ;
            for(long i=0; i<global->nDetectors; i++) 
                global->detector[detID].updateKspace(global, eventData->wavelengthA);
            
        }	
    }
    
    
    /*
     *  Keep track of LCLS wavelength statistics
     */
    global->summedPhotonEnergyeV += eventData->photonEnergyeV;
	global->summedPhotonEnergyeVSquared += eventData->photonEnergyeV*eventData->photonEnergyeV;
	
    
    /*
     *  Remember laser delay
     */
    if(eventData->laserDelay != 0) {
        // Don't mess with events currently being processed
        while (global->nActiveThreads > 0) 
            usleep(10000);
        global->laserDelay = eventData->laserDelay;
    }
    
    
    /*
     *  Copy over any remaining detector info
     */
    DETECTOR_LOOP {
        eventData->detector[detID].detectorZ = global->detector[detID].detectorZ;        
        
    }
}


/*
 *  libCheetah event processing function (multithreaded)
 *  This function simply sets the thread flag for activating multi-threading
 */
void cheetahProcessEventMultithreaded(cGlobal *global, cEventData *eventData){

    eventData->useThreads = 1;
    cheetahProcessEvent(global, eventData);

}

/*
 *  libCheetah event processing function (multithreaded)
 */
void cheetahProcessEvent(cGlobal *global, cEventData *eventData){

    
	/*
	 *	Remember to update global variables 
	 */
    cheetahUpdateGlobal(global, eventData);

  
    
    /*
     *  I/O speed test
     *  How fast is processEvent called?
     *  This measures how fast Cheetah could conceivably process the data
     */
	if(global->ioSpeedTest==2) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #1\n", global->runNumber, eventData->frameNumber, global->datarate);		
		free(eventData);
		return;
	}
	
    
    /*
     *  Spawn worker in single-threaded mode
	 *	Note: worker does not clean up its own eventData structure when done: 
     *      eventData remains available after the worker exits and must be explicitly freed by the user
     */
    if(eventData->useThreads == 0) {
        worker((void *)eventData);
    }
    
  	
	/*
	 *	Spawn worker in multithreaded mode 
	 *	Threads are created detached so we don't have to wait for anything to happen before returning
	 *		(each thread is responsible for cleaning up its own eventData structure when done)
	 */
    if(eventData->useThreads == 1) {
        pthread_t		thread;
        pthread_attr_t	threadAttribute;
        int				returnStatus;
        
        // Wait until we have a spare thread in the thread pool
        while(global->nActiveThreads >= global->nThreads) {
            usleep(1000);
        }
        
        // Set detached state
        pthread_attr_init(&threadAttribute);
        pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);
        
        // Increment threadpool counter
        pthread_mutex_lock(&global->nActiveThreads_mutex);
        global->nActiveThreads += 1;
        eventData->threadNum = ++global->threadCounter;
        pthread_mutex_unlock(&global->nActiveThreads_mutex);
        
        // Create a new worker thread for this data frame
        returnStatus = pthread_create(&thread, &threadAttribute, worker, (void *)eventData); 
        pthread_attr_destroy(&threadAttribute);
    }
	
    /*
     *  Update counters
     */
    global->nprocessedframes += 1;
	global->nrecentprocessedframes += 1;
    
	
	/*
	 *	Save periodic powder patterns
	 */
	if(global->saveInterval!=0 && (global->nprocessedframes%global->saveInterval)==0 && (global->nprocessedframes > global->detector[0].startFrames+50) ){
        for(long detID=0; detID<global->nDetectors; detID++) {
            saveRunningSums(global, detID);
            global->updateLogfile();
        }
        saveRadialStacks(global);
	}
	
}



/*
 *  libCheetah shutdown function
 */
void cheetahExit(cGlobal *global) {
    
    global->meanPhotonEnergyeV = global->summedPhotonEnergyeV/global->nprocessedframes;
	global->photonEnergyeVSigma = sqrt(global->summedPhotonEnergyeVSquared/global->nprocessedframes - global->meanPhotonEnergyeV * global->meanPhotonEnergyeV);
	printf("Mean photon energy: %f eV\n", global->meanPhotonEnergyeV);
	printf("Sigma of photon energy: %f eV\n", global->photonEnergyeVSigma);
	
	// Wait for threads to finish
	while(global->nActiveThreads > 0) {
		printf("Waiting for %li worker threads to terminate\n", global->nActiveThreads);
		usleep(100000);
	}
	
	
	// Save powder patterns
    for(long detID=0; detID<global->nDetectors; detID++) {
        saveRunningSums(global, detID);
    }
    saveRadialStacks(global);
	global->writeFinalLog();
	
	// Hitrate?
	printf("%li files processed, %li hits (%2.2f%%)\n",global->nprocessedframes, global->nhits, 100.*( global->nhits / (float) global->nprocessedframes));
    
    
	// Cleanup
	for(long i=0; i<global->nDetectors; i++) {
		free(global->detector[i].darkcal);
		free(global->detector[i].hotpixelmask);
		free(global->detector[i].wiremask);
		free(global->detector[i].selfdark);
		free(global->detector[i].gaincal);
		free(global->detector[i].peakmask);
		free(global->detector[i].bg_buffer);
		free(global->detector[i].hotpix_buffer);
        
        for(long j=0; j<global->nPowderClasses; j++) {
            free(global->detector[i].powderRaw[j]);
            free(global->detector[i].powderRawSquared[j]);
            free(global->detector[i].powderAssembled[j]);
            free(global->detector[i].radialAverageStack[j]);
            pthread_mutex_destroy(&global->detector[i].powderRaw_mutex[j]);
            pthread_mutex_destroy(&global->detector[i].powderRawSquared_mutex[j]);
            pthread_mutex_destroy(&global->detector[i].powderAssembled_mutex[j]);
            pthread_mutex_destroy(&global->detector[i].radialStack_mutex[j]);
        }
	}
	pthread_mutex_destroy(&global->nActiveThreads_mutex);
	pthread_mutex_destroy(&global->selfdark_mutex);
	pthread_mutex_destroy(&global->hotpixel_mutex);
	pthread_mutex_destroy(&global->bgbuffer_mutex);
	pthread_mutex_destroy(&global->framefp_mutex);
	pthread_mutex_destroy(&global->peaksfp_mutex);
	pthread_mutex_destroy(&global->powderfp_mutex);  
    
    printf("Cheetah clean exit\n");
}


