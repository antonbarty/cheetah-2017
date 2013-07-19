/*
 *  pixelDetector.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <limits>
#include <hdf5.h>
#include <fenv.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "data2d.h"
#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"



/*
 *	Set default values to something reasonable for CSPAD at CXI 
 */
cPixelDetectorCommon::cPixelDetectorCommon() {
    

	// Defaults to CXI cspad configuration
	strcpy(detectorType, "cspad");
	strcpy(detectorName, "CxiDs1");
	//detectorType = Pds::DetInfo::Cspad;
	//detectorPdsDetInfo = Pds::DetInfo::CxiDs1;

	// Calibration files
	strcpy(detectorConfigFile, "No_file_specified");
	strcpy(geometryFile, "No_file_specified");
	strcpy(badpixelFile, "No_file_specified");
	strcpy(baddataFile, "No_file_specified");
	strcpy(darkcalFile, "No_file_specified");
	strcpy(wireMaskFile, "No_file_specified");
	strcpy(gaincalFile, "No_file_specified");
    
	// Default ASIC layout (cspad)
	asic_nx = CSPAD_ASIC_NX;
	asic_ny = CSPAD_ASIC_NY;
	asic_nn = CSPAD_ASIC_NX * CSPAD_ASIC_NY;
	nasics_x = CSPAD_nASICS_X;
	nasics_y = CSPAD_nASICS_Y;
	pix_nx = asic_nx*nasics_x;
	pix_ny = asic_ny*nasics_y;
	pix_nn = pix_nx*pix_ny;
	pixelSize = 109.92e-6;
    
	// Detector Z position
	//strcpy(detectorZpvname, "CXI:DS1:MMS:06.RBV");
	strcpy(detectorZpvname,"");
	//defaultCameraLengthMm = std::numeric_limits<float>::quiet_NaN();
	defaultCameraLengthMm = 0;
	fixedCameraLengthMm = 0;
	detposprev = 0;
	cameraLengthOffset = 500.0 + 79.0;
	cameraLengthScale = 1e-3;

	beamCenterPixX = 0;
	beamCenterPixY = 0;
    
	// Bad pixel mask    
	useBadPixelMask = 0;
	applyBadPixelMask = 1;

	// Saturated pixels
	maskSaturatedPixels = 0;
	pixelSaturationADC = 15564;  // 95% of 2^14 ??

	// Static dark calibration (electronic offsets)
	useDarkcalSubtraction = 0;

	// Common mode subtraction from each ASIC
	cmModule = 0;
	cmFloor = 0.1;
	cspadSubtractUnbondedPixels = 0;
	cspadSubtractBehindWires = 0;

	// Gain calibration correction
	useGaincal = 0;
	invertGain = 0;

	// Subtraction of running background (persistent photon background) 
	useSubtractPersistentBackground = 0;
	bgMemory = 50;
	startFrames = 0;
	scaleBackground = 0;
	useBackgroundBufferMutex = 0;
	bgMedian = 0.5;
	bgRecalc = bgMemory;
	bgIncludeHits = 0;
	bgNoBeamReset = 0;
	bgFiducialGlitchReset = 0;

	// Local background subtraction
	useLocalBackgroundSubtraction = 0;
	localBackgroundRadius = 3;

	// Identify persistently hot pixels
	useAutoHotpixel = 0;
	hotpixFreq = 0.9;
	hotpixADC = 1000;
	hotpixMemory = 50;
	// Kill persistently hot pixels
	applyAutoHotpixel = 0;
    
	// Polarization correction
	usePolarizationCorrection = 0;
    horizontalFractionOfPolarization = 1.0;
    
	// Solid angle correction
	useSolidAngleCorrection = 0;
	solidAngleAlgorithm = 1;
    
	// Identify persistently illuminated pixels (halo)
	useAutoHalopixel = 0;
	halopixMinDeviation = 100;
	halopixRecalc = bgRecalc;
	halopixMemory = bgRecalc;

	// Histogram stack
	histogram = 0;
	histogramMin = -100;
	histogramMax = 100;
	histogramBinSize = 1;
	histogramMaxMemoryGb = 4;
	histogram_count = 0;
    
	// Angular correlation analysis
	useAngularCorrelation = 0;
	sumAngularCorrelation = 0;
	autoCorrelateOnly = 1;
	angularCorrelationAlgorithm = 1;
	angularCorrelationNormalization = 1;
	angularCorrelationQScale = 1;
    angularCorrelationStartQ = 100;
    angularCorrelationStopQ = 600;
    angularCorrelationNumQ = 51;
    angularCorrelationStartPhi = 0;
    angularCorrelationStopPhi = 360;
    angularCorrelationNumPhi = 256;
	angularCorrelationNumDelta = 0;
	angularCorrelationLUTdim1 = 100;
	angularCorrelationLUTdim2 = 100;
	angularCorrelationOutput = 1;
    
	// correction for PNCCD read out artifacts 
	usePnccdOffsetCorrection = 0;

	// Saving options
	saveDetectorCorrectedOnly = 0;
	saveDetectorRaw = 0;

	// No downsampling
	downsampling = 1;
}

void cPixelDetectorCommon::configure(void) {
	
  /*
   * Configure basic detector parameters
   */
  printf("Configuring for detector: %s\n",detectorName);
	
  if(strcmp(detectorName, "CxiDs1") == 0 || strcmp(detectorName, "CxiDs2") == 0 ||
     strcmp(detectorName, "CxiDsd") == 0 || strcmp(detectorName, "XppGon") == 0) {
    strcpy(detectorType, "cspad");
    asic_nx = CSPAD_ASIC_NX;
    asic_ny = CSPAD_ASIC_NY;
    asic_nn = CSPAD_ASIC_NX * CSPAD_ASIC_NY;
    nasics_x = CSPAD_nASICS_X;
    nasics_y = CSPAD_nASICS_Y;
    pix_nx = CSPAD_ASIC_NX*CSPAD_nASICS_X;
    pix_ny = CSPAD_ASIC_NY*CSPAD_nASICS_Y;
    pix_nn = pix_nx * pix_ny;
    pixelSize = 110e-6;
  } 
  else if(strcmp(detectorName, "pnCCD") == 0 ) {
    strcpy(detectorType, "pnccd");
    asic_nx = PNCCD_ASIC_NX;
    asic_ny = PNCCD_ASIC_NY;
    asic_nn = PNCCD_ASIC_NX * PNCCD_ASIC_NY;
    nasics_x = PNCCD_nASICS_X;
    nasics_y = PNCCD_nASICS_Y;
    pix_nx = PNCCD_ASIC_NX * PNCCD_nASICS_X;
    pix_ny = PNCCD_ASIC_NY * PNCCD_nASICS_Y;
    pix_nn = pix_nx * pix_ny;
    pixelSize = 75e-6;
		
  }
  else {
    printf("Error: unknown detector name %s\n", detectorName);
    printf("cPixelDetectorCommon::configure()\n");
    printf("Quitting\n");
    exit(1);
  }
	
  printf("\tDetector size: %lix%li\n",pix_nx,pix_ny);
  printf("\tASIC geometry: %lix%li\n",nasics_x,nasics_y);
  printf("\tASIC size: %lix%li\n",asic_nx,asic_ny);
  printf("\tPixel size: %g (m)\n",pixelSize);
}


/*
 *	Process tags for both configuration file and command line options
 */
int cPixelDetectorCommon::parseConfigTag(char *tag, char *value) {
	
  int fail = 0;

  /*
   *	Convert to lowercase
   */
  for(int i=0; i<((int) strlen(tag)); i++) 
    tag[i] = tolower(tag[i]);

  if (!strcmp(tag, "detectorname")) {
    strcpy(detectorName, value);
  }
  else if (!strcmp(tag, "detectortype")) {
    strcpy(detectorType, value);
  }
  else if (!strcmp(tag, "detectorid")) {
    detectorID = atoi(value);
  }
  else if (!strcmp(tag, "geometry")) {
    strcpy(geometryFile, value);
  }
  else if (!strcmp(tag, "subtractbg")) {
      printf("The keyword subtractBg has been changed.  It is\n"
             "now toggled by darkcal.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "usedarkcalsubtraction")) {
      printf("The keyword useDarkcalSubtraction has been changed.  It is\n"
             "now toggled by darkcal.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "darkcal")) {
    strcpy(darkcalFile, value);
    useDarkcalSubtraction = 1;
  }
  else if (!strcmp(tag, "gaincal")) {
    strcpy(gaincalFile, value);
    useGaincal = 1;
  }
  else if (!strcmp(tag, "badpixelmap")) {
      printf("The keyword badPixelMap has been changed.  It is\n"
             "now known as badPixelMask.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "badpixelmask")) {
      strcpy(badpixelFile, value);
      useBadPixelMask = 1;
  }
  else if (!strcmp(tag, "applybadpixelmap")) {
      printf("The keyword applyBadPixelMap has been changed.  It is\n"
             "now known as applyBadPixelMask.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "applybadpixelmask")) {
      applyBadPixelMask = atoi(value);
  }
  else if (!strcmp(tag, "baddatamap")) {
      printf("The keyword badDataMap has been changed.  It is\n"
             "now known as badDataMask.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "baddatamask")) {
      strcpy(baddataFile, value);
      useBadDataMask = 1;
  }
  else if (!strcmp(tag, "wiremask")) {
    strcpy(wireMaskFile, value);
  }
  else if (!strcmp(tag, "pixelsize")) {
    pixelSize = atof(value);
  }
  else if (!strcmp(tag, "downsampling")) {
    downsampling = atoi(value);
  }
  else if (!strcmp(tag, "savedetectorcorrectedonly")) {
    saveDetectorCorrectedOnly = atoi(value);
  }
  else if (!strcmp(tag, "savedetectorraw")) {
    saveDetectorRaw = atoi(value);
  }
  else if (!strcmp(tag, "beamcenterx")) {
      printf("The keyword beamCenterX has been changed. It is\n"
             "now known as beamCenterPixX.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "beamcentery")) {
      printf("The keyword beamCenterY has been changed. It is\n"
             "now known as beamCenterPixY.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  } 
  else if (!strcmp(tag, "beamcenterpixx")) {
      beamCenterPixX  = atof(value);
  }
  else if (!strcmp(tag, "beamcenterpixy")) {
      beamCenterPixY  = atof(value);
  } 
  else if (!strcmp(tag, "detectorzpvname")) {
    strcpy(detectorZpvname, value);
  }
  else if (!strcmp(tag, "defaultcameralengthmm")) {
    defaultCameraLengthMm = atof(value);
  }
  else if (!strcmp(tag, "fixedcameralengthmm")) {
    fixedCameraLengthMm = atof(value);
  }
  else if (!strcmp(tag, "cameralengthoffset")) {
    cameraLengthOffset = atof(value);
  }
  else if (!strcmp(tag, "cameralengthscale")) {
    cameraLengthScale  = atof(value);
  }
  else if (!strcmp(tag, "masksaturatedpixels")) {
    maskSaturatedPixels = atoi(value);
  }
  else if (!strcmp(tag, "bgmemory")) {
    bgMemory = atoi(value);
  }
  else if (!strcmp(tag, "useautohotpixel")) {
	  printf("The keyword useAutoHotPixel has been changed.  It is\n"
			 "now toggled by hotpixFreq and/or hotpixADC.\n"
			 "Modify your ini file and try again...\n");
	  fail = 1;
  }
  else if (!strcmp(tag, "hotpixfreq")) {
    hotpixFreq = atof(value);
    useAutoHotpixel = 1;
    applyAutoHotpixel = 1;
  }
  else if (!strcmp(tag, "hotpixadc")) {
    hotpixADC = atoi(value);
    useAutoHotpixel = 1;
    applyAutoHotpixel = 1;
  }
  else if (!strcmp(tag, "hotpixmemory")) {
      hotpixMemory = atoi(value);
  }
  else if (!strcmp(tag, "applyautohotpixel")) {
      applyAutoHotpixel = atoi(value);
  }
  else if (!strcmp(tag, "usepolarizationcorrection")) {
      usePolarizationCorrection = atoi(value);
  }
  else if (!strcmp(tag, "horizontalfractionofpolarization")) {
      horizontalFractionOfPolarization = atof(value);
  }
  else if (!strcmp(tag, "usesolidanglecorrection")) {
      useSolidAngleCorrection = atoi(value);
  }
  else if (!strcmp(tag, "solidanglealgorithm")) {
      solidAngleAlgorithm = atoi(value);
  }
  else if (!strcmp(tag, "useautohalopixel")) {
    useAutoHalopixel = atoi(value);
  }
  else if (!strcmp(tag, "halopixelmemory")) {
    halopixMemory = atoi(value);
  }
  else if (!strcmp(tag, "halopixelrecalc")) {
    halopixRecalc = atoi(value);
  }
  else if (!strcmp(tag, "halopixmindeviation")) {
    halopixMinDeviation = atof(value);
  }
  else if (!strcmp(tag, "subtractcmmodule")) {
      printf("The keyword subtractcmModule has been changed. It is\n"
             "now known as cmModule.\n"
             "Modify your ini file and try again...\n");
      fail = 1;
  }
  else if (!strcmp(tag, "cmmodule")) {
    cmModule = atoi(value);
  }
  else if (!strcmp(tag, "cmfloor")) {
    cmFloor = atof(value);
  }
  
  // Local background subtraction
  else if (!strcmp(tag, "uselocalbackgroundsubtraction")) {
    useLocalBackgroundSubtraction = atoi(value);
  }
  else if (!strcmp(tag, "localbackgroundradius")) {
    localBackgroundRadius = atoi(value);
  }
    
  else if (!strcmp(tag, "pixelsaturationadc")) {
    pixelSaturationADC = atoi(value);
  }
  else if (!strcmp(tag, "useselfdarkcal")) {
    printf("The keyword useSelfDarkcal has been changed.  It is\n"
	   "now known as useSubtractPersistentBackground.\n"
	   "Modify your ini file and try again...\n");
    fail = 1;
  }
  else if (!strcmp(tag, "usesubtractpersistentbackground")) {
    useSubtractPersistentBackground = atoi(value);
  }
  else if (!strcmp(tag, "usebackgroundbuffermutex")) {
    useBackgroundBufferMutex = atoi(value);
  }
  else if (!strcmp(tag, "subtractbehindwires")) {
    cspadSubtractBehindWires = atoi(value);
  }
  else if (!strcmp(tag, "invertgain")) {
    invertGain = atoi(value);
  }
  else if (!strcmp(tag, "subtractunbondedpixels")) {
    cspadSubtractUnbondedPixels = atoi(value);
  }
  else if (!strcmp(tag, "usepnccdoffsetcorrection")) {
    //usePnccdOffsetCorrection = atoi(value);
    usePnccdOffsetCorrection = 1;
  }
  else if (!strcmp(tag, "bgrecalc")) {
    bgRecalc = atoi(value);
  }
  else if (!strcmp(tag, "bgmedian")) {
    bgMedian = atof(value);
  }
  else if (!strcmp(tag, "bgincludehits")) {
    bgIncludeHits = atoi(value);
  }
  else if (!strcmp(tag, "bgnobeamreset")) {
    bgNoBeamReset = atoi(value);
  }
  else if (!strcmp(tag, "bgfiducialglitchreset")) {
    bgFiducialGlitchReset = atoi(value);
  }	
  else if (!strcmp(tag, "scalebackground")) {
    scaleBackground = atoi(value);
  }
  else if (!strcmp(tag, "scaledarkcal")) {
    printf("The keyword scaleDarkcal does the same thing as scaleBackground.\n"
	   "Use scaleBackground instead.\n"
	   "Modify your ini file and try again...\n");
    fail = 1;
  }
	else if (!strcmp(tag, "startframes")) {
		startFrames = atoi(value);
  }
	
	// Histograms
	else if (!strcmp(tag, "histogram")) {
	  histogram = atoi(value);
	}
	else if (!strcmp(tag, "histogrammin")) {
	  histogramMin = atoi(value);
	}
	else if (!strcmp(tag, "histogrammax")) {
	  histogramMax = atoi(value);
	}
	else if (!strcmp(tag, "histogrambinsize")) {
	  histogramBinSize = atoi(value);
	}
	else if (!strcmp(tag, "histogrammaxmemorygb")) {
	  histogramMaxMemoryGb = atof(value);
	}
    
    // Angular correlation analysis
	else if (!strcmp(tag, "useangularcorrelation")) {
        useAngularCorrelation = atoi(value);
	}
	else if (!strcmp(tag, "sumangularcorrelation")) {
        sumAngularCorrelation = atoi(value);
	}
	else if (!strcmp(tag, "autocorrelateonly")) {
        autoCorrelateOnly = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationalgorithm")) {
        angularCorrelationAlgorithm = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationnormalization")) {
        angularCorrelationNormalization = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationqscale")) {
        angularCorrelationQScale = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationstartq")) {
        angularCorrelationStartQ = atof(value);
	}
	else if (!strcmp(tag, "angularcorrelationstopq")) {
        angularCorrelationStopQ = atof(value);
	}
	else if (!strcmp(tag, "angularcorrelationnumq")) {
        angularCorrelationNumQ = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationstartphi")) {
        angularCorrelationStartPhi = atof(value);
	}
	else if (!strcmp(tag, "angularcorrelationstopphi")) {
        angularCorrelationStopPhi = atof(value);
	}
	else if (!strcmp(tag, "angularcorrelationnumphi")) {
        angularCorrelationNumPhi = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationnumdelta")) {
        angularCorrelationNumDelta = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationlutdim1")) {
        angularCorrelationLUTdim1 = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationlutdim2")) {
        angularCorrelationLUTdim2 = atoi(value);
	}
	else if (!strcmp(tag, "angularcorrelationoutput")) {
        angularCorrelationOutput = atoi(value);
	}
    
    
	// Unknown tags
	else {
		fail = 1;
	}

	return fail;

}


/*
 *  Allocate arrays for memory, etc
 */
void cPixelDetectorCommon::allocatePowderMemory(cGlobal *global) {
    
    // Constants
    nPowderClasses = global->nPowderClasses;
    radialStackSize = global->radialStackSize;    
    
    // Background buffers and the like
    selfdark = (float*) calloc(pix_nn, sizeof(float));
    bg_buffer = (int16_t*) calloc(bgMemory*pix_nn, sizeof(int16_t)); 
    hotpix_buffer = (int16_t*) calloc(hotpixMemory*pix_nn, sizeof(int16_t)); 
    halopix_buffer = (float*) calloc(halopixRecalc*pix_nn, sizeof(float)); 
    
    // Powder sums and mutexes
    for(long i=0; i<nPowderClasses; i++) {
        nPowderFrames[i] = 0;
        // calloc initializes all bits to zero (eq to 0 for int, long, uint, float, double), so no need for further initialization
        powderRaw[i] = (double*) calloc(pix_nn, sizeof(double));
        powderRawSquared[i] = (double*) calloc(pix_nn, sizeof(double));
        powderCorrected[i] = (double*) calloc(pix_nn, sizeof(double));
        powderCorrectedSquared[i] = (double*) calloc(pix_nn, sizeof(double));
        powderAssembled[i] = (double*) calloc(image_nn, sizeof(double));
        //correctedMin[i] = (float*) calloc(pix_nn, sizeof(float));
        //correctedMax[i] = (float*) calloc(pix_nn, sizeof(float));
        //assembledMin[i] = (float*) calloc(image_nn, sizeof(float));
        //assembledMax[i] = (float*) calloc(image_nn, sizeof(float));
        
        pthread_mutex_init(&powderRaw_mutex[i], NULL);
        pthread_mutex_init(&powderRawSquared_mutex[i], NULL);
        pthread_mutex_init(&powderCorrected_mutex[i], NULL);
        pthread_mutex_init(&powderCorrectedSquared_mutex[i], NULL);
        pthread_mutex_init(&powderAssembled_mutex[i], NULL);
        pthread_mutex_init(&radialStack_mutex[i], NULL);
        pthread_mutex_init(&correctedMin_mutex[i], NULL);
        pthread_mutex_init(&correctedMax_mutex[i], NULL);		
        pthread_mutex_init(&assembledMin_mutex[i], NULL);
        pthread_mutex_init(&assembledMax_mutex[i], NULL);
    }
	
    // Radial stacks
	printf("Allocating radial stacks\n");
	for(long i=0; i<nPowderClasses; i++) {
		radialStackCounter[i] = 0;
		radialAverageStack[i] = (float *) calloc(radial_nn*global->radialStackSize, sizeof(float));
	}
	printf("Radial stacks allocated\n");
    
	// Histogram memory
	if(histogram) {
		printf("Allocating histogram memory\n");
		histogram_nx = pix_nx;
		histogram_ny = pix_ny;
		histogram_depth = (histogramMax - histogramMin + 1) / histogramBinSize;
		histogram_nn = (uint64_t)histogram_depth*(uint64_t)(histogram_nx*histogram_ny);
		
		float	histogramMemory;
		float	histogramMemoryGb;
		histogramMemory = (histogram_nn * sizeof(uint16_t));
		histogramMemoryGb = histogramMemory / (1024LL*1024LL*1024LL);
		if (histogramMemoryGb > histogramMaxMemoryGb) {
			printf("Size of histogram buffer would exceed allowed size:\n");
			printf("Histogram depth: %li\n", histogram_depth);
			printf("Histogram buffer size (GB): %f\n", histogramMemoryGb);
			printf("Maximum histogram buffer size (GB): %f\n", histogramMaxMemoryGb);
			printf("Set histogramMaxMemoryGb to a larger value in cheetah.ini if you really want to use a bigger array\n");
			exit(1);
		}
		printf("Histogram buffer size (GB): %f\n", histogramMemoryGb);
		
		// Allocate memory
		histogramData = (uint16_t*) calloc(histogram_nn, sizeof(uint16_t));
		pthread_mutex_init(&histogram_mutex, NULL);
	}
    
    // Angular correlation memory
	if (useAngularCorrelation) {
		printf("Allocating memory for angular correlations\n");
        
        // define length of correlation arrays
		if (autoCorrelateOnly) {
            angularCorrelation_nn = angularCorrelationNumQ*angularCorrelationNumDelta;
		} else {
			angularCorrelation_nn = angularCorrelationNumQ*angularCorrelationNumQ*angularCorrelationNumDelta;
		}
        
        // allocate arrays for look-up tables (LUT)
        if (angularCorrelationAlgorithm == 2) {
            createLookupTable(global, angularCorrelationLUT, angularCorrelationLUTdim1, angularCorrelationLUTdim2); // <-- important that this is done after detector geometry is determined
        } else {
            angularCorrelationLUT = NULL;
        }
        
        // allocate arrays for powder sums
        for(long i=0; i<nPowderClasses; i++) {
            if (sumAngularCorrelation) {
                powderAngularCorrelation[i] = (double*) calloc(angularCorrelation_nn, sizeof(double));
            } else {
                powderAngularCorrelation[i] = NULL;
            }
        }
        
		printf("Memory for angular correlations allocated\n");
    }
    
}


/*
 *	Free detector specific memory
 */
void cPixelDetectorCommon::freeDetectorMemory(cGlobal* global) {
    // background buffers and detector corrections
	free(darkcal);
	free(selfdark);
	free(gaincal);
	free(bg_buffer);
	free(hotpix_buffer);
	free(halopix_buffer);
    free(pixelmask_shared);
    
    // geometry (real and reciprocal space)
    free(pix_x);
    free(pix_y);
    free(pix_z);
    free(pix_dist);
    free(pix_kx);
    free(pix_ky);
    free(pix_kz);
    free(pix_r);
    free(pix_kr);
    free(pix_res);    
    
    // powder sums and radial stacks
	for(long j=0; j<global->nPowderClasses; j++) {
		free(powderRaw[j]);
        free(powderRawSquared[j]);
		free(powderCorrected[j]);
		free(powderCorrectedSquared[j]);
		free(powderAssembled[j]);
		//free(correctedMin[j]);
		//free(assembledMin[j]);
		//free(correctedMax[j]);
		//free(assembledMax[j]);
		free(radialAverageStack[j]);
        
		pthread_mutex_destroy(&powderRaw_mutex[j]);
		pthread_mutex_destroy(&powderRawSquared_mutex[j]);
		pthread_mutex_destroy(&powderCorrected_mutex[j]);
		pthread_mutex_destroy(&powderCorrectedSquared_mutex[j]);
		pthread_mutex_destroy(&powderAssembled_mutex[j]);
		pthread_mutex_destroy(&correctedMin_mutex[j]);
		pthread_mutex_destroy(&correctedMax_mutex[j]);
		pthread_mutex_destroy(&assembledMin_mutex[j]);
		pthread_mutex_destroy(&assembledMax_mutex[j]);
		pthread_mutex_destroy(&radialStack_mutex[j]);
	}
	
	if(histogram) {
		free(histogramData);
	}
    
	if (useAngularCorrelation) {
        // deallocate array for look-up tables (LUT)
        if (angularCorrelationLUT)
            delete[] angularCorrelationLUT;
        
        // deallocate arrays for powder sums
        for(long i=0; i<nPowderClasses; i++) {
            if (powderAngularCorrelation[i])
                free(powderAngularCorrelation[i]);
        }
	}
}



/*
 *	Create lookup table (LUT) for the fast angular correlation algorithm (angularCorrelationAlgorithm = 2)
 */
void cPixelDetectorCommon::createLookupTable(cGlobal *global, int *LUT, int lutNx, int lutNy) {
	//write lookup table for fast angular correlation
	int lutSize = lutNx*lutNy;
	std::cout << "Creating lookup table (LUT) of size " << lutNx << " x " << lutNy << " (" << lutSize << " entries)" << std::endl;
	if (LUT) {		// free memory of old LUT first, if necessary
  		delete[] LUT;
	}
	LUT = new int[lutSize];
	
	//initialize to zero!
	for (int i=0; i<lutSize; i++) {
  		LUT[i] = 0;
	}
	
	//find max and min of the q-calibration arrays
    double	xmax = -1e9;
    double	xmin =  1e9;
    double	ymax = -1e9;
    double	ymin =  1e9;
    for (long i=0;i<pix_nn;i++) {
        if (pix_x[i] > xmax) xmax = (double) pix_x[i];
        if (pix_x[i] < xmin) xmin = (double) pix_x[i];
        if (pix_y[i] > ymax) ymax = (double) pix_y[i];
        if (pix_y[i] < ymin) ymin = (double) pix_y[i];
    }
	double qx_range = fabs(xmax - xmin);
    double qx_stepsize = qx_range/(double)(lutNx-1);
	double qy_range = fabs(ymax - ymin);
    double qy_stepsize = qy_range/(double)(lutNy-1);
	std::cout << "\tLUT x-values: min=" << xmin << ", max=" << xmax << ", range=" << qx_range << ", stepsize=" << qx_stepsize << std::endl;
	std::cout << "\tLUT y-values: min=" << ymin << ", max=" << ymax << ", range=" << qy_range << ", stepsize=" << qy_stepsize << std::endl;
	
	int lutFailCount = 0;
	for (int i = 0; i < pix_nn; i++){           //go through the whole the q-calibration array
        //get q-values from qx and qy arrays
        //and determine at what index (ix, iy) to put them in the lookup table
		const double ix = (pix_x[i]-xmin) / qx_stepsize;
		const double iy = (pix_y[i]-ymin) / qy_stepsize;
		
		//fill table at the found coordinates with the data index
		//overwriting whatever value it had before
	    //(the add-one-half->floor trick is to achieve reasonable rounded integers)
		int lutindex = (int) ( floor(ix+0.5) + lutNx*floor(iy+0.5) );
		//int lutindex = (int) ( floor(ix) + lutNx*floor(iy) );
        
		if (lutindex < 0 || lutindex >= lutSize) {
			lutFailCount++;
            ERROR("Lookup table index out of bounds\n\t (was trying to set LUT[%d] = %d )\n\tLUT: pix_x, pix_y = (%f, %f) --> ix, iy = (%f, %f)\n", lutindex, i, pix_x[i], pix_y[i], ix, iy);
		} else {
			LUT[lutindex] = i;
		}
        
		/////////////////////////////////////////////////////////////////////////////////////////
		//ATTENTION: THIS METHOD WILL LEAD TO A LOSS OF DATA,
		//ESPECIALLY FOR SMALL TABLE SIZES,
		//BUT IT WILL BUY A LOT OF SPEED IN THE LOOKUP PROCESS
		//--> this should be improved to a more precise version, 
		//    maybe even one that allows the lookup(x,y) to interpolate
		//    for that to work, we need to find the four closest points in the data or so
		//    (for instance, instead of one index, the table could contain 
		//    a vector of all applicable indices)
		/////////////////////////////////////////////////////////////////////////////////////////
		
		if (global->debugLevel>2 && (i<=100 || pix_nn-i<=200) ){	//print the first and the last  entries to check
			std::cout << "setting LUT[" << lutindex << "] = " << i << "";
			std::cout << "   LUT: pix_x, pix_y = (" << pix_x[i] << ", " << pix_y[i] << ") --> ix, iy = (" << ix << ", " << iy << ")" << std::endl;
		}
	}//for
	//after all is done, set the zero element to zero 
	//to make sure a failure of lookup() doesn't result in an actual value
	LUT[0] = 0;		
	
	std::cout << "\tLUT created ";
	std::cout << "(info: LUT assignment failed in " << lutFailCount << " of " << lutSize << " cases)" << std::endl;
	
	
	//explicit output to test...
	if (global->debugLevel>2) {
		std::ostringstream osst;
		osst << "-------------------LUT begin---------------------------------" << std::endl;
        for (int j = 0; j<lutNy; j++){
			osst << " [";
			for (int i = 0; i<lutNx; i++) {
				osst << " " << LUT[i+lutNx*j];
			}
			osst << "]" << std::endl;
		}
		osst << "------------------LUT end----------------------------------" << std::endl;		
		std::cout << osst.str() << std::endl;
	}//if	
}



/*
 *	Read in detector pixel layout
 */
void cPixelDetectorCommon::readDetectorGeometry(char* filename) {

  // Pixel size (measurements in geometry file are in m)
  //module_rows = asic_nx;
  //module_cols = asic_ny;	

	
  // Set filename here 
  printf("Reading detector geometry:\n");
  printf("\t%s\n",filename);
	
	
  // Check whether pixel map file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("Error: Detector geometry file does not exist: %s\n",filename);
    exit(1);
  }
	
	
  // Read pixel locations from file
  cData2d		detector_x;
  cData2d		detector_y;
  cData2d		detector_z;
  detector_x.readHDF5(filename, (char *) "x");
  detector_y.readHDF5(filename, (char *) "y");
  detector_z.readHDF5(filename, (char *) "z");
	
  // Sanity check that all detector arrays are the same size (!)
  if (detector_x.nn != detector_y.nn || detector_x.nn != detector_z.nn) {
    printf("readDetectorGeometry: array size mismatch\n");
    printf("Dimensions of x,y,z data do not match\n");
    printf("%li != %li || %li != %li\n", detector_x.nn, detector_y.nn, detector_x.nn, detector_z.nn);
    exit(1);
  }
	

  // Sanity check that size matches what we expect for cspad (!)
  if (detector_x.nx != pix_nx || detector_x.ny != pix_ny) {
    printf("readDetectorGeometry: array size mismatch\n");
    printf("%ldx%ld != %lix%li\n", pix_nx, pix_ny, detector_x.nx, detector_x.ny);
    exit(1);
  }
	
	
  // Create local arrays for detector pixel locations
  long	nx = pix_nx;
  long	ny = pix_ny;
  long	nn = nx*ny;
  long 	i;
  //pix_nx = nx;
  //pix_ny = ny;
  //pix_nn = nn;
  pix_x = (float *) calloc(nn, sizeof(float));
  pix_y = (float *) calloc(nn, sizeof(float));
  pix_z = (float *) calloc(nn, sizeof(float));
  pix_dist = (float *) calloc(nn, sizeof(float));
  pix_kx = (float *) calloc(nn, sizeof(float));
  pix_ky = (float *) calloc(nn, sizeof(float));
  pix_kz = (float *) calloc(nn, sizeof(float));
  pix_kr = (float *) calloc(nn, sizeof(float));
  pix_res = (float *) calloc(nn, sizeof(float));
  //hitfinderResMask = (int *) calloc(nn, sizeof(int)); // is there a better place for this?
  //for (i=0;i<nn;i++) hitfinderResMask[i]=1;
  printf("\tPixel map is %li x %li pixel array\n",nx,ny);
	
	
  // Copy values from 2D array
  for(long i=0;i<nn;i++){
    pix_x[i] = (float) detector_x.data[i];
    pix_y[i] = (float) detector_y.data[i];
    pix_z[i] = (float) detector_z.data[i];
  }
	
		
 bounds:
	
  // Find bounds of image array
  float	xmax = -1e9;
  float	xmin =  1e9;
  float	ymax = -1e9;
  float	ymin =  1e9;
  for(long i=0;i<nn;i++){
    if (pix_x[i] > xmax) xmax = pix_x[i];
    if (pix_x[i] < xmin) xmin = pix_x[i];
    if (pix_y[i] > ymax) ymax = pix_y[i];
    if (pix_y[i] < ymin) ymin = pix_y[i];
  }
  printf("\tGeometry range:\n");
  printf("\tx range %f to %f\n",xmin,xmax);
  printf("\ty range %f to %f\n",ymin,ymax);
	
  // If physical detector size is less than 1 x 1, detector coordinates
  // must have been specified in physical distance (m) and not in pixels.
  if (fabs(xmax-xmin)<1 && fabs(ymax-ymin)<1 ) {
    printf("\tData range <1m in both x and y directions.\n");
    printf("\tConverting detector coordinates from meters to pixels\n");
    printf("\tPixel size: %g m\n",pixelSize);
    for(i=0;i<nn;i++){
      pix_x[i] /= pixelSize;
      pix_y[i] /= pixelSize;
      pix_z[i] /= pixelSize;
    }
    goto bounds;
  }

  fesetround(1);
  xmax = lrint(xmax);
  xmin = lrint(xmin);
  ymax = lrint(ymax);
  ymin = lrint(ymin);
  printf("\tImage bounds:\n");
  printf("\tx range %.0f to %.0f\n",xmin,xmax);
  printf("\ty range %.0f to %.0f\n",ymin,ymax);
	
	
  // How big must we make the output image?
  float max = xmax;
  if(ymax > max) max = ymax;
  if(fabs(xmin) > max) max = fabs(xmin);
  if(fabs(ymin) > max) max = fabs(ymin);
  image_nx = 2*(unsigned)max;
  image_nn = image_nx*image_nx;
  printf("\tImage output array will be %li x %li\n",image_nx,image_nx);
	
  // Apply image center shift
  for(i=0;i<nn;i++){
    pix_x[i] -= beamCenterPixX;
    pix_y[i] -= beamCenterPixY;
  }
	

  // Compute radial distances
  pix_r = (float *) calloc(nn, sizeof(float));
  radial_max = 0.0;
  for(long i=0;i<nn;i++){
    pix_r[i] = sqrt(pix_x[i]*pix_x[i]+pix_y[i]*pix_y[i]);
    if(pix_r[i] > radial_max)
      radial_max = pix_r[i];
  }	
  radial_nn = (long int) ceil(radial_max)+1;

  // How big must we make the output downsampled image?
  imageXxX_nx = image_nx/downsampling;
  imageXxX_nn = image_nn/downsampling/downsampling;
}



/*
 *  Update K-space variables
 *  (called whenever detector has moved)
 */
void cPixelDetectorCommon::updateKspace(cGlobal *global, float wavelengthA) {
  double   x, y, z, r;
  double   kx,ky,kz,kr;
  double   res,minres,maxres;
  double	 sin_theta;
  long     minres_pix,maxres_pix;
  long c = 0;	
  minres = 100000;
  maxres = 0.0;
  minres_pix = 10000000;
  maxres_pix = 0;
  
  // also update constant term of solid angle when detector has moved
  solidAngleConst = pixelSize*pixelSize/(detectorZ*cameraLengthScale*detectorZ*cameraLengthScale);
  
  printf("Recalculating K-space coordinates\n");

  for (long i=0; i<pix_nn; i++ ) {
    x = pix_x[i]*pixelSize;
    y = pix_y[i]*pixelSize;
    z = pix_z[i]*pixelSize + detectorZ*cameraLengthScale;
    r = sqrt(x*x + y*y + z*z);
    
    // assuming incident beam is along +z direction, unit is in inverse A (without 2*pi)
    kx = (x/r)/wavelengthA;
    ky = (y/r)/wavelengthA;
    kz = (z/r - 1)/wavelengthA;
    kr = sqrt(kx*kx + ky*ky + kz*kz);
    //res = 1.0/kr;
    sin_theta = sqrt(x*x+y*y)/r;
    res = wavelengthA/(sin_theta);
        
    pix_kx[i] = kx;
    pix_ky[i] = ky;
    pix_kz[i] = kz;
    pix_kr[i] = kr;
    pix_res[i] = res;
    pix_dist[i] = r;
        
    if ( res < minres ){
      minres = res;
      minres_pix = pix_r[i];
    }
    if ( res > maxres ){
      maxres = res;
      maxres_pix = pix_r[i];
    }
    
    
    // Generate resolution limit mask
    if (!global->hitfinderResolutionUnitPixel){
      // (resolution in Angstrom (!!!))
      if (pix_r[i] < global->hitfinderMaxRes && pix_r[i] > global->hitfinderMinRes ) 
	pixelmask_shared[i] &= ~PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
      else
	pixelmask_shared[i] |= PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
    }
    else{
      // (resolution in pixel (!!!))
      if (pix_r[i] < global->hitfinderMaxRes && pix_r[i] > global->hitfinderMinRes )
	pixelmask_shared[i] &= ~PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
      else
	pixelmask_shared[i] |= PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
    }
  }

  printf("Current resolution (i.e. d-spacing) range is %.2f - %.2f A (%li - %li det. pixels)\n", minres, maxres,minres_pix,maxres_pix);

  if (global->hitfinderResolutionUnitPixel){
    printf("Defined resolution limits for hitfinders: %i - %i detector pixels\n",(int) global->hitfinderMinRes, (int) global->hitfinderMaxRes);
  } else {
    printf("Defined resolution limits for hitfinders: %.2f - %.2f A\n",global->hitfinderMinRes,global->hitfinderMaxRes);
  }


}


/*
 *	Read in darkcal file
 */
void cPixelDetectorCommon::readDarkcal(char *filename){	
	
  // Create memory space and pad with zeros
  darkcal = (float*) calloc(pix_nn, sizeof(float));
  for(long i=0; i<pix_nn; i++)
    darkcal[i] = 0;

  // Do we need a darkcal file?	
  if (useDarkcalSubtraction == 0){
    return;
  }
	
  // Check if a darkcal file has been specified
  if ( strcmp(filename,"") == 0 ){
    printf("Darkcal file path was not specified.\n");
    exit(1);
  }	

  printf("Reading darkcal configuration:\n");
  printf("\t%s\n",filename);

  // Check whether file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("\tDarkcal file does not exist: %s\n",filename);
    printf("\tAborting...\n");
    exit(1);
  }
	
	
  // Read darkcal data from file
  cData2d		temp2d;
  temp2d.readHDF5(filename);
	
  // Correct geometry?
  if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
    printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
    printf("\tAborting...\n");
    exit(1);
  } 
	
  // Copy into darkcal array
  for(long i=0;i<pix_nn;i++)
    darkcal[i] = temp2d.data[i];
	
}


/*
 *	Read in gaincal file
 */
void cPixelDetectorCommon::readGaincal(char *filename){
	

  // Create memory space and set default gain to 1 everywhere
  gaincal = (float*) calloc(pix_nn, sizeof(float));
  for(long i=0;i<pix_nn;i++)
    gaincal[i] = 1;

  // Do we even need a gaincal file?
  if ( useGaincal == 0 ){
    return;
  }

  // Check if a gain calibration file has been specified
  if ( strcmp(filename,"") == 0 ){
    printf("Gain calibration file path was not specified.\n");
    printf("Aborting...\n");
    exit(1);
  }	

  printf("Reading detector gain calibration:\n");
  printf("\t%s\n",filename);

		
  // Check whether gain calibration file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("\tGain calibration file does not exist: %s\n",filename);
    printf("\tAborting...\n");
    exit(1);
  }
	
	
  // Read darkcal data from file
  cData2d		temp2d;
  temp2d.readHDF5(filename);
	

  // Correct geometry?
  if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
    printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
    printf("\tAborting...\n");
    exit(1);
  } 
	
	
  // Copy into gaincal array
  for(long i=0;i<pix_nn;i++)
    gaincal[i] = (float) temp2d.data[i];


  // Invert the gain so we have an array that all we need to do is simple multiplication
  // Pixels with zero gain become dead pixels
  if(invertGain) {
    for(long i=0;i<pix_nn;i++) {
      if(gaincal[i] != 0)
	gaincal[i] = 1.0/gaincal[i];
      else 
	gaincal[i] = 0;
    }
  }
	
}


/*
 *	Read in peaksearch mask
 */
void cPixelDetectorCommon::readPeakmask(cGlobal *global, char *filename){
		
  // Do we even need a peakmask file?
  if ( global->hitfinderUsePeakmask == 0 ){
    return;
  }

  // Check if a peakmask file has been specified
  if ( strcmp(filename,"") == 0 ){
    printf("Peakmask file path was not specified.\n");
    printf("Aborting...\n");
    exit(1);
  }	

  printf("Reading peak search mask:\n");
  printf("\t%s\n",filename);
	
  // Check whether file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("\tPeak search mask does not exist: %s\n",filename);
    printf("\tAborting...\n");
    exit(1);
  }
	
	
  // Read darkcal data from file
  cData2d		temp2d;
  temp2d.readHDF5(filename);
	
	
  // Correct geometry?
  if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
    printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
    printf("\tAborting...\n");
    exit(1);
  } 
	
  for(long i=0;i<pix_nn;i++){
    if((int) temp2d.data[i]==0){
      pixelmask_shared[i] |= PIXEL_IS_IN_PEAKMASK;
    }
    else{
      pixelmask_shared[i] &= ~PIXEL_IS_IN_PEAKMASK;
    }
  }
}

/*
 *	Read in bad pixel mask
 *  (Pixels will be set to zero before any analysis and when data is exported)
 */
void cPixelDetectorCommon::readBadpixelMask(char *filename){
     	
	
  // Do we need a bad pixel map?
  if ( useBadPixelMask == 0 ){
    return;
  }

  // Check if a bad pixel mask file has been specified
  if ( strcmp(filename,"") == 0 ){
    printf("Bad pixel mask file path was not specified.\n");
    printf("Aborting...\n");
    exit(1);
  }	

  printf("Reading bad pixel mask:\n");
  printf("\t%s\n",filename);

  // Check whether file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("\tBad pixel mask does not exist: %s\n",filename);
    printf("\tAborting...\n");
    exit(1);
  }
	
	
  // Read darkcal data from file
  cData2d		temp2d;
  temp2d.readHDF5(filename);
	
	
  // Correct geometry?
  if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
    printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
    printf("\tAborting...\n");
    exit(1);
  } 
	
	
  // Copy back into array
  for(long i=0;i<pix_nn;i++){
    if((int) temp2d.data[i]==0){
      pixelmask_shared[i] |= PIXEL_IS_BAD;
    }
    else{
      pixelmask_shared[i] &= ~PIXEL_IS_BAD;
    }
  }
}

/*
 *	Read in bad data mask
 *  (Pixels will be ignored in analysis, but passed through when data is exported)
 */
void cPixelDetectorCommon::readBaddataMask(char *filename){
	
		
  // Do we need a bad pixel map?
  if ( useBadDataMask == 0 ){
    return;
  }
    
  // Check if a bad pixel mask file has been specified
  if ( strcmp(filename,"") == 0 ){
    printf("Bad data mask file path was not specified.\n");
    printf("Aborting...\n");
    exit(1);
  }	
    
  printf("Reading bad data mask:\n");
  printf("\t%s\n",filename);
    
  // Check whether file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("\tBad data mask does not exist: %s\n",filename);
    printf("\tAborting...\n");
    exit(1);
  }
	
	
  // Read darkcal data from file
  cData2d		temp2d;
  temp2d.readHDF5(filename);
	
	
  // Correct geometry?
  if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
    printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
    printf("\tAborting...\n");
    exit(1);
  } 
	
	
  // Copy back into array
  for(long i=0;i<pix_nn;i++){
    if((int) temp2d.data[i]==0){
      pixelmask_shared[i] |= PIXEL_IS_TO_BE_IGNORED;
    }
    else{
      pixelmask_shared[i] &= ~PIXEL_IS_TO_BE_IGNORED;
    }
  }
}


/*
 *	Read in wire mask
 */
void cPixelDetectorCommon::readWireMask(char *filename){
	
	
  // Do we need this file?
  if ( cspadSubtractBehindWires == 0 ){
    return;
  }
	
  printf("Reading wire mask:\n");
  printf("\t%s\n",filename);

  // Check if a wire mask file has been specified.
  // We need to exit if this is expected but does not
  // exist.
  if ( strcmp(filename,"") == 0 ){
    printf("Wire mask file path was not specified.\n");
    printf("Aborting...\n");
    exit(1);
  }	

  // Check whether file exists!
  FILE* fp = fopen(filename, "r");
  if (fp) 	// file exists
    fclose(fp);
  else {		// file doesn't exist
    printf("\tWire mask does not exist: %s\n",filename);
    printf("\tAborting...\n");
    exit(1);
  }
	
	
  // Read darkcal data from file
  cData2d		temp2d;
  temp2d.readHDF5(filename);
	
	
  // Correct geometry?
  if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
    printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
    printf("\tAborting...\n");
    exit(1);
  } 
	
	
  // Copy into darkcal array
  for(long i=0;i<pix_nn;i++){
    if((int) temp2d.data[i]==0){
      pixelmask_shared[i] |= PIXEL_IS_SHADOWED;
    }
    else{
      pixelmask_shared[i] &= ~PIXEL_IS_SHADOWED;
    }
  }

}

cPixelDetectorEvent::cPixelDetectorEvent() {

  detectorZ=0;

}

