/*
 *  worker.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ConfigV3.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad/CspadTemp.hh"
#include "cspad/CspadCorrector.hh"
#include "cspad/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "setup.h"
#include "worker.h"
#include "median.h"




/*
 *	Worker thread function for processing each cspad data frame
 */
void *worker(void *threadarg) {

	/*
	 *	Turn threadarg into a more useful form
	 */
	cGlobal			*global;
	tThreadInfo		*threadInfo;
	threadInfo = (tThreadInfo*) threadarg;
	global = threadInfo->pGlobal;
	int	hit = 0;

	
	
	/*
	 *	Assemble data from all four quadrants into one large array (rawdata format)
	 */
	threadInfo->raw_data = (uint16_t*) calloc(8*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY,sizeof(uint16_t));
	for(int quadrant=0; quadrant<4; quadrant++) {
		long	i,j,ii;
		for(long k=0; k<2*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY; k++) {
			i = k % (2*CSPAD_ASIC_NX) + quadrant*(2*CSPAD_ASIC_NX);
			j = k / (2*CSPAD_ASIC_NX);
			ii  = i+(8*CSPAD_ASIC_NX)*j;
			threadInfo->raw_data[ii] = threadInfo->quad_data[quadrant][k];
		}
	}
	
	/*
	 *	Create arrays for corrected data, etc needed by this thread
	 */
	threadInfo->corrected_data = (float*) calloc(8*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY,sizeof(float));
	threadInfo->corrected_data_int16 = (int16_t*) calloc(8*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY,sizeof(int16_t));
	threadInfo->detector_corrected_data = (float*) calloc(8*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY,sizeof(float));
	threadInfo->image = (int16_t*) calloc(global->image_nn,sizeof(int16_t));
	threadInfo->saturatedPixelMask = (int16_t *) calloc(8*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY,sizeof(int16_t));

	threadInfo->radialAverage = (float *) calloc(global->radial_nn, sizeof(float));
	threadInfo->radialAverageCounter = (float *) calloc(global->radial_nn, sizeof(float));
	
	threadInfo->peak_com_index = (long *) calloc(global->hitfinderNpeaksMax, sizeof(long));
	threadInfo->peak_intensity = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));	
	threadInfo->peak_npix = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));	
	threadInfo->peak_com_x = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_y = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_x_assembled = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_y_assembled = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_r_assembled = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->good_peaks = (int *) calloc(global->hitfinderNpeaksMax, sizeof(int));



	for(long i=0;i<global->pix_nn;i++){
		threadInfo->saturatedPixelMask[i] = 0;
		threadInfo->corrected_data[i] = threadInfo->raw_data[i];
	}
	
	/*
	 *	Create a unique name for this event
	 */
	nameEvent(threadInfo, global);
	

	/*
	 * Check for saturated pixels, before any other corrections
	 */
	if ( global->maskSaturatedPixels == 1 ) {
		checkSaturatedPixels(threadInfo, global);
	}
	
	/*
	 *	Subtract darkcal image (static electronic offsets)
	 */
	if(global->useDarkcalSubtraction) {
		subtractDarkcal(threadInfo, global);
	}

	
	/*
	 *	Subtract common mode offsets (electronic offsets)
	 */
	if(global->cmModule) {
		cmModuleSubtract(threadInfo, global);
	}
	if(global->cmSubtractUnbondedPixels) {
		cmSubtractUnbondedPixels(threadInfo, global);
	}
	if(global->cmSubtractBehindWires) {
		cmSubtractBehindWires(threadInfo, global);
	}
	
	/*
	 *	Apply gain correction
	 */
	if(global->useGaincal) {
		applyGainCorrection(threadInfo, global);
	}

	/*
	 *	Apply bad pixel map
	 */
	if(global->useBadPixelMask) {
		applyBadPixelMask(threadInfo, global);
	} 
	
	
	/*
	 *	Recalculate running background from time to time
	 */
	if(global->useSubtractPersistentBackground){
		pthread_mutex_lock(&global->bgbuffer_mutex);
		if( ( (global->bgCounter % global->bgRecalc) == 0 || global->bgCounter == global->bgMemory) && global->bgCounter != global->last_bg_update ) {
			calculatePersistentBackground(global);
		}
		pthread_mutex_unlock(&global->bgbuffer_mutex);
	}
	
	/*
	 *	Recalculate hot pixel maskfrom time to time
	 */
	pthread_mutex_lock(&global->hotpixel_mutex);
	if( ( (global->hotpixCounter % global->hotpixRecalc) == 0 || global->hotpixCounter == global->hotpixMemory) && global->hotpixCounter != global->last_hotpix_update ) {
		calculateHotPixelMask(global);
	}
	pthread_mutex_unlock(&global->hotpixel_mutex);
	
	
	/* 
	 *	Keep memory of data with only detector artefacts subtracted (needed for later reference)
	 */
	memcpy(threadInfo->detector_corrected_data, threadInfo->corrected_data, global->pix_nn*sizeof(float));
	for(long i=0;i<global->pix_nn;i++){
		threadInfo->corrected_data_int16[i] = (int16_t) lrint(threadInfo->corrected_data[i]);
	}
	

	/*
	 *	Subtract running photon background
	 */
	if(global->useSubtractPersistentBackground) {
		subtractPersistentBackground(threadInfo, global);
	}
	

	/*
	 *	Local background subtraction
	 */
	if(global->useLocalBackgroundSubtraction) {
		subtractLocalBackground(threadInfo, global);
	}
		

	/*
	 *	Identify and remove hot pixels
	 */
	if(global->useAutoHotpixel){
		killHotpixels(threadInfo, global);
	}
	if(global->useBadPixelMask) {
		applyBadPixelMask(threadInfo, global);
	} 
		

	/*
	 *	Skip first set of frames to build up running estimate of background...
	 */
	if (threadInfo->threadNum < global->startFrames || 
		(global->useSubtractPersistentBackground && global->bgCounter < global->bgMemory) || 
		(global->useAutoHotpixel && global->hotpixCounter < global->hotpixRecalc) ) {
			updateBackgroundBuffer(threadInfo, global); 
			printf("r%04u:%li (%3.1fHz): Digesting initial frames\n", global->runNumber, threadInfo->threadNum,global->datarate);
			goto cleanup;
	}
	
	/*
	 *	Hitfinding
	 */
	if(global->hitfinder){
		hit = hitfinder(threadInfo, global);
	}
	
	
	/*
	 *	Update running backround estimate based on non-hits
	 */
	if (hit==0 || global->bgIncludeHits) {
		updateBackgroundBuffer(threadInfo, global); 
	}		

	
	/*
	 *	Revert to detector-corrections-only data if we don't want to export data with photon bacground subtracted
	 */
	if(global->saveDetectorCorrectedOnly) {
		memcpy(threadInfo->corrected_data, threadInfo->detector_corrected_data, global->pix_nn*sizeof(float));
	}
	
	/*
	 *	If using detector raw, do it here
	 */
	if(global->saveDetectorRaw) {
		for(long i=0;i<global->pix_nn;i++)
			threadInfo->corrected_data[i] = threadInfo->raw_data[i];
	}
	
	
	/*
	 *	Keep int16 copy of corrected data (needed for saving images)
	 */
	for(long i=0;i<global->pix_nn;i++){
		threadInfo->corrected_data_int16[i] = (int16_t) lrint(threadInfo->corrected_data[i]);
	}

	
	/*
	 *	Assemble quadrants into a 'realistic' 2D image
	 */
	assemble2Dimage(threadInfo, global);

	
	/*
	 *	Calculate radial average
	 */
	calculateRadialAverage(threadInfo, global);
	
	
	
	/*
	 *	Maintain a running sum of data (powder patterns)
	 */
	if(hit && global->powderSumHits) {
		addToPowder(threadInfo, global, hit);
	}
	if(!hit && global->powderSumBlanks){
		addToPowder(threadInfo, global, hit);
	} 
	if(global->generateDarkcal || global->generateGaincal){
		addToPowder(threadInfo, global, 1);
	} 
		
	
	
	/*
	 *	If this is a hit, write out to our favourite HDF5 format
	 */
	save:
	if(hit && global->savehits)
		writeHDF5(threadInfo, global);
	else if((global->hdf5dump > 0) && ((threadInfo->threadNum % global->hdf5dump) == 0))
		writeHDF5(threadInfo, global);
	else
		printf("r%04u:%li (%3.1fHz): Processed (npeaks=%i)\n", global->runNumber,threadInfo->threadNum,global->datarate, threadInfo->nPeaks);

	
	/*
	 *	If this is a hit, write out peak info to peak list file
	 */
	//if(hit && global->savePeakList) {
	if(hit && global->savePeakInfo) {
		writePeakFile(threadInfo, global);
	}

	
	

	/*
	 *	Write out information on each frame to a log file
	 */
	pthread_mutex_lock(&global->framefp_mutex);
	fprintf(global->framefp, "%li, %i, %s, %i, %g, %g, %g, %g\n",threadInfo->threadNum, threadInfo->seconds, threadInfo->eventname, threadInfo->nPeaks, threadInfo->peakNpix, threadInfo->peakTotal, threadInfo->peakResolution, threadInfo->peakDensity);
	pthread_mutex_unlock(&global->framefp_mutex);
	
	pthread_mutex_lock(&global->powderfp_mutex);
	fprintf(global->powderlogfp[hit], "r%04u/%s, %i, %g, %g, %g, %g\n",global->runNumber, threadInfo->eventname, threadInfo->nPeaks, threadInfo->peakNpix, threadInfo->peakTotal, threadInfo->peakResolution, threadInfo->peakDensity);
	pthread_mutex_unlock(&global->powderfp_mutex);
	
	
	/*
	 *	Cleanup and exit
	 */
	cleanup:
	// Decrement thread pool counter by one
	pthread_mutex_lock(&global->nActiveThreads_mutex);
	global->nActiveThreads -= 1;
	pthread_mutex_unlock(&global->nActiveThreads_mutex);
	
	// Free memory
	for(int quadrant=0; quadrant<4; quadrant++) 
		free(threadInfo->quad_data[quadrant]);	
	free(threadInfo->raw_data);
	free(threadInfo->corrected_data);
	free(threadInfo->detector_corrected_data);
	free(threadInfo->corrected_data_int16);
	free(threadInfo->image);
	free(threadInfo->radialAverage);
	free(threadInfo->radialAverageCounter);
	free(threadInfo->peak_com_index);
	free(threadInfo->peak_com_x);
	free(threadInfo->peak_com_y);
	free(threadInfo->peak_com_x_assembled);
	free(threadInfo->peak_com_y_assembled);
	free(threadInfo->peak_com_r_assembled);
	free(threadInfo->peak_intensity);
	free(threadInfo->peak_npix);
	free(threadInfo->good_peaks);
	free(threadInfo->saturatedPixelMask);
	//TOF stuff.
	if(threadInfo->TOFPresent==1){
		free(threadInfo->TOFTime);
		free(threadInfo->TOFVoltage); 
	}

	free(threadInfo);

	// Exit thread
	pthread_exit(NULL);
}


/*
 *	Subtract pre-loaded darkcal file
 */
void subtractDarkcal(tThreadInfo *threadInfo, cGlobal *global){
	
	
	// Do darkcal subtraction
	for(long i=0;i<global->pix_nn;i++) {
		threadInfo->corrected_data[i] -= global->darkcal[i]; 
	}

	// If corrected_data is int16 we need to watch for int16 wraparound
	/*int32_t diff;
	for(long i=0;i<global->pix_nn;i++) {
		diff = (int32_t) threadInfo->corrected_data[i] - (int32_t) global->darkcal[i];	
		if(diff < -32766) diff = -32767;
		if(diff > 32766) diff = 32767;
		threadInfo->corrected_data[i] = (int16_t) diff;
	}
	 */
	
}


/*
 *	Identify and kill hot pixels
 */
void killHotpixels(tThreadInfo *threadInfo, cGlobal *global){
	
	
	// First update global hot pixel buffer
	int16_t	*buffer = (int16_t *) calloc(global->pix_nn,sizeof(int16_t));
	for(long i=0;i<global->pix_nn;i++){
		buffer[i] = (fabs(threadInfo->corrected_data[i])>global->hotpixADC)?(1):(0);
	}
	pthread_mutex_lock(&global->hotpixel_mutex);
	long frameID = global->hotpixCounter%global->hotpixMemory;	
	memcpy(global->hotpix_buffer+global->pix_nn*frameID, buffer, global->pix_nn*sizeof(int16_t));
	global->hotpixCounter += 1;
	pthread_mutex_unlock(&global->hotpixel_mutex);
	free(buffer);

	
	// Apply the current hot pixel mask 
	for(long i=0;i<global->pix_nn;i++){
		threadInfo->corrected_data[i] *= global->hotpixelmask[i];
	}
	threadInfo->nHot = global->nhot;

	

}


void calculateHotPixelMask(cGlobal *global){

	long	cutoff = lrint((global->hotpixMemory*global->hotpixFreq));
	printf("Recalculating hot pixel mask at %li/%i\n",cutoff,global->hotpixMemory);	
	
	// Loop over all pixels 
	long	counter;
	long	nhot;
	for(long i=0; i<global->pix_nn; i++) {
		
		counter = 0;
		for(long j=0; j< global->hotpixMemory; j++) {
			counter += global->hotpix_buffer[j*global->pix_nn+i]; 
		}
		
		// Apply threshold
		if(counter < cutoff) {
			global->hotpixelmask[i] = 1;
		}
		else{
			global->hotpixelmask[i] = 0;
			nhot++;				
		}		
	}	
	global->nhot = nhot;
	global->last_hotpix_update = global->hotpixCounter;
}



/*
 *	Calculate persistent background from stack of remembered frames
 */
void calculatePersistentBackground(cGlobal *global) {


	long	median_element = lrint((global->bgMemory*global->bgMedian));
	int16_t	*buffer = (int16_t*) calloc(global->bgMemory, sizeof(int16_t));
	printf("Finding %lith smallest element of buffer depth %li\n",median_element,global->bgMemory);	
	
	// Lock the global variables
	//pthread_mutex_lock(&global->bgbuffer_mutex);
	pthread_mutex_lock(&global->selfdark_mutex);

	// Loop over all pixels 
	for(long i=0; i<global->pix_nn; i++) {
		
		// Create a local array for sorting
		for(long j=0; j< global->bgMemory; j++) {
			buffer[j] = global->bg_buffer[j*global->pix_nn+i];
		}
		
		// Find median value of the temporary array
		global->selfdark[i] = kth_smallest(buffer, global->bgMemory, median_element);
	}
	
	global->last_bg_update = global->bgCounter;
	//pthread_mutex_unlock(&global->bgbuffer_mutex);
	pthread_mutex_unlock(&global->selfdark_mutex);

	free (buffer);
}



/*
 *	Update background buffer
 */
void updateBackgroundBuffer(tThreadInfo *threadInfo, cGlobal *global) {
	
	pthread_mutex_lock(&global->bgbuffer_mutex);
	long frameID = global->bgCounter%global->bgMemory;	
	
	memcpy(global->bg_buffer+global->pix_nn*frameID, threadInfo->corrected_data_int16, global->pix_nn*sizeof(int16_t));

	//for(long i=0;i<global->pix_nn;i++)
	//	global->bg_buffer[global->pix_nn*frameID + i] = threadInfo->corrected_data_int16[i];
	
	global->bgCounter += 1;
	pthread_mutex_unlock(&global->bgbuffer_mutex);
	
}



/*
 *	Subtract persistent background 
 */
void subtractPersistentBackground(tThreadInfo *threadInfo, cGlobal *global){
	
	float	top = 0;
	float	s1 = 0;
	float	s2 = 0;
	float	v1, v2;
	float	factor;
	float	gmd;
	
	
	// Add current (uncorrected) image to self darkcal
	pthread_mutex_lock(&global->selfdark_mutex);
	//for(long i=0;i<global->pix_nn;i++){
	//	global->selfdark[i] = ( threadInfo->corrected_data[i] + (global->bgMemory-1)*global->selfdark[i]) / global->bgMemory;
	//}
	gmd = (threadInfo->gmd21+threadInfo->gmd22)/2;
	global->avgGMD = ( gmd + (global->bgMemory-1)*global->avgGMD) / global->bgMemory;
	pthread_mutex_unlock(&global->selfdark_mutex);
	
	
	// Find appropriate scaling factor 
	if(global->scaleBackground) {
		for(long i=0;i<global->pix_nn;i++){
			//v1 = pow(global->selfdark[i], 0.25);
			//v2 = pow(threadInfo->corrected_data[i], 0.25);
			v1 = global->selfdark[i];
			v2 = threadInfo->corrected_data[i];
			if(v2 > global->hitfinderADC)
				continue;
			
			// Simple inner product gives cos(theta), which is always less than zero
			// Want ( (a.b)/|b| ) * (b/|b|)
			top += v1*v2;
			s1 += v1*v1;
			s2 += v2*v2;
		}
		factor = top/s1;
	}
	else 
		factor=1;
	
	
	// Do the weighted subtraction
	for(long i=0;i<global->pix_nn;i++) {
		threadInfo->corrected_data[i] -= (factor*global->selfdark[i]);	
	}	

}


/*
 *	Apply gain correction
 *	Assumes the gaincal array is appropriately 'prepared' when loaded so that all we do is a multiplication.
 *	All that checking for division by zero (and inverting when required) needs only be done once, right? 
 */
void applyGainCorrection(tThreadInfo *threadInfo, cGlobal *global){
	
	for(long i=0;i<global->pix_nn;i++) 
		threadInfo->corrected_data[i] *= global->gaincal[i];
	
}



/*
 * Make a saturated pixel mask
 */
void checkSaturatedPixels(tThreadInfo *threadInfo, cGlobal *global){

	for(long i=0;i<global->pix_nn;i++) { 
		if ( threadInfo->raw_data[i] >= global->pixelSaturationADC) 
			threadInfo->saturatedPixelMask[i] = 0;
		else
			threadInfo->saturatedPixelMask[i] = 1;
	}

}

/*
 *	Apply bad pixel mask
 *	Assumes that all we have to do here is a multiplication.
 */
void applyBadPixelMask(tThreadInfo *threadInfo, cGlobal *global){
	
	for(long i=0;i<global->pix_nn;i++) 
		threadInfo->corrected_data[i] *= global->badpixelmask[i];
	
}


/*
 *	Subtract common mode on each module
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cmModuleSubtract(tThreadInfo *threadInfo, cGlobal *global){

	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		e;
	long		mval;
	float		median;
	
	float	*buffer; 
	buffer = (float*) calloc(CSPAD_ASIC_NX*CSPAD_ASIC_NY, sizeof(float));
	
	mval = lrint((CSPAD_ASIC_NX*CSPAD_ASIC_NY)*global->cmFloor);
	if(mval < 0) 
		mval = 1;
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){

			// Zero array
			for(long i=0; i<CSPAD_ASIC_NX*CSPAD_ASIC_NY; i++)
				buffer[i] = 0;
			
			// Loop over pixels within a module
			for(long j=0; j<CSPAD_ASIC_NY; j++){
				for(long i=0; i<CSPAD_ASIC_NX; i++){
					e = (j + mj*CSPAD_ASIC_NY) * (8*CSPAD_ASIC_NX);
					e += i + mi*CSPAD_ASIC_NX;
					buffer[i+j*CSPAD_ASIC_NX] = threadInfo->corrected_data[e];
				}
			}
			
			// Calculate background using median value
			median = kth_smallest(buffer, CSPAD_ASIC_NX*CSPAD_ASIC_NY, mval);

			// Subtract median value
			for(long j=0; j<CSPAD_ASIC_NY; j++){
				for(long i=0; i<CSPAD_ASIC_NX; i++){
					e = (j + mj*CSPAD_ASIC_NY) * (8*CSPAD_ASIC_NX);
					e += i + mi*CSPAD_ASIC_NX;
					threadInfo->corrected_data[e] -= median;
				}
			}
		}
	}
	free(buffer);
}


/*
 *	Subtract offset estimated from unbonded pixels
 *	In the upstream detector, the unbonded pixels are in Q0:0-3 and Q2:4-5 and are at the 
 *	corners of each asic and at row=col (row<194) or row-194==col (row>194) for col%10=0.  
 */
void cmSubtractUnbondedPixels(tThreadInfo *threadInfo, cGlobal *global){
	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		e;
	double		counter;
	double		background;
	
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){
						
			// Only asics in Q0:0-3 and Q2:4-5 are unbonded
			if( ! ((mi<=1 && mj<=3) || (mi >= 4 && mi<=5 && mj >= 4 && mj<=5)) )
				continue;

			
			// Loop over unbonded pixels within each ASIC
			background = 0.0;
			counter = 0.0;
			for(long j=0; j<CSPAD_ASIC_NY-1; j+=10){
				long i=j;
				e = (j + mj*CSPAD_ASIC_NY) * (8*CSPAD_ASIC_NX);
				e += i + mi*CSPAD_ASIC_NX;
				background += threadInfo->corrected_data[e];
				counter += 1;
			}
			background /= counter;
			
			//printf("%f ",background);
						
			// Subtract background from entire ASIC
			for(long j=0; j<CSPAD_ASIC_NY; j++){
				for(long i=0; i<CSPAD_ASIC_NX; i++){
					e = (j + mj*CSPAD_ASIC_NY) * (8*CSPAD_ASIC_NX);
					e += i + mi*CSPAD_ASIC_NX;
					threadInfo->corrected_data[e] -= background;
					
				}
			}
		}
	}
	
}

/*
 *	Subtract common mode estimated from signal behind wires
 *	Common mode is the kth lowest pixel value in the whole ASIC (similar to a median calculation)
 */
void cmSubtractBehindWires(tThreadInfo *threadInfo, cGlobal *global){
	
	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		p;
	long		counter;
	long		mval;
	float		median;
	
	float	*buffer; 
	buffer = (float*) calloc(CSPAD_ASIC_NY*CSPAD_ASIC_NX, sizeof(float));
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){
			
			
			// Loop over pixels within a module, remembering signal behind wires
			counter = 0;
			for(long j=0; j<CSPAD_ASIC_NY; j++){
				for(long i=0; i<CSPAD_ASIC_NX; i++){
					p = (j + mj*CSPAD_ASIC_NY) * (8*CSPAD_ASIC_NX);
					p += i + mi*CSPAD_ASIC_NX;
					if(global->wiremask[i]) {
						buffer[counter] = threadInfo->corrected_data[p];
						counter++;
					}
				}
			}
			
			// Median value of pixels behind wires
			if(counter>0) {
				mval = lrint(counter*global->cmFloor);
				median = kth_smallest(buffer, counter, mval);
			}
			else 
				median = 0;


			// Subtract median value
			for(long i=0; i<CSPAD_ASIC_NX; i++){
				for(long j=0; j<CSPAD_ASIC_NY; j++){
					p = (j + mj*CSPAD_ASIC_NY) * (8*CSPAD_ASIC_NX);
					p += i + mi*CSPAD_ASIC_NX;
					threadInfo->corrected_data[p] -= median;
				}
			}
		}
	}
	free(buffer);
}




/*
 *	Local background subtraction
 */
void subtractLocalBackground(tThreadInfo *threadInfo, cGlobal *global){
	
	long		e,ee;
	long		counter;
	
	
	// Search subunits
	if(global->localBackgroundRadius <= 0 || global->localBackgroundRadius >= CSPAD_ASIC_NY/2 )
		return;
	long nn = (2*global->localBackgroundRadius+1);
	nn=nn*nn;
	
	
	// Create local arrays needed for background subtraction
	float	*localBg = (float*) calloc(global->pix_nn, sizeof(float)); 
	float	*buffer = (float*) calloc(nn, sizeof(float));

	
	
	// Loop over ASIC modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){
			
			// Loop over pixels within a module
			for(long j=0; j<CSPAD_ASIC_NY; j++){
				for(long i=0; i<CSPAD_ASIC_NX; i++){

					counter = 0;
					e = (j+mj*CSPAD_ASIC_NY)*global->pix_nx;
					e += i+mi*CSPAD_ASIC_NX;
					
					// Loop over median window
					for(long jj=-global->localBackgroundRadius; jj<=global->localBackgroundRadius; jj++){
						for(long ii=-global->localBackgroundRadius; ii<=global->localBackgroundRadius; ii++){

							// Quick array bounds check
							if((i+ii) < 0)
								continue;
							if((i+ii) >= CSPAD_ASIC_NX)
								continue;
							if((j+jj) < 0)
								continue;
							if((j+jj) >= CSPAD_ASIC_NY)
								continue;

							ee = (j+jj+mj*CSPAD_ASIC_NY)*global->pix_nx;
							ee += i+ii+mi*CSPAD_ASIC_NX;

							if(ee < 0 || ee >= global->pix_nn){
								printf("Error: Array bounds error: e = %li > %li\n",e,global->pix_nn);
								continue;
							}
							
							buffer[counter] = threadInfo->corrected_data[ee];
							counter++;
						}
					}
							
					// No elements -> trap an error
					if(counter == 0) {
						printf("Error: Local background counter == 0\n");
						localBg[e] = 0;
						continue;
					}
					if(counter > nn) {
						printf("Error: counter == %li > %li\n",counter,nn);
						continue;
					}
					
					// Find median value
					localBg[e] = kth_smallest(buffer, counter, counter/2);
				}
			}
		}
	}
	
	
	// Actually do the local background subtraction
	for(long i=0;i<global->pix_nn;i++) {
		threadInfo->corrected_data[i] -= localBg[i];	
	}	
	
	
	// Cleanup
	free(localBg);
	free(buffer);
}





/*
 *	Various flavours of hitfinder
 *		1 - Number of pixels above ADC threshold
 *		2 - Total intensity above ADC threshold
 *		3 - Count Bragg peaks
 *		4 - Use TOF
 */
int  hitfinder(tThreadInfo *threadInfo, cGlobal *global){

	long	nat, lastnat;
	long	counter;
	int		hit=0;
	float	total;
	int search_x[] = {-1,0,1,-1,1,-1,0,1};
	int search_y[] = {-1,-1,-1,0,0,1,1,1};
	int	search_n = 8;
	long e;
	long *inx = (long *) calloc(global->pix_nn, sizeof(long));
	long *iny = (long *) calloc(global->pix_nn, sizeof(long));
	float totI;
	float peak_com_x;
	float peak_com_y;
	long thisx;
	long thisy;
	long fs, ss;
	float grad;
	float lbg, imbg; /* local background nearby peak */
	float *lbg_buffer;
	int fsmin, fsmax, ssmin, ssmax;
	int lbg_ss, lbg_fs, lbg_e;
	int thisfs, thisss;
	float mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;

	nat = 0;
	counter = 0;
	total = 0.0;
	
	
	/*
	 *	Default values for some metrics
	 */
	threadInfo->peakNpix = 0;
	threadInfo->peakTotal = 0;
	threadInfo->peakResolution = 0;
	threadInfo->peakDensity = 0;
	

	/*
	 *	Use a data buffer so we can zero out pixels already counted
	 */
	float *temp = (float*) calloc(global->pix_nn, sizeof(float));
	memcpy(temp, threadInfo->corrected_data, global->pix_nn*sizeof(float));
	
	
	/*
	 *	Apply peak search mask 
	 *	(multiply data by 0 to ignore regions)
	 */
	if(global->hitfinderUsePeakmask) {
		for(long i=0;i<global->pix_nn;i++){
			temp[i] *= global->peakmask[i]; 
		}
	}
	
	
	/*
	 *	Use one of various hitfinder algorithms
	 */
	switch(global->hitfinderAlgorithm) {
		
		case 1 :	// Count the number of pixels above ADC threshold
			for(long i=0;i<global->pix_nn;i++){
				if(temp[i] > global->hitfinderADC){
					total += temp[i];
					nat++;
				}
			}
			if(nat >= global->hitfinderNAT)
				hit = 1;

			threadInfo->peakNpix = nat;
			threadInfo->nPeaks = nat;
			threadInfo->peakTotal = total;
			break;

	
		case 2 :	//	integrated intensity above threshold
			for(long i=0;i<global->pix_nn;i++){
				if(temp[i] > global->hitfinderADC){
					total += temp[i];
					nat++;
				}
			}
			if(total >= global->hitfinderTAT) 
				hit = 1;

			threadInfo->peakNpix = nat;
			threadInfo->nPeaks = nat;
			threadInfo->peakTotal = total;
			break;
			

		case 4 :	// Use TOF signal to find hits
			if ((global->hitfinderUseTOF==1) && (threadInfo->TOFPresent==1)){
				double total_tof = 0.;
				for(int i=global->hitfinderTOFMinSample; i<global->hitfinderTOFMaxSample; i++){
					total_tof += threadInfo->TOFVoltage[i];
				}
				if (total_tof > global->hitfinderTOFThresh)
					hit = 1;
			}
			// Use cspad threshold if TOF is not present 
			else {
				for(long i=0;i<global->pix_nn;i++){
					if(temp[i] > global->hitfinderADC){
						nat++;
					}
				}
				if(nat >= global->hitfinderNAT)
					hit = 1;
			}
			break;


		case 5 : 	// Count number of peaks (and do other statistics)

			threadInfo->peakResolution = 0;
			threadInfo->peakDensity = 0;

			// Loop over modules (8x8 array)
			for(long mj=0; mj<8; mj++){
			for(long mi=0; mi<8; mi++){	
			// Loop over pixels within a module
			for(long j=1; j<CSPAD_ASIC_NY-1; j++){
			for(long i=1; i<CSPAD_ASIC_NX-1; i++){

				ss = (j+mj*CSPAD_ASIC_NY);
				fs = i+mi*CSPAD_ASIC_NX;
				e = ss*global->pix_nx + fs;

				if ( global->hitfinderResMask[e] != 1 ) continue;

				if ( temp[e] < global->hitfinderADC ) continue;

				if ( global->hitfinderCheckGradient == 1 ){
				
					float dx1, dx2, dy1, dy2, dxs, dys;
					
					/* can't measure gradient where bad pixels present */
					if ( global->badpixelmask[e] != 1 ) continue;
					if ( global->badpixelmask[e+1] != 1 ) continue;
					if ( global->badpixelmask[e-1] != 1 ) continue;
					if ( global->badpixelmask[e+global->pix_nx] != 1 ) continue;
					if ( global->badpixelmask[e-global->pix_nx] != 1 ) continue;

					/* Get gradients */
					dx1 = temp[e] - temp[e+1];
					dx2 = temp[e-1] - temp[e];
					dy1 = temp[e] - temp[e+global->pix_nx];
					dy2 = temp[e-global->pix_nx] - temp[e];
					/* this is sort of like the mean squared gradient, times 4... */
					dxs = ((dx1*dx1) + (dx2*dx2)) ;
					dys = ((dy1*dy1) + (dy2*dy2)) ;	
					grad = dxs + dys;

					if ( grad < mingrad ) continue;
				}

				lbg = 0; /* local background value */
				if ( global->hitfinderSubtractLocalBG == 1 ) {
					int lbg_counter = 0;
					/* region nearby the peak */
					fsmin = fs - global->hitfinderLocalBGRadius;
					fsmax = fs + global->hitfinderLocalBGRadius;
					ssmin = ss - global->hitfinderLocalBGRadius;
					ssmax = ss + global->hitfinderLocalBGRadius;
					/* check module bounds */
					if ( fsmin < mi*CSPAD_ASIC_NX ) fsmin = mi*CSPAD_ASIC_NX;
					if ( fsmax >= (mi+1)*CSPAD_ASIC_NX ) fsmax = (mi+1)*CSPAD_ASIC_NX - 1; 
					if ( ssmin < mj*CSPAD_ASIC_NY ) ssmin = mj*CSPAD_ASIC_NY;
					if ( ssmax >= (mj+1)*CSPAD_ASIC_NY ) ssmax = (mj+1)*CSPAD_ASIC_NY - 1;
					/* buffer for calculating median */
					lbg_buffer = (float *) calloc((fsmax-fsmin+1)*(ssmax-ssmin+1),sizeof(float));
					/* now calculate median */
					for ( lbg_ss = ssmin; lbg_ss <= ssmax; lbg_ss++) {
					for ( lbg_fs = fsmin; lbg_fs <= fsmax; lbg_fs++ ) {
						thisss = (j+mj*CSPAD_ASIC_NY)*global->pix_nx;
						thisfs = i+mi*CSPAD_ASIC_NX;
						lbg_e = thisss + thisfs;
						/* check if we're ignoring this pixel*/ 
						if ( global->badpixelmask[lbg_e] == 1 ) {
							lbg_buffer[lbg_counter] = temp[lbg_e];
							lbg_counter++;		
						}
					}}
					if ( lbg_counter > 0 )
						lbg = kth_smallest(lbg_buffer,lbg_counter,lbg_counter/2);	
					free(lbg_buffer);
					if ( (temp[e]-lbg) > global->hitfinderADC ) continue;								
				}

				inx[0] = i;
				iny[0] = j;
				nat = 1;
				totI = 0; 
				peak_com_x = 0; 
				peak_com_y = 0; 

				// Keep looping until the pixel count within this peak does not change
				do {

					lastnat = nat;
					// Loop through points known to be within this peak
					for(long p=0; p<nat; p++){
						// Loop through search pattern
						for(long k=0; k<search_n; k++){

							// Array bounds check
							if((inx[p]+search_x[k]) < 0) continue;
							if((inx[p]+search_x[k]) >= CSPAD_ASIC_NX) continue;
							if((iny[p]+search_y[k]) < 0) continue;
							if((iny[p]+search_y[k]) >= CSPAD_ASIC_NY) continue;
							
							// Neighbour point 
							thisx = inx[p]+search_x[k]+mi*CSPAD_ASIC_NX;
							thisy = iny[p]+search_y[k]+mj*CSPAD_ASIC_NY;
							e = thisx + thisy*global->pix_nx;
							
							// Above threshold?
							imbg = temp[e] - lbg; /* "intensitiy minus background" */
							if(imbg > global->hitfinderADC){
								totI += imbg; // add to integrated intensity
								peak_com_x += imbg*( (float) thisx ); // for center of mass x
								peak_com_y += imbg*( (float) thisy ); // for center of mass y
								temp[e] = 0; // zero out this intensity so that we don't count it again
								inx[nat] = inx[p]+search_x[k];
								iny[nat] = iny[p]+search_y[k];
								nat++;
							}
						}
					}
				} while(lastnat != nat);

	 			// Peak or junk?
				if(nat>=global->hitfinderMinPixCount && nat<=global->hitfinderMaxPixCount) {
					
					threadInfo->peakNpix += nat;
					threadInfo->peakTotal += totI;
					
					// Only space to save the first NpeaksMax peaks
					// (more than this and the pattern is probably junk)
					if ( counter > global->hitfinderNpeaksMax ) {
						//counter++;
						threadInfo->nPeaks = counter;
						hit = 0;
						goto quitHitfinder5;
					}
					
					// Remember peak information
					threadInfo->peak_intensity[counter] = totI;
					threadInfo->peak_com_x[counter] = peak_com_x/totI;
					threadInfo->peak_com_y[counter] = peak_com_y/totI;
					threadInfo->peak_npix[counter] = nat;

					e = lrint(peak_com_x/totI) + lrint(peak_com_y/totI)*global->pix_nx;
					threadInfo->peak_com_index[counter] = e;
					threadInfo->peak_com_x_assembled[counter] = global->pix_x[e];
					threadInfo->peak_com_y_assembled[counter] = global->pix_y[e];
					threadInfo->peak_com_r_assembled[counter] = global->pix_r[e];
					counter++;
					
				}
			}}}}	
			threadInfo->nPeaks = counter;

			/* check peak separations?  get rid of clusters? */
			if ( global->hitfinderCheckPeakSeparation == 1 ) {
				int peakNum;
				int peakNum1;
				int peakNum2;
				float diffX,diffY;
				float maxPeakSepSq = global->hitfinderMaxPeakSeparation*global->hitfinderMaxPeakSeparation;
				float peakSepSq;
				
				/* all peaks assumed "good" to start */
				for ( peakNum = 0; peakNum < threadInfo->nPeaks; peakNum++ ) threadInfo->good_peaks[peakNum] = 1;
				
				/* loop through unique peak pairs, checking that they are not too close */
				for ( peakNum1 = 0; peakNum1 < threadInfo->nPeaks - 1; peakNum1++ ) {
					if ( threadInfo->good_peaks[peakNum1] == 0 ) continue;
					for (peakNum2 = peakNum1 + 1; peakNum2 < threadInfo->nPeaks; peakNum2++ ) {
						if ( threadInfo->good_peaks[peakNum2] == 0 ) continue;
						/* check the distance between these two peaks */
						diffX = threadInfo->peak_com_x[peakNum1] - threadInfo->peak_com_x[peakNum2];
						diffY = threadInfo->peak_com_y[peakNum1] - threadInfo->peak_com_y[peakNum2];
						peakSepSq = diffX*diffX + diffY*diffY;
						if ( peakSepSq < maxPeakSepSq ) {
							if (threadInfo->peak_intensity[peakNum1] > threadInfo->peak_intensity[peakNum2]) 
							threadInfo->good_peaks[peakNum2] = 0;
							else 
							threadInfo->good_peaks[peakNum2] = 0;
						}
					}
				}
				/* now repopulate the peak list with good ones */
				int gpc = 0;
				for ( peakNum = 0; peakNum < threadInfo->nPeaks; peakNum++ ) {
					if ( threadInfo->good_peaks[peakNum] == 1 ) {
						threadInfo->peak_com_x[gpc] = threadInfo->peak_com_x[peakNum];
						threadInfo->peak_com_y[gpc] = threadInfo->peak_com_y[peakNum];
						threadInfo->peak_com_x_assembled[gpc] = threadInfo->peak_com_x_assembled[peakNum];
						threadInfo->peak_com_y_assembled[gpc] = threadInfo->peak_com_y_assembled[peakNum];
						threadInfo->peak_com_r_assembled[gpc] = threadInfo->peak_com_r_assembled[peakNum];
						threadInfo->peak_com_index[gpc] = threadInfo->peak_com_index[peakNum];
						threadInfo->peak_intensity[gpc] = threadInfo->peak_intensity[peakNum];
						threadInfo->peak_npix[gpc] =threadInfo->peak_npix[peakNum];
						gpc++;
					}
				}
				counter = gpc;
				threadInfo->nPeaks = counter;
			}	
		
			// Statistics on the peaks
			if(counter > 1) {
				long	np;
				long  kk;
				float	resolution;
				float	cutoff = 0.8;
				
				np = counter;
				if(counter >= global->hitfinderNpeaksMax) 
					np = global->hitfinderNpeaksMax; 
				
				float *buffer1 = (float*) calloc(global->hitfinderNpeaksMax, sizeof(float));
				for(long k=0; k<np; k++) {
						buffer1[k] = threadInfo->peak_com_r_assembled[k];
				}
				kk = (long) floor(cutoff*np);
				resolution = kth_smallest(buffer1, np, kk);
				
				threadInfo->peakResolution = resolution;
				if(resolution > 0) {
					float	area = (3.141*resolution*resolution)/(CSPAD_ASIC_NY*CSPAD_ASIC_NX);
					threadInfo->peakDensity = (cutoff*np)/area;
					
				}
					
				free(buffer1);
			} 
			
			
			// Now figure out whether this is a hit
			if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
				hit = 1;
	
			quitHitfinder5:	

			free(inx); 			
			free(iny);
		
			break;
	
		case 3 : 	// Count number of peaks (and do other statistics)
		default:
			long fs, ss;
			double dx1, dx2, dy1, dy2;
			double dxs, dys;
			double grad = global->hitfinderMinGradient;

		
			threadInfo->peakResolution = 0;
			threadInfo->peakDensity = 0;

			// Loop over modules (8x8 array)
			for(long mj=0; mj<8; mj++){
				for(long mi=0; mi<8; mi++){
					
					// Loop over pixels within a module
					for(long j=1; j<CSPAD_ASIC_NY-1; j++){
						for(long i=1; i<CSPAD_ASIC_NX-1; i++){


							ss = (j+mj*CSPAD_ASIC_NY)*global->pix_nx;
							fs = i+mi*CSPAD_ASIC_NX;
							e = ss + fs;

							//if(e >= global->pix_nn)
							//	printf("Array bounds error: e=%i\n");
							
							if(temp[e] > global->hitfinderADC){
							// This might be the start of a peak - start searching
								
								if ( global->hitfinderMinGradient > 0 ){
			
									/* Get gradients */
									dx1 = temp[e] - temp[e+1];
									dx2 = temp[e-1] - temp[e];
									dy1 = temp[e] - temp[e+global->pix_nx];
									dy2 = temp[e-global->pix_nx] - temp[e];
				
									/* Average gradient measurements from both sides */
									dxs = ((dx1*dx1) + (dx2*dx2)) / 2;
									dys = ((dy1*dy1) + (dy2*dy2)) / 2;
				
									/* Calculate overall gradient */
									grad = dxs + dys;
			
								}
								
								if ( grad < global->hitfinderMinGradient ) continue;


								inx[0] = i;
								iny[0] = j;
								nat = 1;
								totI = 0; 
								peak_com_x = 0; 
								peak_com_y = 0; 
								
								// Keep looping until the pixel count within this peak does not change
								do {

									//if ( nat > global->hitfinderNAT ) break;
									lastnat = nat;
									// Loop through points known to be within this peak
									for(long p=0; p<nat; p++){
										// Loop through search pattern
										for(long k=0; k<search_n; k++){
											// Array bounds check
											if((inx[p]+search_x[k]) < 0)
												continue;
											if((inx[p]+search_x[k]) >= CSPAD_ASIC_NX)
												continue;
											if((iny[p]+search_y[k]) < 0)
												continue;
											if((iny[p]+search_y[k]) >= CSPAD_ASIC_NY)
												continue;
											
											// Neighbour point 
											thisx = inx[p]+search_x[k]+mi*CSPAD_ASIC_NX;
											thisy = iny[p]+search_y[k]+mj*CSPAD_ASIC_NY;
											e = thisx + thisy*global->pix_nx;
											
											//if(e < 0 || e >= global->pix_nn){
											//	printf("Array bounds error: e=%i\n",e);
											//	continue;
											//}
											
											// Above threshold?
											if(temp[e] > global->hitfinderADC){
												//if(nat < 0 || nat >= global->pix_nn) {
												//	printf("Array bounds error: nat=%i\n",nat);
												//	break
												//}
												totI += temp[e]; // add to integrated intensity
												peak_com_x += temp[e]*( (float) thisx ); // for center of mass x
												peak_com_y += temp[e]*( (float) thisy ); // for center of mass y
												temp[e] = 0; // zero out this intensity so that we don't count it again
												inx[nat] = inx[p]+search_x[k];
												iny[nat] = iny[p]+search_y[k];
												nat++;

											}
										}
									}
								} while(lastnat != nat);
								
								// Peak or junk?
								if(nat>=global->hitfinderMinPixCount && nat<=global->hitfinderMaxPixCount) {
									
									threadInfo->peakNpix += nat;
									threadInfo->peakTotal += totI;
									

									// Only space to save the first NpeaksMax peaks
									// (more than this and the pattern is probably junk)
									if ( counter > global->hitfinderNpeaksMax ) {
										counter++;
										continue;
									}
									
									// Remember peak information
									threadInfo->peak_intensity[counter] = totI;
									threadInfo->peak_com_x[counter] = peak_com_x/totI;
									threadInfo->peak_com_y[counter] = peak_com_y/totI;
									threadInfo->peak_npix[counter] = nat;

									e = lrint(peak_com_x/totI) + lrint(peak_com_y/totI)*global->pix_nx;
									threadInfo->peak_com_index[counter] = e;
									threadInfo->peak_com_x_assembled[counter] = global->pix_x[e];
									threadInfo->peak_com_y_assembled[counter] = global->pix_y[e];
									threadInfo->peak_com_r_assembled[counter] = global->pix_r[e];
									counter++;
								}
							}
						}
					}
				}
			}	
			threadInfo->nPeaks = counter;
			free(inx);
			free(iny);
		

			// Statistics on the peaks
			if(counter > 1) {
				long	np;
				long  kk;
				float	resolution;
				float	cutoff = 0.8;
				
				np = counter;
				if(counter >= global->hitfinderNpeaksMax) 
					np = global->hitfinderNpeaksMax; 
				
				float *buffer1 = (float*) calloc(global->hitfinderNpeaksMax, sizeof(float));
				for(long k=0; k<np; k++) {
						buffer1[k] = threadInfo->peak_com_r_assembled[k];
				}
				kk = (long) floor(cutoff*np);
				resolution = kth_smallest(buffer1, np, kk);
				
				threadInfo->peakResolution = resolution;
				if(resolution > 0) {
					float	area = (3.141*resolution*resolution)/(CSPAD_ASIC_NY*CSPAD_ASIC_NX);
					threadInfo->peakDensity = (cutoff*np)/area;
					
				}
					
				free(buffer1);
			} 
			
			
			// Now figure out whether this is a hit
			if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
				hit = 1;
			
			break;
			
			
	}
		
	
	
	// Update central hit counter
	if(hit) {
		pthread_mutex_lock(&global->nhits_mutex);
		global->nhits++;
		global->nrecenthits++;
		pthread_mutex_unlock(&global->nhits_mutex);
	}
	
	free(temp);
	return(hit);
}



	

/*
 *	Maintain running powder patterns
 *	(currently supports both old and new ways of writing out powder data)
 */
void addToPowder(tThreadInfo *threadInfo, cGlobal *global, int hit){
	
	
	double  *buffer;
	int		powderClass = hit;
	
		
	/*
	 *	New way of summing powder patterns (handles multiple classifications)
	 */

	// Raw data
	pthread_mutex_lock(&global->powderRaw_mutex[powderClass]);
	for(long i=0; i<global->pix_nn; i++) {
		if(threadInfo->corrected_data[i] > global->powderthresh)
			global->powderRaw[powderClass][i] += threadInfo->corrected_data[i];
	}
	pthread_mutex_unlock(&global->powderRaw_mutex[powderClass]);			
	
	
	// Raw data squared (for calculating variance)
	buffer = (double*) calloc(global->pix_nn, sizeof(double));
	for(long i=0; i<global->pix_nn; i++) 
		buffer[i] = 0;
	for(long i=0; i<global->pix_nn; i++){
		if(threadInfo->corrected_data[i] > global->powderthresh)
			buffer[i] = (threadInfo->corrected_data[i])*(threadInfo->corrected_data[i]);
	}
	pthread_mutex_lock(&global->powderRawSquared_mutex[powderClass]);
	for(long i=0; i<global->pix_nn; i++) 
		global->powderRawSquared[powderClass][i] += buffer[i];
	pthread_mutex_unlock(&global->powderRawSquared_mutex[powderClass]);			
	free(buffer);

	
	// Assembled data
	pthread_mutex_lock(&global->powderAssembled_mutex[powderClass]);
	for(long i=0; i<global->image_nn; i++){
		if(threadInfo->image[i] > global->powderthresh)
			global->powderAssembled[powderClass][i] += threadInfo->image[i];
	}
	pthread_mutex_unlock(&global->powderAssembled_mutex[powderClass]);

	// Update frame counter
	global->nPowderFrames[powderClass] += 1;

	
}



/*
 *	Interpolate raw (corrected) cspad data into a physical 2D image
 *	using pre-defined pixel mapping (as loaded from .h5 file)
 */
void assemble2Dimage(tThreadInfo *threadInfo, cGlobal *global){
	
	
	// Allocate temporary arrays for pixel interpolation (needs to be floating point)
	float	*data = (float*) calloc(global->image_nn,sizeof(float));
	float	*weight = (float*) calloc(global->image_nn,sizeof(float));
	for(long i=0; i<global->image_nn; i++){
		data[i] = 0;
		weight[i]= 0;
	}
	
	
	// Loop through all pixels and interpolate onto regular grid
	float	x, y;
	float	pixel_value, w;
	long	ix, iy;
	float	fx, fy;
	long	image_index;

	for(long i=0;i<global->pix_nn;i++){
		// Pixel location with (0,0) at array element (0,0) in bottom left corner
		x = global->pix_x[i] + global->image_nx/2;
		y = global->pix_y[i] + global->image_nx/2;
		pixel_value = threadInfo->corrected_data[i];
		
		// Split coordinate into integer and fractional parts
		ix = (long) floor(x);
		iy = (long) floor(y);
		fx = x - ix;
		fy = y - iy;
		
		// Interpolate intensity over adjacent 4 pixels using fractional overlap as the weighting factor
		// (0,0)
		if(ix>=0 && iy>=0 && ix<global->image_nx && iy<global->image_nx) {
			w = (1-fx)*(1-fy);
			image_index = ix + global->image_nx*iy;
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (+1,0)
		if((ix+1)>=0 && iy>=0 && (ix+1)<global->image_nx && iy<global->image_nx) {
			w = (fx)*(1-fy);
			image_index = (ix+1) + global->image_nx*iy;
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (0,+1)
		if(ix>=0 && (iy+1)>=0 && ix<global->image_nx && (iy+1)<global->image_nx) {
			w = (1-fx)*(fy);
			image_index = ix + global->image_nx*(iy+1);
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (+1,+1)
		if((ix+1)>=0 && (iy+1)>=0 && (ix+1)<global->image_nx && (iy+1)<global->image_nx) {
			w = (fx)*(fy);
			image_index = (ix+1) + global->image_nx*(iy+1);
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
	}
	
	
	// Reweight pixel interpolation
	for(long i=0; i<global->image_nn; i++){
		if(weight[i] < 0.05)
			data[i] = 0;
		else
			data[i] /= weight[i];
	}

	

	// Check for int16 overflow
	for(long i=0;i<global->image_nn;i++){
		if(lrint(data[i]) > 32767) 
			data[i]=32767;
		if(lrint(data[i]) < -32767) 
			data[i]=-32767;
	}
	
	
	// Copy interpolated image across into int_16 image array
	for(long i=0;i<global->image_nn;i++){
		threadInfo->image[i] = (int16_t) lrint(data[i]);
	}
	
	
	// Free temporary arrays
	free(data);
	free(weight);
	
}


/*
 *	Calculate radial average of data
 *	(do this based on the raw data and pix_r rather than the assembled image -> to avoid couble interpolation effects)
 */
void calculateRadialAverage(tThreadInfo *threadInfo, cGlobal *global){
	
	// Zero arrays
	for(long i=0; i<global->radial_nn; i++) {
		threadInfo->radialAverage[i] = 0;
		threadInfo->radialAverageCounter[i] = 0;
	}
		
		
	// Radial average
	long	rbin;
	for(long i=0; i<global->pix_nn; i++){
		rbin = lrint(global->pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) rbin = 0;
		
		threadInfo->radialAverage[rbin] += threadInfo->corrected_data[i];
		threadInfo->radialAverageCounter[rbin] += 1;
	}

	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<global->radial_nn; i++) {
		if (threadInfo->radialAverageCounter[i] != 0)
			threadInfo->radialAverage[i] /= threadInfo->radialAverageCounter[i];
	}
	
}

	
	
	

void nameEvent(tThreadInfo *info, cGlobal *global){
	/*
	 *	Create filename based on date, time and fiducial for this image
	 */
	char buffer1[80];
	char buffer2[80];	
	time_t eventTime = info->seconds;
	
	struct tm *timestatic, timelocal;
	timestatic=localtime_r( &eventTime, &timelocal );	
	strftime(buffer1,80,"%Y_%b%d",&timelocal);
	strftime(buffer2,80,"%H%M%S",&timelocal);
	sprintf(info->eventname,"LCLS_%s_r%04u_%s_%x_cspad.h5",buffer1,global->runNumber,buffer2,info->fiducial);
}
	

	
/*
 *	Write out processed data to our 'standard' HDF5 format
 */
void writeHDF5(tThreadInfo *info, cGlobal *global){
	/*
	 *	Create filename based on date, time and fiducial for this image
	 */
	char outfile[1024];
	strcpy(outfile, info->eventname);
	printf("r%04u:%li (%2.1f Hz): Writing data to: %s\n",global->runNumber, info->threadNum,global->datarate, outfile);

	pthread_mutex_lock(&global->framefp_mutex);
	fprintf(global->cleanedfp, "r%04u/%s, %i, %g, %g, %g, %g\n",global->runNumber, info->eventname, info->nPeaks, info->peakNpix, info->peakTotal, info->peakResolution, info->peakDensity);
	pthread_mutex_unlock(&global->framefp_mutex);
	
		
	/* 
 	 *  HDF5 variables
	 */
	hid_t		hdf_fileID;
	hid_t		dataspace_id;
	hid_t		dataset_id;
	hid_t		datatype;
	hsize_t 	size[2],max_size[2];
	herr_t		hdf_error;
	hid_t   	gid, gidHitfinder;
	//char 		fieldname[100]; 
	
	
	/*
	 *	Create the HDF5 file
	 */
	hdf_fileID = H5Fcreate(outfile,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	
	
	
	
	
	/*
	 *	Save image data into '/data' part of HDF5 file
	 */
	gid = H5Gcreate(hdf_fileID, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gid < 0 ) {
		ERROR("%li: Couldn't create group\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}
	
	// Assembled image
	if(global->saveAssembled) {
		size[0] = global->image_nx;	// size[0] = height
		size[1] = global->image_nx;	// size[1] = width
		max_size[0] = global->image_nx;
		max_size[1] = global->image_nx;
		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gid, "assembleddata", H5T_STD_I16LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%li: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_STD_I16LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, info->image);
		if ( hdf_error < 0 ) {
			ERROR("%li: Couldn't write data\n", info->threadNum);
			H5Dclose(dataspace_id);
			H5Fclose(hdf_fileID);
			return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
	}	

	// Save raw data
	if(global->saveRaw) {
		size[0] = 8*CSPAD_ASIC_NY;	// size[0] = height
		size[1] = 8*CSPAD_ASIC_NX;	// size[1] = width
		max_size[0] = 8*CSPAD_ASIC_NY;
		max_size[1] = 8*CSPAD_ASIC_NX;
		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gid, "rawdata", H5T_STD_I16LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%li: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_STD_I16LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, info->corrected_data_int16);
		if ( hdf_error < 0 ) {
			ERROR("%li: Couldn't write data\n", info->threadNum);
			H5Dclose(dataspace_id);
			H5Fclose(hdf_fileID);
			return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
	}

	// Create symbolic link from /data/data to whatever is deemed the 'main' data set 
	if(global->saveAssembled) {
		hdf_error = H5Lcreate_soft( "/data/assembleddata", hdf_fileID, "/data/data",0,0);
	}
	else {
		hdf_error = H5Lcreate_soft( "/data/rawdata", hdf_fileID, "/data/data",0,0);
	}
	

	/*
	 *	Save radial average (always, it's not much space)
	 */
	//if(global->saveRadialAverage) 
	{
		size[0] = global->radial_nn;
		dataspace_id = H5Screate_simple(1, size, NULL);
		
		dataset_id = H5Dcreate(gid, "radialAverage", H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		//dataset_id = H5Dcreate1(hdf_fileID, "LCLS/cspadQuadTemperature", H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT);
		H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, info->radialAverage);
		H5Dclose(dataset_id);
		
		dataset_id = H5Dcreate(gid, "radialAverageCounter", H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, info->radialAverageCounter);
		H5Dclose(dataset_id);
		
		H5Sclose(dataspace_id);
	}
	
	
	/*
	 *	Save TOF data (Aqiris)
	 */
	if(info->TOFPresent==1) {
		size[0] = 2;	
		size[1] = global->AcqNumSamples;	
		max_size[0] = 2;
		max_size[1] = global->AcqNumSamples;

		double tempData[2][global->AcqNumSamples];
		memcpy(&tempData[0][0], info->TOFTime, global->AcqNumSamples);
		memcpy(&tempData[1][0], info->TOFVoltage, global->AcqNumSamples);

		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gid, "tof", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%li: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, tempData);
		if ( hdf_error < 0 ) {
			ERROR("%li: Couldn't write data\n", info->threadNum);
			H5Dclose(dataspace_id);
			H5Fclose(hdf_fileID);
			return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
	}	


	// Done with the /data group
	H5Gclose(gid);



	/*
	 * save processing info
	 */

	// Create sub-groups
	gid = H5Gcreate(hdf_fileID, "processing", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gid < 0 ) {
		ERROR("%li: Couldn't create group\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}
	gidHitfinder = H5Gcreate(gid, "hitfinder", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gid < 0 ) {
		ERROR("%li: Couldn't create group\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}

	if(global->savePeakInfo && global->hitfinder && info->nPeaks > 0 ) {
		size[0] = info->nPeaks;			// size[0] = height
		size[1] = 4;					// size[1] = width
		max_size[0] = info->nPeaks;
		max_size[1] = 4;
		double *peak_info = (double *) calloc(4*info->nPeaks, sizeof(double));
		
		// Save peak info in Assembled layout
		for (int i=0; i<info->nPeaks;i++){
			peak_info[i*4] = info->peak_com_x_assembled[i];
			peak_info[i*4+1] = info->peak_com_y_assembled[i];
			peak_info[i*4+2] = info->peak_intensity[i];
			peak_info[i*4+3] = info->peak_npix[i];
		}
		
		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gidHitfinder, "peakinfo-assembled", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%li: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, peak_info);
		if ( hdf_error < 0 ) {
			ERROR("%li: Couldn't write data\n", info->threadNum);
			H5Dclose(dataspace_id);
			H5Fclose(hdf_fileID);
			return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
		

		// Save peak info in Raw layout
		for (int i=0; i<info->nPeaks;i++){
			peak_info[i*4] = info->peak_com_x[i];
			peak_info[i*4+1] = info->peak_com_y[i];
			peak_info[i*4+2] = info->peak_intensity[i];
			peak_info[i*4+3] = info->peak_npix[i];
		}

		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gidHitfinder, "peakinfo-raw", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%li: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, peak_info);
		if ( hdf_error < 0 ) {
			 ERROR("%li: Couldn't write data\n", info->threadNum);
			 H5Dclose(dataspace_id);
			 H5Fclose(hdf_fileID);
			 return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
		
		
		// Create symbolic link from /processing/hitfinder/peakinfo to whatever is deemed the 'main' data set 
		if(global->saveAssembled) {
			hdf_error = H5Lcreate_soft( "/processing/hitfinder/peakinfo-assembled", hdf_fileID, "/processing/hitfinder/peakinfo",0,0);
		}
		else {
			hdf_error = H5Lcreate_soft( "/processing/hitfinder/peakinfo-raw", hdf_fileID, "/processing/hitfinder/peakinfo",0,0);
		}
		
		

		/*
		 * Save pixelmaps
		 * Here's the plan so far:
		 *    Pixelmaps are saved as an 8-bit unsigned int array in the hdf5 file
		 *    All bits set to 0 by default.  A pixel is flagged by setting the bit to 1.
		 *    Bit 0: if equal to 1, this is a "bad pixel".
		 *    Bit 1: if equal to 1, this is a "hot pixel".
		 *    Bit 2: if equal to 1, this is a "saturated pixel".
		 *    Bit 3: unused
		 *    Bit 4: unused
		 *    Bit 5: unused
		 *    Bit 6: unused
		 *    Bit 7: if equal to 1, this pixel is bad for miscellaneous reasions (e.g. ice rings).  
		 */	
		
		long i;
		char * pixelmasks = (char *) calloc(global->pix_nn,sizeof(char));
		for (i=0; i<global->pix_nn; i++) {
			pixelmasks[i] = 0; // default: all bits are equal to 1
			if ( global->badpixelmask[i] == 0 )
				pixelmasks[i] |= (1 << 0);
			if ( global->hotpixelmask[i] == 0 ) // Should use a mutex lock here...
				pixelmasks[i] |= (1 << 1);
			if ( info->saturatedPixelMask[i] == 0 )
				pixelmasks[i] |= (1 << 2);		
		}

		size[0] = 8*CSPAD_ASIC_NY;	// size[0] = height
		size[1] = 8*CSPAD_ASIC_NX;	// size[1] = width
		max_size[0] = 8*CSPAD_ASIC_NY;
		max_size[1] = 8*CSPAD_ASIC_NX;
		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gid, "pixelmasks", H5T_NATIVE_CHAR, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%li: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, pixelmasks);
		if ( hdf_error < 0 ) {
			ERROR("%li: Couldn't write data\n", info->threadNum);
			H5Dclose(dataspace_id);
			H5Fclose(hdf_fileID);
			return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
	


		free(pixelmasks);
		free(peak_info);
	}

	// Done with this group
	H5Gclose(gid);
	H5Gclose(gidHitfinder);
	

	
	
	//double		phaseCavityTime1;
	//double		phaseCavityTime2;
	//double		phaseCavityCharge1;
	//double		phaseCavityCharge2;
	
	/*
	 *	Write LCLS event information
	 */
	gid = H5Gcreate1(hdf_fileID,"LCLS",0);
	size[0] = 1;
	dataspace_id = H5Screate_simple( 1, size, NULL );
	//dataspace_id = H5Screate(H5S_SCALAR);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/machineTime", H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->seconds );
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/fiducial", H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fiducial );
	H5Dclose(dataset_id);
		
	// Electron beam data
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamCharge", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamCharge );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamL3Energy", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamL3Energy );
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamPkCurrBC2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamPkCurrBC2 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUPosX", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUPosX );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUPosY", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUPosY );
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUAngX", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUAngX );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUAngY", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUAngY );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityTime1", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityTime1 );
	H5Dclose(dataset_id);
	
	// Phase cavity information
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityTime2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityTime2 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityCharge1", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityCharge1 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityCharge2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityCharge2 );
	H5Dclose(dataset_id);
	
	// Calculated photon energy
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_energy_eV", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->photonEnergyeV);
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_wavelength_A", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->wavelengthA);
	H5Dclose(dataset_id);
	
	
	// Gas detector values
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_11_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd11 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_12_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd12 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_21_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd21 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_22_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd22 );	
	H5Dclose(dataset_id);

	
	// Motor positions
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/detectorPosition", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &global->detectorZ );	
	H5Dclose(dataset_id);
	
	// LaserOn event code
	int LaserOnVal = (info->laserEventCodeOn)?1:0;
	//printf("LaserOnVal %d \n", LaserOnVal);
	dataset_id = H5Dcreate1(hdf_fileID, "LCLS/evr41", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &LaserOnVal);
	H5Dclose(dataset_id);

	
	// Finished with scalar dataset ID
	H5Sclose(dataspace_id);
	

	// cspad temperature
	size[0] = 4;
	dataspace_id = H5Screate_simple(1, size, NULL);
	dataset_id = H5Dcreate1(hdf_fileID, "LCLS/cspadQuadTemperature", H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->quad_temperature[0]);
	H5Dclose(dataset_id);
	H5Sclose(dataspace_id);
	

	
	// Time in human readable format
	// Writing strings in HDF5 is a little tricky --> this could be improved!
	char* timestr;
	time_t eventTime = info->seconds;
	timestr = ctime(&eventTime);
	dataspace_id = H5Screate(H5S_SCALAR);
	datatype = H5Tcopy(H5T_C_S1);  
	H5Tset_size(datatype,strlen(timestr)+1);
	dataset_id = H5Dcreate1(hdf_fileID, "LCLS/eventTimeString", datatype, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, timestr );
	H5Dclose(dataset_id);
	H5Sclose(dataspace_id);
	hdf_error = H5Lcreate_soft( "/LCLS/eventTimeString", hdf_fileID, "/LCLS/eventTime",0,0);
	
	
	
	// Close group and flush buffers
	H5Gclose(gid);
	H5Fflush(hdf_fileID,H5F_SCOPE_LOCAL);

	
	/*
	 *	Clean up stale HDF5 links
	 *		(thanks Tom/Filipe)
	 */
	int n_ids;
	hid_t ids[256];
	n_ids = H5Fget_obj_ids(hdf_fileID, H5F_OBJ_ALL, 256, ids);
	for ( int i=0; i<n_ids; i++ ) {
		hid_t id;
		H5I_type_t type;
		id = ids[i];
		type = H5Iget_type(id);
		if ( type == H5I_GROUP ) H5Gclose(id);
		if ( type == H5I_DATASET ) H5Dclose(id);
		if ( type == H5I_DATATYPE ) H5Tclose(id);
		if ( type == H5I_DATASPACE ) H5Sclose(id);
		if ( type == H5I_ATTR ) H5Aclose(id);
	}
	
	H5Fclose(hdf_fileID); 
}


void writePeakFile(tThreadInfo *threadInfo, cGlobal *global){

	// No peaks --> go home
	if(threadInfo->nPeaks <= 0) {
		return;
	}
	
	// Dump peak info to file
	pthread_mutex_lock(&global->peaksfp_mutex);
	fprintf(global->peaksfp, "%s\n", threadInfo->eventname);
	fprintf(global->peaksfp, "photonEnergy_eV=%f\n", threadInfo->photonEnergyeV);
	fprintf(global->peaksfp, "wavelength_A=%f\n", threadInfo->wavelengthA);
	fprintf(global->peaksfp, "pulseEnergy_mJ=%f\n", (float)(threadInfo->gmd21+threadInfo->gmd21)/2);
	fprintf(global->peaksfp, "npeaks=%i\n", threadInfo->nPeaks);
	fprintf(global->peaksfp, "peakResolution=%g\n", threadInfo->peakResolution);
	fprintf(global->peaksfp, "peakDensity=%g\n", threadInfo->peakDensity);
	fprintf(global->peaksfp, "peakNpix=%g\n", threadInfo->peakNpix);
	fprintf(global->peaksfp, "peakTotal=%g\n", threadInfo->peakTotal);
	
	for(long i=0; i<threadInfo->nPeaks; i++) {
		fprintf(global->peaksfp, "%f, %f, %f, %f, %g, %g\n", threadInfo->peak_com_x_assembled[i], threadInfo->peak_com_y_assembled[i], threadInfo->peak_com_x[i], threadInfo->peak_com_y[i], threadInfo->peak_npix[i], threadInfo->peak_intensity[i]);
	}
	pthread_mutex_unlock(&global->peaksfp_mutex);
	
	
}



/*
 *	Write test data to a simple HDF5 file
 */

void writeSimpleHDF5(const char *filename, const void *data, int width, int height, int type) 
{
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	herr_t r;
	hsize_t size[2];
	hsize_t max_size[2];
	
	fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if ( fh < 0 ) {
		ERROR("Couldn't create file: %s\n", filename);
	}
	
	gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gh < 0 ) {
		ERROR("Couldn't create group\n");
		H5Fclose(fh);
	}
	
	size[0] = height;
	size[1] = width;
	max_size[0] = height;
	max_size[1] = width;
	sh = H5Screate_simple(2, size, max_size);
	
	dh = H5Dcreate(gh, "data", type, sh,
	               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( dh < 0 ) {
		ERROR("Couldn't create dataset\n");
		H5Fclose(fh);
	}
	
	/* Muppet check */
	H5Sget_simple_extent_dims(sh, size, max_size);
	
	r = H5Dwrite(dh, type, H5S_ALL,
	             H5S_ALL, H5P_DEFAULT, data);
	if ( r < 0 ) {
		ERROR("Couldn't write data\n");
		H5Dclose(dh);
		H5Fclose(fh);
	}
	
	H5Gclose(gh);
	H5Dclose(dh);
	
	
	/*
	 *	Clean up stale HDF5 links
	 *		(thanks Tom/Filipe)
	 */
	int n_ids;
	hid_t ids[256];
	n_ids = H5Fget_obj_ids(fh, H5F_OBJ_ALL, 256, ids);
	for ( int i=0; i<n_ids; i++ ) {
		hid_t id;
		H5I_type_t type;
		id = ids[i];
		type = H5Iget_type(id);
		if ( type == H5I_GROUP ) H5Gclose(id);
		if ( type == H5I_DATASET ) H5Dclose(id);
		if ( type == H5I_DATATYPE ) H5Tclose(id);
		if ( type == H5I_DATASPACE ) H5Sclose(id);
		if ( type == H5I_ATTR ) H5Aclose(id);
	}
	
	
	H5Fclose(fh);
}


void saveRunningSums(cGlobal *global) {
	char	filename[1024];

	/*
	 *	Save powder patterns from different classes
	 */
	printf("Writing intermediate powder patterns to file\n");
	for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
		double *buffer;
		
		// Raw data
		sprintf(filename,"r%04u-class%i-sumRaw.h5",global->runNumber, powderType);
		buffer = (double*) calloc(global->pix_nn, sizeof(double));
		pthread_mutex_lock(&global->powderRaw_mutex[powderType]);
		memcpy(buffer, global->powderRaw[powderType], global->pix_nn*sizeof(double));
		pthread_mutex_unlock(&global->powderRaw_mutex[powderType]);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		// Blanks squared (for calculation of variance)
		sprintf(filename,"r%04u-class%i-sumRawSquared.h5",global->runNumber, powderType);
		buffer = (double*) calloc(global->pix_nn, sizeof(double));
		pthread_mutex_lock(&global->powderRawSquared_mutex[powderType]);
		memcpy(buffer, global->powderRawSquared[powderType], global->pix_nn*sizeof(double));
		pthread_mutex_unlock(&global->powderRawSquared_mutex[powderType]);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		// Sigma (variance)
		sprintf(filename,"r%04u-class%i-sumRawSigma.h5",global->runNumber, powderType);
		buffer = (double*) calloc(global->pix_nn, sizeof(double));
		pthread_mutex_lock(&global->powderRaw_mutex[powderType]);
		pthread_mutex_lock(&global->powderRawSquared_mutex[powderType]);
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = sqrt(global->powderRawSquared[powderType][i]/global->nPowderFrames[powderType] - (global->powderRaw[powderType][i]*global->powderRaw[powderType][i]/(global->nPowderFrames[powderType]*global->nPowderFrames[powderType])));
		pthread_mutex_unlock(&global->powderRaw_mutex[powderType]);
		pthread_mutex_unlock(&global->powderRawSquared_mutex[powderType]);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		// Assembled sum
		sprintf(filename,"r%04u-class%i-sumAssembled.h5",global->runNumber, powderType);
		buffer = (double*) calloc(global->image_nn, sizeof(double));
		pthread_mutex_lock(&global->powderAssembled_mutex[powderType]);
		memcpy(buffer, global->powderAssembled[powderType], global->image_nn*sizeof(double));
		pthread_mutex_unlock(&global->powderAssembled_mutex[powderType]);
		writeSimpleHDF5(filename, buffer, global->image_nx, global->image_nx, H5T_NATIVE_DOUBLE);	
		free(buffer);
	}	

	/*
	 *	Compute and save darkcal
	 */
	if(global->generateDarkcal) {
		printf("Processing darkcal\n");
		sprintf(filename,"r%04u-darkcal.h5",global->runNumber);
		int16_t *buffer = (int16_t*) calloc(global->pix_nn, sizeof(int16_t));
		pthread_mutex_lock(&global->powderRaw_mutex[1]);
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = (int16_t) lrint(global->powderRaw[1][i]/global->nPowderFrames[1]);
		pthread_mutex_unlock(&global->powderRaw_mutex[1]);
		printf("Saving darkcal to file: %s\n", filename);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_STD_I16LE);	
		free(buffer);
	}

	/*
	 *	Compute and save gain calibration
	 */
	else if(global->generateGaincal) {
		printf("Processing gaincal\n");
		sprintf(filename,"r%04u-gaincal.h5",global->runNumber);
		// Calculate average intensity per frame
		pthread_mutex_lock(&global->powderRaw_mutex[1]);
		double *buffer = (double*) calloc(global->pix_nn, sizeof(double));
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = (global->powderRaw[1][i]/global->nPowderFrames[1]);
		pthread_mutex_unlock(&global->powderRaw_mutex[1]);

		// Find median value (this value will become gain=1)
		float *buffer2 = (float*) calloc(global->pix_nn, sizeof(float));
		for(long i=0; i<global->pix_nn; i++) {
			buffer2[i] = buffer[i];
		}
		float	dc;
		dc = kth_smallest(buffer2, global->pix_nn, lrint(0.5*global->pix_nn));
		printf("offset=%f\n",dc);
		free(buffer2);
		if(dc <= 0){
			printf("Error calculating gain, offset = %i\n",dc);
			return;
		}
		// gain=1 for a median value pixel, and is bounded between a gain of 0.1 and 10
		for(long i=0; i<global->pix_nn; i++) {
			buffer[i] /= (double) dc;
			if(buffer[i] < 0.1 || buffer[i] > 10)
				buffer[i]=0;
		}
		printf("Saving gaincal to file: %s\n", filename);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		
	}

	
	
}

