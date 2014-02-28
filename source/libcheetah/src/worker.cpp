/*
 *  worker.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "cheetah.h"
#include "cheetahmodules.h"
#include "median.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

/*
 *	Worker thread function for processing each cspad data frame
 */
void *worker(void *threadarg) {

    /*
     *	Turn threadarg into a more useful form
     */
    cGlobal			*global;
    cEventData		*eventData;
    int             hit = 0;
	int				powderClass = 0;
    eventData = (cEventData*) threadarg;
    global = eventData->pGlobal;
	
    std::vector<int> myvector;
    std::stringstream sstm;
    std::string result;
    std::ofstream outFlu;
    //std::ios_base::openmode mode;
    std::stringstream sstm1;
    std::ofstream outHit;

    
    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==3) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #3 (exiting within thread)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}
    

    
    /*
     * Andy's nasty fudge for evr41 (i.e. "optical pump laser is on") signal when only
     * Acqiris data (i.e. temporal profile of the laser diode signal) is available...
     * Hopefully this never happens again... 
     */
    if ( global->fudgeevr41 == 1 ) {
        evr41fudge(eventData,global);	
    }
	
    /*
     *	Create a unique name for this event
     */
    nameEvent(eventData, global);
	
    /*
     * Copy pixelmask_shared into pixelmask
     * and raw detector data into corrected array as starting point for corrections
     */
    DETECTOR_LOOP {
        for(long i=0;i<global->detector[detID].pix_nn;i++){
            eventData->detector[detID].pixelmask[i] = global->detector[detID].pixelmask_shared[i];
            eventData->detector[detID].corrected_data[i] = eventData->detector[detID].raw_data[i];
        }
    }
    
    

    // Init background buffer
    initBackgroundBuffer(eventData, global);

    
    
    /*
     * Check for saturated pixels before applying any other corrections
     */
    checkSaturatedPixels(eventData, global);

	
    /*
     *	Subtract darkcal image (static electronic offsets)
     */
    subtractDarkcal(eventData, global);

    /*
     *	Subtract common mode offsets (electronic offsets)
     *	cmModule = 1
     */
    cspadModuleSubtract(eventData, global);
    cspadSubtractUnbondedPixels(eventData, global);
    cspadSubtractBehindWires(eventData, global);

	
    /*
     *	Apply gain correction
     */
    applyGainCorrection(eventData, global);

	
    /*
     *	Apply bad pixel map
     */
    applyBadPixelMask(eventData, global);
	
	
    /*
     *  Fix pnCCD errors:
     *      pnCCD offset correction (read out artifacts prominent in lines with high signal)
     *      pnCCD wiring error (shift in one set of rows relative to another - and yes, it's a wiring error).
     *  (these corrections will be automatically skipped for any non-pnCCD detector)
     */
    pnccdOffsetCorrection(eventData, global);
    pnccdFixWiringError(eventData, global);
	

	
    /*
     *	Keep memory of data with only detector artefacts subtracted 
     *	(possibly needed later)
     */
    DETECTOR_LOOP {
        //memcpy(eventData->detector[detID].detector_corrected_data, eventData->detector[detID].corrected_data, global->detector[detID].pix_nn*sizeof(float));
		
        for(long i=0;i<global->detector[detID].pix_nn;i++){
            eventData->detector[detID].detector_corrected_data[i] = eventData->detector[detID].corrected_data[i];
            eventData->detector[detID].corrected_data_int16[i] = (int16_t) lrint(eventData->detector[detID].corrected_data[i]);
        }
    }
    

    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==4) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test 4 (after detector correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}
    
	
	/*
	 *	This bit looks at the inner part of the detector first to see whether it's worth looking at the rest
	 *	Useful for local background subtraction (which is effective but slow)
	 */
	if(global->hitfinder && global->hitfinderFastScan && (global->hitfinderAlgorithm==3 || global->hitfinderAlgorithm==6)) {
		hit = hitfinderFastScan(eventData, global);
		if(hit)
			goto localBG;
		else
			goto hitknown;
	}
	
    
	/*
	 *	Subtract persistent photon background
	 */
	subtractPersistentBackground(eventData, global);


	/*
	 *	Radial background subtraction
	 */
    subtractRadialBackground(eventData, global);

	/*
	 *	Local background subtraction
	 */
	subtractLocalBackground(eventData, global);
	
			
localBG:
	
	/*
	 *	Subtract residual common mode offsets (cmModule=2)
	 */
	cspadModuleSubtract2(eventData, global);

	
	/*
	 *	Apply bad pixels
	 */
	applyBadPixelMask(eventData, global);
	
	
	/*
	 *	Identify and kill hot pixels
	 */
	identifyHotPixels(eventData, global);
	calculateHotPixelMask(global);
	applyHotPixelMask(eventData,global);


	
    
    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==5) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #5 (photon background correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}
    
    
    /*
     *	Skip first set of frames to build up running estimate of background...
     */
    DETECTOR_LOOP {
		if (eventData->threadNum < global->detector[detID].startFrames || 
            (global->detector[detID].useSubtractPersistentBackground && global->detector[detID].bgCounter < global->detector[detID].bgMemory) ||
            (global->detector[detID].useAutoHotpixel && global->detector[detID].hotpixCounter < global->detector[detID].hotpixRecalc) ) {
                updateBackgroundBuffer(eventData, global, 0);
                updateHaloBuffer(eventData,global,0);
                printf("r%04u:%li (%3.1fHz): Digesting initial frames\n", global->runNumber, eventData->threadNum,global->datarateWorker);
            goto cleanup;
		}
    }

	
	/*
	 *	Hitfinding
	 */
	if(global->hitfinder){
		hit = hitfinder(eventData, global);
		eventData->hit = hit;
	}
	
hitknown:
	/*
	 *	Sort event into different classes (eg: laser on/off)
	 */
	sortPowderClass(eventData, global);
		
		
	/*
	 *	Identify halo pixels
	 */
	updateHaloBuffer(eventData,global,hit);
	calculateHaloPixelMask(global);
	
	
	/*
	 *	Update running backround estimate based on non-hits
	 */
	updateBackgroundBuffer(eventData, global, hit); 
	
	
    
    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==6) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #6 (after hitfinding)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}
    
    
    
	/*
	 *	Maintain a running sum of data (powder patterns) with whatever background subtraction has been applied to date.
	 */
    if(global->powderSumWithBackgroundSubtraction)
        addToPowder(eventData, global);

	
    

	/*
	 *	Revert to uncorrected data
	 */
    // Revert to data without photon background subtracted, only detector corrections applied
	DETECTOR_LOOP {
		if(global->detector[detID].saveDetectorCorrectedOnly) 
		  memcpy(eventData->detector[detID].corrected_data, eventData->detector[detID].detector_corrected_data, global->detector[detID].pix_nn*sizeof(float));
	}
	
	
    // Revert to raw detector data
	DETECTOR_LOOP {
		if(global->detector[detID].saveDetectorRaw)
			for(long i=0;i<global->detector[detID].pix_nn;i++)
				eventData->detector[detID].corrected_data[i] = eventData->detector[detID].raw_data[i];
	}
	
	
    /*
     *	Keep int16 copy of corrected data (needed for saving images)
     */
    DETECTOR_LOOP {
        for(long i=0;i<global->detector[detID].pix_nn;i++){
            eventData->detector[detID].corrected_data_int16[i] = (int16_t) lrint(eventData->detector[detID].corrected_data[i]);
        }
    }


    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==7) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #7 (after powder sum and reverting images)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}
    

    /*
	 *	Maintain a running sum of data (powder patterns) without whatever background subtraction has been for hitfinding.
	 */
    if(!global->powderSumWithBackgroundSubtraction)
        addToPowder(eventData, global);

    
    /*
     *  Calculate radial average
     *  Maintain radial average stack
     */
    calculateRadialAverage(eventData, global);
    addToRadialAverageStack(eventData, global);

	
		
    /*
     * calculate the one dimesional beam spectrum from CXI camera
     */
    integrateSpectrum(eventData, global);
    integrateRunSpectrum(eventData, global);
    
	
    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==8) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #8 (radial average and spectrum)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}

    
    /*
     *	Maintain a running sum of data (powder patterns)
     *    and strongest non-hit and weakest hit
     */
    addToHistogram(eventData, global);


    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==9) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #9 (After histograms)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}

logfile:
	updateDatarate(eventData,global);

    /*
     *	If this is a hit, write out to our favourite HDF5 format
     *
     *  Put here anything only needed for data saved to file (why waste the time on events that are not saved)
     *  eg: only assemble 2D images, 2D masks and downsample if we are actually saving this frame
     */
	if( (hit && global->savehits) || ((global->hdf5dump > 0) && ((eventData->frameNumber % global->hdf5dump) == 0) ) ){
        
        //  Assemble quadrants into a 'realistic' 2D image and downsample if requested
        assemble2Dimage(eventData, global);
        assemble2Dmask(eventData, global);
        downsample(eventData,global);
        
        // Which save format?
        if(global->saveCXI==1){
            pthread_mutex_lock(&global->saveCXI_mutex);
            writeCXI(eventData, global);
            pthread_mutex_unlock(&global->saveCXI_mutex);
            printf("r%04u:%li (%2.1lf Hz): Writing %s to %s slice %u (npeaks=%i)\n",global->runNumber, eventData->threadNum,global->datarateWorker, eventData->eventname, global->cxiFilename, eventData->stackSlice, eventData->nPeaks);
        }
        else {
            writeHDF5(eventData, global);
            printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing to: %s (npeaks=%i)\n",global->runNumber, eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nprocessedframes), eventData->eventname, eventData->nPeaks);
        }
    }
    // This frame is not going to be saved, but print anyway
    else {
        printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Processed (npeaks=%i)\n", global->runNumber,eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nprocessedframes), eventData->nPeaks);
    }
    

	/*
	 *	FEE spectrometer data stack 
	 *	(needs knowledge of subdirectory for file list, which is why it's done here)
	 */
	addFEEspectrumToStack(eventData, global, hit);
    
	

    /*
     *	If this is a hit, write out peak info to peak list file
     */
    if(hit && global->savePeakInfo) {
        writePeakFile(eventData, global);
    }
	

	
    /*
    *	Write out information on each frame to a log file
    */
    pthread_mutex_lock(&global->framefp_mutex);
    fprintf(global->framefp, "%s/%s, ", eventData->eventSubdir, eventData->eventname);
    fprintf(global->framefp, "%li, ", eventData->frameNumber);
    fprintf(global->framefp, "%li, ", eventData->threadNum);
    fprintf(global->framefp, "%i, ", eventData->hit);
    fprintf(global->framefp, "%i, ", eventData->powderClass);
    fprintf(global->framefp, "%g, ", eventData->photonEnergyeV);
    fprintf(global->framefp, "%g, ", eventData->wavelengthA);
    fprintf(global->framefp, "%g, ", eventData->gmd1);
    fprintf(global->framefp, "%g, ", eventData->gmd2);
    fprintf(global->framefp, "%g, ", eventData->detector[0].detectorZ);
    fprintf(global->framefp, "%i, ", eventData->energySpectrumExist);
    fprintf(global->framefp, "%d, ", eventData->nPeaks);
    fprintf(global->framefp, "%g, ", eventData->peakNpix);
    fprintf(global->framefp, "%g, ", eventData->peakTotal);
    fprintf(global->framefp, "%g, ", eventData->peakResolution);
    fprintf(global->framefp, "%g, ", eventData->peakDensity);
    fprintf(global->framefp, "%d, ", eventData->laserEventCodeOn);
    fprintf(global->framefp, "%g, ", eventData->laserDelay);
    fprintf(global->framefp, "%d\n", eventData->pumpLaserOn);
    pthread_mutex_unlock(&global->framefp_mutex);

    // Keep track of what has gone into each image class
	powderClass = eventData->powderClass;
    if(global->powderlogfp[powderClass] != NULL) {
        pthread_mutex_lock(&global->powderfp_mutex);
        fprintf(global->powderlogfp[powderClass], "%s/%s, ", eventData->eventSubdir, eventData->eventname);
        fprintf(global->powderlogfp[powderClass], "%li, ", eventData->frameNumber);
        fprintf(global->powderlogfp[powderClass], "%li, ", eventData->threadNum);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->photonEnergyeV);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->wavelengthA);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->detector[0].detectorZ);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->gmd1);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->gmd2);
        fprintf(global->powderlogfp[powderClass], "%i, ", eventData->energySpectrumExist);
        fprintf(global->powderlogfp[powderClass], "%d, ", eventData->nPeaks);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakNpix);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakTotal);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakResolution);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakDensity);
        fprintf(global->powderlogfp[powderClass], "%d, ", eventData->laserEventCodeOn);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->laserDelay);
        fprintf(global->powderlogfp[powderClass], "%d\n", eventData->pumpLaserOn);
        pthread_mutex_unlock(&global->powderfp_mutex);
    }
    /*
     *  Inside-thread speed test
     */
    if(global->ioSpeedTest==10) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #1 (after saving frames)\n", global->runNumber, eventData->frameNumber, global->datarate);
        goto cleanup;
	}
    

    
	
  /*
   *	Cleanup and exit
   */
 cleanup:
  // Decrement thread pool counter by one
  pthread_mutex_lock(&global->nActiveThreads_mutex);
  global->nActiveThreads -= 1;
  pthread_mutex_unlock(&global->nActiveThreads_mutex);
    
  // Free memory only if running multi-threaded
  if(eventData->useThreads == 1) {
    cheetahDestroyEvent(eventData);
    pthread_exit(NULL);
  }
  else {
    return(NULL);
  }
}


/*
 * Nasty little bit of code that aims to toggle the evr41 signal based on the Acqiris
 * signal.  Very simple: scan along the Acqiris trace (starting from the ini keyword 
 * hitfinderTOFMinSample and ending at hitfinderTOFMaxSample) and check that there is 
 * at least one sample above the threshold set by the hitfinderTOFThresh keyword. Oh,
 * and don't forget to specify the Acqiris channel with:
 * tofChannel=0
 * And you'll need to do this as well:
 * fudgeevr41=1
 * tofName=CxiSc1
 *
 * Agreed - doesn't sound very robust.  As far as I can tell at the moment, only a single
 * sample rises above threshold when the laser trigger is on... maybe bad sampling interval?
 * Or, I could be wrong...
 * 
 * -Rick  
 */
void evr41fudge(cEventData *t, cGlobal *g){
	
	if ( g->TOFPresent == 0 ) {
		//printf("Acqiris not present; can't fudge EVR41...\n");
		return;
	}
 
	//int nCh = g->AcqNumChannels;
	//int nSamp = g->AcqNumSamples;
	double * Vtof = t->TOFVoltage;
	int i;
	double Vtot = 0;
	double Vmax = 0;
	int tCounts = 0;
	for(i=g->hitfinderTOFMinSample; i<g->hitfinderTOFMaxSample; i++){
		Vtot += Vtof[i];
		if ( Vtof[i] > Vmax ) Vmax = Vtof[i];
		if ( Vtof[i] >= g->hitfinderTOFThresh ) tCounts++;
	}
	

	bool acqLaserOn = false;
	if ( tCounts >= 1 ) {
		acqLaserOn = true;
	}
	//if ( acqLaserOn ) printf("acqLaserOn = true\n"); else printf("acqLaserOn = false\n");
	//if ( t->laserEventCodeOn ) printf("laserEventCodeOn = true\n"); else printf("laserEventCodeOn = false\n");
	if ( acqLaserOn != t->laserEventCodeOn ) {
		if ( acqLaserOn ) {
			printf("MESSAGE: Acqiris and evr41 disagree.  We trust acqiris (set evr41 = 1 )\n");
		} else {
			printf("MESSAGE: Acqiris and evr41 disagree.  We trust acqiris (set evr41 = 0 )\n");
		}
		t->laserEventCodeOn = acqLaserOn;
	}
}


double difftime_timeval(timeval t1, timeval t2)
{
  return ((t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec) / 1000000.0 );
}

void updateDatarate(cEventData *eventData, cGlobal *global){

  timeval timevalNow;
  double mem = global->datarateWorkerMemory;
  double dtNew,dtNow;
  gettimeofday(&timevalNow, NULL);
  
  pthread_mutex_lock(&global->datarateWorker_mutex);
  if (timercmp(&timevalNow,&global->datarateWorkerTimevalLast,!=)){
    dtNow = difftime_timeval(timevalNow,global->datarateWorkerTimevalLast) / (1+global->datarateWorkerSkipCounter);
    dtNew = 1/global->datarateWorker * mem + dtNow * (1-mem);
    global->datarateWorker = 1/dtNew;
    global->datarateWorkerTimevalLast = timevalNow;
    global->datarateWorkerSkipCounter = 0;
  }else{
    global->datarateWorkerSkipCounter += 1;
  }
  pthread_mutex_unlock(&global->datarateWorker_mutex);

  
}


