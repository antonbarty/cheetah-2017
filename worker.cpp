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
#include "cspad-gjw/CspadTemp.hh"
#include "cspad-gjw/CspadCorrector.hh"
#include "cspad-gjw/CspadGeometry.hh"

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
	threadInfo->raw_data = (uint16_t*) calloc(8*ROWS*8*COLS,sizeof(uint16_t));
	for(int quadrant=0; quadrant<4; quadrant++) {
		long	i,j,ii;
		for(long k=0; k<2*ROWS*8*COLS; k++) {
			i = k % (2*ROWS) + quadrant*(2*ROWS);
			j = k / (2*ROWS);
			ii  = i+(8*ROWS)*j;
			threadInfo->raw_data[ii] = threadInfo->quad_data[quadrant][k];
		}
	}
	
	/*
	 *	Create additional arrays for corrected data, etc
	 */
	threadInfo->corrected_data = (float*) calloc(8*ROWS*8*COLS,sizeof(float));
	threadInfo->corrected_data_int16 = (int16_t*) calloc(8*ROWS*8*COLS,sizeof(int16_t));
	threadInfo->detector_corrected_data = (float*) calloc(8*ROWS*8*COLS,sizeof(float));
	threadInfo->image = (int16_t*) calloc(global->image_nn,sizeof(int16_t));
	threadInfo->peak_com_x = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_y = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_intensity = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));	
	threadInfo->peak_npix = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));	
	for(long i=0;i<global->pix_nn;i++)
		threadInfo->corrected_data[i] = threadInfo->raw_data[i];

	
	
	/*
	 *	Create a unique name for this event
	 */
	nameEvent(threadInfo, global);
		
	
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
	pthread_mutex_lock(&global->bgbuffer_mutex);
	if( ( (global->bgCounter % global->bgRecalc) == 0 || global->bgCounter == global->bgMemory) && global->bgCounter != global->last_bg_update ) {
		calculatePersistentBackground(global);
	}
	pthread_mutex_unlock(&global->bgbuffer_mutex);

	
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
	 *	Maintain a running sum of data (powder patterns)
	 */
	if(global->powderSumHits==1 && hit==1) {
		addToPowder(threadInfo, global, hit);
	}
	if(global->powderSumBlanks==1 && hit==0){
		addToPowder(threadInfo, global, hit);
	} 
	if(global->generateDarkcal==1){
		addToPowder(threadInfo, global, 1);
	} 
	if(global->generateGaincal==1){
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
	fprintf(global->framefp, "%li, %i, %s, %i\n",threadInfo->threadNum, threadInfo->seconds, threadInfo->eventname, threadInfo->nPeaks);
	pthread_mutex_unlock(&global->framefp_mutex);
	
	
	
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
	free(threadInfo->peak_com_x);
	free(threadInfo->peak_com_y);
	free(threadInfo->peak_intensity);
	free(threadInfo->peak_npix);
	free(threadInfo);

	//TOF stuff.
	if(threadInfo->TOFPresent){
		free(threadInfo->TOFTime);
		free(threadInfo->TOFVoltage); 
	}
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
 *	Apply bad pixel mask
 *	Assumes that all we have to do here is a multiplication.
 */
void applyBadPixelMask(tThreadInfo *threadInfo, cGlobal *global){
	
	for(long i=0;i<global->pix_nn;i++) 
		threadInfo->corrected_data[i] *= global->badpixelmask[i];
	
}


/*
 *	Subtract common mode on each module
 *	Common mode is the kth lowest pixel value in the ASIC (similar to a median calculation)
 */
void cmModuleSubtract(tThreadInfo *threadInfo, cGlobal *global){

	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		e;
	//long		counter;
	//uint16_t	value;
	int			mval = lrint(global->cmFloor*ROWS*COLS);
	uint16_t	median;
	
	// Create histogram array

	//int			nhist = 65535;
	//uint16_t	*histogram; 
	//histogram = (uint16_t*) calloc(nhist, sizeof(uint16_t));

	int16_t	*buffer; 
	buffer = (int16_t*) calloc(COLS*ROWS, sizeof(int16_t));
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){

			// Zero histogram
			//memset(histogram, 0, nhist*sizeof(uint16_t));
			
			
			// Loop over pixels within a module
			for(long j=0; j<COLS; j++){
				for(long i=0; i<ROWS; i++){
					e = (j + mj*COLS) * (8*ROWS);
					e += i + mi*ROWS;
					//histogram[lrint(threadInfo->corrected_data[e])] += 1;
					buffer[j+i*COLS] = lrint(threadInfo->corrected_data[e]);
				}
			}
			
			median = kth_smallest(buffer, COLS*ROWS, mval);
			// Find median value
			//counter = 0;
			//for(long i=0; i<nhist; i++){
			//	counter += histogram[i];
			//	if(counter > (global->cmFloor*ROWS*COLS)) {
			//		median = i;
			//		break;
			//	}
			//}
			//DEBUGL2_ONLY printf("Median of module (%i,%i) = %i\n",mi,mj,median);

			// Subtract median value
			for(long i=0; i<ROWS; i++){
				for(long j=0; j<COLS; j++){
					e = (j + mj*COLS) * (8*ROWS);
					e += i + mi*ROWS;
					threadInfo->corrected_data[e] -= median;

					// Zero checking only needed if corrected data is uint16
					//value = threadInfo->corrected_data[e];
					//if(value > median)
					//	threadInfo->corrected_data[e] -= median;
					//else
					//	threadInfo->corrected_data[e] = 0;
				}
			}
		}
	}
	free(buffer);
	//free(histogram);
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
			for(long j=0; j<COLS-1; j+=10){
				long i=j;
				e = (j + mj*COLS) * (8*ROWS);
				e += i + mi*ROWS;
				background += threadInfo->corrected_data[e];
				counter += 1;
			}
			background /= counter;
			
			//printf("%f ",background);
						
			// Subtract background from entire ASIC
			for(long j=0; j<COLS; j++){
				for(long i=0; i<ROWS; i++){
					e = (j + mj*COLS) * (8*ROWS);
					e += i + mi*ROWS;
					threadInfo->corrected_data[e] -= background;
					
				}
			}
		}
	}
	//printf("\n");
	
}




/*
 *	Local background subtraction
 */
void subtractLocalBackground(tThreadInfo *threadInfo, cGlobal *global){
	
	// ROWS = 194;
	// COLS = 185;
	
	long		e,ee;
	long		counter;
	
	
	// Search subunits
	if(global->localBackgroundRadius <= 0 || global->localBackgroundRadius >= COLS/2 )
		return;
	long nn = (2*global->localBackgroundRadius+1);
	nn=nn*nn;
	
	
	// Create local arrays needed for background subtraction
	int16_t *localBg = (int16_t*) calloc(global->pix_nn, sizeof(int16_t)); 
	int16_t	*buffer = (int16_t*) calloc(nn, sizeof(int16_t));

	
	
	// Loop over ASIC modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){
			
			// Loop over pixels within a module
			for(long j=0; j<COLS; j++){
				for(long i=0; i<ROWS; i++){

					counter = 0;
					e = (j+mj*COLS)*global->pix_nx;
					e += i+mi*ROWS;
					
					// Loop over median window
					for(long jj=-global->localBackgroundRadius; jj<=global->localBackgroundRadius; jj++){
						for(long ii=-global->localBackgroundRadius; ii<=global->localBackgroundRadius; ii++){

							// Quick array bounds check
							if((i+ii) < 0)
								continue;
							if((i+ii) >= ROWS)
								continue;
							if((j+jj) < 0)
								continue;
							if((j+jj) >= COLS)
								continue;

							ee = (j+jj+mj*COLS)*global->pix_nx;
							ee += i+ii+mi*ROWS;

							if(ee < 0 || ee >= global->pix_nn){
								printf("Error: Array bounds error: e = %li > %li\n",e,global->pix_nn);
								continue;
							}
							
							buffer[counter] = lrint(threadInfo->corrected_data[ee]);
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
 *	A basic hitfinder
 */
int  hitfinder(tThreadInfo *threadInfo, cGlobal *global){

	long	nat, lastnat;
	long	counter;
	int		hit=0;
	long	ii,nn;

	nat = 0;
	counter = 0;

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
		
		case 1 :	// Simply count the number of pixels above ADC threshold (very basic)
			for(long i=0;i<global->pix_nn;i++){
				if(temp[i] > global->hitfinderADC){
					nat++;
				}
			}
			if(nat >= global->hitfinderNAT)
				hit = 1;
			break;

	
		case 2 :	//	Count clusters of pixels above threshold
			for(long j=1; j<8*COLS-1; j++){
				for(long i=1; i<8*ROWS-1; i++) {
					nn = 0;
					ii = i+(8*ROWS)*j;
					if(temp[i+(8*ROWS)*j] > global->hitfinderADC) {
						nn += 1;
						if(temp[i+1+(8*ROWS)*j] > global->hitfinderADC) nn++;
						if(temp[i-1+(8*ROWS)*j] > global->hitfinderADC) nn++;
						if(temp[i+(8*ROWS)*(j+1)] > global->hitfinderADC) nn++;
						if(temp[i+1+(8*ROWS)*(j+1)] > global->hitfinderADC) nn++;
						if(temp[i-1+(8*ROWS)*(j+1)] > global->hitfinderADC) nn++;
						if(temp[i+(8*ROWS)*(j-1)] > global->hitfinderADC) nn++;
						if(temp[i+1+(8*ROWS)*(j-1)] > global->hitfinderADC) nn++;
						if(temp[i-1+(8*ROWS)*(j-1)] > global->hitfinderADC) nn++;
					}
					if(nn >= global->hitfinderCluster) {
						nat++;
						temp[i+(8*ROWS)*j] = 0;
						temp[i+1+(8*ROWS)*j] = 0;
						temp[i-1+(8*ROWS)*j] = 0;
						temp[i+(8*ROWS)*(j+1)] = 0;
						temp[i+1+(8*ROWS)*(j+1)] = 0;
						temp[i-1+(8*ROWS)*(j+1)] = 0;
						temp[i+(8*ROWS)*(j-1)] = 0;
						temp[i+1+(8*ROWS)*(j-1)] = 0;
						temp[i-1+(8*ROWS)*(j-1)] = 0;
					}
				}
			}
			threadInfo->nPeaks = nat;
			if(nat >= global->hitfinderMinPixCount)
				hit = 1;
			break;


	
	
		case 3 : 	// Real peak counter
		default:
			int search_x[] = {-1,0,1,-1,1,-1,0,1};
			int search_y[] = {-1,-1,-1,0,0,1,1,1};
			int	search_n = 8;
			long e;
			long *inx = (long *) calloc(global->pix_nn, sizeof(long));
			long *iny = (long *) calloc(global->pix_nn, sizeof(long));
			float totI;
			float peak_com_x;
			float peak_com_y;
			int thisx;
			int thisy;

			// Loop over modules (8x8 array)
			for(long mj=0; mj<8; mj++){
				for(long mi=0; mi<8; mi++){
					
					// Loop over pixels within a module
					for(long j=1; j<COLS-1; j++){
						for(long i=1; i<ROWS-1; i++){

							e = (j+mj*COLS)*global->pix_nx;
							e += i+mi*ROWS;

							//if(e >= global->pix_nn)
							//	printf("Array bounds error: e=%i\n");
							
							if(temp[e] > global->hitfinderADC){
								// This might be the start of a peak - start searching
								inx[0] = i;
								iny[0] = j;
								nat = 1;
								totI = 0; 
								peak_com_x = 0; 
								peak_com_y = 0; 
								
								// Keep looping until the pixel count within this peak does not change (!)
								do {
									lastnat = nat;
									// Loop through points known to be within this peak
									for(long p=0; p<nat; p++){
										// Loop through search pattern
										for(long k=0; k<search_n; k++){
											// Array bounds check
											if((inx[p]+search_x[k]) < 0)
												continue;
											if((inx[p]+search_x[k]) >= ROWS)
												continue;
											if((iny[p]+search_y[k]) < 0)
												continue;
											if((iny[p]+search_y[k]) >= COLS)
												continue;
											
											// Neighbour point 
											thisx = (iny[p]+search_y[k]+mj*COLS);
											thisy = inx[p]+search_x[k]+mi*ROWS;
											e = thisx*global->pix_nx + thisy;
											
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
									
									if ( counter > global->hitfinderNpeaksMax ) 
										continue;
									
									threadInfo->peak_intensity[counter] = totI;
									threadInfo->peak_com_x[counter] = peak_com_x/totI;
									threadInfo->peak_com_y[counter] = peak_com_y/totI;
									threadInfo->peak_npix[counter] = nat;
									counter++;
								}
							}
						}
					}
				}
			}	
			// Hit?
			threadInfo->nPeaks = counter;
			if(counter >= global->hitfinderNpeaks && counter <= global->hitfinderNpeaksMax)
				hit = 1;
			
			free(inx);
			free(iny);
			break;
	}
		
	
	
	// Update central hit counter
	if(hit) {
		pthread_mutex_lock(&global->nhits_mutex);
		global->nhits++;
		pthread_mutex_unlock(&global->nhits_mutex);
	}
	
	free(temp);
	return(hit);
}



	

/*
 *	Maintain running powder patterns
 *	Use a buffer to minimise time spent within mutex locks
 */
void addToPowder(tThreadInfo *threadInfo, cGlobal *global, int hit){
	
	
	int		useBuffer=0;
	double  *buffer;
	

	
	/*
	 *	Version without using a buffer (comparison in mutex block)
	 */
	if(useBuffer == 0) {
		
		if(hit == 1) {
			// Assembled data
			pthread_mutex_lock(&global->powderHitsAssembled_mutex);
			global->npowderHits += 1;
			for(long i=0; i<global->image_nn; i++){
				if(threadInfo->image[i] > global->powderthresh)
					global->powderHitsAssembled[i] += threadInfo->image[i];
			}
			pthread_mutex_unlock(&global->powderHitsAssembled_mutex);
			
			// Raw data
			pthread_mutex_lock(&global->powderHitsRaw_mutex);
			for(long i=0; i<global->pix_nn; i++) {
				if(threadInfo->corrected_data[i] > global->powderthresh)
					global->powderHitsRaw[i] += threadInfo->corrected_data[i];
			}
			pthread_mutex_unlock(&global->powderHitsRaw_mutex);			

			// Raw data squared (for calculating variance)
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			for(long i=0; i<global->pix_nn; i++) 
				buffer[i] = 0;
			for(long i=0; i<global->pix_nn; i++) 
				if(threadInfo->corrected_data[i] > global->powderthresh)
					buffer[i] = (threadInfo->corrected_data[i]*threadInfo->corrected_data[i]);
			pthread_mutex_lock(&global->powderHitsRawSquared_mutex);
			for(long i=0; i<global->pix_nn; i++) 
				global->powderHitsRawSquared[i] += buffer[i];
			pthread_mutex_unlock(&global->powderHitsRawSquared_mutex);			
			free(buffer);
		}
		else {
			// Assembled data
			pthread_mutex_lock(&global->powderBlanksAssembled_mutex);
			global->npowderBlanks += 1;
			for(long i=0; i<global->image_nn; i++){
				if(threadInfo->image[i] > global->powderthresh)
					global->powderBlanksAssembled[i] += threadInfo->image[i];
			}
			pthread_mutex_unlock(&global->powderBlanksAssembled_mutex);
			
			// Raw data
			pthread_mutex_lock(&global->powderBlanksRaw_mutex);
			for(long i=0; i<global->pix_nn; i++) {
				if(threadInfo->corrected_data[i] > global->powderthresh)
					global->powderBlanksRaw[i] += threadInfo->corrected_data[i];
			}
			pthread_mutex_unlock(&global->powderBlanksRaw_mutex);			
		
			// Raw data squared (for calculating variance)
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			for(long i=0; i<global->pix_nn; i++) 
				buffer[i] = 0;
			for(long i=0; i<global->pix_nn; i++) 
				if(threadInfo->corrected_data[i] > global->powderthresh)
					buffer[i] = (threadInfo->corrected_data[i]*threadInfo->corrected_data[i]);
			pthread_mutex_lock(&global->powderBlanksRawSquared_mutex);
			for(long i=0; i<global->pix_nn; i++) 
				global->powderBlanksRawSquared[i] += buffer[i];
			pthread_mutex_unlock(&global->powderBlanksRawSquared_mutex);			
			free(buffer);
		}
	}
	
	/*
	 *	Version with comparison and type conversion outside mutex block
	 * 
	 */
	else {
		if(hit == 1) {
			// Assembled data
			buffer = (double*) calloc(global->image_nn, sizeof(double));
			for(long i=0; i<global->image_nn; i++){
				if(threadInfo->image[i] > global->powderthresh)
					buffer[i] = threadInfo->image[i];
				else
					buffer[i] = 0;
			}
			pthread_mutex_lock(&global->powderHitsAssembled_mutex);
			global->npowderHits += 1;
			for(long i=0; i<global->image_nn; i++){
				global->powderHitsAssembled[i] += buffer[i];
			}
			pthread_mutex_unlock(&global->powderHitsAssembled_mutex);
			free(buffer);
			
			// Raw data
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			for(long i=0; i<global->pix_nn; i++){
				if(threadInfo->corrected_data[i] > global->powderthresh)
					buffer[i] = threadInfo->corrected_data[i];
				else
					buffer[i] = 0;
			}
			pthread_mutex_lock(&global->powderHitsRaw_mutex);
			for(long i=0; i<global->pix_nn; i++) {
				global->powderHitsRaw[i] += buffer[i];
			}
			pthread_mutex_unlock(&global->powderHitsRaw_mutex);			
			free(buffer);
		}
		else {
			// Assembled data
			buffer = (double*) calloc(global->image_nn, sizeof(double));
			for(long i=0; i<global->image_nn; i++){
				if(threadInfo->image[i] > global->powderthresh)
					buffer[i] = threadInfo->image[i];
				else
					buffer[i] = 0;
			}
			pthread_mutex_lock(&global->powderBlanksAssembled_mutex);
			global->npowderBlanks += 1;
			for(long i=0; i<global->image_nn; i++){
				global->powderBlanksAssembled[i] += buffer[i];
			}
			pthread_mutex_unlock(&global->powderBlanksAssembled_mutex);
			free(buffer);
			
			// Raw data
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			for(long i=0; i<global->pix_nn; i++){
				if(threadInfo->corrected_data[i] > global->powderthresh)
					buffer[i] = threadInfo->corrected_data[i];
				else
					buffer[i] = 0;
			}
			pthread_mutex_lock(&global->powderBlanksRaw_mutex);
			for(long i=0; i<global->pix_nn; i++) {
				global->powderBlanksRaw[i] += buffer[i];
			}
			pthread_mutex_unlock(&global->powderBlanksRaw_mutex);			
			free(buffer);
		}
	}
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



void nameEvent(tThreadInfo *info, cGlobal *global){
	/*
	 *	Create filename based on date, time and fiducial for this image
	 */
	//char outfile[1024];
	char buffer1[80];
	char buffer2[80];	
	time_t eventTime = info->seconds;
	
	//setenv("TZ","US/Pacific",1);		// <--- Dangerous (not thread safe!)
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
	//char buffer1[80];
	//char buffer2[80];	
	//time_t eventTime = info->seconds;

	//setenv("TZ","US/Pacific",1);		// <--- Dangerous (not thread safe!)
	//struct tm *timestatic, timelocal;
	//timestatic=localtime_r( &eventTime, &timelocal );	
	//strftime(buffer1,80,"%Y_%b%d",&timelocal);
	//strftime(buffer2,80,"%H%M%S",&timelocal);
	//sprintf(outfile,"LCLS_%s_r%04u_%s_%x_cspad.h5",buffer1,global->runNumber,buffer2,info->fiducial);

	strcpy(outfile, info->eventname);
	printf("r%04u:%li (%2.1f Hz): Writing data to: %s\n",global->runNumber, info->threadNum,global->datarate, outfile);

	pthread_mutex_lock(&global->framefp_mutex);
	fprintf(global->cleanedfp, "r%04u/%s, %i\n",global->runNumber, info->eventname, info->nPeaks);
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
	hid_t   	gid, gid2;
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
		size[0] = 8*COLS;	// size[0] = height
		size[1] = 8*ROWS;	// size[1] = width
		max_size[0] = 8*COLS;
		max_size[1] = 8*ROWS;
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
	
	
	// Done with thie /data group
	H5Gclose(gid);



	/*
	 * save peak info
	 */
	gid = H5Gcreate(hdf_fileID, "processing", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gid < 0 ) {
		ERROR("%li: Couldn't create group\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}

	gid2 = H5Gcreate(gid, "hitfinder", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gid < 0 ) {
		ERROR("%li: Couldn't create group\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}

   if(global->savePeakInfo && global->hitfinder && info->nPeaks > 0 ) {

		size[0] = info->nPeaks;		// size[0] = height
		size[1] = 3;					// size[1] = width
		max_size[0] = info->nPeaks;
		max_size[1] = 3;
		double * peak_info = (double *) calloc(3*info->nPeaks, sizeof(double));
		for (int i=0; i<info->nPeaks;i++){
			peak_info[i*3] = info->peak_com_y[i];
			peak_info[i*3+1] = info->peak_com_x[i];
			peak_info[i*3+2] = info->peak_intensity[i];
		}
		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gid2, "peakinfo", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
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
	   free(peak_info);
   }

   // Done with this group
   H5Gclose(gid);
   H5Gclose(gid2);

	
	
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
	for(long i=0; i<threadInfo->nPeaks; i++) {
		fprintf(global->peaksfp, "%f, %f, %f, %f\n", threadInfo->peak_com_x[i], threadInfo->peak_com_y[i], threadInfo->peak_npix[i], threadInfo->peak_intensity[i]);
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
	H5Fclose(fh);
}


void saveRunningSums(cGlobal *global) {
	char	filename[1024];

	/*
	 *	Compute and save darkcal
	 */
	if(global->generateDarkcal) {
		printf("Processing darkcal\n");
		sprintf(filename,"r%04u-darkcal.h5",global->runNumber);
		int16_t *buffer = (int16_t*) calloc(global->pix_nn, sizeof(int16_t));
		pthread_mutex_lock(&global->powderHitsRaw_mutex);
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = (int16_t) lrint((global->powderHitsRaw[i]/global->npowderHits));
		pthread_mutex_unlock(&global->powderHitsRaw_mutex);
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
		pthread_mutex_lock(&global->powderHitsRaw_mutex);
		double *buffer = (double*) calloc(global->pix_nn, sizeof(double));
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = (global->powderHitsRaw[i]/global->npowderHits);
		pthread_mutex_unlock(&global->powderHitsRaw_mutex);
		// Find median value (this value will become gain=1)
		int16_t *buffer2 = (int16_t*) calloc(global->pix_nn, sizeof(int16_t));
		for(long i=0; i<global->pix_nn; i++) {
			buffer2[i] = (int16_t) lrint(buffer[i]);
		}
		long median_element = lrint(0.5*global->pix_nn);
		printf("Finding %lith smallest element of %li pixels\n",median_element,global->pix_nn);	
		int16_t	offset;
		offset = kth_smallest(buffer2, global->pix_nn, median_element);
		printf("offset=%i\n",offset);
		free(buffer2);
		if(offset <= 0){
			printf("Error calculating gain, offset = %i\n",offset);
			return;
		}
		// gain=1 for a median value pixel, and is bounded between a gain of 0.1 and 10
		for(long i=0; i<global->pix_nn; i++) {
			buffer[i] /= (double) offset;
			if(buffer[i] < 0.1 || buffer[i] > 10)
				buffer[i]=0;
		}
		printf("Saving gaincal to file: %s\n", filename);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		
	}

	
	else {
		double *buffer;

		/*
		 *	Save assembled powder patterns
		 *	(Use buffer so that mutex does not have to wait for HDF5 writing to complete)
		 */		
		if(global->powderSumHits) {
			// Hits
			printf("Saving summed hits assembled to file\n");
			sprintf(filename,"r%04u-sumHitsAssembled.h5",global->runNumber);
			buffer = (double*) calloc(global->image_nn, sizeof(double));
			pthread_mutex_lock(&global->powderHitsAssembled_mutex);
			memcpy(buffer, global->powderHitsAssembled, global->image_nn*sizeof(double));
			pthread_mutex_unlock(&global->powderHitsAssembled_mutex);
			writeSimpleHDF5(filename, buffer, global->image_nx, global->image_nx, H5T_NATIVE_DOUBLE);	
			free(buffer);
		}

		if(global->powderSumBlanks) {
			// Blanks
			printf("Saving summed blanks assembled to file\n");
			sprintf(filename,"r%04u-sumBlanksAssembled.h5",global->runNumber);
			buffer = (double*) calloc(global->image_nn, sizeof(double));
			pthread_mutex_lock(&global->powderBlanksAssembled_mutex);
			memcpy(buffer, global->powderBlanksAssembled, global->image_nn*sizeof(double));
			pthread_mutex_unlock(&global->powderBlanksAssembled_mutex);
			writeSimpleHDF5(filename, buffer, global->image_nx, global->image_nx, H5T_NATIVE_DOUBLE);	
			free(buffer);
		}		

		/*
		 *	Save powder patterns in raw layout
		 *	(Use buffer so that mutex does not have to wait for HDF5 writing to complete)
		 */		
		if(global->powderSumHits) {
			// Hits
			printf("Saving summed hits raw to file\n");
			sprintf(filename,"r%04u-sumHitsRaw.h5",global->runNumber);
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			pthread_mutex_lock(&global->powderHitsRaw_mutex);
			memcpy(buffer, global->powderHitsRaw, global->pix_nn*sizeof(double));
			pthread_mutex_unlock(&global->powderHitsRaw_mutex);
			writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
			free(buffer);
			
			// Hits squared (for calculation of variance)
			sprintf(filename,"r%04u-sumHitsRawSquared.h5",global->runNumber);
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			pthread_mutex_lock(&global->powderHitsRawSquared_mutex);
			memcpy(buffer, global->powderHitsRawSquared, global->pix_nn*sizeof(double));
			pthread_mutex_unlock(&global->powderHitsRawSquared_mutex);
			writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
			free(buffer);
		}
		
		if(global->powderSumBlanks) {
			// Blanks
			printf("Saving summed blanks raw to file\n");
			sprintf(filename,"r%04u-sumBlanksRaw.h5",global->runNumber);
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			pthread_mutex_lock(&global->powderBlanksRaw_mutex);
			memcpy(buffer, global->powderBlanksRaw, global->pix_nn*sizeof(double));
			pthread_mutex_unlock(&global->powderBlanksRaw_mutex);
			writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
			free(buffer);
			
			// Blanks squared (for calculation of variance)
			sprintf(filename,"r%04u-sumBlanksRawSquared.h5",global->runNumber);
			buffer = (double*) calloc(global->pix_nn, sizeof(double));
			pthread_mutex_lock(&global->powderBlanksRawSquared_mutex);
			memcpy(buffer, global->powderBlanksRawSquared, global->pix_nn*sizeof(double));
			pthread_mutex_unlock(&global->powderBlanksRawSquared_mutex);
			writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
			free(buffer);
		}
		
	}

	
}

