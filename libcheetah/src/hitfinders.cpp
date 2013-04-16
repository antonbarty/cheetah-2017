
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "median.h"
#include "hitfinders.h"
#include "peakfinders.h"


// Peakfinders local to this routine
int hitfinder1(cGlobal*, cEventData*, int);
int hitfinder_tof(cGlobal*, cEventData*, int);



/*
 *	Various flavours of hitfinder
 *		0 - Everything is a hit
 *		1 - Number of pixels above ADC threshold
 *		2 - Total intensity above ADC threshold
 *		3 - Count Bragg peaks
 *		4 - Use TOF
 *		5 - Like 3, but with extras
 *		6 - Experimental - find peaks by SNR criteria
 *      7 - Laser on event code (usually EVR41)
 */
int  hitfinder(cEventData *eventData, cGlobal *global){
	
	// Dereference stuff
	int			detID = global->hitfinderDetector;
	long		pix_nn = global->detector[detID].pix_nn;
	long		asic_nx = global->detector[detID].asic_nx;
	long		asic_ny = global->detector[detID].asic_ny;
	uint16_t	*mask = eventData->detector[detID].pixelmask;

	long	nat = 0;
	long	counter;
	int		hit=0;
	float	total;
	long	*inx = (long *) calloc(pix_nn, sizeof(long));
	long	*iny = (long *) calloc(pix_nn, sizeof(long));
	float	mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;
	
	nat = 0;
	counter = 0;
	total = 0.0;
	
	/*
	 *	Default values for some metrics
	 */
	eventData->peakNpix = 0;
	eventData->peakTotal = 0;
	eventData->peakResolution = 0;
	eventData->peakDensity = 0;
	
	
	/*
	 *	Create a buffer for image data so we don't nuke the main image by mistake 
	 */
	float *temp = (float*) calloc(pix_nn, sizeof(float));
	memcpy(temp, eventData->detector[detID].corrected_data, pix_nn*sizeof(float));
	

	// combine pixelmask bits
	uint16_t combined_pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED;
	
	/*
	 *	Apply masks
	 *	(multiply data by 0 to ignore regions)
	 */
	for(long i=0;i<pix_nn;i++){
		temp[i] *= isNoneOfBitOptionsSet(mask[i], combined_pixel_options);
	}
    
	/*
	 *	Use one of various hitfinder algorithms
	 */
	switch(global->hitfinderAlgorithm) {
		
		case 0 :	// Everything is a hit. Used for converting xtc to hdf
			hit = 1;
	
			
		case 1 :	// Count the number of pixels above ADC threshold
			hit = hitfinder1(global, eventData,detID);
			if(eventData->nPeaks >= global->hitfinderMinPixCount)
				hit = 1;
			break;
			
			
		case 2 :	//	integrated intensity above threshold
			hit = hitfinder1(global, eventData,detID);
			if(eventData->peakTotal >= global->hitfinderTAT)
				hit = 1;
			break;


		case 4 :	// Use TOF signal to find hits
			hit = hitfinder_tof(global, eventData,detID);
			break;
			
			
		case 3 : 	// Count number of Bragg peaks
			hit = peakfinder3(global, eventData, detID);			
			break;
			
						
		case 6 : 	// Count number of Bragg peaks
			hit = peakfinder6(global,eventData, detID);
			break;
			
            
		case 7 : 	// Return laser on event code
            hit = eventData->laserEventCodeOn;
            eventData->nPeaks = eventData->laserEventCodeOn;
			break;
			
			
		default :
			printf("Unknown hit finding algorithm selected: %i\n", global->hitfinderAlgorithm);
			printf("Stopping in confusion.\n");
			exit(1);
			break;
	}
	
	// Statistics on the peaks, for certain hitfinders
	if( eventData->nPeaks > 1 &&
	   ( global->hitfinderAlgorithm == 3 || global->hitfinderAlgorithm == 5 || global->hitfinderAlgorithm == 6 ) ) {
		   
		long	np;
		long  kk;
		float	resolution;
		float	resolutionA;	
		float	cutoff = 0.95;
	   
		np = eventData->nPeaks;
		if(np >= global->hitfinderNpeaksMax) 
		   np = global->hitfinderNpeaksMax; 
		kk = (long) floor(cutoff*np);
	
	

		// Pixel radius resolution (bigger is better)
		float *buffer1 = (float*) calloc(global->hitfinderNpeaksMax, sizeof(float));
		for(long k=0; k<np; k++) 
			buffer1[k] = eventData->peak_com_r_assembled[k];
		resolution = kth_smallest(buffer1, np, kk);		   
		eventData->peakResolution = resolution;
		free(buffer1);
	
		// Resolution to real space (in Angstrom)
		// Crystallographic resolution d = lambda/sin(theta)
		float z = global->detector[0].detectorZ;
		float dx = global->detector[0].pixelSize;
		double r = sqrt(z*z+dx*dx*resolution*resolution);
		double sintheta = dx*resolution/r;
		resolutionA = eventData->wavelengthA/sintheta;
		eventData->peakResolutionA = resolutionA;

	
		if(resolution > 0) {
			float	area = (3.141*resolution*resolution)/(asic_ny*asic_nx);
			eventData->peakDensity = (cutoff*np)/area;
		}
	   
	}
	
	// Update central hit counter
	if(hit) {
		pthread_mutex_lock(&global->nhits_mutex);
		global->nhits++;
		global->nrecenthits++;
		pthread_mutex_unlock(&global->nhits_mutex);
	}
	

	free(inx); 			
	free(iny);	
	free(temp);
	
	return(hit);
	
}

/*
 *	Hitfinder #1
 *	Count the number of pixels above ADC threshold
 */
int hitfinder1(cGlobal *global, cEventData *eventData, int detID){

	long		nat = 0;
	long            hit = 0;
	float		total = 0.;
	long		pix_nn = global->detector[detID].pix_nn;
	uint16_t         *mask = eventData->detector[detID].pixelmask;
	
	/*
	*	Create a buffer for image data so we don't nuke the main image by mistake
	*/	
	float *temp = (float*) calloc(pix_nn, sizeof(float));
	memcpy(temp, eventData->detector[detID].corrected_data, pix_nn*sizeof(float));
	
	
	// combine pixelmask bits
	uint16_t combined_pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED;

	if(global->hitfinderDisregardHalopix)
		combined_pixel_options |= PIXEL_IS_IN_HALO;
	
	/*
	 *	Apply masks
	 *	(multiply data by 0 to ignore regions)
	 */
	for(long i=0;i<pix_nn;i++){
		temp[i] *= isNoneOfBitOptionsSet(mask[i], combined_pixel_options);
	}
	
	for(long i=0;i<pix_nn;i++) {
		if(temp[i] > global->hitfinderADC) {
		  total += temp[i];
		  nat++;
		}
	}
	
	if(nat >= global->hitfinderMinPixCount)
		hit = 1;
	
	eventData->peakNpix = nat;
	eventData->nPeaks = nat;
	eventData->peakTotal = total;
	
	free(temp);

	return hit;
}


int hitfinder_tof(cGlobal *global, cEventData *eventData, int detID){
	if ((global->hitfinderUseTOF==1) && (eventData->TOFPresent==1)){
		double total_tof = 0.;
		for(int i=global->hitfinderTOFMinSample; i<global->hitfinderTOFMaxSample; i++){
			total_tof += eventData->TOFVoltage[i];
		}
		if (total_tof > global->hitfinderTOFThresh)
			hit = 1;
	}
	// Use cspad threshold if TOF is not present
	else {
		for(long i=0;i<pix_nn;i++){
			if(temp[i] > global->hitfinderADC){
				nat++;
			}
		}
		if(nat >= global->hitfinderMinPixCount)
			hit = 1;
	}

}



