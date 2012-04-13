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
	global->defaultConfiguration();
	global->parseConfigFile(global->configFile);
	for(long i=0; i<global->nDetectors; i++) {
        global->detector[i].parseConfigFile(global->detector[i].detectorConfigFile);
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
cEventData* cheetahNewEvent(void) {
    
    cEventData	*eventData;
    eventData = (cEventData*) malloc(sizeof(cEventData));
    return eventData;
}


/*
 *  libCheetah function to clean up all memory allocated in event struture
 */
void cheetahDestroyEvent(cEventData *eventData) {
    
}


/*
 *  libCheetah function to update global variables where needed from new event data
 */
void cheetahUpdateGlobal(cGlobal *global, cEventData *eventData){
    
    /*
	 *	How quickly are we processing the data? (average over last 10 events)
	 */	
	time_t	tnow;
	double	dt, datarate1;
	double	dtime, datarate2;
    
	time(&tnow);
	dtime = difftime(tnow, global->tlast);
	dt = clock() - global->lastclock;
	
	if(dtime > 0) {
		datarate1 = ((float)CLOCKS_PER_SEC)/dt;
		datarate2 = (eventData->frameNumber - global->lastTimingFrame)/dtime;
		global->lastclock = clock();
		global->lastTimingFrame = eventData->frameNumber;
		time(&global->tlast);
        
		global->datarate = datarate2;
	}
    
    
    /*
     *  Fix up detector Z position, which can be flakey
     *  event->detector[detID].detectorZ holds the read-out value
     *  fail=std::numeric_limits<float>::quiet_NaN();
     */
    DETECTOR_LOOP {
        float detposnew;
        int update_camera_length;
        
        detposnew = eventData->detector[detID].detectorZ;
        if ( !isnan(detposnew) ) {
            /* FYI: the function getPvFloat seems to misbehave.  Firstly, if you
             * skip the first few XTC datagrams, you will likely get error messages
             * telling you that the EPICS PV is invalid.  Seems that this PV is
             * updated at only about 1 Hz.  More worrysome is the fact that it
             * occasionally gives a bogus value of detposnew=0, without a fail
             * message.  Hardware problem? */
            if ( detposnew == 0 ) {
                detposnew = global->detector[detID].detposprev;
                printf("WARNING: detector position is zero, which could be an error\n"
                       "         will use previous position (%s=%f) instead...\n",global->detector[detID].detectorZpvname, detposnew);
            }
            /* When encoder reads -500mm, detector is at its closest possible
             * position to the specimen, and is 79mm from the centre of the 
             * 8" flange where the injector is mounted.  The injector itself is
             * about 4mm further away from the detector than this. */
            global->detector[detID].detposprev = detposnew;
            global->detector[detID].detectorZ = detposnew + global->detector[detID].cameraLengthOffset;
            global->detector[detID].detectorEncoderValue = detposnew;
            /* Let's round to the nearest two decimal places 
             * (10 micron, much less than a pixel size) */
            global->detector[detID].detectorZ = floorf(global->detector[detID].detectorZ*100+0.5)/100;
            update_camera_length = 1;
        }	 
        
        if ( global->detector[detID].detectorZ == 0 ) {
            /* What to do if there is no camera length information?  Keep skipping
             * frames until this info is found?  In some cases, our analysis doesn't
             * need to know about this, so OK to skip in that case.  For now, the
             * solution is for the user to set a (non-zero) default camera length.
             */
            if ( global->detector[detID].defaultCameraLengthMm == 0 ) {
                printf("======================================================\n");
                printf("WARNING: Camera length %s is zero!\n", global->detector[detID].detectorZpvname);
                printf("I'm skipping this frame.  If the problem persists, try\n");
                printf("setting the keyword defaultCameraLengthMm in your ini\n"); 
                printf("file.\n");
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
 */
void cheetahProcessEvent(cGlobal *global, cEventData *eventData){

    
	/*
	 *	Remember to update global variables 
	 */
    cheetahUpdateGlobal(global, eventData);

  
    
    /*
     *  I/O speed test
     *  How fast is processEvent called?
     */
	if(global->ioSpeedTest==1) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #1\n", global->runNumber, eventData->frameNumber, global->datarate);		
		for(long i=0; i<global->nDetectors; i++) {
			for(int quadrant=0; quadrant<4; quadrant++) {
				free(eventData->detector[i].quad_data[quadrant]);
			}
		}
		free(eventData);
		return;
	}
	

	/*
	 *	Skip frames if we only want a part of the data set
	 */
	if(global->startAtFrame != 0 && eventData->frameNumber < global->startAtFrame) {
		printf("r%04u:%li (%3.1fHz): Skipping to start frame %li\n", global->runNumber, eventData->frameNumber, global->datarate, global->startAtFrame);		
		return;
	}
	if(global->stopAtFrame != 0 && eventData->frameNumber > global->stopAtFrame) {
		printf("r%04u:%li (%3.1fHz): Skipping from end frame %li\n", global->runNumber, eventData->frameNumber, global->datarate, global->stopAtFrame);		
		return;
	}

    
  
	
	/*
	 *	Spawn worker thread to process this frame
	 *	Threads are created detached so we don't have to wait for anything to happen before returning
	 *		(each thread is responsible for cleaning up its own eventData structure when done)
	 */
	pthread_t		thread;
	pthread_attr_t	threadAttribute;
	int				returnStatus;
    
	
	
	// Wait until we have a spare thread in the thread pool
	while(global->nActiveThreads >= global->nThreads) {
		usleep(1000);
	}
    
    
	// Increment threadpool counter
	pthread_mutex_lock(&global->nActiveThreads_mutex);
	global->nActiveThreads += 1;
	eventData->threadNum = ++global->threadCounter;
	pthread_mutex_unlock(&global->nActiveThreads_mutex);
	
	// Set detached state
	pthread_attr_init(&threadAttribute);
	pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);
	
	// Create a new worker thread for this data frame
	returnStatus = pthread_create(&thread, &threadAttribute, worker, (void *)eventData); 
	pthread_attr_destroy(&threadAttribute);
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


