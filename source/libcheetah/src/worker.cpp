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
  eventData = (cEventData*) threadarg;
  global = eventData->pGlobal;
  int	hit = 0;

  std::vector<int> myvector;
  std::stringstream sstm;
  std::string result;
  std::ofstream outFlu;
  //std::ios_base::openmode mode;
  std::stringstream sstm1;
  std::ofstream outHit;


  //--------MONITORING---------//

  updateDatarate(eventData,global);


  //---INITIALIZATIONS-AND-PREPARATIONS---//

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

  // Check for saturated pixels before applying any other corrections
  checkSaturatedPixels(eventData, global);

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
  //  (these corrections will be automatically skipped for any non-pnCCD detector)
  pnccdOffsetCorrection(eventData, global);
  pnccdFixWiringError(eventData, global);
	
  // Apply gain correction
  applyGainCorrection(eventData, global);
	
  // Apply bad pixel map
  applyBadPixelMask(eventData, global);

  // Local background subtraction
  subtractLocalBackground(eventData, global);
			
  // Subtract residual common mode offsets (cmModule=2)
  cspadModuleSubtract2(eventData, global);

  // Identify and kill hot pixels
  identifyHotPixels(eventData, global);
  calculateHotPixelMask(eventData,global);
  applyHotPixelMask(eventData,global);

  // Save detector-corrections-only data (possibly needed later)
  DETECTOR_LOOP {
    memcpy(eventData->detector[detID].detector_corrected_data, eventData->detector[detID].corrected_data, global->detector[detID].pix_nn*sizeof(float));
  }

  //---BACKGROUND-CORRECTION---//
	  
  // If darkcal file available: Init background buffer here (background = photon background)
  initBackgroundBuffer(eventData,global);
  // If darkcal file available: Subtract persistent background here (background = photon background)
  subtractPersistentBackground(eventData, global);

  //---HITFINDING---//
     
  // Hitfinding
  if(global->hitfinder){ 
    hit = hitfinder(eventData, global);
    eventData->hit = hit;
  }


  //---PROCEDURES-DEPENDENT-ON-HIT-TAG---//

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


  //---ASSEMBLE-AND-ACCUMULATE-DATA---//

  // Revert to detector-corrections-only data if we don't want to export data with photon background subtracted
  DETECTOR_LOOP {
    if(global->detector[detID].saveDetectorCorrectedOnly) 
      memcpy(eventData->detector[detID].corrected_data, eventData->detector[detID].detector_corrected_data, global->detector[detID].pix_nn*sizeof(float));
  }

  // If using detector raw, do it here
  DETECTOR_LOOP {
    if(global->detector[detID].saveDetectorRaw)
      for(long i=0;i<global->detector[detID].pix_nn;i++)
	eventData->detector[detID].corrected_data[i] = eventData->detector[detID].raw_data[i];
  }

  // Assemble to realistic image
  assemble2Dimage(eventData, global);
  assemble2Dmask(eventData, global);

  // Downsample assembled image
  downsample(eventData,global);

  // Maintain a running sums of data ("powder" patterns)
  addToPowder(eventData, global);

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


  //---WRITE-DATA-TO-H5---//

  // Keep int16 copy of corrected data (needed for saving images)
  DETECTOR_LOOP {
    for(long i=0;i<global->detector[detID].pix_nn;i++){
      eventData->detector[detID].corrected_data_int16[i] = (int16_t) lrint(eventData->detector[detID].corrected_data[i]);
    }
  }
  
  // If this is a hit, write out to our favourite HDF5 format
  eventData->writeFlag =  ((hit && global->savehits) || ((global->hdf5dump > 0) && ((eventData->frameNumber % global->hdf5dump) == 0) ));
  if(global->saveCXI==1){
    pthread_mutex_lock(&global->saveCXI_mutex);
    writeCXI(eventData, global);
    pthread_mutex_unlock(&global->saveCXI_mutex);
    if(eventData->writeFlag){
      printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing %s to %s slice %u (npeaks=%i)\n",global->runNumber, eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nprocessedframes), eventData->eventname, global->cxiFilename, eventData->stackSlice, eventData->nPeaks);
    }
  } else {
    if(eventData->writeFlag){
      writeHDF5(eventData, global);
      printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Writing to: %s (npeaks=%i)\n",global->runNumber, eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nprocessedframes), eventData->eventname, eventData->nPeaks);
    }
  }
  if(!eventData->writeFlag){
    printf("r%04u:%li (%2.1lf Hz, %3.3f %% hits): Processed (npeaks=%i)\n", global->runNumber,eventData->threadNum,global->datarateWorker, 100.*( global->nhits / (float) global->nprocessedframes), eventData->nPeaks);
  }

  // If this is a hit, write out peak info to peak list file
  if(hit && global->savePeakInfo) {
    writePeakFile(eventData, global);
  }


  //---LOGBOOK-KEEPING---//

  // Write out information on each frame to a log file
  pthread_mutex_lock(&global->framefp_mutex);
  fprintf(global->framefp, "%s, ", eventData->eventname);
  fprintf(global->framefp, "%li, ", eventData->frameNumber);
  fprintf(global->framefp, "%li, ", eventData->threadNum);
  fprintf(global->framefp, "%i, ", eventData->hit);
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
  fprintf(global->framefp, "%d\n", eventData->samplePumped);
  pthread_mutex_unlock(&global->framefp_mutex);

  // Keep track of what has gone into each image class
  pthread_mutex_lock(&global->powderfp_mutex);
  fprintf(global->powderlogfp[hit], "%s, ", eventData->eventname);
  fprintf(global->powderlogfp[hit], "%li, ", eventData->frameNumber);
  fprintf(global->powderlogfp[hit], "%li, ", eventData->threadNum);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->photonEnergyeV);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->wavelengthA);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->detector[0].detectorZ);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->gmd1);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->gmd2);
  fprintf(global->powderlogfp[hit], "%i, ", eventData->energySpectrumExist);
  fprintf(global->powderlogfp[hit], "%d, ", eventData->nPeaks);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->peakNpix);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->peakTotal);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->peakResolution);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->peakDensity);
  fprintf(global->powderlogfp[hit], "%d, ", eventData->laserEventCodeOn);
  fprintf(global->powderlogfp[hit], "%g, ", eventData->laserDelay);
  pthread_mutex_unlock(&global->powderfp_mutex);
	

  //---CLEANUP-AND-EXIT----//
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
