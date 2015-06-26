
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


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

	strcpy(eventData->eventname,"---");
	strcpy(eventData->filename,"---");

	/*
	 *	Initialise any common default values
	 */
	eventData->useThreads = 0;
	eventData->hit = 0;
	eventData->hitScore = 0;
	eventData->powderClass = 0;
	eventData->pumpLaserOn = 0;
	eventData->peakResolution=0.;
	eventData->nPeaks=0;
	eventData->peakNpix=0.;
	eventData->peakTotal=0.;
	eventData->stackSlice=-1;

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
		eventData->detector[detIndex].data_forPersistentBackgroundBuffer = (float*) calloc(pix_nn,sizeof(float));
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
	long NpeaksMax = global->hitfinderNpeaksMax;
	allocatePeakList(&(eventData->peaklist), NpeaksMax);
	

	
	/*
	 *	Make it clear we dont know certain things until this data is read
	 */
	eventData->TimeTool_present=0;
	eventData->FEEspec_present=0;
	eventData->TOFPresent = 0;
	eventData->Pulnix_present = false;
	eventData->CXIspec_present = false;


	
	/*
	 *	Create arrays for various spectrum data 
	 *	Setting non-allocated arrays to NULL is useful for preventing double free() errors 
	 *	(ie: we are not completely clean with knowing when we have allocated arrays and when we haven't)
	 */
	int spectrumLength = global->espectrumLength;
	eventData->energySpectrum1D = (double *) calloc(spectrumLength, sizeof(double));
	eventData->energySpectrumExist = 0;
	
	eventData->FEEspec_hproj = NULL;
	eventData->FEEspec_vproj = NULL;
	eventData->TimeTool_hproj = NULL;
	eventData->TimeTool_vproj = NULL;
	eventData->pulnixImage = NULL;
	eventData->CXIspec_image = NULL;
	
	
	// Return
	return eventData;
}




/*
 *  libCheetah function to clean up all memory allocated in event struture
 */
void cheetahDestroyEvent(cEventData *eventData) {
    
    cGlobal	*global = eventData->pGlobal;;
    
    // Free primary detector memory
	DETECTOR_LOOP {
		free(eventData->detector[detIndex].data_raw16);
		free(eventData->detector[detIndex].data_raw);
		free(eventData->detector[detIndex].data_detCorr);
		free(eventData->detector[detIndex].data_detPhotCorr);
		free(eventData->detector[detIndex].data_forPersistentBackgroundBuffer);
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

	// Free peak lists
	freePeakList(eventData->peaklist);
	
	
	// Pulnix external camera
	if(eventData->Pulnix_present == true && eventData->pulnixImage != NULL){
		free(eventData->pulnixImage);
	}
	// Opal spectrum camera
	if(eventData->CXIspec_present == true && eventData->CXIspec_image != NULL){
		free(eventData->CXIspec_image);
	}

	// FEE spectrum trace
	if(eventData->FEEspec_present == 1) {
		free(eventData->FEEspec_hproj);
		free(eventData->FEEspec_vproj);
	}

	// TimeTool trace
	if(eventData->TimeTool_present == 1) {
		free(eventData->TimeTool_hproj);
		free(eventData->TimeTool_vproj);
	}

    free(eventData->energySpectrum1D);
   
	delete eventData;
}
