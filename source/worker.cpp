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
	threadInfo->raw_data = (uint16_t*) calloc(CSPAD_nASICS_X*CSPAD_ASIC_NX*CSPAD_nASICS_Y*CSPAD_ASIC_NY,sizeof(uint16_t));
	for(int quadrant=0; quadrant<4; quadrant++) {
		long	i,j,ii;
		for(long k=0; k<2*CSPAD_ASIC_NX*8*CSPAD_ASIC_NY; k++) {
			i = k % (2*CSPAD_ASIC_NX) + quadrant*(2*CSPAD_ASIC_NX);
			j = k / (2*CSPAD_ASIC_NX);
			ii  = i+(CSPAD_nASICS_X*CSPAD_ASIC_NX)*j;
			threadInfo->raw_data[ii] = threadInfo->quad_data[quadrant][k];
		}
	}
	
	/*
	 *	Create arrays for corrected data, etc needed by this thread
	 */
	threadInfo->corrected_data = (float*) calloc(CSPAD_nASICS_X*CSPAD_ASIC_NX*CSPAD_nASICS_Y*CSPAD_ASIC_NY,sizeof(float));
	threadInfo->corrected_data_int16 = (int16_t*) calloc(CSPAD_nASICS_X*CSPAD_ASIC_NX*CSPAD_nASICS_Y*CSPAD_ASIC_NY,sizeof(int16_t));
	threadInfo->detector_corrected_data = (float*) calloc(CSPAD_nASICS_X*CSPAD_ASIC_NX*CSPAD_nASICS_Y*CSPAD_ASIC_NY,sizeof(float));
	threadInfo->image = (int16_t*) calloc(global->image_nn,sizeof(int16_t));
	threadInfo->saturatedPixelMask = (int16_t *) calloc(CSPAD_nASICS_X*CSPAD_ASIC_NX*CSPAD_nASICS_Y*CSPAD_ASIC_NY,sizeof(int16_t));

	threadInfo->radialAverage = (float *) calloc(global->radial_nn, sizeof(float));
	threadInfo->radialAverageCounter = (float *) calloc(global->radial_nn, sizeof(float));
	
	threadInfo->peak_com_index = (long *) calloc(global->hitfinderNpeaksMax, sizeof(long));
	threadInfo->peak_intensity = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));	
	threadInfo->peak_npix = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));	
	threadInfo->peak_snr = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_x = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_y = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_x_assembled = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_y_assembled = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->peak_com_r_assembled = (float *) calloc(global->hitfinderNpeaksMax, sizeof(float));
	threadInfo->good_peaks = (int *) calloc(global->hitfinderNpeaksMax, sizeof(int));

	for(long i=0;i<global->pix_nn;i++){
		threadInfo->saturatedPixelMask[i] = 1;
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
	//calculateRadialAverage(threadInfo, global);
	calculateRadialAverage(threadInfo->corrected_data, threadInfo->radialAverage, threadInfo->radialAverageCounter, global);
	
	
	
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
	fprintf(global->framefp, "%li, %i, %s, %i, %g, %g, %g, %g, %i, %g\n",threadInfo->threadNum, threadInfo->seconds, threadInfo->eventname, threadInfo->nPeaks, threadInfo->peakNpix, threadInfo->peakTotal, threadInfo->peakResolution, threadInfo->peakDensity, hit, threadInfo->photonEnergyeV);
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
	free(threadInfo->peak_snr);
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



void calculateRadialAverage(float *data, float *radialAverage, float *radialAverageCounter, cGlobal *global){
	// Zero arrays
	for(long i=0; i<global->radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<global->pix_nn; i++){
		rbin = lrint(global->pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) rbin = 0;
		
		radialAverage[rbin] += data[i];
		radialAverageCounter[rbin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<global->radial_nn; i++) {
		if (radialAverageCounter[i] != 0)
			radialAverage[i] /= radialAverageCounter[i];
	}
	
}

void calculateRadialAverage(double *data, double *radialAverage, double *radialAverageCounter, cGlobal *global){	
	// Zero arrays
	for(long i=0; i<global->radial_nn; i++) {
		radialAverage[i] = 0.;
		radialAverageCounter[i] = 0.;
	}
	
	// Radial average
	long	rbin;
	for(long i=0; i<global->pix_nn; i++){
		rbin = lrint(global->pix_r[i]);
		
		// Array bounds check (paranoia)
		if(rbin < 0) rbin = 0;
		
		radialAverage[rbin] += data[i];
		radialAverageCounter[rbin] += 1;
	}
	
	// Divide by number of actual pixels in ring to get the average
	for(long i=0; i<global->radial_nn; i++) {
		if (radialAverageCounter[i] != 0)
			radialAverage[i] /= radialAverageCounter[i];
	}
	
}



	
