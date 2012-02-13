/*
 *  setup.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
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
#include <sys/time.h>
#include <math.h>
#include <limits>
#include <hdf5.h>
#include <fenv.h>
#include <stdlib.h>

#include "data2d.h"
#include "pixelDetector.h"
#include "setup.h"
#include "worker.h"


/*
 *	Default settings/configuration
 */
void cGlobal::defaultConfiguration(void) {

	// ini file to use
	strcpy(configFile, "cheetah.ini");

	// Default experiment info (in case beamline data is missing...)
	defaultPhotonEnergyeV = 0; 	
	
	// Detector info
    nDetectors = 1;
	for(long i=0; i<MAX_DETECTORS; i++) {
		// pdsinfo
		strcpy(detector[i].detectorTypeName, "cspad");
		strcpy(detector[i].detectorName, "CxiDs1");
		detector[i].detectorType = Pds::DetInfo::Cspad;
		detector[i].detectorPdsDetInfo = Pds::DetInfo::CxiDs1;

		// Calibration files
		strcpy(detector[i].geometryFile, "No_file_specified");
		strcpy(detector[i].badpixelFile, "No_file_specified");
		strcpy(detector[i].darkcalFile, "No_file_specified");
		strcpy(detector[i].wireMaskFile, "No_file_specified");
		strcpy(detector[i].gaincalFile, "No_file_specified");
		
		detector[i].asic_nx = CSPAD_ASIC_NX;
		detector[i].asic_ny = CSPAD_ASIC_NY;
		detector[i].asic_nn = CSPAD_ASIC_NX * CSPAD_ASIC_NY;
		detector[i].nasics_x = CSPAD_nASICS_X;
		detector[i].nasics_y = CSPAD_nASICS_Y;
		detector[i].pixelSize = 110e-6;
	}

	// Statistics 
	summedPhotonEnergyeV = 0;
	meanPhotonEnergyeV = 0;	
    
	
    // Detector Z position
	strcpy(detectorZpvname, "CXI:DS1:MMS:06.RBV");
	defaultCameraLengthMm = std::numeric_limits<float>::quiet_NaN();
	defaultCameraLengthMm = 0;
	detposprev = 0;
    cameraLengthOffset = 500.0 + 79.0;
    cameraLengthScale = 1e-3;

    // Pv values
    strcpy(laserDelayPV, "LAS:FS5:Angle:Shift:Ramp:rd");
    laserDelay = std::numeric_limits<float>::quiet_NaN();
    laserDelay = 0;
	
	// Start and stop frames
	startAtFrame = 0;
	stopAtFrame = 0;
	
	// Bad pixel mask
	useBadPixelMask = 0;

	// Saturated pixels
	maskSaturatedPixels = 0;
	pixelSaturationADC = 15564;  // 95% of 2^14 ??

	// Static dark calibration (electronic offsets)
	useDarkcalSubtraction = 0;
	generateDarkcal = 0;
	
	// Common mode subtraction from each ASIC
	cmModule = 0;
	cmFloor = 0.1;
	cmSubtractUnbondedPixels = 0;
	cmSubtractBehindWires = 0;


	// Gain calibration correction
	useGaincal = 0;
	invertGain = 0;
	generateGaincal = 0;
	
	// Subtraction of running background (persistent photon background) 
	useSubtractPersistentBackground = 0;
	bgMemory = 50;
	startFrames = 0;
	scaleBackground = 0;
	useBackgroundBufferMutex = 1;
	bgMedian = 0.5;
	bgRecalc = bgMemory;
	bgIncludeHits = 0;
	bgNoBeamReset = 0;
	bgFiducialGlitchReset = 0;
	
	// Local background subtraction
	useLocalBackgroundSubtraction = 0;
	localBackgroundRadius = 3;
	
	// Kill persistently hot pixels
	useAutoHotpixel = 1;
	hotpixFreq = 0.9;
	hotpixADC = 1000;
	hotpixMemory = 50;
	
	// Hitfinding
	hitfinder = 0;
	hitfinderADC = 100;
	hitfinderNAT = 100;
	hitfinderTAT = 1e3;
	hitfinderNpeaks = 50;
	hitfinderNpeaksMax = 100000;
	hitfinderAlgorithm = 3;
	hitfinderMinPixCount = 3;
	hitfinderMaxPixCount = 20;
	hitfinderUsePeakmask = 0;
	hitfinderCheckGradient = 0;
	hitfinderMinGradient = 0;
	strcpy(peaksearchFile, "");
	savePeakInfo = 1;
	hitfinderCheckPeakSeparation = 0;
	hitfinderMaxPeakSeparation = 50;
	hitfinderSubtractLocalBG = 0;
	hitfinderLocalBGRadius = 4;
	hitfinderLocalBGThickness = 1;
	hitfinderLimitRes = 0;
	hitfinderMinRes = 1e20;
	hitfinderMaxRes = 0;
	hitfinderMinSNR = 40;
	
	// TOF (Aqiris)
	hitfinderUseTOF = 0;
	hitfinderTOFMinSample = 0;
	hitfinderTOFMaxSample = 1000;
	hitfinderTOFThresh = 100;
	
	
	// TOF configuration
	TOFPresent = 0;
	TOFchannel = 1;
	strcpy(tofName, "CxiSc1");
	tofType = Pds::DetInfo::Acqiris;
	tofPdsDetInfo = Pds::DetInfo::CxiSc1;
	


	// Powder pattern generation
	nPowderClasses = 2;
	powderthresh = -20000;
	powderSumHits = 1;
	powderSumBlanks = 0;

    // Radial average stacks
    saveRadialStacks=0;
    radialStackSize=10000;
	
	// Saving options
	savehits = 0;
	saveAssembled = 1;
	saveRaw = 0;
	hdf5dump = 0;
	saveDetectorCorrectedOnly = 0;
	saveDetectorRaw = 0;
	saveInterval = 1000;
	
	// Peak lists
	savePeakList = 1;
	
	// Verbosity
	debugLevel = 2;
	
	// I/O speed test?
	ioSpeedTest = 0;

    // cspad default parameters

	
	// Default to only a few threads
	nThreads = 16;
	useHelperThreads = 0;
	threadPurge = 10000;
	
	// Log files
	strcpy(logfile, "log.txt");
	strcpy(framefile, "frames.txt");
	strcpy(cleanedfile, "cleaned.txt");
	strcpy(peaksfile, "peaks.txt");
	
	// Fudge EVR41 (modify EVR41 according to the Acqiris trace)...
	fudgeevr41 = 0; // this means no fudge by default	
	
}



/*
 *	Setup stuff to do with thread management, settings, etc.
 */
void cGlobal::setup() {
	
	/*
	 *	Determine detector type
	 *	This section of code possibly redundant if we know detector type from the address 
	 *	(eg: CxiDs1 is always a cspad)
	 */
	for(long i=0; i<nDetectors; i++) {
		if(!strcmp(detector[i].detectorTypeName, "cspad"))
			detector[i].detectorType = Pds::DetInfo::Cspad;
		else if(!strcmp(detector[i].detectorTypeName, "pnccd")) {
			detector[i].detectorType = Pds::DetInfo::Cspad;
		}
		else {
			printf("Error: unknown detector type %s\n", detector[i].detectorTypeName);
			printf("Quitting\n");
			exit(1);
		}
	}
	
	/*
	 *	Determine detector address
	 *	A list of addresses can be found in:
	 *		release/pdsdata/xtc/Detinfo.hh
	 *		release/pdsdata/xtc/src/Detinfo.cc
	 */
	for(long i=0; i<nDetectors; i++) {
		if(!strcmp(detector[i].detectorName, "CxiDs1")) {
			detector[i].detectorType = Pds::DetInfo::Cspad;
			detector[i].detectorPdsDetInfo = Pds::DetInfo::CxiDs1;
		}
		else if (!strcmp(detector[i].detectorName, "CxiDs2")) {
			detector[i].detectorType = Pds::DetInfo::Cspad;
			detector[i].detectorPdsDetInfo = Pds::DetInfo::CxiDs2;
		}
		else if (!strcmp(detector[i].detectorName, "CxiDsd")) {
			detector[i].detectorType = Pds::DetInfo::Cspad;
			detector[i].detectorPdsDetInfo = Pds::DetInfo::CxiDsd;
		}
		else if (!strcmp(detector[i].detectorName, "XppGon")) {
			detector[i].detectorType = Pds::DetInfo::Cspad;
			detector[i].detectorPdsDetInfo = Pds::DetInfo::XppGon;
		}
		else {
			printf("Error: unknown detector %s\n", detector[i].detectorName);
			printf("Quitting\n");
			exit(1);
		}
	}
	
	/*
	 *	Detector parameters
	 */
	for(long i=0; i<nDetectors; i++) {
		switch(detector[i].detectorType) {
			case Pds::DetInfo::Cspad : 
				detector[i].asic_nx = CSPAD_ASIC_NX;
				detector[i].asic_ny = CSPAD_ASIC_NY;
				detector[i].asic_nn = CSPAD_ASIC_NX * CSPAD_ASIC_NY;
				detector[i].nasics_x = CSPAD_nASICS_X;
				detector[i].nasics_y = CSPAD_nASICS_Y;
				break;
				
			default:
				printf("Error: unknown detector %s\n", detector[i].detectorName);
				printf("Quitting\n");
				exit(1);
				break;
		}
	}	
	
	
	/*
	 *	Determine TOF (Acqiris) address
	 *	A list of addresses can be found in:
	 *		release/pdsdata/xtc/Detinfo.hh
	 *		release/pdsdata/xtc/src/Detinfo.cc
	 */
	if(!strcmp(tofName, "CxiSc1")) {
		tofType = Pds::DetInfo::Acqiris;
		tofPdsDetInfo = Pds::DetInfo::CxiSc1;
	}
	
	/*
     * How many types of powder pattern do we need?
     */
    if(hitfinder==0)
        nPowderClasses=1;
    else
        nPowderClasses=2;

    if(generateDarkcal || generateGaincal)
        nPowderClasses=1;
    
	
	/*
	 *	Set up arrays for remembering powder data, background, etc.
	 */
	for(long i=0; i<nDetectors; i++) {
		detector[i].selfdark = (float*) calloc(detector[i].pix_nn, sizeof(float));
		detector[i].bg_buffer = (int16_t*) calloc(bgMemory*detector[i].pix_nn, sizeof(int16_t)); 
		detector[i].hotpix_buffer = (int16_t*) calloc(hotpixMemory*detector[i].pix_nn, sizeof(int16_t)); 
		detector[i].hotpixelmask = (int16_t*) calloc(detector[i].pix_nn, sizeof(int16_t));
		detector[i].wiremask = (int16_t*) calloc(detector[i].pix_nn, sizeof(int16_t));
		for(long j=0; j<detector[i].pix_nn; j++) {
			detector[i].selfdark[j] = 0;
			detector[i].hotpixelmask[j] = 1;
			detector[i].wiremask[j] = 1;
		}
	}
	
	hitfinderResMask = (int	*) calloc(detector[0].pix_nn, sizeof(int));
	for(long j=0; j<detector[0].pix_nn; j++) {
		hitfinderResMask[j] = 1;
	}	
	
	
	/*
     *  Set up arrays for powder classes and radial stacks
	 *	Currently only tracked for detector[0]  (generalise this later)
     */
	for(long i=0; i<nPowderClasses; i++) {
		nPowderFrames[i] = 0;
		powderRaw[i] = (double*) calloc(detector[0].pix_nn, sizeof(double));
		powderRawSquared[i] = (double*) calloc(detector[0].pix_nn, sizeof(double));
		powderAssembled[i] = (double*) calloc(detector[0].image_nn, sizeof(double));

        radialStackCounter[i] = 0;
        radialAverageStack[i] = (float *) calloc(detector[0].radial_nn*radialStackSize, sizeof(float));
        
		pthread_mutex_init(&powderRaw_mutex[i], NULL);
		pthread_mutex_init(&powderRawSquared_mutex[i], NULL);
		pthread_mutex_init(&powderAssembled_mutex[i], NULL);
        pthread_mutex_init(&radialStack_mutex[i], NULL);
		
		for(long j=0; j<detector[0].pix_nn; j++) {
			powderRaw[i][j] = 0;
			powderRawSquared[i][j] = 0;
		}
		for(long j=0; j<detector[0].image_nn; j++) {
			powderAssembled[i][j] = 0;
		}
        
        for(long j=0; j<detector[0].radial_nn*radialStackSize; j++) {
            radialAverageStack[i][j] = 0;
        }

		char	filename[1024];
        powderlogfp[i] = NULL;
        if(runNumber > 0) {
            sprintf(filename,"r%04u-class%ld-log.txt",runNumber,i);
            powderlogfp[i] = fopen(filename, "w");
        }
	}
	
	

	
	/*
	 *	Set up thread management
	 */
	nActiveThreads = 0;
	threadCounter = 0;
	pthread_mutex_init(&nActiveThreads_mutex, NULL);
	pthread_mutex_init(&hotpixel_mutex, NULL);
	pthread_mutex_init(&selfdark_mutex, NULL);
	pthread_mutex_init(&bgbuffer_mutex, NULL);
	pthread_mutex_init(&nhits_mutex, NULL);
	pthread_mutex_init(&framefp_mutex, NULL);
	pthread_mutex_init(&powderfp_mutex, NULL);
	pthread_mutex_init(&peaksfp_mutex, NULL);
	threadID = (pthread_t*) calloc(nThreads, sizeof(pthread_t));

	
	/*
	 *	Trap specific configurations and mutually incompatible options
	 */
	if(generateDarkcal) {
		cmModule = 0;
		cmSubtractUnbondedPixels = 0;
		useDarkcalSubtraction = 0;
		useGaincal=0;
		useAutoHotpixel = 0;
		useSubtractPersistentBackground = 0;
		hitfinder = 0;
		savehits = 0;
		hdf5dump = 0;
		saveRaw = 0;
		saveDetectorRaw = 1;
		powderSumHits = 0;
		powderSumBlanks = 0;
		powderthresh = -30000;
		startFrames = 0;
		saveDetectorCorrectedOnly = 1;
		printf("keyword generatedarkcal set: overriding some keyword values!!!");
	}

	if(generateGaincal) {
		cmModule = 0;
		cmSubtractUnbondedPixels = 0;
		useDarkcalSubtraction = 1;
		useAutoHotpixel = 0;
		useSubtractPersistentBackground = 0;
		useGaincal=0;
		hitfinder = 0;
		savehits = 0;
		hdf5dump = 0;
		saveRaw = 0;
		saveDetectorRaw = 1;
		powderSumHits = 0;
		powderSumBlanks = 0;
		powderthresh = -30000;
		startFrames = 0;
		saveDetectorCorrectedOnly = 1;
		printf("keyword generategaincal set: overriding some keyword values!!!");
	}

	
	if(saveRaw==0 && saveAssembled == 0) {
		saveAssembled = 1;
	}
	
	
	
	
	/*
	 *	Other stuff
	 */
	npowderHits = 0;
	npowderBlanks = 0;
	nhits = 0;
	nrecenthits = 0;
	nprocessedframes = 0;
	nrecentprocessedframes = 0;
	lastclock = clock()-10;
	datarate = 1;
	detectorZ = 0;
	detectorEncoderValue = 0;
	runNumber = getRunNumber();
	time(&tstart);
	avgGMD = 0;
	bgCounter = 0;
	last_bg_update = 0;
	hotpixCounter = 0;
	last_hotpix_update = 0;
	hotpixRecalc = bgRecalc;
	nhot = 0;
	detectorZprevious = 0;	
	
	time(&tlast);
	lastTimingFrame=0;

	// Make sure to use SLAC timezone!
	setenv("TZ","US/Pacific",1);
	
}




/*
 *	Parse command line arguments 
 */
void cGlobal::parseCommandLineArguments(int argc, char **argv) {
	
	// No arguments specified = ask for help
	if (argc == 1) {
		printf("No configuration file spcified\n");
		printf("\t--> using default settings\n");
		return;
	}
	
	// First argument is always the configuration file
	parseConfigFile(argv[1]);
	
	// Other arguments are optional switches but take same syntax prefixed by an '-'
	if (argc > 2) {
		for (long i=2; i<argc; i++) {
			if (argv[i][0] == '-' && i+1 < argc) {
				parseConfigTag(argv[i]+1, argv[++i]);
			}
		}
	}
}




/*
 *	Read and process configuration file
 */
void cGlobal::parseConfigFile(char* filename) {
	char		cbuf[cbufsize];
	char		tag[cbufsize];
	char		value[cbufsize];
	char		*cp;
	FILE		*fp;
	
	
	/*
	 *	Open configuration file for reading
	 */
	printf("Parsing input configuration file: %s\n",filename);
	printf("\t%s\n",filename);
	
	fp = fopen(filename,"r");
	if (fp == NULL) {
		printf("\tCould not open configuration file \"%s\"\n",filename);
		printf("\tUsing default values\n");
		return;
	}
	
	/*
	 *	Loop through configuration file until EOF 
	 *	Ignore lines beginning with a '#' (comments)
	 *	Split each line into tag and value at the '=' sign
	 */
	while (feof(fp) == 0) {
		
		cp = fgets(cbuf, cbufsize, fp);
		if (cp == NULL) 
			break;
		
		if (cbuf[0] == '#')
			continue;
		
		cp = strpbrk(cbuf, "=");
		if (cp == NULL)
			continue;
		
		*(cp) = '\0';
		sscanf(cp+1,"%s",value);
		sscanf(cbuf,"%s",tag);
		
		parseConfigTag(tag, value);
	}
	
	fclose(fp);
	
}

/*
 *	Process tags for both configuration file and command line options
 */
void cGlobal::parseConfigTag(char *tag, char *value) {
	
	/*
	 *	Convert to lowercase
	 */
	for(int i=0; i<strlen(tag); i++) 
		tag[i] = tolower(tag[i]);
	
	/*
	 *	Parse known tags
	 */
	if (!strcmp(tag, "defaultphotonenergyev")) {
		defaultPhotonEnergyeV = atof(value);
	} 
	else if (!strcmp(tag, "defaultcameralengthmm")) {
		defaultCameraLengthMm = atof(value);
	}
	else if (!strcmp(tag, "detectorzname")) {
		strcpy(detectorZpvname, value);
	}
	else if (!strcmp(tag, "cameralengthoffset")) {
		cameraLengthOffset = atof(value);
	}
	else if (!strcmp(tag, "cameraLengthScale")) {
		cameraLengthScale  = atof(value);
	}
	else if (!strcmp(tag, "detectortype")) {
		strcpy(detector[0].detectorTypeName, value);
	}
	else if (!strcmp(tag, "detectorname")) {
		strcpy(detector[0].detectorName, value);
	}
	else if (!strcmp(tag, "startatframe")) {
		startAtFrame = atoi(value);
	}
	else if (!strcmp(tag, "stopatframe")) {
		stopAtFrame = atoi(value);
	}
	else if (!strcmp(tag, "nthreads")) {
		nThreads = atoi(value);
	}
	else if (!strcmp(tag, "usehelperthreads")) {
		useHelperThreads = atoi(value);
	}
	else if (!strcmp(tag, "iospeedtest")) {
		ioSpeedTest = atoi(value);
	}
	else if (!strcmp(tag, "threadpurge")) {
		threadPurge = atoi(value);
	}
	else if (!strcmp(tag, "geometry")) {
		strcpy(detector[0].geometryFile, value);
	}
	else if (!strcmp(tag, "darkcal")) {
		strcpy(detector[0].darkcalFile, value);
	}
	else if (!strcmp(tag, "gaincal")) {
		strcpy(detector[0].gaincalFile, value);
	}
	else if (!strcmp(tag, "peakmask")) {
		strcpy(peaksearchFile, value);
	}
	else if (!strcmp(tag, "badpixelmap")) {
		strcpy(detector[0].badpixelFile, value);
	}
	// Processing options
	else if (!strcmp(tag, "subtractcmmodule")) {
		printf("The keyword subtractcmModule has been changed. It is\n"
		       "now known as cmModule.\n"
		       "Modify your ini file and try again...\n");
		exit(1);
	}
	else if (!strcmp(tag, "cmmodule")) {
		cmModule = atoi(value);
	}
	else if (!strcmp(tag, "subtractunbondedpixels")) {
		cmSubtractUnbondedPixels = atoi(value);
	}
	else if (!strcmp(tag, "wiremaskfile")) {
		strcpy(detector[0].wireMaskFile, value);
	}
	else if (!strcmp(tag, "subtractbehindwires")) {
		cmSubtractBehindWires = atoi(value);
	}
	else if (!strcmp(tag, "usegaincal")) {
		useGaincal = atoi(value);
	}
	else if (!strcmp(tag, "invertgain")) {
		invertGain = atoi(value);
	}
	else if (!strcmp(tag, "generatedarkcal")) {
		generateDarkcal = atoi(value);
	}
	else if (!strcmp(tag, "generategaincal")) {
		generateGaincal = atoi(value);
	}
	else if (!strcmp(tag, "subtractbg")) {
		printf("The keyword subtractBg has been changed.  It is\n"
             "now known as useDarkcalSubtraction.\n"
             "Modify your ini file and try again...\n");
		exit(1);
	}
	else if (!strcmp(tag, "usebadpixelmap")) {
		useBadPixelMask = atoi(value);
	}
	else if (!strcmp(tag, "usedarkcalsubtraction")) {
		useDarkcalSubtraction = atoi(value);
	}
	else if (!strcmp(tag, "hitfinder")) {
		hitfinder = atoi(value);
	}
	else if (!strcmp(tag, "savehits")) {
		savehits = atoi(value);
	}
	else if (!strcmp(tag, "savepeakinfo")) {
		savePeakInfo = atoi(value);
	}
	else if (!strcmp(tag, "powdersum")) {
		printf("The keyword powdersum has been changed.  It is\n"
		       "now known as powderSumHits and powderSumBlanks.\n"
		       "Modify your ini file and try again...\n");
		exit(1);
    }
	else if (!strcmp(tag, "saveraw")) {
		saveRaw = atoi(value);
	}
	else if (!strcmp(tag, "saveassembled")) {
		saveAssembled = atoi(value);
	}
	else if (!strcmp(tag, "savedetectorcorrectedonly")) {
		saveDetectorCorrectedOnly = atoi(value);
	}
	else if (!strcmp(tag, "savedetectorraw")) {
		saveDetectorRaw = atoi(value);
	}
	else if (!strcmp(tag, "hdf5dump")) {
		hdf5dump = atoi(value);
	}
	else if (!strcmp(tag, "saveinterval")) {
		saveInterval = atoi(value);
	}
	else if (!strcmp(tag, "useautohotpixel")) {
		useAutoHotpixel = atoi(value);
	}
	else if (!strcmp(tag, "masksaturatedpixels")) {
		maskSaturatedPixels = atoi(value);
	}
	else if (!strcmp(tag, "pixelsaturationadc")) {
		pixelSaturationADC = atoi(value);
	}
	else if (!strcmp(tag, "useselfdarkcal")) {
		printf("The keyword useSelfDarkcal has been changed.  It is\n"
             "now known as useSubtractPersistentBackground.\n"
             "Modify your ini file and try again...\n");
		exit(1);
	}
	else if (!strcmp(tag, "usesubtractpersistentbackground")) {
		useSubtractPersistentBackground = atoi(value);
	}
	else if (!strcmp(tag, "usebackgroundbuffermutex")) {
		useBackgroundBufferMutex = atoi(value);
	}
	
	// Local background subtraction
	else if (!strcmp(tag, "uselocalbackgroundsubtraction")) {
		useLocalBackgroundSubtraction = atoi(value);
	}
	else if (!strcmp(tag, "localbackgroundradius")) {
		localBackgroundRadius = atoi(value);
	}
	
	//TOF
	else if (!strcmp(tag, "tofname")) {
		strcpy(tofName, value);
	}	
	else if (!strcmp(tag, "tofchannel")) {
		TOFchannel = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderusetof")) {
		hitfinderUseTOF = atoi(value);
	}
	else if (!strcmp(tag, "hitfindertofminsample")) {
		hitfinderTOFMinSample = atoi(value);
	}
	else if (!strcmp(tag, "hitfindertofmaxsample")) {
		hitfinderTOFMaxSample = atoi(value);
	}
	else if (!strcmp(tag, "hitfindertofthresh")) {
		hitfinderTOFThresh = atof(value);
	}	
	
	
	// Radial average stacks
	else if (!strcmp(tag, "saveradialstacks")) {
		saveRadialStacks = atoi(value);
	}
	else if (!strcmp(tag, "radialstacksize")) {
		radialStackSize = atoi(value);
	}

	// Power user settings
	else if (!strcmp(tag, "cmfloor")) {
		cmFloor = atof(value);
	}
	else if (!strcmp(tag, "pixelsize")) {
		detector[0].pixelSize = atof(value);
	}
	else if (!strcmp(tag, "debuglevel")) {
		debugLevel = atoi(value);
	}
	else if (!strcmp(tag, "hotpixfreq")) {
		hotpixFreq = atof(value);
	}
	else if (!strcmp(tag, "hotpixadc")) {
		hotpixADC = atoi(value);
	}
	else if (!strcmp(tag, "hotpixmemory")) {
		hotpixMemory = atoi(value);
	}
	else if (!strcmp(tag, "powderthresh")) {
		powderthresh = atoi(value);
	}
	else if (!strcmp(tag, "powdersumhits")) {
		powderSumHits = atoi(value);
	}
	else if (!strcmp(tag, "powdersumblanks")) {
		powderSumBlanks = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderadc")) {
		hitfinderADC = atoi(value);
	}
	else if (!strcmp(tag, "hitfindernat")) {
		hitfinderNAT = atoi(value);
	}
	else if (!strcmp(tag, "hitfindertit")) {
		hitfinderTAT = atof(value);
	}
	else if (!strcmp(tag, "hitfindercheckgradient")) {
		hitfinderCheckGradient = atoi(value);
	}
	else if (!strcmp(tag, "hitfindermingradient")) {
		hitfinderMinGradient = atof(value);
	}
	else if (!strcmp(tag, "hitfindercluster")) {
		hitfinderCluster = atoi(value);
	}
	else if (!strcmp(tag, "hitfindernpeaks")) {
		hitfinderNpeaks = atoi(value);
	}
	else if (!strcmp(tag, "hitfindernpeaksmax")) {
		hitfinderNpeaksMax = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderalgorithm")) {
		hitfinderAlgorithm = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderminpixcount")) {
		hitfinderMinPixCount = atoi(value);
	}
	else if (!strcmp(tag, "hitfindermaxpixcount")) {
		hitfinderMaxPixCount = atoi(value);
	}
	else if (!strcmp(tag, "hitfindercheckpeakseparation")) {
		hitfinderCheckPeakSeparation = atoi(value);
	}
	else if (!strcmp(tag, "hitfindermaxpeakseparation")) {
		hitfinderMaxPeakSeparation = atof(value);
	}
	else if (!strcmp(tag, "hitfindersubtractlocalbg")) {
		hitfinderSubtractLocalBG = atoi(value);
	}	
	else if (!strcmp(tag, "hitfinderlocalbgradius")) {
		hitfinderLocalBGRadius = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderlocalbgthickness")) {
		hitfinderLocalBGThickness = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderlimitres")) {
		hitfinderLimitRes = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderminres")) {
		hitfinderMinRes = atof(value);
	}
	else if (!strcmp(tag, "hitfindermaxres")) {
		hitfinderMaxRes = atof(value);
	}
	else if (!strcmp(tag, "hitfinderusepeakmask")) {
		hitfinderUsePeakmask = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderminsnr")) {
		hitfinderMinSNR = atof(value);
	}
	// Backgrounds
	else if (!strcmp(tag, "selfdarkmemory")) {
		printf("The keyword selfDarkMemory has been changed.  It is\n"
             "now known as bgMemory.\n"
             "Modify your ini file and try again...\n");
		exit(1);
	}
	else if (!strcmp(tag, "bgmemory")) {
		bgMemory = atoi(value);
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
		exit(1);
	}
	else if (!strcmp(tag, "fudgeevr41")) {
		fudgeevr41 = atoi(value);
	}
	else if (!strcmp(tag, "startframes")) {
		startFrames = atoi(value);
	}
	
	
	
	// Unknown tags
	else {
		printf("\tUnknown tag: %s = %s\n",tag,value);
		printf("Aborting...\n");
		exit(1);
	}
}





/*
 *	Write initial log file
 */
void cGlobal::writeInitialLog(void){
	FILE *fp;
	
	
	// Start time
	char	timestr[1024];
	time_t	rawtime;
	tm		*timeinfo;
	time(&rawtime);
	timeinfo=localtime(&rawtime);
	strftime(timestr,80,"%c",timeinfo);
	
	
	
	// Logfile name
	printf("Writing log file: %s\n", logfile);

	fp = fopen(logfile,"w");
	if(fp == NULL) {
		printf("Error: Can not open %s for writing\n",logfile);
		printf("Aborting...");
		exit(1);
	}
	
	fprintf(fp, "start time: %s\n",timestr);

	fprintf(fp,"\n\n");
	fprintf(fp, ">-------- Start of ini params --------<\n");
	fprintf(fp, "defaultPhotonEnergyeV=%f\n",defaultPhotonEnergyeV);
	fprintf(fp, "defaultCameraLengthMm=%f\n",defaultCameraLengthMm);
	fprintf(fp, "detectorType=%s\n",detector[0].detectorTypeName);
	fprintf(fp, "detectorName=%s\n",detector[0].detectorName);
	fprintf(fp, "startAtFrame=%d\n",startAtFrame);
	fprintf(fp, "stopAtFrame=%d\n",stopAtFrame);
	fprintf(fp, "nThreads=%ld\n",nThreads);
	fprintf(fp, "useHelperThreads=%d\n",useHelperThreads);
	fprintf(fp, "ioSpeedTest=%d\n",ioSpeedTest);
	fprintf(fp, "threadPurge=%d\n",threadPurge);
	fprintf(fp, "geometry=%s\n",detector[0].geometryFile);
	fprintf(fp, "darkcal=%s\n",detector[0].darkcalFile);
	fprintf(fp, "gaincal=%s\n",detector[0].gaincalFile);
	fprintf(fp, "peakmask=%s\n",peaksearchFile);
	fprintf(fp, "badPixelMap=%s\n",detector[0].badpixelFile);
	fprintf(fp, "subtractcmModule=%d\n",cmModule);
	fprintf(fp, "cmModule=%d\n",cmModule);
	fprintf(fp, "subtractUnbondedPixels=%d\n",cmSubtractUnbondedPixels);
	fprintf(fp, "wiremaskFile=%s\n",detector[0].wireMaskFile);
	fprintf(fp, "subtractBehindWires=%d\n",cmSubtractBehindWires);
	fprintf(fp, "useGaincal=%d\n",useGaincal);
	fprintf(fp, "invertGain=%d\n",invertGain);
	fprintf(fp, "generateDarkcal=%d\n",generateDarkcal);
	fprintf(fp, "generateGaincal=%d\n",generateGaincal);
	fprintf(fp, "useBadPixelMap=%d\n",useBadPixelMask);
	fprintf(fp, "useDarkcalSubtraction=%d\n",useDarkcalSubtraction);
	fprintf(fp, "hitfinder=%d\n",hitfinder);
	fprintf(fp, "saveHits=%d\n",savehits);
	fprintf(fp, "savePeakInfo=%d\n",savePeakInfo);
	fprintf(fp, "saveRaw=%d\n",saveRaw);
	fprintf(fp, "saveAssembled=%d\n",saveAssembled);
	fprintf(fp, "saveDetectorCorrectedOnly=%d\n",saveDetectorCorrectedOnly);
	fprintf(fp, "saveDetectorRaw=%d\n",saveDetectorRaw);
	fprintf(fp, "hdf5dump=%d\n",hdf5dump);
	fprintf(fp, "saveInterval=%d\n",saveInterval);
	fprintf(fp, "useAutoHotPixel=%d\n",useAutoHotpixel);
	fprintf(fp, "maskSaturatedPixels=%d\n",maskSaturatedPixels);
	fprintf(fp, "pixelSaturationADC=%ld\n",pixelSaturationADC);
	fprintf(fp, "maskSaturatedPixels=%d\n",maskSaturatedPixels);
	fprintf(fp, "pixelSaturationADC=%d\n",pixelSaturationADC);
	fprintf(fp, "useSubtractPersistentBackground=%d\n",useSubtractPersistentBackground);
	fprintf(fp, "useBackgroundBufferMutex=%d\n",useBackgroundBufferMutex);
	fprintf(fp, "useLocalBackgroundSubtraction=%d\n",useLocalBackgroundSubtraction);
	fprintf(fp, "localBackgroundRadius=%ld\n",localBackgroundRadius);
	fprintf(fp, "tofName=%s\n",tofName);
	fprintf(fp, "tofChannel=%d\n",TOFchannel);
	fprintf(fp, "hitfinderUseTOF=%d\n",hitfinderUseTOF);
	fprintf(fp, "hitfinderTOFMinSample=%d\n",hitfinderTOFMinSample);
	fprintf(fp, "hitfinderTOFMaxSample=%d\n",hitfinderTOFMaxSample);
	fprintf(fp, "hitfinderTOFThresh=%f\n",hitfinderTOFThresh);
	fprintf(fp, "saveRadialStacks=%d\n",saveRadialStacks);
	fprintf(fp, "radialStackSize=%d\n",radialStackSize);
	fprintf(fp, "cmFloor=%f\n",cmFloor);
	fprintf(fp, "pixelSize=%f\n",detector[0].pixelSize);
	fprintf(fp, "debugLevel=%d\n",debugLevel);
	fprintf(fp, "hotpixFreq=%f\n",hotpixFreq);
	fprintf(fp, "hotpixADC=%d\n",hotpixADC);
	fprintf(fp, "hotpixMemory=%d\n",hotpixMemory);
	fprintf(fp, "powderThresh=%d\n",powderthresh);
	fprintf(fp, "powderSumHits=%d\n",powderSumHits);
	fprintf(fp, "powderSumBlanks=%d\n",powderSumBlanks);
	fprintf(fp, "hitfinderADC=%d\n",hitfinderADC);
	fprintf(fp, "hitfinderNAT=%ld\n",hitfinderNAT);
	fprintf(fp, "hitfinderTIT=%f\n",hitfinderTAT);
	fprintf(fp, "hitfinderCheckGradient=%d\n",hitfinderCheckGradient);
	fprintf(fp, "hitfinderMinGradient=%f\n",hitfinderMinGradient);
	fprintf(fp, "hitfinderCluster=%d\n",hitfinderCluster);
	fprintf(fp, "hitfinderNPeaks=%d\n",hitfinderNpeaks);
	fprintf(fp, "hitfinderNPeaksMax=%d\n",hitfinderNpeaksMax);
	fprintf(fp, "hitfinderAlgorithm=%d\n",hitfinderAlgorithm);
	fprintf(fp, "hitfinderMinPixCount=%d\n",hitfinderMinPixCount);
	fprintf(fp, "hitfinderMaxPixCount=%d\n",hitfinderMaxPixCount);
	fprintf(fp, "hitfinderCheckPeakSeparation=%d\n",hitfinderCheckPeakSeparation);
	fprintf(fp, "hitfinderMaxPeakSeparation=%f\n",hitfinderMaxPeakSeparation);
	fprintf(fp, "hitfinderSubtractLocalBG=%d\n",hitfinderSubtractLocalBG);
	fprintf(fp, "hitfinderLocalBGRadius=%d\n",hitfinderLocalBGRadius);
	fprintf(fp, "hitfinderLocalBGThickness=%d\n",hitfinderLocalBGThickness);
	fprintf(fp, "hitfinderLimitRes=%d\n",hitfinderLimitRes);
	fprintf(fp, "hitfinderMinRes=%f\n",hitfinderMinRes);
	fprintf(fp, "hitfinderMaxRes=%f\n",hitfinderMaxRes);
	fprintf(fp, "hitfinderUsePeakMask=%d\n",hitfinderUsePeakmask);
	fprintf(fp, "hitfinderMinSNR=%f\n",hitfinderMinSNR);
	fprintf(fp, "selfdarkMemory=%li\n",bgMemory);
	fprintf(fp, "bgMemory=%li\n",bgMemory);
	fprintf(fp, "bgRecalc=%ld\n",bgRecalc);
	fprintf(fp, "bgMedian=%f\n",bgMedian);
	fprintf(fp, "bgIncludeHits=%d\n",bgIncludeHits);
	fprintf(fp, "bgNoBeamReset=%d\n",bgNoBeamReset);
	fprintf(fp, "bgFiducialGlitchReset=%d\n",bgFiducialGlitchReset);
	fprintf(fp, "scaleBackground=%d\n",scaleBackground);
	//fprintf(fp, "scaleDarkcal=%d\n",scaleBackground);
	fprintf(fp, "startFrames=%d\n",startFrames);
	fprintf(fp, ">-------- End of ini params --------<\n");
	fprintf(fp, "\n\n");	
	
	fprintf(fp, ">-------- Start of job --------<\n");
	fclose (fp);
	
	
	// Open a new frame file at the same time
	pthread_mutex_lock(&framefp_mutex);
	
	sprintf(framefile,"frames.txt");
	framefp = fopen (framefile,"w");
	if(framefp == NULL) {
		printf("Error: Can not open %s for writing\n",framefile);
		printf("Aborting...");
		exit(1);
	}

	
	fprintf(framefp, "# EventName, hit, UnixTime, FrameNumber, npeaks, nPixels, totalIntensity, peakResolution, peakDensity, photonEnergyeV, gmd1, gmd2, detectorZ, EVR41, laserDelay\n");

	
	sprintf(cleanedfile,"cleaned.txt");
	cleanedfp = fopen (cleanedfile,"w");
	if(cleanedfp == NULL) {
		printf("Error: Can not open %s for writing\n",cleanedfile);
		printf("Aborting...");
		exit(1);
	}
	fprintf(cleanedfp, "# Filename, npeaks, nPixels, totalIntensity, peakResolution, peakDensity\n");	
	pthread_mutex_unlock(&framefp_mutex);
	
	
	pthread_mutex_lock(&peaksfp_mutex);
	sprintf(peaksfile,"peaks.txt");
	peaksfp = fopen (peaksfile,"w");
	if(peaksfp == NULL) {
		printf("Error: Can not open %s for writing\n",peaksfile);
		printf("Aborting...");
		exit(1);
	}
	if(savePeakInfo==0) {
		fprintf(peaksfp, "savePeakInfo has been turned off in the config file.\n");
	}
	pthread_mutex_unlock(&peaksfp_mutex);
	
}


/*
 *	Update log file
 */
void cGlobal::updateLogfile(void){
	FILE *fp;
	
	// Calculate overall hit rate
	float hitrate;
	hitrate = 100.*( nhits / (float) nprocessedframes);
	
	// Calculate recent hit rate
	float recenthitrate=0;
	if(nrecentprocessedframes != 0)
		recenthitrate = 100.*( nrecenthits / (float) nrecentprocessedframes);

	
	// Elapsed processing time
	double	dtime;
	int		hrs, mins, secs; 
	time(&tend);
	dtime = difftime(tend,tstart);
	hrs = (int) floor(dtime / 3600);
	mins = (int) floor((dtime-3600*hrs)/60);
	secs = (int) floor(dtime-3600*hrs-60*mins);
	
	// Average data rate
	float	fps;
	fps = nprocessedframes / dtime;
	
	
	// Update logfile
	printf("Writing log file: %s\n", logfile);
	fp = fopen (logfile,"a");
	fprintf(fp, "nFrames: %li,  nHits: %li (%2.2f%%), recentHits: %li (%2.2f%%), wallTime: %ihr %imin %isec (%2.1f fps)\n", nprocessedframes, nhits, hitrate, nrecenthits, recenthitrate, hrs, mins, secs, fps);
	fclose (fp);
	
	nrecenthits = 0;
	nrecentprocessedframes = 0;

	
	// Flush frame file buffer
	pthread_mutex_lock(&framefp_mutex);
	fflush(framefp);
	fflush(cleanedfp);
	pthread_mutex_unlock(&framefp_mutex);

	pthread_mutex_lock(&powderfp_mutex);
	for(long i=0; i<nPowderClasses; i++) {
		fflush(powderlogfp[i]);
	}
	pthread_mutex_unlock(&powderfp_mutex);
	
	pthread_mutex_lock(&peaksfp_mutex);
	fflush(peaksfp);
	pthread_mutex_unlock(&peaksfp_mutex);
	
	
}

/*
 *	Write final log file
 */
void cGlobal::writeFinalLog(void){

	
	FILE *fp;
	
	// Logfile name
	printf("Writing log file: %s\n", logfile);
	fp = fopen (logfile,"a");

	
	// Calculate hit rate
	float hitrate;
	hitrate = 100.*( nhits / (float) nprocessedframes);
	

	// End time
	char	timestr[1024];
	time_t	rawtime;
	tm		*timeinfo;
	time(&rawtime);
	timeinfo=localtime(&rawtime);
	strftime(timestr,80,"%c",timeinfo);
	
	
	// Elapsed processing time
	double	dtime;
	int		hrs, mins, secs; 
	time(&tend);
	dtime = difftime(tend,tstart);
	hrs = (int) floor(dtime / 3600);
	mins = (int) floor((dtime-3600*hrs)/60);
	secs = (int) floor(dtime-3600*hrs-60*mins);
	

	// Average data rate
	float	fps, mbs;
	fps = nprocessedframes / dtime;
	mbs = fps*detector[0].pix_nn*nDetectors*sizeof(uint16_t);
	mbs /= (1024.*1024.);
				 
				 
	
	// Save log file
	fprintf(fp, ">-------- End of job --------<\n");
	fprintf(fp, "End time: %s\n",timestr);
	fprintf(fp, "Elapsed time: %ihr %imin %isec\n",hrs,mins,secs);
	fprintf(fp, "Frames processed: %li\n",nprocessedframes);
	fprintf(fp, "nFrames in powder patterns:\n");
	for(long i=0; i<nPowderClasses; i++) {
		fprintf(fp, "\tclass%ld: %li\n", i, nPowderFrames[i]);
	}
	fprintf(fp, "Number of hits: %li\n",nhits);
	fprintf(fp, "Average hit rate: %2.2f %%\n",hitrate);
	fprintf(fp, "Average frame rate: %2.2f fps\n",fps);
	fprintf(fp, "Average data rate: %2.2f MB/sec\n",mbs);
	fprintf(fp, "Average photon energy: %7.2f	eV\n",meanPhotonEnergyeV);
	fprintf(fp, "Photon energy sigma: %5.2f eV\n",photonEnergyeVSigma);

	fclose (fp);

	
	// Close frame buffers
	pthread_mutex_lock(&framefp_mutex);
	fclose(framefp);
	fclose(cleanedfp);
	pthread_mutex_unlock(&framefp_mutex);
	
	pthread_mutex_lock(&powderfp_mutex);
	for(long i=0; i<nPowderClasses; i++) {
		fclose(powderlogfp[i]);
	}
	pthread_mutex_unlock(&powderfp_mutex);
	
	pthread_mutex_lock(&peaksfp_mutex);
	fclose(peaksfp);
	pthread_mutex_unlock(&peaksfp_mutex);
	
	
}

