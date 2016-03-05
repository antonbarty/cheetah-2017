//
//  libcheetah.cpp
//  cheetah
//
//  Created by Anton Barty on 11/04/12.
//  Copyright (c) 2012 CFEL. All rights reserved.
//

#include <Python.h>
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

#include "cheetah.h"

void spawnPython(char*);
void* pythonWorker(void*);


/*
 *  libCheetah initialisation function
 */
int cheetahInit(cGlobal *global) {
    
	// Check if we're using psana of the same git commit
	if(!getenv("PSANA_GIT_SHA") || strcmp(getenv("PSANA_GIT_SHA"),GIT_SHA1)){
		fprintf(stderr,    "*******************************************************************************************\n");
		fprintf(stderr,"*** WARNING %s:%d ***\n",__FILE__,__LINE__);
		
		if(getenv("PSANA_GIT_SHA")){
			fprintf(stderr,"***        Using psana from git commit %s         ***\n",getenv("PSANA_GIT_SHA"));
			fprintf(stderr,"***        and cheetah_ana_mod from git commit %s ***\n",GIT_SHA1);
		}
		else{
			fprintf(stderr,"***         Using a psana version not compiled with cheetah!                            ***\n");
		}
		fprintf(stderr,    "*******************************************************************************************\n");
		sleep(10);
	}
	setenv("LIBCHEETAH_GIT_SHA",GIT_SHA1,0);

	global->self = global;
	//global->defaultConfiguration();
	global->parseConfigFile(global->configFile);
	if(global->validateConfiguration()){
		ERROR("Validation of given configuration failed");
		return 1;
	}

	global->setup();
	global->writeInitialLog();
	global->writeConfigurationLog();
	global->writeStatus("Started");

	// Set better error handlers for HDF5
	H5Eset_auto(H5E_DEFAULT, cheetahHDF5ErrorHandler, NULL);
	//H5Eset_auto(cheetahHDF5ErrorHandler, NULL);

	if (global->pythonFile[0]) {
		printf("Initialising embedded Python visualisation now\n");
		spawnPython(global->pythonFile);
	}

	// Initialise streak finder (will skip contents if streakfinder not in use)
	initStreakFinder(global);

	printf("Cheetah clean initialisation\n");
	return 0;
}


/*
 *  libCheetah function for start of a new run
 */
void cheetahNewRun(cGlobal *global) {
	// Wait for all workers to finish
	while(global->nActiveCheetahThreads > 0) {
		printf("Waiting for %li worker threads to terminate\n", global->nActiveCheetahThreads);
		usleep(500000);
	}
    
	// Reset the powder log files
    pthread_mutex_lock(&global->powderfp_mutex);

	if(global->runNumber > 0) {
		for(long i=0; i<global->nPowderClasses; i++) {
            char	filename[1024];

			sprintf(filename,"r%04u-class%ld-log.txt",global->runNumber,i);
            if(global->powderlogfp[i] != NULL)
                fclose(global->powderlogfp[i]);
			global->powderlogfp[i] = fopen(filename, "w");
			fprintf(global->powderlogfp[i], "eventData->eventname, eventData->filename, eventData->stackSlice, eventData->xtcFrameNumber, eventData->hitScore, eventData->photonEnergyeV, eventData->wavelengthA, eventData->detector[0].detectorZ, eventData->gmd1, eventData->gmd2, eventData->energySpectrumExist, eventData->nPeaks, eventData->peakNpix, eventData->peakTotal, eventData->peakResolution, eventData->peakDensity, eventData->pumpLaserCode, eventData->pumpLaserDelay\n");

			sprintf(filename,"r%04u-class%ld.lst",global->runNumber,i);
			if(global->framelist[i] != NULL)
				fclose(global->framelist[i]);
			global->framelist[i] = fopen(filename, "w");

			if(global->useFEEspectrum) {
				sprintf(filename,"r%04u-FEEspectrum-class%ld-index.txt",global->runNumber,i);
				if(global->FEElogfp[i] != NULL)
					fclose(global->FEElogfp[i]);
				global->FEElogfp[i] = fopen(filename, "w");
				fprintf(global->FEElogfp[i], "Stack element, eventData->frameNumber, eventData->eventname\n");
			}

			if(global->useTimeTool) {
				sprintf(filename,"r%04u-TimeTool-class%ld-index.txt",global->runNumber,i);
				if(global->TimeToolLogfp[i] != NULL)
					fclose(global->TimeToolLogfp[i]);
				global->TimeToolLogfp[i] = fopen(filename, "w");
				fprintf(global->TimeToolLogfp[i], "Stack element, eventData->frameNumber, eventDaya->stackSlice, eventData->eventname\n");
			}
		}
    }
    pthread_mutex_unlock(&global->powderfp_mutex);
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
	 *  event->detector[detIndex].detectorZ holds the read-out value
	 *  fail=std::numeric_limits<float>::quiet_NaN();
	 */
	DETECTOR_LOOP {
		
		float detposnew;
		int update_camera_length = 0;

		// what's the detector "camera length" for this shot?
		if ( global->detector[detIndex].fixedCameraLengthMm != 0 ) {
			// If fixed detector camera length is provided, override it here... it's a bit of a hack for now...
			detposnew = global->detector[detIndex].fixedCameraLengthMm;
		} else {
			detposnew = eventData->detector[detIndex].detectorZ;
		}

		if ( !isnan(detposnew) ) {

			// New detector position = 0 could be an error
            if ( detposnew == 0 ) {
                detposnew = global->detector[detIndex].detposprev;
                printf("WARNING: detector position is zero, which could be an error\n"
                       "         will use previous position (%s=%f) instead...\n",global->detector[detIndex].detectorZpvname, detposnew);
            }
			
            //	Apply offsets
			//	When encoder reads -500mm, detector is at its closest possible
			//	position to the specimen, and is 79mm from the centre of the 
			//	8" flange where the injector is mounted.  
			//	The injector itself is about 4mm further away from the detector than this. 
            global->detector[detIndex].detposprev = detposnew;
            global->detector[detIndex].detectorEncoderValue = detposnew;
            global->detector[detIndex].detectorZ = detposnew + global->detector[detIndex].cameraLengthOffset;

            //	Round to the nearest two decimal places 
			//	(10 micron, much less than a pixel size) 
            global->detector[detIndex].detectorZ = floorf(global->detector[detIndex].detectorZ*100+0.5)/100;
            update_camera_length = 1;
        }	 
        
		//	What to do if there is no camera length information?  
		//	Keep skipping frames until this info is found?  
		//	For now, set a (non-zero) default camera length.
		if ( global->detector[detIndex].detectorZ == 0 ) {

            if ( global->detector[detIndex].defaultCameraLengthMm == 0 ) {
                printf("======================================================\n");
                printf("WARNING: Camera length %s is zero!\n", global->detector[detIndex].detectorZpvname);
                printf("If the problem persists, try setting the keyword\n");
                printf("defaultCameraLengthMm in your ini file\n"); 
                printf("======================================================\n");
                return;
            } 
            else {
                printf("MESSAGE: Setting default camera length (%gmm).\n",global->detector[detIndex].defaultCameraLengthMm);
                global->detector[detIndex].detectorZ = global->detector[detIndex].defaultCameraLengthMm;	
                update_camera_length = 1;
            }
        }
        
		
        /*
         * Recalculate reciprocal space geometry if the camera length has changed, 
         */
        if ( update_camera_length && ( global->detector[detIndex].detectorZprevious != global->detector[detIndex].detectorZ ) ) {
            // don't tinker with cheetahGlobal geometry while there are active threads...
            while (global->nActiveCheetahThreads > 0) 
                usleep(10000);
            
            printf("Camera length changed from %gmm to %gmm.\n", global->detector[detIndex].detectorZprevious,global->detector[detIndex].detectorZ);
            if ( isnan(eventData->wavelengthA ) ) {
                printf("MESSAGE: Bad wavelength data (NaN). Consider using defaultPhotonEnergyeV keyword.\n");
            }	
            global->detector[detIndex].detectorZprevious = global->detector[detIndex].detectorZ;
            for(long i=0; i<global->nDetectors; i++) 
                global->detector[detIndex].updateKspace(global, eventData->wavelengthA);
            
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
    if(eventData->pumpLaserDelay != 0) {
        global->pumpLaserDelay = eventData->pumpLaserDelay;
    }
    
    
    /*
     *  Copy over any remaining detector info
     */
    DETECTOR_LOOP {
        eventData->detector[detIndex].detectorZ = global->detector[detIndex].detectorZ;        
        
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
	pthread_mutex_lock(&global->process_mutex);
	/*
	 * In case people forget to turn on the beamline data.
	 */
	if (global->fixedPhotonEnergyeV > 0) {
		eventData->photonEnergyeV = global->fixedPhotonEnergyeV;
		eventData->wavelengthA = 12398.42/eventData->photonEnergyeV;
	}
   
	/* Further wavelength testing */
	if ( ! isfinite(eventData->photonEnergyeV ) ) {
		if ( global->defaultPhotonEnergyeV > 0 ) {
			eventData->photonEnergyeV = global->defaultPhotonEnergyeV;
			eventData->wavelengthA = 12398.42/eventData->photonEnergyeV;
		} else {
			printf("Bad value for photon energy.\n");
			printf("Try setting the keyword defaultPhotonEnergyeV or fixedPhotonEnergyeV\n");
			exit(1);
		}
	}

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
		pthread_mutex_unlock(&global->process_mutex);
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
		pthread_mutex_unlock(&global->process_mutex);
        pthread_t		thread;
        pthread_attr_t	threadAttribute;
        int				returnStatus;
        
        /*
         *  Wait until we have a spare thread in the thread pool
         *  If nothing happens for some time, assume we have some sort of thread lockup and keep going anyway
         */
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += global->threadTimeoutInSeconds;
		int sem_ret = sem_timedwait(&global->availableCheetahThreads, &ts);
		if(sem_ret == ETIMEDOUT){
			printf("\tApparent thread lock - no free thread for %d seconds.\n", global->threadTimeoutInSeconds);
			printf("\tGiving up and resetting the thread counter\n");
			global->unlockMutexes();
			global->nActiveCheetahThreads = 0;
			int semValue = 0;
			while(semValue < global->nThreads){
				sem_post(&global->availableCheetahThreads);
				sem_getvalue(&global->availableCheetahThreads, &semValue);
			}
				
		}
        
        // Set detached state
        pthread_attr_init(&threadAttribute);
        pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);

        // Create a new worker thread for this data frame
		// Lock acquired before creation to avoid race condition where nActiveThreads decremented before incremented
		pthread_mutex_lock(&global->nActiveThreads_mutex);
        eventData->threadNum = global->threadCounter;
        returnStatus = pthread_create(&thread, &threadAttribute, worker, (void *)eventData);

		if (returnStatus == 0) { // creation successful
			// Increment threadpool counter
			global->nActiveCheetahThreads += 1;
			global->threadCounter += 1;
		}
		else{
			sem_post(&global->availableCheetahThreads);
			printf("Error: thread creation failed (frame skipped)\n");
        }
		pthread_mutex_unlock(&global->nActiveThreads_mutex);
        pthread_attr_destroy(&threadAttribute);
//		pthread_mutex_lock(&global->process_mutex);
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
	global->waitForThreadsToFinish(5*60);
	
	//time_t	tstart, tnow;
	//time(&tstart);
    //double	dtime;
    //float	maxwait = 10*60.;
    //while(global->nActiveCheetahThreads > 0) {
	//	printf("Waiting for %li worker threads to terminate\n", global->nActiveCheetahThreads);
	//	usleep(100000);
	//	time(&tnow);
	//	dtime = difftime(tnow, tstart);
	//	if(dtime > maxwait) {
	//		printf("\t%li threads still active after waiting %f seconds\n", global->nActiveCheetahThreads, dtime);
	//		printf("\tGiving up and exiting anyway\n");
	//		global->unlockMutexes();
	//		break;
	//	}
    //}
    
    // Calculate mean photon energy
    global->meanPhotonEnergyeV = global->summedPhotonEnergyeV/global->nhitsandblanks;
    global->photonEnergyeVSigma = sqrt(global->summedPhotonEnergyeVSquared/global->nhitsandblanks - global->meanPhotonEnergyeV * global->meanPhotonEnergyeV);
    printf("Mean photon energy: %f eV\n", global->meanPhotonEnergyeV);
    printf("Sigma of photon energy: %f eV\n", global->photonEnergyeVSigma);
    
	
    // Save powder patterns and other stuff
	if(global->writeRunningSumsFiles){
		saveRunningSums(global);
		saveHistograms(global);
	}
	if(global->useFEEspectrum)
		saveSpectrumStacks(global);
	if(global->useTimeTool)
		saveTimeToolStacks(global);
	if(global->saveRadialStacks)
		saveRadialStacks(global);
	
	
    global->writeFinalLog();

    // Close all CXI files
	if(global->saveCXI)
		closeCXIFiles(global);

	
    // Save integrated run spectrum
    //saveIntegratedRunSpectrum(global);	<-- this was causing crashes (debug!)
    
	
    // Hitrate?
    if (global->nPowderClasses){
		printf("Hits: %li (%2.2f%%) ",global->nhits, 100.*( global->nhits / (float) global->nhitsandblanks));
		printf("with Npeaks ranging from %i to %i\n",global->nPeaksMin[1],global->nPeaksMax[1]);
		printf("Blanks: %li (%2.2f%%) ",global->nhitsandblanks-global->nhits, 100.*( (global->nhitsandblanks-global->nhits)/ (float) global->nhitsandblanks));
		printf("with Npeaks ranging from %i to %i\n",global->nPeaksMin[0],global->nPeaksMax[0]);
    }
	else {
		printf("%li hits (%2.2f%%)\n",global->nhits, 100.*( global->nhits / (float) global->nhitsandblanks));
    }
    printf("%li frames processed\n",global->nprocessedframes);

    
    
    // Cleanup

	// Destroy memory and destroy mutexes
	global->freeMemory();
	destroyStreakFinder(global);

    global->writeStatus("Finished");    
    printf("Cheetah clean exit\n");
}

/*
 *	Error handling
 *	Output directed to stdout so that it appears in batch system log files for debugging
 *	(stderr goes elsewhere)
 */
void cheetahDebug(const char *filename, int line, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	fprintf(stdout,"*********************************************\n");
	fprintf(stdout,"cheetahDebug(...) in libCheetah.cpp triggered\n");
	fprintf(stdout,"CHEETAH-DEBUG in %s:%d: ",filename,line);
	vfprintf(stdout,format,ap);
	fprintf(stdout,"*********************************************\n");
	va_end(ap);
	puts("");
}

void cheetahError(const char *filename, int line, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	fprintf(stdout,"*********************************************\n");
	fprintf(stdout,"cheetahError(...) in libCheetah.cpp triggered\n");
	fprintf(stdout,"CHEETAH-ERROR in %s:%d: ",filename,line);
	vfprintf(stdout,format,ap);
	fprintf(stdout,"*********************************************\n");
	va_end(ap);
	puts("");
	abort();
}




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
	if(returnStatus){
		ERROR("Failed to create python thread!");
	}
}




