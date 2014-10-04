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


	//--------MONITORING---------//

	DEBUGL2_ONLY {
		DEBUG("Monitoring");
	}

	updateDatarate(eventData,global);


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
	
	// Copy pixelmask_shared into pixelmask and raw detector data into corrected array as starting point for corrections
	DETECTOR_LOOP {
		for(long i=0;i<global->detector[detID].pix_nn;i++){
			eventData->detector[detID].pixelmask[i] = global->detector[detID].pixelmask_shared[i];
			eventData->detector[detID].corrected_data[i] = eventData->detector[detID].raw_data[i];
		}
	}
	
	//---DETECTOR-CORRECTION---//

	DEBUGL2_ONLY {
		DEBUG("Detector correction");
	}

	// Check for saturated pixels before applying any other corrections
	checkSaturatedPixels(eventData, global);
	checkPnccdSaturatedPixels(eventData, global);

	// If no darkcal file: Init background buffer here (background = photon background + static electronic offsets)
	initBackgroundBuffer(eventData,global);
	// If no darkcal file: Subtract persistent background here (background = photon background + static electronic offsets)
	subtractPersistentBackground(eventData, global);
	// Subtract darkcal image (static electronic offsets)
	subtractDarkcal(eventData, global);

	// Subtract common mode offsets (electronic offsets)
	// cmModule = 1
	cspadModuleSubtract(eventData, global);
	cspadSubtractUnbondedPixels(eventData, global);
	cspadSubtractBehindWires(eventData, global);

	
	// Fix pnCCD errors:
	// pnCCD offset correction (read out artifacts prominent in lines with high signal)
	// pnCCD wiring error (shift in one set of rows relative to another - and yes, it's a wiring error).
	// pnCCD signal drop in every second line (fast changing dimension) is fixed by interpolation
	//  (these corrections will be automatically skipped for any non-pnCCD detector)
	pnccdOffsetCorrection(eventData, global);
	pnccdFixWiringError(eventData, global);
	pnccdLineInterpolation(eventData, global);
	pnccdLineMasking(eventData, global);
	
	// Apply gain correction
	applyGainCorrection(eventData, global);
	
	// Apply bad pixel map
	applyBadPixelMask(eventData, global);
 
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
	// Keep memory of data with only detector artefacts subtracted (possibly needed later)
	DETECTOR_LOOP {
		memcpy(eventData->detector[detID].detector_corrected_data, eventData->detector[detID].corrected_data, global->detector[detID].pix_nn*sizeof(float));
	}

	// Subtract residual common mode offsets (cmModule=2) 
	cspadModuleSubtract2(eventData, global);
  
	// Apply bad pixels
	applyBadPixelMask(eventData, global);
	
	// Identify and kill hot pixels
	identifyHotPixels(eventData, global);
	calculateHotPixelMask(eventData,global);
	applyHotPixelMask(eventData,global);
	
	// Inside-thread speed test
	if(global->ioSpeedTest==5) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #5 (photon background correction)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
    
	//---BACKGROUND-CORRECTION---//

	DEBUGL2_ONLY {
		DEBUG("Background correction");
	}
	  
	// If darkcal file available: Init background buffer here (background = photon background)
	initBackgroundBuffer(eventData,global);
	// If darkcal file available: Subtract persistent background here (background = photon background)
	subtractPersistentBackground(eventData, global);
	// Radial background subtraction (!!! I assume that the radial background subtraction subtracts a photon background, therefore moved here to the end - not to be crunched with detector )
	subtractRadialBackground(eventData, global);

	//---HITFINDING---//

	DEBUGL2_ONLY {
		DEBUG("Hit finding");
	}

	if(global->hitfinder && (global->hitfinderForInitials ||
							 !(eventData->threadNum < global->nInitFrames || !global->calibrated))){ 
		hit = hitfinder(eventData, global);
		//if (global->hitfinderInvertHit == 1){
		//	if ( hit == 1 )
		//		hit = 0;
		//	else
		//		hit = 1;
		//}
		eventData->hit = hit;

		pthread_mutex_lock(&global->hitclass_mutex);
		for (int coord = 0; coord < 3; coord++) {
			if (eventData->nPeaks < 100) continue;
			global->hitClasses[coord][std::make_pair(eventData->samplePos[coord] * 1000, hit)]++;
		}
		pthread_mutex_unlock(&global->hitclass_mutex);
		sortPowderClass(eventData, global);		
	}

	//---PROCEDURES-DEPENDENT-ON-HIT-TAG---//
hitknown: 
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

	// Identify halo pixels
	updateHaloBuffer(eventData,global,hit);
	calculateHaloPixelMask(eventData,global);

	// Skip first set of frames to build up running estimate of background...
	if (eventData->threadNum < global->nInitFrames || !global->calibrated){
		// Update running backround estimate based on non-hits and calculate background from buffer
		updateBackgroundBuffer(eventData, global, 0); 
		calculatePersistentBackground(eventData,global);  
		global->updateCalibrated();
		printf("r%04u:%li (%3.1fHz): Digesting initial frames (npeaks=%i)\n", global->runNumber, eventData->threadNum,global->datarateWorker, eventData->nPeaks);
		goto cleanup;
	}  else {
		// Update running backround estimate based on non-hits and calculate background from buffer
		updateBackgroundBuffer(eventData, global, hit); 
		calculatePersistentBackground(eventData,global);  
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

	// Maintain a running sum of data (powder patterns) with whatever background subtraction has been applied to date.
	if(global->powderSumWithBackgroundSubtraction){
		// If we want assembled powders etc. we need to do the assembly and downsampling here. Otherwise we might skip it if image is not going to be saved
		if(global->assemblePowders){
			// Assemble to realistic image
			assemble2Dimage(eventData, global);
			assemble2Dmask(eventData, global);
			// Downsample assembled image
			downsample(eventData,global);
		}
		addToPowder(eventData, global);
	}

	DETECTOR_LOOP {
		// Revert to raw detector data
		if(global->detector[detID].saveDetectorRaw){
			for(long i=0;i<global->detector[detID].pix_nn;i++){
				eventData->detector[detID].corrected_data[i] = eventData->detector[detID].raw_data[i];
			}
		}
		// Revert to detector-corrections-only data if we don't want to export data with photon background subtracted
		else if (global->detector[detID].saveDetectorCorrectedOnly) {
			memcpy(eventData->detector[detID].corrected_data, eventData->detector[detID].detector_corrected_data, global->detector[detID].pix_nn*sizeof(float));
		}
	}


	// Inside-thread speed test
	if(global->ioSpeedTest==7) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #7 (after powder sum and reverting images)\n", global->runNumber, eventData->frameNumber, global->datarate);
		goto cleanup;
	}
  
	// Maintain a running sum of data (powder patterns) without whatever background subtraction has been for hitfinding.
	if(!global->powderSumWithBackgroundSubtraction){
		// If we want assembled powders etc. we need to do the assembly and downsampling here. Otherwise we might skip it if image is not going to be saved
		if(global->assemblePowders){
			// Assemble to realistic image
			assemble2Dimage(eventData, global);
			assemble2Dmask(eventData, global);
			// Downsample assembled image
			downsample(eventData,global);
		}
		addToPowder(eventData, global);
	}

	// Calculate radial average and maintain radial average stack
	calculateRadialAverage(eventData, global); 
	addToRadialAverageStack(eventData, global);
	
	// calculate the one dimesional beam spectrum
	integrateSpectrum(eventData, global);
	integrateRunSpectrum(eventData, global);
  
	// update GMD average
	updateAvgGMD(eventData,global);

	// integrate pattern
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

	updateDatarate(eventData,global);  

	if(global->saveCXI==1){
		writeCXIHitstats(eventData, global);
	}

//logfile:
	eventData->writeFlag =  ((hit && global->savehits) || ((global->hdf5dump > 0) && ((eventData->frameNumber % global->hdf5dump) == 0) ));


	// If this is a hit, write out to our favourite HDF5 format
	// Put here anything only needed for data saved to file (why waste the time on events that are not saved)
	// eg: only assemble 2D images, 2D masks and downsample if we are actually saving this frame

	// If we have not assembled and downsampled yet we do it here.
	if(!global->assemblePowders){
		// Assemble to realistic image
		assemble2Dimage(eventData, global);
		assemble2Dmask(eventData, global);
		// Downsample assembled image
		downsample(eventData,global);
	}
	
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

	// Write out information on each frame to a log file
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
	fprintf(global->framefp, "%d, ", eventData->pumpLaserCode);
	fprintf(global->framefp, "%g, ", eventData->pumpLaserDelay);
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
        fprintf(global->powderlogfp[powderClass], "%d, ", eventData->pumpLaserCode);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->pumpLaserDelay);
        fprintf(global->powderlogfp[powderClass], "%d\n", eventData->pumpLaserOn);
		pthread_mutex_unlock(&global->powderfp_mutex);
	}
  
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

    pthread_mutex_unlock(&global->saveinterval_mutex);
    /*
     *  Update counters
     */
    global->nprocessedframes += 1;
	global->nrecentprocessedframes += 1;
    
	/*
	 *	Save some types of information from time to timeperiodic powder patterns
	 */
	if(global->saveInterval!=0 && (global->nprocessedframes%global->saveInterval)==0 && (global->nprocessedframes > global->detector[0].startFrames+50) ){
		if(global->saveCXI){
			writeAccumulatedCXI(global);
		} 
		saveRunningSums(global);
		saveHistograms(global);
		saveRadialStacks(global);
		saveSpectrumStacks(global);
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

void updateAvgGMD(cEventData *eventData, cGlobal *global){
	/*
	 *	Remember GMD values  (why is this here?)
	 */
	float	gmd;
	pthread_mutex_lock(&global->gmd_mutex);
	gmd = (eventData->gmd21+eventData->gmd22)/2;
	global->avgGMD = ( gmd + (global->detector[0].bgMemory-1)*global->avgGMD) / global->detector[0].bgMemory;
	pthread_mutex_unlock(&global->gmd_mutex);
}
