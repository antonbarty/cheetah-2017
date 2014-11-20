
#include <pthread.h>

#include "cheetah.h"

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
	eventData = new cEventData();
	eventData->pGlobal = global;

	/*
	 *	Initialise any common default values
	 */
	eventData->useThreads = 0;
	eventData->hit = 0;
	eventData->powderClass = 0;
	eventData->pumpLaserOn = 0;
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
		long	pix_nn = global->detector[detIndex].pix_nn;
		long	image_nn = global->detector[detIndex].image_nn;
		long	imageXxX_nn = global->detector[detIndex].imageXxX_nn;
		long	radial_nn = global->detector[detIndex].radial_nn;

		eventData->detector[detIndex].data_raw16 = (uint16_t*) calloc(pix_nn,sizeof(uint16_t));
		eventData->detector[detIndex].data_raw = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detIndex].data_detCorr = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detIndex].data_detPhotCorr = (float*) calloc(pix_nn,sizeof(float));
		eventData->detector[detIndex].pixelmask = (uint16_t*) calloc(pix_nn,sizeof(uint16_t));

		eventData->detector[detIndex].image_raw = (float*) calloc(image_nn,sizeof(float));
		eventData->detector[detIndex].image_detCorr = (float*) calloc(image_nn,sizeof(float));
		eventData->detector[detIndex].image_detPhotCorr = (float*) calloc(image_nn,sizeof(float));
		eventData->detector[detIndex].image_pixelmask = (uint16_t*) calloc(image_nn,sizeof(uint16_t));

		eventData->detector[detIndex].imageXxX_raw = (float*) calloc(imageXxX_nn,sizeof(float));
		eventData->detector[detIndex].imageXxX_detCorr = (float*) calloc(imageXxX_nn,sizeof(float));
		eventData->detector[detIndex].imageXxX_detPhotCorr = (float*) calloc(imageXxX_nn,sizeof(float));
		eventData->detector[detIndex].imageXxX_pixelmask = (uint16_t*) calloc(imageXxX_nn,sizeof(uint16_t));

		eventData->detector[detIndex].radialAverage_raw = (float *) calloc(radial_nn, sizeof(float));
		eventData->detector[detIndex].radialAverage_detCorr = (float *) calloc(radial_nn, sizeof(float));
		eventData->detector[detIndex].radialAverage_detPhotCorr = (float *) calloc(radial_nn, sizeof(float));
		eventData->detector[detIndex].radialAverage_pixelmask = (uint16_t*) calloc(radial_nn,sizeof(uint16_t));

		eventData->detector[detIndex].pedSubtracted=0;
		eventData->detector[detIndex].sum=0.;		
		
	}

	/*
	 *	Create arrays for remembering Bragg peak data
	 */
	//global->hitfinderPeakBufferSize = global->hitfinderNpeaksMax*2;
	long NpeaksMax = global->hitfinderNpeaksMax;
	//eventData->good_peaks = (int *) calloc(NpeaksMax, sizeof(int));
	
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
		free(eventData->detector[detIndex].data_raw16);
		free(eventData->detector[detIndex].data_raw);
		free(eventData->detector[detIndex].data_detCorr);
		free(eventData->detector[detIndex].data_detPhotCorr);
		free(eventData->detector[detIndex].pixelmask);

		free(eventData->detector[detIndex].image_raw);
		free(eventData->detector[detIndex].image_detCorr);
		free(eventData->detector[detIndex].image_detPhotCorr);
		free(eventData->detector[detIndex].image_pixelmask);

		free(eventData->detector[detIndex].imageXxX_raw);
		free(eventData->detector[detIndex].imageXxX_detCorr);
		free(eventData->detector[detIndex].imageXxX_detPhotCorr);
		free(eventData->detector[detIndex].imageXxX_pixelmask);

		free(eventData->detector[detIndex].radialAverage_raw);
		free(eventData->detector[detIndex].radialAverage_detCorr);
		free(eventData->detector[detIndex].radialAverage_detPhotCorr);
		free(eventData->detector[detIndex].radialAverage_pixelmask);
	}
	
	freePeakList(eventData->peaklist);
	//free(eventData->good_peaks);
	
	
	// Pulnix external camera
	if(eventData->pulnixFail == 0){
		free(eventData->pulnixImage);
	}
	// Opal spectrum camera
	if(eventData->specFail == 0){
		free(eventData->specImage);
	}

	if(eventData->FEEspec_present == 1) {
		free(eventData->FEEspec_hproj);
		free(eventData->FEEspec_vproj);
	}
    
    free(eventData->energySpectrum1D);
   
	delete eventData;
}
