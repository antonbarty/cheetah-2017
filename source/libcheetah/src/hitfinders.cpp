
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
#include "cheetahmodules.h"

/*
 *	Various flavours of hitfinder
 *		0 - Everything is a hit
 *		1 - Number of pixels above ADC threshold
 *		2 - Total intensity above ADC threshold
 *		3 - Count Bragg peaks
 *		4 - Use TOF
 *		5 - Depreciated and no longer exists
 *		6 - Experimental - find peaks by SNR criteria
 *              7 - Laser on event code (usually EVR41)
 */
int  hitfinder(cEventData *eventData, cGlobal *global){
	
	// Dereference stuff
	int	    detID = global->hitfinderDetector;
	int		hit=0;
	int		powderClass=0;
	int		nPeaks;
	
	/*
	 *	Default values for some metrics
	 */
	eventData->peakNpix = 0;
	eventData->peakTotal = 0;
	eventData->peakResolution = 0;
	eventData->peakDensity = 0;
      
 	/*
	 *	Use one of various hitfinder algorithms
	 */
 	switch(global->hitfinderAlgorithm) {
		
	case 0 :	// Everything is a hit. Used for converting xtc to hdf5
	  hit = 1;
	  break;	

	case 1 :	// Count the number of pixels above ADC threshold
	  hit = hitfinder1(global,eventData,detID);
	  break;	
	  
	case 2 :	// Integrated intensity above ADC threshold
	  hit = hitfinder2(global,eventData,detID);
	  break;
			
	case 3 : 	// Count number of Bragg peaks (Anton's "number of connected peaks above threshold" algorithm)
	  nPeaks = peakfinder(global,eventData, detID);
	  eventData->nPeaks = nPeaks;
	  if(nPeaks >= global->hitfinderNpeaks && nPeaks <= global->hitfinderNpeaksMax)
	    hit = 1;
	  //hit = peakfinder3(global,eventData, detID);
	  break;	

	case 4 :	// Use TOF signal to find hits
	  hit = hitfinder4(global,eventData,detID);
	  break;
						
	case 6 : 	// Count number of Bragg peaks (Rick's algorithm)
	  nPeaks = peakfinder(global,eventData, detID);
	  eventData->nPeaks = nPeaks;
	  if(nPeaks >= global->hitfinderNpeaks && nPeaks <= global->hitfinderNpeaksMax)
	    hit = 1;
	  //hit = peakfinder6(global,eventData, detID);
	  break;
            
	case 7 : 	// Return laser on event code
	  hit = eventData->laserEventCodeOn;
	  eventData->nPeaks = eventData->laserEventCodeOn;
	  break;
	case 8 : 	// Count number of Bragg peaks (Anton's noise-varying algorithm)
	  nPeaks = peakfinder(global,eventData, detID);
	  eventData->nPeaks = nPeaks;
	  if(nPeaks >= global->hitfinderNpeaks && nPeaks <= global->hitfinderNpeaksMax)
	    hit = 1;
	  break;
	case 9 :	// Use TOF signal, maximum peak, to find hits
	  hit = hitfinder8(global,eventData,detID);
	  break;
	case 10 :	// Use TOF signal, maximum peak, excluding classical htis (this was 8 earlier, but it overlapped with Anton's new hitfinder)
	  hit = hitfinder8(global,eventData,detID);
	  if (hit)
	    {
	      int nPeaks = eventData->nPeaks;
	      hit = !(hitfinder1(global,eventData,detID));
	      eventData->nPeaks = nPeaks;
	    }
	  break;
	case 11 : // Use TOF signal, voltage above threshold
	  hit = hitfinderTOF(global, eventData, detID);
	  break;


	case 12 : // Use list of hits as hitfinding algorithm
	  nameEvent(eventData, global);
			// containsEvent returns bool, but is typeCasted to int according to
			// standard conversion (§4.7/4 from the C++ Standard):
			// (bool containsEvent()) ? 1 : 0
			// http://stackoverflow.com/questions/5369770/bool-to-int-conversion
	  hit = (int) containsEvent((std::string) eventData->eventname, global);
	  break; 
			

	default :
	  printf("Unknown hit finding algorithm selected: %i\n", global->hitfinderAlgorithm);
	  printf("Stopping in confusion.\n");
	  exit(1);
	  break;
			
	}
	
	// Update central hit counter
	if(hit) {
		pthread_mutex_lock(&global->nhits_mutex);
		global->nhits++;
		global->nrecenthits++;
		pthread_mutex_unlock(&global->nhits_mutex);
	}
	
	// Set the appropriate powder class
	eventData->powderClass = hit;
		   
	return(hit);
	
}


/*
 *	Sort into powder classes
 */
void  sortPowderClass(cEventData *eventData, cGlobal *global){
	
	eventData->powderClass = eventData->hit;
	
	/*
	 *	Pump laser sorting
	 *		hit = 0 or 1
	 *		laserOn = 0 or 1
	 *	then
	 *		output = hit + 2*laserOn
	 */
	if(global->sortPumpLaserOn == 1) {
		int hit = eventData->hit;
		int	pumpLaserOn = eventData->pumpLaserOn;
		
		eventData->powderClass = hit + 2*pumpLaserOn;
	}
	
}


/*
 *	Find peaks on the inner 4 2x2 modules
 *	Calculate rest of detector only if needed
 *	Tries to avoid bottleneck in subtractLocalBackground() on the whole detector even for blanks
 */
long hitfinderFastScan(cEventData *eventData, cGlobal *global){
	
	// Bad detector??
	int	    detID = global->hitfinderDetector;
	if(detID > global->nDetectors) {
		printf("peakfinder: false detectorID %i\n",detID);
		exit(1);
	}

	long	pix_nx = global->detector[detID].pix_nx;
	long	pix_ny = global->detector[detID].pix_ny;
		long		pix_nn = global->detector[detID].pix_nn;
		long		asic_nx = global->detector[detID].asic_nx;
		long		asic_ny = global->detector[detID].asic_ny;
	long	nasics_x = global->detector[detID].nasics_x;
	long	nasics_y = global->detector[detID].nasics_y;
	long	radius = global->detector[detID].localBackgroundRadius;
	float	*pix_r = global->detector[detID].pix_r;
	float	*data = eventData->detector[detID].corrected_data;

	float	hitfinderADCthresh = global->hitfinderADC;
	float	hitfinderMinSNR = global->hitfinderMinSNR;
	long	hitfinderMinPixCount = global->hitfinderMinPixCount;
	long	hitfinderMaxPixCount = global->hitfinderMaxPixCount;
	long	hitfinderLocalBGRadius = global->hitfinderLocalBGRadius;
	float	hitfinderMinPeakSeparation = global->hitfinderMinPeakSeparation;
	tPeakList	*peaklist = &eventData->peaklist;

	char	*mask = (char*) calloc(pix_nn, sizeof(char));
	
	//	Bad region masks  (data=0 to ignore regions)
	uint16_t	combined_pixel_options = PIXEL_IS_IN_PEAKMASK|PIXEL_IS_BAD|PIXEL_IS_HOT|PIXEL_IS_BAD|PIXEL_IS_SATURATED|PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
	for(long i=0;i<pix_nn;i++)
		mask[i] = isNoneOfBitOptionsSet(eventData->detector[detID].pixelmask[i], combined_pixel_options);
	

	subtractLocalBackground(data, radius, asic_nx, asic_ny, nasics_x, 2);
	

	/*
	 *	Call the appropriate peak finding algorithm
	 */
	long	nPeaks;
	switch(global->hitfinderAlgorithm) {
	
		case 3 : 	// Count number of Bragg peaks
			nPeaks = peakfinder3(peaklist, data, mask, asic_nx, asic_ny, nasics_x, 2, hitfinderADCthresh, hitfinderMinSNR, hitfinderMinPixCount, hitfinderMaxPixCount, hitfinderLocalBGRadius);
			break;

		case 6 : 	// Count number of Bragg peaks
			nPeaks = peakfinder6(peaklist, data, mask, asic_nx, asic_ny, nasics_x, 2, hitfinderADCthresh, hitfinderMinSNR, hitfinderMinPixCount, hitfinderMaxPixCount, hitfinderLocalBGRadius, hitfinderMinPeakSeparation);
			break;
	   
		case 8 : 	// Count number of Bragg peaks
			nPeaks = peakfinder8(peaklist, data, mask, pix_r, asic_nx, asic_ny, nasics_x, 2, hitfinderADCthresh, hitfinderMinSNR, hitfinderMinPixCount, hitfinderMaxPixCount, hitfinderLocalBGRadius);
			break;
	
		default :
			printf("Unknown peak finding algorithm selected: %i\n", global->hitfinderAlgorithm);
			printf("Stopping in hitfinderFastScan.\n");
			exit(1);
			break;
	}
	
	/*
	 *	Is this a potential hit?
	 */
	int		hit;
	eventData->nPeaks = nPeaks;
	if(nPeaks >= global->hitfinderNpeaks/2 && nPeaks <= global->hitfinderNpeaksMax/2) {

		hit = 1;
		//printf("%li : Potential hit, npeaks(prescan) = %li\n", eventData->threadNum, nPeaks);
	
		// Do the rest of the local background subtraction
		long offset = (2*asic_ny)*pix_nx;
		subtractLocalBackground(data+offset, radius, asic_nx, asic_ny, nasics_x, 6);
	}
	
	free(mask);
	
	return hit;
}





/*
 *	Start of calculations for hitfinders
 */

void integratePixAboveThreshold(float *data,uint16_t *mask,long pix_nn,float ADC_threshold,uint16_t pixel_options,long *nat,float *tat){

  *nat = 0;
  *tat = 0.0;

  for(long i=0;i<pix_nn;i++){
    if(isNoneOfBitOptionsSet(mask[i],pixel_options) && (data[i] > ADC_threshold)){
      *tat += data[i];
      *nat += 1;
    }
  }
}


/*
 *	Hitfinder #1
 *	Hit if number of pixels above ADC threshold exceeds MinPixCount
 */

int hitfinder1(cGlobal *global, cEventData *eventData, long detID){

  int       hit = 0;
  long      nat = 0;
  float     tat = 0.;
  float *hitfinderData;
  uint16_t  *mask;
  uint16_t  mask_out_bits;
  float     *data;
  long	    pix_nn;
  float     ADC_threshold = global->hitfinderADC;
  // Combine pixel options for pixels to be ignored
  uint16_t  pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_MISSING;

  if (global->hitfinderIgnoreHaloPixels) {
    pixel_options |= PIXEL_IS_IN_HALO;
  }
  
  if (global->hitfinderOnDetectorCorrectedData) {
    hitfinderData = eventData->detector[detID].detector_corrected_data;
  } else {
    hitfinderData = eventData->detector[detID].corrected_data;
  }

  if (global->hitfinderDownsampling > 1) {
    long pix_nn_0 = global->detector[detID].pix_nn;  
    long pix_nx_0 = global->detector[detID].pix_nx;  
    long pix_ny_0 = global->detector[detID].pix_ny;  
    float *data_0 = hitfinderData;
    uint16_t *mask_0 = eventData->detector[detID].pixelmask;
    long pix_nx = (long)ceil(pix_nx_0/(double)global->hitfinderDownsampling);
    long pix_ny = pix_nx;
    pix_nn = pix_nx*pix_ny;
    data = (float *) calloc(pix_nn,sizeof(float));
    mask = (uint16_t *) calloc(pix_nn,sizeof(uint16_t));
    downsampleImageNonConservative(data_0,data,pix_nn_0,pix_nx_0,pix_nn,pix_nx,1.,mask_0,global->hitfinderDownsampling);
    downsampleMaskNonConservative(mask_0,mask,pix_nn_0,pix_nx_0,pix_nn,pix_nx,global->hitfinderDownsampling);    
  } else {
    pix_nn = global->detector[detID].pix_nn;  
    data = hitfinderData;
    mask = eventData->detector[detID].pixelmask;
  }

  integratePixAboveThreshold(data,mask,pix_nn,ADC_threshold,pixel_options,&nat,&tat);
  eventData->peakTotal = tat;
  eventData->peakNpix = nat;
  eventData->nPeaks = nat;

  if(global->hitfinderDownsampling > 1){
    free(data);
    free(mask);
  }

  // n pixels above threshold
  if(nat >= global->hitfinderMinPixCount){
    hit = 1;
  }
  // But not more than max (unless set to 0)
  if(global->hitfinderMaxPixCount != 0 && nat > global->hitfinderMaxPixCount){
    hit = 0;
  }
	
  return hit;
}


/*
 *	Hitfinder #2
 *	Hit if integrated value of pixels above ADC threshold exceeds TAT threshold
 */

int hitfinder2(cGlobal *global, cEventData *eventData, long detID){
  
  int       hit = 0;
  long      nat = 0;
  float     tat = 0.;
  uint16_t  *mask = eventData->detector[detID].pixelmask;
  float     *data = eventData->detector[detID].corrected_data;
  long	    pix_nn = global->detector[detID].pix_nn;  
  float     ADC_threshold = global->hitfinderADC;
  // Combine pixel options for pixels to be ignored
  uint16_t  pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_MISSING;
  
  if (global->hitfinderIgnoreHaloPixels) {
    pixel_options |= PIXEL_IS_IN_HALO;
  }

  integratePixAboveThreshold(data,mask,pix_nn,ADC_threshold,pixel_options,&nat,&tat);
  eventData->peakTotal = tat;
  eventData->peakNpix = nat;
  eventData->nPeaks = nat;

  if(tat >= global->hitfinderTAT){
    hit = 1;
  }
  return hit;
}

int hitfinder4(cGlobal *global,cEventData *eventData,long detID){
  int hit = 0;
  long		pix_nn = global->detector[detID].pix_nn;
  uint16_t      *mask = eventData->detector[detID].pixelmask;
  long	nat = 0;
  long	counter;
  float	total;
  float	mingrad = global->hitfinderMinGradient*2;
  mingrad *= mingrad;
  
  nat = 0;
  counter = 0;
  total = 0.0;

  /*
   *	Create a buffer for image data so we don't nuke the main image by mistake 
   */
  float *temp = (float*) calloc(pix_nn, sizeof(float));
  memcpy(temp, eventData->detector[detID].corrected_data, pix_nn*sizeof(float));
	

  // combine pixelmask bits
  uint16_t combined_pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED;

  if (global->hitfinderIgnoreHaloPixels) {
    combined_pixel_options |= PIXEL_IS_IN_HALO;
  }
	
  /*
   *	Apply masks
   *	(multiply data by 0 to ignore regions)
   */
  for(long i=0;i<pix_nn;i++){
    temp[i] *= isNoneOfBitOptionsSet(mask[i], combined_pixel_options);
  }
		  
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
  free(temp);
  return hit;
}

int hitfinder8(cGlobal *global,cEventData *eventData,long detID){
  int hit = 0;
  //long		pix_nn = global->detector[detID].pix_nn;
  uint16_t      *mask = eventData->detector[detID].pixelmask;
  long	nat = 0;
  long	counter;
  float	total;
  float	mingrad = global->hitfinderMinGradient*2;
  mingrad *= mingrad;
  
  nat = 0;
  counter = 0;
  total = 0.0;

  /*
   *	Apply masks
   *	(multiply data by 0 to ignore regions)
   */

		  
  if ((eventData->TOFPresent==1)){
    const int nback = global->hitfinderTOFWindow;
    float olddata[nback];
    for (int k = 0; k < nback; k++)
      {
	olddata[k] = NAN;
      }
    int count = 0;
    for(int i=global->hitfinderTOFMinSample; i<global->hitfinderTOFMaxSample; i++){
      olddata[i % nback] = eventData->TOFVoltage[i];
      double sum = 0;
      for (int k = 0; k < nback; k++)
	{
	  sum += olddata[k];
	}
      if (sum < global->hitfinderTOFThresh * nback) count++;
    }
    hit = (count >= global->hitfinderTOFMinCount);
    eventData->nPeaks = count;
  }


  return hit;
}



/* A TOF hitfinder counting the number of protons in the TOF signal given the TOF mean background, the proton window
   and TOF voltage threshold for protons. Every frame with nr. of protons at least TOFMinCount is considered a hit. */
 
int hitfinderTOF(cGlobal *global, cEventData *eventData, long detID){
  int hit = 0;
  if (eventData->TOFPresent==1){
    int count = 0;
    for (int i=global->hitfinderTOFMinSample; i<global->hitfinderTOFMaxSample; i++){
      count += floor(fmax((eventData->TOFVoltage[i] - global->hitfinderTOFMeanBackground) / global->hitfinderTOFThresh, 0)) ;
    }
    hit = (count >= global->hitfinderTOFMinCount);
    eventData->nPeaks = count;
  }
  return hit;
}


/*
 *	Check if the list of hits contains the current event
 */
bool containsEvent(std::string event, cGlobal *global) {
	if (global->nhits < global->hitlist.size()) {
		return (event == global->hitlist[global->nhits]);
	} else {
		return false;
	}
}
