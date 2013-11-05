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
#include <vector>
#include <Python.h>

#include "cheetah.h"

/* Very crude embedding of a Python interpreter for shared memory visualization */
/* Note that this code implicitly assumes to be the only Python interpreter within the process */
/* No synchronization at all, not even proper signal handling */

void* pythonWorker(void* threadarg)
{
  char* pythonFile = (char*) threadarg;
  FILE* fileHandle = fopen(pythonFile, "r");
  if (!fileHandle)
    {
      fprintf(stderr, "Unable to open Python script %s, error code %d, continuing without Python visualizer.", pythonFile, errno);
    }
  // Note: no call to Py_SetProgramName for now
  //char buffer [50];
  //sprintf (buffer, "/reg/neh/home/hantke/software/bin/python");
  //Py_SetProgramName(buffer);
  Py_Initialize();
  //  PyThreadState* ourThread = Py_NewInterpreter();
  PyRun_SimpleFile(fileHandle, pythonFile);
  //  Py_EndInterpreter(ourThread);
  //  Py_Finalize();

  return 0;
}

void spawnPython(char* pythonFile)
{
  pthread_t         thread;
  pthread_attr_t    threadAttribute;
  pthread_attr_init(&threadAttribute);
  pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);
  int returnStatus = pthread_create(&thread, &threadAttribute, pythonWorker, (void *) pythonFile);
}

/*
 *  libCheetah initialisation function
 */
void cheetahInit(cGlobal *global) {
    
	global->self = global;
	//global->defaultConfiguration();
	global->parseConfigFile(global->configFile);

	global->setup();
	global->writeInitialLog();
	global->writeConfigurationLog();
    global->writeStatus("Started");

	// Set better error handlers for HDF5
	H5Eset_auto(H5E_DEFAULT, cheetahHDF5ErrorHandler, NULL);
	//H5Eset_auto(cheetahHDF5ErrorHandler, NULL);

	printf("Cheetah clean initialisation\n");
	if (global->pythonFile[0]) {
	  printf("Initialising embedded Python visualisation now\n");
	  spawnPython(global->pythonFile);
	}
}


/*
 *  libCheetah function for start of a new run
 */
void cheetahNewRun(cGlobal *global) {
  // Wait for all workers to finish
  while(global->nActiveThreads > 0) {
    printf("Waiting for %li worker threads to terminate\n", global->nActiveThreads);
    usleep(100000);
  }
    
  // Reset the powder log files
    pthread_mutex_lock(&global->powderfp_mutex);

  if(global->runNumber > 0) {
    for(long i=0; i<global->nPowderClasses; i++) {
      if(global->powderlogfp[i] != NULL)
	fclose(global->powderlogfp[i]);

            char	filename[1024];
      sprintf(filename,"r%04u-class%ld-log.txt",global->runNumber,i);
      global->powderlogfp[i] = fopen(filename, "w");
      fprintf(global->powderlogfp[i], "eventData->eventname, eventData->frameNumber, eventData->threadNum, eventData->photonEnergyeV, eventData->wavelengthA, eventData->detector[0].detectorZ, eventData->gmd1, eventData->gmd2, eventData->energySpectrumExist, eventData->nPeaks, eventData->peakNpix, eventData->peakTotal, eventData->peakResolution, eventData->peakDensity, eventData->laserEventCodeOn, eventData->laserDelay\n");
    }
  }
    pthread_mutex_unlock(&global->powderfp_mutex);
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
	eventData = (cEventData*) calloc(sizeof(cEventData),1);
	eventData->pGlobal = global;

	/*
	 *	Initialise any common default values
	 */
	eventData->useThreads = 0;
	eventData->hit = 0;
	eventData->samplePumped = 0;
	eventData->peakResolution=0.;
	eventData->nPeaks=0;
	eventData->peakNpix=0.;
	eventData->peakTotal=0.;
	eventData->stackSlice=0;

	//long		pix_nn1 = global->detector[0].pix_nn;
	//long		asic_nx = global->detector[0].asic_nx;
	//long		asic_ny = global->detector[0].asic_ny;	
	//printf("************>>> %li, %li, %li\n", asic_nx, asic_ny, pix_nn1);

	/*
	 *	Create arrays for intermediate detector data, etc 
	 */
	DETECTOR_LOOP {
		long	pix_nn = global->detector[detID].pix_nn;
		long	image_nn = global->detector[detID].image_nn;
		long	imageXxX_nn = global->detector[detID].imageXxX_nn;
		long	radial_nn = global->detector[detID].radial_nn;
		
		eventData->detector[detID].corrected_data = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detID].detector_corrected_data = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detID].pixelmask = (uint16_t*) calloc(pix_nn,sizeof(uint16_t));

		eventData->detector[detID].image = (float*) calloc(image_nn,sizeof(float));
		eventData->detector[detID].image_pixelmask = (uint16_t*) calloc(image_nn,sizeof(uint16_t));

		eventData->detector[detID].imageXxX = (float*) calloc(imageXxX_nn,sizeof(float));
		eventData->detector[detID].imageXxX_pixelmask = (uint16_t*) calloc(imageXxX_nn,sizeof(uint16_t));

		eventData->detector[detID].radialAverage = (float *) calloc(radial_nn, sizeof(float));
		eventData->detector[detID].radialAverageCounter = (float *) calloc(radial_nn, sizeof(float));

		eventData->detector[detID].pedSubtracted=0;
		eventData->detector[detID].sum=0.;
	}	
	
		
	/*
	 *	Create arrays for remembering Bragg peak data
	 */
	global->hitfinderPeakBufferSize = global->hitfinderNpeaksMax*2;	
	long NpeaksMax = global->hitfinderPeakBufferSize;
	eventData->good_peaks = (int *) calloc(NpeaksMax, sizeof(int));
	
	allocatePeakList(&(eventData->peaklist), NpeaksMax);
	
	
	/*
	 *	Create arrays for energy spectrum data
	 */
	int spectrumLength = global->espectrumLength;
	eventData->energySpectrum1D = (double *) calloc(spectrumLength, sizeof(double));
	eventData->energySpectrumExist = 0;
		
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
		free(eventData->detector[detID].image);
		free(eventData->detector[detID].pixelmask);
		free(eventData->detector[detID].image_pixelmask);
		
		//if(global->detector[detID].downsampling > 1){
		  free(eventData->detector[detID].imageXxX);
		  free(eventData->detector[detID].imageXxX_pixelmask);
		//}

		free(eventData->detector[detID].radialAverage);
		free(eventData->detector[detID].radialAverageCounter);
	}
	
	freePeakList(eventData->peaklist);
	free(eventData->good_peaks);
	
	
	// Pulnix external camera
	if(eventData->pulnixFail == 0){
		free(eventData->pulnixImage);
	}
	// Opal spectrum camera
	if(eventData->specFail == 0){
		free(eventData->specImage);
	}
	//TOF stuff.
	if(eventData->TOFPresent==1){
		free(eventData->TOFTime);
		free(eventData->TOFVoltage); 
	}
    
    free(eventData->energySpectrum1D);
    
    
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
		int update_camera_length = 0;

		// what's the detector "camera length" for this shot?
		if ( global->detector[detID].fixedCameraLengthMm != 0 ) {
			// If fixed detector camera length is provided, override it here... it's a bit of a hack for now...
			detposnew = global->detector[detID].fixedCameraLengthMm;
		} else {
			detposnew = eventData->detector[detID].detectorZ;
		}

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
            
            printf("Camera length changed from %gmm to %gmm.\n", global->detector[detID].detectorZprevious,global->detector[detID].detectorZ);
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
		printf("r%04u:%li (%3.1fHz): I/O Speed test #2 (data read rate)\n", global->runNumber, eventData->frameNumber, global->datarate);
        cheetahDestroyEvent(eventData);
		return;
	}
	
    
    /*
     *  Spawn worker in single-threaded mode
     *
     *  Calling worker() as a function rather than a thread means this code waits until worker() is done before proceeding,
     *  so every event is called in sequence, and cheetah runs in a single thread.
     *
	 *	In non-threaded mode, the worker does not clean up its own eventData structure when done:
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
        
	time_t	tstart, tnow;
	time(&tstart);
	double	dtime;
	float	maxwait = 60.;
	double  dnextmsg = 1;
        
        /*
         *  Wait until we have a spare thread in the thread pool
         *  If nothing happens for 2 minutes, assume we have some sort of thread lockup and keep going anyway
         */
        while(global->nActiveThreads >= global->nThreads || (global->useSingleThreadCalibration && (global->nActiveThreads == 1) && !global->calibrated)) {
	  usleep(10000);
	  if (!(global->useSingleThreadCalibration && (global->nActiveThreads == 1) && !global->calibrated)){
	      time(&tnow);
	      dtime = difftime(tnow, tstart);
	      if(dtime > dnextmsg) {
		printf("Waiting for available worker thread (%li active)\n", global->nActiveThreads);
		dnextmsg += 1;
	      }
	      if(dtime > maxwait) {
		printf("\tApparent thread lock - no free thread for %li seconds.\n", dtime);
		printf("\tGiving up and resetting the thread counter\n");
		global->freeMutexes();
		global->nActiveThreads = 0;
		break;
	      }
	    }
	}
        
        // Set detached state
        pthread_attr_init(&threadAttribute);
        pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);

        // Create a new worker thread for this data frame
        eventData->threadNum = global->threadCounter;
        returnStatus = pthread_create(&thread, &threadAttribute, worker, (void *)eventData);

	if (returnStatus == 0) { // creation successful
	  // Increment threadpool counter
	  pthread_mutex_lock(&global->nActiveThreads_mutex);
	  global->nActiveThreads += 1;
	  global->threadCounter += 1;
	  pthread_mutex_unlock(&global->nActiveThreads_mutex);
		}
		else{
	  printf("Error: thread creation failed (frame skipped)\n");
        }
        pthread_attr_destroy(&threadAttribute);
    }
	
    /*
     *  Update counters
     */
    global->nprocessedframes += 1;
	global->nrecentprocessedframes += 1;
    
	
	/*
	 *	Save some types of information from time to timeperiodic powder patterns
	 */
	if(global->saveInterval!=0 && (global->nprocessedframes%global->saveInterval)==0 && (global->nprocessedframes > global->detector[0].startFrames+50) ){
        saveRunningSums(global);
		saveHistograms(global);
        saveRadialStacks(global);
		saveEspectrumStacks(global);
		global->updateLogfile();
        global->writeStatus("Not finished");
	}
	
}



/*
 *  libCheetah shutdown function
 */
void cheetahExit(cGlobal *global) {


    /*
     *	Wait for all worker threads to finish
     *	Sometimes the program hangs here, so wait no more than 10 minutes before exiting anyway
     */
    time_t	tstart, tnow;
	time(&tstart);
    double	dtime;
    float	maxwait = 10*60.;

    while(global->nActiveThreads > 0) {
      printf("Waiting for %li worker threads to terminate\n", global->nActiveThreads);
      usleep(100000);
      time(&tnow);
      dtime = difftime(tnow, tstart);
      if(dtime > maxwait) {
        printf("\t%li threads still active after waiting %f seconds\n", global->nActiveThreads, dtime);
        printf("\tGiving up and exiting anyway\n");
		global->freeMutexes();
        break;
      }
    }
    
    // Calculate mean photon energy
    global->meanPhotonEnergyeV = global->summedPhotonEnergyeV/global->nprocessedframes;
    global->photonEnergyeVSigma = sqrt(global->summedPhotonEnergyeVSquared/global->nprocessedframes - global->meanPhotonEnergyeV * global->meanPhotonEnergyeV);
    printf("Mean photon energy: %f eV\n", global->meanPhotonEnergyeV);
    printf("Sigma of photon energy: %f eV\n", global->photonEnergyeVSigma);
    
	
    // Save powder patterns and other stuff
    saveRunningSums(global);
    saveRadialStacks(global);
	saveEspectrumStacks(global);
	global->writeFinalLog();

    // Close all CXI files
	if(global->saveCXI)
    closeCXIFiles(global);

	
    // Save integrated run spectrum
    //saveIntegratedRunSpectrum(global);	<-- this was causing crashes (debug!)
    
	
    // Hitrate?
    if (global->nPowderClasses){
      printf("Hits: %li (%2.2f%%) ",global->nhits, 100.*( global->nhits / (float) global->nprocessedframes));
      printf("with Npeaks ranging from %i to %i\n",global->nPeaksMin[1],global->nPeaksMax[1]);
      printf("Blanks: %li (%2.2f%%) ",global->nprocessedframes-global->nhits, 100.*( (global->nprocessedframes-global->nhits)/ (float) global->nprocessedframes));
      printf("with Npeaks ranging from %i to %i\n",global->nPeaksMin[0],global->nPeaksMax[0]);
    } else {
      printf("%li hits (%2.2f%%)\n",global->nhits, 100.*( global->nhits / (float) global->nprocessedframes));
    }
    printf("%li files processed\n",global->nprocessedframes);

    
    
    // Cleanup
    for(long i=0; i<global->nDetectors; i++) {
      global->detector[i].freePowderMemory(global);
    }
    pthread_mutex_destroy(&global->nActiveThreads_mutex);
    pthread_mutex_destroy(&global->selfdark_mutex);
    pthread_mutex_destroy(&global->hotpixel_mutex);
    pthread_mutex_destroy(&global->halopixel_mutex);
    pthread_mutex_destroy(&global->bgbuffer_mutex);
    pthread_mutex_destroy(&global->framefp_mutex);
    pthread_mutex_destroy(&global->peaksfp_mutex);
    pthread_mutex_destroy(&global->powderfp_mutex);
    pthread_mutex_destroy(&global->subdir_mutex);
    pthread_mutex_destroy(&global->espectrumRun_mutex);
    pthread_mutex_destroy(&global->nespechits_mutex);
    pthread_mutex_destroy(&global->gmd_mutex);

    global->writeStatus("Finished");    
    printf("Cheetah clean exit\n");
}


