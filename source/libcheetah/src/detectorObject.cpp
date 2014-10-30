/*
 *  detectorObject.cpp
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
	//strcpy(detectorConfigFile, "No_file_specified");
	strcpy(geometryFile, "No_file_specified");
	strcpy(badpixelFile, "No_file_specified");
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
	pixelSize = 110e-6;

    
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
	maskPnccdSaturatedPixels = 0;

	// Static dark calibration (electronic offsets)
	useDarkcalSubtraction = 0;

	// Common mode subtraction from each ASIC
	cmModule = 0;
	cmFloor = 0.1;
    cmStart = -100;
    cmStop = 100;
    cmDelta = 10;
	cspadSubtractUnbondedPixels = 0;
	cspadSubtractBehindWires = 0;
    
	// Gain calibration correction
	useGaincal = 0;
	invertGain = 0;
	
	// Polarization correction
	usePolarizationCorrection = 0;
    horizontalFractionOfPolarization = 1.0;
    
	// Solid angle correction
	useSolidAngleCorrection = 0;
	solidAngleAlgorithm = 1;
    
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
	
	// Radial background subtraction
	useRadialBackgroundSubtraction = 0;

	// Identify persistently hot pixels
	useAutoHotpixel = 0;
	hotpixFreq = 0.9;
	hotpixADC = 1000;
	hotpixMemory = 50;
	// Kill persistently hot pixels
	applyAutoHotpixel = 0;

	// Identify persistently illuminated pixels (halo)
	useAutoHalopixel = 0;
	halopixIncludeHits = 0;
	halopixMinDeviation = 100;
	halopixRecalc = bgRecalc;
	halopixMemory = bgRecalc;
    
	// Histogram stack
	histogram = 0;
	histogramMin = -100;
	histogramNbins = 200;
	histogramBinSize = 1;
	histogram_fs_min = 0;
	histogram_fs_max = 1552;
	histogram_nfs = 1552;
	histogram_ss_min = 0;
	histogram_ss_max = 1480;
	histogram_nss = 1480;
	histogramMaxMemoryGb = 4;
	histogram_count = 0;
  
	// correction for PNCCD read out artifacts 
	usePnccdOffsetCorrection = 0;
	usePnccdFixWiringError = 0;
	usePnccdLineInterpolation = 0;
	usePnccdLineMasking = 0;

	// Saving options
	saveDetectorCorrectedOnly = 0;
	saveDetectorRaw = 0;

	// Downsampling factor (1: no downsampling)
	downsampling = 1;
	downsamplingConservative = 1;
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
	} else if(strcmp(detectorName, "CxiSc2") == 0 || strcmp(detectorName, "CxiDg2") == 0) {
		strcpy(detectorType, "cspad2x2");
		asic_nx = CSPAD_ASIC_NX;
		asic_ny = CSPAD_ASIC_NY;
		asic_nn = CSPAD_ASIC_NX * CSPAD_ASIC_NY;
		nasics_x = CSPAD2x2_nASICS_X;
		nasics_y = CSPAD2x2_nASICS_Y;
		pix_nx = nasics_x*asic_nx;
		pix_ny = nasics_y*asic_ny;
		pix_nn = pix_nx * pix_ny;
		pixelSize = 110e-6;
	} else if(strcmp(detectorName, "pnCCD") == 0 ) {
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
	else if(strcmp(detectorName, "sacla_mpCCD") == 0 ) {
		strcpy(detectorType, "sacla_mpCCD");
		asic_nx = mpCCD_ASIC_NX;
		asic_ny = mpCCD_ASIC_NY;
		asic_nn = mpCCD_ASIC_NX * mpCCD_ASIC_NY;
		nasics_x = mpCCD_nASICS_X;
		nasics_y = mpCCD_nASICS_Y;
		pix_nx = mpCCD_ASIC_NX * mpCCD_nASICS_X;
		pix_ny = mpCCD_ASIC_NY * mpCCD_nASICS_Y;
		pix_nn = pix_nx * pix_ny;
		pixelSize = 50e-6;
      
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
		if(strcasecmp(value,"tof") == 0){
            fprintf(stderr,"Error: detectortype = tof found, but was not first line in [group]\n");
            fprintf(stderr,"If you want to specify a TOF detector make sure the detectortype is the first line of the [group]\n");
			fprintf(stderr,"Quitting...\n");
			exit(1);
		}
		strcpy(detectorType, value);
	}
	else if (!strcmp(tag, "detectorid")) {
		detectorID = atoi(value);
	}
	else if (!strcmp(tag, "geometry")) {
		strcpy(geometryFile, value);
	}
	else if (!strcmp(tag, "darkcal")) {
		strcpy(darkcalFile, value);
		useDarkcalSubtraction = 1;
	}
	else if (!strcmp(tag, "gaincal")) {
		strcpy(gaincalFile, value);
		useGaincal = 1;
	}
	else if ((!strcmp(tag, "badpixelmap")) || (!strcmp(tag, "badpixelmask"))) {
		strcpy(badpixelFile, value);
		useBadPixelMask = 1;
	}
	else if ((!strcmp(tag, "setbadpixelstozero")) || (!strcmp(tag, "applybadpixelmap")) || (!strcmp(tag, "applybadpixelmask"))) {
		applyBadPixelMask = atoi(value);
	}
	else if (!strcmp(tag, "baddatamap")) {
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
	else if (!strcmp(tag, "downsamplingrescale")) {
		printf("The keyword downsamplingRescale is depreciated. The option is no longer supported.\n"
			   "Modify your ini file and try again...\n");
		fail = 1;		
	}
	else if (!strcmp(tag, "downsamplingconservative")) {
		downsamplingConservative = atoi(value);
	}
	else if (!strcmp(tag, "savedetectorcorrectedonly")) {
		saveDetectorCorrectedOnly = atoi(value);
	}
	else if (!strcmp(tag, "savedetectorraw")) {
		saveDetectorRaw = atoi(value);
	}
	else if (!strcmp(tag, "beamcenterx")) {
		beamCenterPixX  = atof(value);
	}
	else if (!strcmp(tag, "beamcentery")) {
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
	else if ((!strcmp(tag, "maskpnccdsaturatedpixels"))){
		maskPnccdSaturatedPixels = atoi(value);
	}
	else if ((!strcmp(tag, "masksaturatedpixels")) || (!strcmp(tag, "usemasksaturatedpixels"))) {
		maskSaturatedPixels = atoi(value);
	}
	else if (!strcmp(tag, "bgmemory")) {
		bgMemory = atoi(value);
	}
	else if (!strcmp(tag, "useautohotpixel")) {
		useAutoHotpixel = atoi(value);
		// Eventually delete this, but not during beamtime!
	}
	else if (!strcmp(tag, "hotpixfreq")) {
		hotpixFreq = atof(value);
		//useAutoHotpixel = 1;
		//applyAutoHotpixel = 1;
	}
	else if (!strcmp(tag, "hotpixadc")) {
		hotpixADC = atoi(value);
		//useAutoHotpixel = 1;
		//applyAutoHotpixel = 1;
	}
	else if ((!strcmp(tag, "sethotpixelstozero")) || (!strcmp(tag, "applyautohotpixel"))) {
		applyAutoHotpixel = atoi(value);
	}
	else if (!strcmp(tag, "hotpixmemory")) {
		hotpixMemory = atoi(value);
	}
	else if (!strcmp(tag, "useautohalopixel")) {
		useAutoHalopixel = atoi(value);
	}
	else if ((!strcmp(tag, "halopixmemory")) || (!strcmp(tag, "halopixelmemory"))) {
		halopixMemory = atoi(value);
	}
	else if ((!strcmp(tag, "halopixrecalc")) || (!strcmp(tag, "halopixelrecalc"))) {
		halopixRecalc = atoi(value);
	}
	else if (!strcmp(tag, "halopixmindeviation")) {
		halopixMinDeviation = atof(value);
	}
	else if (!strcmp(tag, "halopixincludehits")) {
		halopixIncludeHits = atoi(value);
	}
	else if (!strcmp(tag, "cmmodule")) {
		cmModule = atoi(value);
	}
	else if (!strcmp(tag, "cmfloor")) {
		cmFloor = atof(value);
	}
    else if (!strcmp(tag, "cmstart")) {
        cmStart = atoi(value);
    }
    else if (!strcmp(tag, "cmstop")) {
        cmStop = atoi(value);
    }
    else if (!strcmp(tag, "cmdelta")) {
        cmDelta = atof(value);
    }
	// Local background subtraction
	else if (!strcmp(tag, "uselocalbackgroundsubtraction")) {
		useLocalBackgroundSubtraction = atoi(value);
	}
	else if (!strcmp(tag, "localbackgroundradius")) {
		localBackgroundRadius = atoi(value);
	}
	else if (!strcmp(tag, "useradialbackgroundsubtraction")) {
		useRadialBackgroundSubtraction = atoi(value);
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
	else if ( (!strcmp(tag, "subtractbehindwires")) || (!strcmp(tag, "usesubtractbehindwires")) ){
		cspadSubtractBehindWires = atoi(value);
	}
	else if (!strcmp(tag, "invertgain")) {
		invertGain = atoi(value);
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
	else if ( (!strcmp(tag, "subtractunbondedpixels")) || (!strcmp(tag, "usesubtractunbondedpixels")) ) {
		cspadSubtractUnbondedPixels = atoi(value);
	}
	else if (!strcmp(tag, "usepnccdoffsetcorrection")) {
		usePnccdOffsetCorrection = atoi(value);
	}
	else if (!strcmp(tag, "usepnccdfixwiringerror")) {
		usePnccdFixWiringError = atoi(value);
	}
	else if (!strcmp(tag, "usepnccdlineinterpolation")) {
		usePnccdLineInterpolation = atoi(value);
	}
	else if (!strcmp(tag, "usepnccdlinemasking")) {
		usePnccdLineMasking = atoi(value);
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
	// depreciated?
	else if (!strcmp(tag, "bgnobeamreset")) {
		bgNoBeamReset = atoi(value);
	}
	// depreciated?
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
	else if (!strcmp(tag, "histogramnbins")) {
		histogramNbins = atoi(value);
	}
	else if (!strcmp(tag, "histogrambinsize")) {
		histogramBinSize = atof(value);
	}
	else if (!strcmp(tag, "histogram_fs_min")) {
		histogram_fs_min = atoi(value);
	}
	else if (!strcmp(tag, "histogram_fs_max")) {
		histogram_fs_max = atoi(value);
	}
	else if (!strcmp(tag, "histogram_ss_min")) {
		histogram_ss_min = atoi(value);
	}
	else if (!strcmp(tag, "histogram_ss_max")) {
		histogram_ss_max = atoi(value);
	}
	else if (!strcmp(tag, "histogrammaxmemorygb")) {
		histogramMaxMemoryGb = atof(value);
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
	halopix_buffer = (float*) calloc(halopixMemory*pix_nn, sizeof(float));
	halopix_mutexes = (pthread_mutex_t*) calloc(halopixMemory, sizeof(pthread_mutex_t));
	for (long j=0; j<halopixMemory; j++) {
		pthread_mutex_init(&halopix_mutexes[j], NULL);
	}

	for(long j=0; j<pix_nn; j++) {
		selfdark[j] = 0;
	}
    
	// Powder sums and mutexes
	for(long i=0; i<nPowderClasses; i++) {
		nPowderFrames[i] = 0;
		powderRaw[i] = (double*) calloc(pix_nn, sizeof(double));
		powderRawSquared[i] = (double*) calloc(pix_nn, sizeof(double));
		powderCorrected[i] = (double*) calloc(pix_nn, sizeof(double));
		powderCorrectedSquared[i] = (double*) calloc(pix_nn, sizeof(double));
		powderPeaks[i] = (double*) calloc(pix_nn, sizeof(double));
		powderAssembled[i] = (double*) calloc(image_nn, sizeof(double));
		powderAssembledSquared[i] = (double*) calloc(image_nn, sizeof(double));
		powderDownsampled[i] = (double*) calloc(image_nn, sizeof(double));
		powderDownsampledSquared[i] = (double*) calloc(image_nn, sizeof(double));
		correctedMin[i] = (float*) calloc(pix_nn, sizeof(float));
		correctedMax[i] = (float*) calloc(pix_nn, sizeof(float));
		assembledMin[i] = (float*) calloc(image_nn, sizeof(float));
		assembledMax[i] = (float*) calloc(image_nn, sizeof(float));
        
		pthread_mutex_init(&powderRaw_mutex[i], NULL);
		pthread_mutex_init(&powderRawSquared_mutex[i], NULL);
		pthread_mutex_init(&powderCorrected_mutex[i], NULL);
		pthread_mutex_init(&powderCorrectedSquared_mutex[i], NULL);
		pthread_mutex_init(&powderAssembled_mutex[i], NULL);
		pthread_mutex_init(&powderAssembledSquared_mutex[i], NULL);
		pthread_mutex_init(&powderDownsampled_mutex[i], NULL);
		pthread_mutex_init(&powderDownsampledSquared_mutex[i], NULL);
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
        
		for(long j=0; j<radial_nn*global->radialStackSize; j++) {
			radialAverageStack[i][j] = 0;
		}
	}
	printf("Radial stacks allocated\n");
	
	
	// Histogram memory
	if(histogram) {
		printf("Allocating histogram memory\n");
		histogram_nfs = histogram_fs_max - histogram_fs_min;
		histogram_nss = histogram_ss_max - histogram_ss_min;
		histogram_nn = histogram_nfs*histogram_nss;
		histogram_nnn = (uint64_t) histogramNbins * (uint64_t)(histogram_nn);
		
		float	histogramMemory;
		float	histogramMemoryGb;
		histogramMemory = (histogram_nnn * sizeof(uint16_t));
		histogramMemoryGb = histogramMemory / (1024LL*1024LL*1024LL);
		if (histogramMemoryGb > histogramMaxMemoryGb) {
			printf("Size of histogram buffer would exceed allowed size:\n");
			printf("Histogram depth: %li\n", histogramNbins);
			printf("Histogram buffer size (GB): %f\n", histogramMemoryGb);
			printf("Maximum histogram buffer size (GB): %f\n", histogramMaxMemoryGb);
			printf("Set histogramMaxMemoryGb to a larger value in cheetah.ini if you really want to use a bigger array\n");
			exit(1);
		}
		printf("Histogram buffer size (GB): %f\n", histogramMemoryGb);
		
		// Allocate memory
		histogramData = (uint16_t*) calloc(histogram_nnn, sizeof(uint16_t));
		pthread_mutex_init(&histogram_mutex, NULL);
    
	}
}


/*
 *	Free detector specific memory
 */
void cPixelDetectorCommon::freePowderMemory(cGlobal* global) {
	free(darkcal);
	free(selfdark);
	free(gaincal);
	free(bg_buffer);
	free(hotpix_buffer);
	free(halopix_buffer);
	
	for(long j=0; j<global->nPowderClasses; j++) {
		free(powderRaw[j]);
		free(powderCorrected[j]);
		free(powderCorrectedSquared[j]);
		free(powderPeaks[j]);
		free(powderAssembled[j]);
		free(radialAverageStack[j]);
		pthread_mutex_destroy(&powderRaw_mutex[j]);
		pthread_mutex_destroy(&powderCorrected_mutex[j]);
		pthread_mutex_destroy(&powderCorrectedSquared_mutex[j]);
		pthread_mutex_destroy(&powderAssembled_mutex[j]);
		pthread_mutex_destroy(&radialStack_mutex[j]);
	}
	
	if(histogram) {
		free(histogramData);
	}

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
	
	
	cData2d		detector_x;
	cData2d		detector_y;
	cData2d		detector_z;
  
	// Check whether pixel map file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) { 	// file exists
		fclose(fp);
		// Read pixel locations from file
		detector_x.readHDF5(filename, (char *) "x");
		detector_y.readHDF5(filename, (char *) "y");
		detector_z.readHDF5(filename, (char *) "z");
	} else {		// file doesn't exist
		printf("Detector geometry file does not exist: %s, make standard geometry.\n",filename);
		detector_x.create(pix_nx,pix_ny);
		detector_y.create(pix_nx,pix_ny);
		detector_z.create(pix_nx,pix_ny);
		for (long i=0;i<pix_ny;i++){
			for(long j=0;j<pix_nx;j++){
				detector_x.data[j+i*pix_nx] = j-pix_nx/2.;
				detector_y.data[j+i*pix_nx] = i-pix_ny/2.;
				detector_z.data[j+i*pix_nx] = 0.;
			}
		}
	}
	
	
	
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

	xmax = (long) (xmax + 0.5);
	xmin = (long) (xmin - 0.5);
	ymax = (long) (ymax + 0.5);
	ymin = (long) (ymin - 0.5);
	printf("\tImage bounds:\n");
	printf("\tx range %.0f to %.0f\n",xmin,xmax);
	printf("\ty range %.0f to %.0f\n",ymin,ymax);
	
	
	// How big must we make the output image?
	float max = xmax;
	if(ymax > max) max = ymax;
	if(fabs(xmin) > max) max = fabs(xmin);
	if(fabs(ymin) > max) max = fabs(ymin);
	image_nx = 2*(unsigned)max;
	image_ny = image_nx;
	image_nn = image_nx*image_ny;
	printf("\tImage output array will be %li x %li\n",image_ny,image_nx);
	
	// Apply image center shift
	for(i=0;i<nn;i++){
		pix_x[i] -= beamCenterPixX + 0.5;
		pix_y[i] -= beamCenterPixY + 0.5;
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
	imageXxX_nx = (long)ceil(image_nx/(double)downsampling);
	imageXxX_ny = imageXxX_nx;
	imageXxX_nn = imageXxX_nx*imageXxX_ny;
	if (downsampling>1){
		printf("\tDownsampled image output array will be %li x %li\n",imageXxX_ny,imageXxX_nx);
	}
}



/*
 *  Update K-space variables
 *  (called whenever detector has moved)
 */
void cPixelDetectorCommon::updateKspace(cGlobal *global, float wavelengthA) {
	double   x, y, z, r;
	double   kx,ky,kz,kr;
	double   res,minres,maxres;
	double     minres_pix,maxres_pix;
	minres = 1.e10;
	maxres = 0.;
	minres_pix = 0;
	maxres_pix = 1000000;
    
	printf("Recalculating K-space coordinates\n");

	for (long i=0; i<pix_nn; i++ ) {
		x = pix_x[i]*pixelSize;
		y = pix_y[i]*pixelSize;
		z = pix_z[i]*pixelSize + detectorZ*cameraLengthScale;
		r = sqrt(x*x + y*y + z*z);
		
		kx = (x/r)/wavelengthA;
		ky = (y/r)/wavelengthA;
		kz = (z/r - 1)/wavelengthA;                 // assuming incident beam is along +z direction
		kr = sqrt(kx*kx + ky*ky + kz*kz);
		res = 1.0/kr;
        
		pix_kx[i] = kx;
		pix_ky[i] = ky;
		pix_kz[i] = kz;
		pix_kr[i] = kr;
		pix_res[i] = res;
        
		if ( res > minres ){
			minres = res;
			
			maxres_pix = pix_r[i];
		}
		if ( res < maxres ){
			maxres = res;
			minres_pix = pix_r[i];
		}
    
		// Generate resolution limit mask
		if (!global->hitfinderResolutionUnitPixel){
			// (resolution in Angstrom (!!!))
			if (pix_res[i] < global->hitfinderMaxRes && pix_res[i] > global->hitfinderMinRes ) 
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

	printf("Current resolution (i.e. d-spacing) range is %.2f - %.2f A (%f - %f det. pixels)\n", minres, maxres,minres_pix,maxres_pix);

	if (global->hitfinderResolutionUnitPixel){
		printf("Defined resolution limits for hitfinders: %i - %i detector pixels\n",(int) global->hitfinderMinRes, (int) global->hitfinderMaxRes);
	} else {
		printf("Defined resolution limits for hitfinders: %.2f - %.2f A\n",global->hitfinderMinRes,global->hitfinderMaxRes);
	}
    
	// also update constant term of solid angle when detector has moved
	solidAngleConst = pixelSize*pixelSize/(detectorZ*cameraLengthScale*detectorZ*cameraLengthScale);
    
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
	
	// Correct detector name?
	if (strcmp(temp2d.detectorName,"") == 0) {
		printf("\tWARNING: The attribute detectorName could not be read from the darkcal file. Cannot verify whether the darkcal file was generated from the detector of the specified detector type.\n");
	}else{
		if (strcmp(temp2d.detectorName,detectorName) == 0){
			printf("\tDetector names from darkcal (%s) and the associated detector (%s) match.\n",temp2d.detectorName,detectorName);
		}else{
			printf("\tDetector names from darkcal (%s) and the associated detector (%s) do not match.\n",temp2d.detectorName,detectorName);
			printf("\tAborting...\n");
			exit(1);
		}
	}

	// Correct detector ID?
	if (temp2d.detectorID == -1) {
		printf("\tWARNING: The attribute detectorID could not be read from the darkcal file. Cannot verify whether the darkcal file was generated from the detector of the specified detector ID.\n");
	}else{
		if (temp2d.detectorID == detectorID){
			printf("\tDetector IDs from darkcal (%li) and the associated detector (%li) match.\n",temp2d.detectorID,detectorID);
		}else{
			printf("\tDetector IDs from darkcal (%li) and the associated detector (%li) do not match.\n",temp2d.detectorID,detectorID);
			printf("\tAborting...\n");
			exit(1);
		}
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


	// By default the gaincal should be the number we want to multiply by.
    // Offer the option to invert the gain if what is supplied is inverted
    //  so we have an array that all we need to do is simple multiplication
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
	
	
	// Read badpixel maskl data from file
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
	
	// Read wire mask from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	
	
	// Correct geometry?
	if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
		printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
		printf("\tAborting...\n");
		exit(1);
	} 
	
	
	// Copy into pixel mask
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
	/* FM: Warning. This is not run when malloc'ed*/
	detectorZ=0;

}

