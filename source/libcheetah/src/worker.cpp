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

	// Turn threadarg into a more useful form
	cGlobal			*global;
	cEventData		*eventData;
	int             hit = 0;
	eventData = (cEventData*) threadarg;
	global = eventData->pGlobal;

	std::vector<int> myvector;
	std::stringstream sstm;
	std::string result;
	std::ofstream outFlu;
	//std::ios_base::openmode mode;
	std::stringstream sstm1;
	std::ofstream outHit;


	//--------MONITORING---------//

	DEBUGL2_ONLY {
		DEBUG("Monitoring");
	}

	updateDatarate(global);


	//---INITIALIZATIONS-AND-PREPARATIONS---//

	DEBUGL2_ONLY {
		DEBUG("Initializations and preparations");
	}

	/*
	 *  Inside-thread speed test
	 */
	if(global->ioSpeedTest==3) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #3 (exiting within thread)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
    
	// Nasty fudge for evr41 (i.e. "optical pump laser is on") signal when only 
	// Acqiris data (i.e. temporal profile of the laser diode signal) is available...
	// Hopefully this never happens again... 
	if ( global->fudgeevr41 == 1 ) {
		evr41fudge(eventData,global);	
	}
	
	// Create a unique name for this event
	nameEvent(eventData, global);

	// GMD
	calculateGmd(eventData);
	if (gmdBelowThreshold(eventData, global)) {
		printf("r%04u:%li Skipping frame (GMD below threshold: %f mJ < %f mJ).\n",global->runNumber, eventData->frameNumber,eventData->gmd,global->gmdThreshold);
		goto cleanup; 
	}

	// Initialise pixelmask with pixelmask_shared
	initPixelmask(eventData, global);
	
	// Initialise raw data array (float) THIS MIGHT SLOW THINGS DOWN, WE MIGHT WANT TO CHANGE THIS
	initRaw(eventData, global);
	
	//---DETECTOR-CORRECTION---//

	DEBUGL2_ONLY {
		DEBUG("Detector correction");
	}

	// Initialise data_detCorr with data_raw16
	initDetectorCorrection(eventData,global);

	// Check for saturated pixels before applying any other corrections
	checkSaturatedPixels(eventData, global);

	// Subtract darkcal image (static electronic offsets)
	subtractDarkcal(eventData, global);
	// If no darkcal file: Subtract persistent background here (background = photon background + static electronic offsets)
	subtractPersistentBackground(eventData, global);

	// Fix CSPAD artefacts:
	// Subtract common mode offsets (electronic offsets)
	// cmModule = 1
	// (these corrections will be automatically skipped for any non-CSPAD detector)
	cspadModuleSubtract(eventData, global);
	cspadSubtractUnbondedPixels(eventData, global);
	cspadSubtractBehindWires(eventData, global);

	// Fix pnCCD artefacts:
	// pnCCD offset correction (read out artifacts prominent in lines with high signal)
	// pnCCD wiring error (shift in one set of rows relative to another - and yes, it's a wiring error).
	// pnCCD signal drop in every second line (fast changing dimension) can be fixed by interpolation and/or masking of the affected lines
	//  (these corrections will be automatically skipped for any non-pnCCD detector)
    pnccdModuleSubtract(eventData, global);
	pnccdOffsetCorrection(eventData, global);
	pnccdFixWiringError(eventData, global);
	pnccdLineInterpolation(eventData, global);
	pnccdLineMasking(eventData, global);
	
	// Apply gain correction
	applyGainCorrection(eventData, global);
	
    // Apply polarization correction
	applyPolarizationCorrection(eventData, global);
    
    // Apply solid angle correction
	applySolidAngleCorrection(eventData, global);
	
	// Zero out bad pixels
	setBadPixelsToZero(eventData, global);
 
	//  Inside-thread speed test
	if(global->ioSpeedTest==4) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test 4 (after detector correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
  
	// This bit looks at the inner part of the detector first to see whether it's worth looking at the rest
	// Useful for local background subtraction (which is effective but slow)
	if(global->hitfinder && global->hitfinderFastScan && (global->hitfinderAlgorithm==3 || global->hitfinderAlgorithm==6 || global->hitfinderAlgorithm==8)) {
		hit = hitfinderFastScan(eventData, global);
		if(hit)
			goto localBGCalculated;
		else
			goto hitknown;
	}

	// Local background subtraction 
	subtractLocalBackground(eventData, global);

localBGCalculated:
	// Subtract residual common mode offsets (cmModule=2) 
	cspadModuleSubtract2(eventData, global);
  
	// Set bad pixels to zero
	setBadPixelsToZero(eventData, global);
	
	// Identify hot pixels and set them to zero
	identifyHotPixels(eventData, global);
	calculateHotPixelMask(eventData,global);
	setHotPixelsToZero(eventData,global);
	
	// Inside-thread speed test
	if(global->ioSpeedTest==5) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #5 (photon background correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
    
	//---PHOTON-BACKGROUND-CORRECTION---//

	DEBUGL2_ONLY {
		DEBUG("Background correction");
	}
	  
	// Initialise data_detPhotCorr with data_detCorr
	initPhotonCorrection(eventData,global);

	// If darkcal file available: Subtract persistent background here (background = photon background)
	subtractPersistentBackground(eventData, global);
	// Radial background subtraction (!!! I assume that the radial background subtraction subtracts a photon background, therefore moved here to the end - not to be crunched with detector /Max)
	subtractRadialBackground(eventData, global);

	//---HITFINDING---//

	if(global->hitfinder && (global->hitfinderForInitials ||
							 !(eventData->threadNum < global->nInitFrames || !global->calibrated))){ 

		DEBUGL2_ONLY {
			DEBUG("Hit finding");
		}

		hit = hitfinder(eventData, global);
		eventData->hit = hit;

		pthread_mutex_lock(&global->hitclass_mutex);
		for (int coord = 0; coord < 3; coord++) {
			if (eventData->nPeaks < 100) continue;
			global->hitClasses[coord][std::make_pair(eventData->samplePos[coord] * 1000, hit)]++;
		}
		pthread_mutex_unlock(&global->hitclass_mutex);
		sortPowderClass(eventData, global);		
	}

hitknown: 
	//---PROCEDURES-DEPENDENT-ON-HIT-TAG---//
	// Slightly wrong that all initial frames are blanks
	// when hitfinderForInitials is 0
    /*
     *	Sort event into different classes (eg: laser on/off)
     */
  
	DEBUGL2_ONLY {
		DEBUG("Procedures depending on hit tag");
	}
	
	// Update running backround estimate based on non-hits
	updateBackgroundBuffer(eventData, global, hit); 
	calculatePersistentBackground(eventData,global);  

	// Identify noisy pixels
	updateNoisyPixelBuffer(eventData,global,hit);
	calculateNoisyPixelMask(eventData,global);

	// Skip first set of frames to build up running estimate of background...
	if (eventData->threadNum < global->nInitFrames || !global->calibrated){
		// Update running backround estimate based on non-hits and calculate background from buffer
		global->updateCalibrated();
		printf("r%04u:%li (%3.1fHz): Digesting initial frames (npeaks=%i)\n", global->runNumber, eventData->threadNum,global->datarateWorker, eventData->nPeaks);
		goto cleanup;
	}
    
	// Inside-thread speed test
	if(global->ioSpeedTest==6) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #6 (after hitfinding)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
    
	//---ASSEMBLE-AND-ACCUMULATE-DATA---//

	DEBUGL2_ONLY {
		DEBUG("Assemble and accumulate data");
	}

	// Inside-thread speed test
	if(global->ioSpeedTest==7) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #7 (after powder sum and reverting images)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}

	// Assemble, downsample and radially average current frame
	assemble2D(eventData, global);
	downsample(eventData, global);
  
	// Powder
	// Maintain a running sum of data (powder patterns)
	addToPowder(eventData, global);
	
	// Calculate the one dimesional beam spectrum
	integrateSpectrum(eventData, global);
	integrateRunSpectrum(eventData, global);
  
	// Update GMD average
	updateAvgGmd(eventData,global);

	// Integrate pattern
	integratePattern(eventData,global);

	// Inside-thread speed test
	if(global->ioSpeedTest==8) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #8 (radial average and spectrum)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}

	// Histogram
	addToHistogram(eventData, global);

	// Inside-thread speed test
	if(global->ioSpeedTest==9) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #9 (After histograms)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}

	//---WRITE-DATA-TO-H5---//

	DEBUGL2_ONLY {
		DEBUG("Write data to h5");
	}

	updateDatarate(global);  

	if(global->saveCXI==1){
		writeCXIHitstats(eventData, global);
	}

	eventData->writeFlag =  ((hit && global->saveHits) || (!hit && global->saveBlanks) || ((global->hdf5dump > 0) && ((eventData->frameNumber % global->hdf5dump) == 0) ));

	// If this is a hit, write out to our favourite HDF5 format
	// Put here anything only needed for data saved to file (why waste the time on events that are not saved)
	// eg: only assemble 2D images, 2D masks and downsample if we are actually saving this frame
	
	// Update central hit counter
	pthread_mutex_lock(&global->nhits_mutex);	
    global->nhitsandblanks++;
	if(hit) {
		global->nhits++;
		global->nrecenthits++;
	}
	pthread_mutex_unlock(&global->nhits_mutex);

	if(eventData->writeFlag){
		// one CXI or many H5?
		if(global->saveCXI){
			printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing %s to %s (npeaks=%i)\n",global->runNumber, 
				   eventData->threadNum, global->processRateMonitor.getRate(), 
				   100.*( global->nhits / (float) global->nhitsandblanks), eventData->eventStamp, 
				   global->cxiFilename, eventData->nPeaks);
		    //pthread_mutex_lock(&global->saveCXI_mutex);
			writeCXI(eventData, global);
			//pthread_mutex_unlock(&global->saveCXI_mutex);
		} else {
			printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing to: %s.h5 (npeaks=%i)\n",global->runNumber, eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nhitsandblanks), eventData->eventStamp, eventData->nPeaks);
			writeHDF5(eventData, global);
		}
	}
	// This frame is not going to be saved, but print anyway
	else {
		printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Processed (npeaks=%i)\n", global->runNumber,eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nhitsandblanks), eventData->nPeaks);
	}

	// FEE spectrometer data stack 
	// (needs knowledge of subdirectory for file list, which is why it's done here)
	addFEEspectrumToStack(eventData, global, hit);
    
	// If this is a hit, write out peak info to peak list file	
	if(hit && global->savePeakInfo) {
		writePeakFile(eventData, global);
	}

	//---LOGBOOK-KEEPING---//

	DEBUGL2_ONLY {
		DEBUG("Logbook keeping");
	}

	writeLog(eventData, global);
  
	// Inside-thread speed test
	if(global->ioSpeedTest==10) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #1 (after saving frames)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
	

	//---CLEANUP-AND-EXIT----//
cleanup:

	DEBUGL2_ONLY {
		DEBUG("Clean up and exit");

	}

	// Save accumulated data periodically
    pthread_mutex_lock(&global->saveinterval_mutex);
	// Update counters
    global->nprocessedframes += 1;
	global->nrecentprocessedframes += 1;
	// Save some types of information from time to timeperiodic powder patterns
	if(global->saveInterval!=0 && (global->nprocessedframes%global->saveInterval)==0 && (global->nprocessedframes > global->detector[0].startFrames+50) ){
		DEBUG3("Save data.");
		// Assemble, downsample and radially average powder
		assemble2DPowder(global);
		downsamplePowder(global);
		calculateRadialAveragePowder(global);
		// Save accumulated data
		if(global->saveCXI){
			writeAccumulatedCXI(global);
		} 
		if(global->writeRunningSumsFiles){
			saveRunningSums(global);
			saveHistograms(global);
			saveSpectrumStacks(global);
		}
		global->updateLogfile();
		global->writeStatus("Not finished");
	}
	pthread_mutex_unlock(&global->saveinterval_mutex);

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
	double * Vtof = &(t->tofDetector[0].voltage[0]);
	int i;
	double Vtot = 0;
	double Vmax = 0;
	int tCounts = 0;
	for(i=g->tofDetector[0].hitfinderMinSample; i<g->tofDetector[0].hitfinderMaxSample; i++){
		Vtot += Vtof[i];
		if ( Vtof[i] > Vmax ) Vmax = Vtof[i];
		if ( Vtof[i] >= g->tofDetector[0].hitfinderThreshold ) tCounts++;
	}
	

	bool acqLaserOn = false;
	if ( tCounts >= 1 ) {
		acqLaserOn = true;
	}
	//if ( acqLaserOn ) printf("acqLaserOn = true\n"); else printf("acqLaserOn = false\n");
	//if ( t->pumpLaserCode ) printf("pumpLaserCode = true\n"); else printf("pumpLaserCode = false\n");
	if ( acqLaserOn != t->pumpLaserCode ) {
		if ( acqLaserOn ) {
			printf("MESSAGE: Acqiris and evr41 disagree.  We trust acqiris (set evr41 = 1 )\n");
		} else {
			printf("MESSAGE: Acqiris and evr41 disagree.  We trust acqiris (set evr41 = 0 )\n");
		}
		t->pumpLaserCode = acqLaserOn;
	}
}


double difftime_timeval(timeval t1, timeval t2)
{
	return ((t1.tv_sec - t2.tv_sec) + (t1.tv_usec - t2.tv_usec) / 1000000.0 );
}

void updateDatarate(cGlobal *global){

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

