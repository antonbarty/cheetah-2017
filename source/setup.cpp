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
#include <hdf5.h>
#include <fenv.h>
#include <stdlib.h>

#include "setup.h"
#include "worker.h"
#include "data2d.h"


/*
 *	Default settings/configuration
 */
void cGlobal::defaultConfiguration(void) {

	// ini file to use
	strcpy(configFile, "cheetah.ini");

	// Default experiment info (in case beamline data is missing...)
	defaultPhotonEnergyeV = 0; 	
	
	// Detector info
	strcpy(detectorTypeName, "cspad");
	strcpy(detectorName, "CxiDs1");
	detectorType = Pds::DetInfo::Cspad;
	detectorPdsDetInfo = Pds::DetInfo::CxiDs1;

	// Statistics 
	summedPhotonEnergyeV = 0;
	meanPhotonEnergyeV = 0;	
    
    // Detector Z position
	strcpy(detectorZpvname, "CXI:DS1:MMS:06.RBV");
	defaultCameraLengthMm = 0;
	detposprev = 0;
    cameraLengthOffset = 500.0 + 79.0;
    cameraLengthScale = 1e-3;

	
	// Start and stop frames
	startAtFrame = 0;
	stopAtFrame = 0;


	// Geometry
	strcpy(geometryFile, "No_file_specified");
	pixelSize = 110e-6;
    
	
	// Bad pixel mask
	strcpy(badpixelFile, "No_file_specified");
	useBadPixelMask = 0;

	// Saturated pixels
	maskSaturatedPixels = 0;
	pixelSaturationADC = 15564;  // 95% of 2^14 ??

	// Static dark calibration (electronic offsets)
	strcpy(darkcalFile, "No_file_specified");
	useDarkcalSubtraction = 0;
	generateDarkcal = 0;
	
	// Common mode subtraction from each ASIC
	strcpy(wireMaskFile, "No_file_specified");
	cmModule = 0;
	cmFloor = 0.1;
	cmSubtractUnbondedPixels = 0;
	cmSubtractBehindWires = 0;


	// Gain calibration correction
	strcpy(gaincalFile, "No_file_specified");
	useGaincal = 0;
	invertGain = 0;
	generateGaincal = 0;
	
	// Subtraction of running background (persistent photon background) 
	useSubtractPersistentBackground = 0;
	//subtractBg = 0;
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
    asic_nx = CSPAD_ASIC_NX;
    asic_ny = CSPAD_ASIC_NY;
    asic_nn = asic_nx * asic_ny;
    nasics_x = CSPAD_nASICS_X;
    nasics_y = CSPAD_nASICS_Y;

	
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
	if(!strcmp(detectorTypeName, "cspad"))
		detectorType = Pds::DetInfo::Cspad;
	else if(!strcmp(detectorTypeName, "pnccd")) {
		detectorType = Pds::DetInfo::Cspad;
	}
	else {
		printf("Error: unknown detector type %s\n", detectorTypeName);
		printf("Quitting\n");
		exit(1);
	}
	
	/*
	 *	Determine detector address
	 *	A list of addresses can be found in:
	 *		release/pdsdata/xtc/Detinfo.hh
	 *		release/pdsdata/xtc/src/Detinfo.cc
	 */
	if(!strcmp(detectorName, "CxiDs1")) {
		detectorType = Pds::DetInfo::Cspad;
		detectorPdsDetInfo = Pds::DetInfo::CxiDs1;
	}
	else if (!strcmp(detectorName, "CxiDs2")) {
		detectorType = Pds::DetInfo::Cspad;
		detectorPdsDetInfo = Pds::DetInfo::CxiDs2;
	}
	else if (!strcmp(detectorName, "CxiDsd")) {
		detectorType = Pds::DetInfo::Cspad;
		detectorPdsDetInfo = Pds::DetInfo::CxiDsd;
	}
	else if (!strcmp(detectorName, "XppGon")) {
		detectorType = Pds::DetInfo::Cspad;
		detectorPdsDetInfo = Pds::DetInfo::XppGon;
	}
	else {
		printf("Error: unknown detector %s\n", detectorName);
		printf("Quitting\n");
		exit(1);
	}

	
	/*
	 *	Detector parameters
	 */
	switch(detectorType) {
		case Pds::DetInfo::Cspad : 
			asic_nx = CSPAD_ASIC_NX;
			asic_ny = CSPAD_ASIC_NY;
			asic_nn = asic_nx * asic_ny;
			nasics_x = CSPAD_nASICS_X;
			nasics_y = CSPAD_nASICS_Y;
			break;
			
		default:
			printf("Error: unknown detector %s\n", detectorName);
			printf("Quitting\n");
			exit(1);
			break;
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
	selfdark = (float*) calloc(pix_nn, sizeof(float));
	bg_buffer = (int16_t*) calloc(bgMemory*pix_nn, sizeof(int16_t)); 
	hotpix_buffer = (int16_t*) calloc(hotpixMemory*pix_nn, sizeof(int16_t)); 
	hotpixelmask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	wiremask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0; i<pix_nn; i++) {
		selfdark[i] = 0;
		hotpixelmask[i] = 1;
		wiremask[i] = 1;
	}

	/*
     *  Set up arrays for powder classes and radial stacks
     */
	for(long i=0; i<nPowderClasses; i++) {
		nPowderFrames[i] = 0;
		powderRaw[i] = (double*) calloc(pix_nn, sizeof(double));
		powderRawSquared[i] = (double*) calloc(pix_nn, sizeof(double));
		powderAssembled[i] = (double*) calloc(image_nn, sizeof(double));

        radialStackCounter[i] = 0;
        radialAverageStack[i] = (float *) calloc(radial_nn*radialStackSize, sizeof(float));
        
		pthread_mutex_init(&powderRaw_mutex[i], NULL);
		pthread_mutex_init(&powderRawSquared_mutex[i], NULL);
		pthread_mutex_init(&powderAssembled_mutex[i], NULL);
        pthread_mutex_init(&radialStack_mutex[i], NULL);
		
		for(long j=0; j<pix_nn; j++) {
			powderRaw[i][j] = 0;
			powderRawSquared[i][j] = 0;
		}
		for(long j=0; j<image_nn; j++) {
			powderAssembled[i][j] = 0;
		}
        
        for(long j=0; j<radial_nn*radialStackSize; j++) {
            radialAverageStack[i][j] = 0;
        }

		char	filename[1024];
		sprintf(filename,"r%04u-class%ld-sumLog.txt",runNumber,i);
		powderlogfp[i] = fopen(filename, "w");
        
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
		//subtractBg = 0;
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
		//subtractBg = 0;
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
		strcpy(detectorTypeName, value);
	}
	else if (!strcmp(tag, "detectorname")) {
		strcpy(detectorName, value);
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
		strcpy(geometryFile, value);
	}
	else if (!strcmp(tag, "darkcal")) {
		strcpy(darkcalFile, value);
	}
	else if (!strcmp(tag, "gaincal")) {
		strcpy(gaincalFile, value);
	}
	else if (!strcmp(tag, "peakmask")) {
		strcpy(peaksearchFile, value);
	}
	else if (!strcmp(tag, "badpixelmap")) {
		strcpy(badpixelFile, value);
	}
	// Processing options
	else if (!strcmp(tag, "subtractcmmodule")) {
		cmModule = atoi(value);
	}
	else if (!strcmp(tag, "cmmodule")) {
		cmModule = atoi(value);
	}
	else if (!strcmp(tag, "subtractunbondedpixels")) {
		cmSubtractUnbondedPixels = atoi(value);
	}
	else if (!strcmp(tag, "wiremaskfile")) {
		strcpy(wireMaskFile, value);
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

	//	subtractBg = atoi(value);
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
	//	useSubtractPersistentBackground = atoi(value);
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
		pixelSize = atof(value);
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
		bgMemory = atoi(value);
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
	//	scaleBackground = atoi(value);
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
 *	Read in detector configuration
 */
void cGlobal::readDetectorGeometry(char* filename) {
	

	// Pixel size (measurements in geometry file are in m)
	module_rows = asic_nx;
	module_cols = asic_ny;	
	pix_dx = pixelSize;

	
	// Set filename here 
	printf("Reading detector configuration:\n");
	printf("\t%s\n",filename);
	
	
	// Check whether pixel map file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("Error: Detector configuration file does not exist: %s\n",filename);
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
		exit(1);
	}
	

	// Sanity check that size matches what we expect for cspad (!)
	if (detector_x.nx != 8*asic_nx || detector_x.ny != 8*asic_ny) {
		printf("readDetectorGeometry: array size mismatch\n");
		printf("%ux%u != %lix%li\n", 8*asic_nx, 8*asic_ny, detector_x.nx, detector_x.ny);
		exit(1);
	}
	
	
	// Create local arrays for detector pixel locations
	long	nx = 8*asic_nx;
	long	ny = 8*asic_ny;
	long	nn = nx*ny;
	long 	i;
	pix_nx = nx;
	pix_ny = ny;
	pix_nn = nn;
	pix_x = (float *) calloc(nn, sizeof(float));
	pix_y = (float *) calloc(nn, sizeof(float));
	pix_z = (float *) calloc(nn, sizeof(float));
	pix_kx = (float *) calloc(nn, sizeof(float));
	pix_ky = (float *) calloc(nn, sizeof(float));
	pix_kz = (float *) calloc(nn, sizeof(float));
	pix_kr = (float *) calloc(nn, sizeof(float));
	pix_res = (float *) calloc(nn, sizeof(float));
	hitfinderResMask = (int *) calloc(nn, sizeof(int)); // is there a better place for this?
	for (i=0;i<nn;i++) hitfinderResMask[i]=1;
	printf("\tPixel map is %li x %li pixel array\n",nx,ny);
	
	
	// Copy values from 2D array
	for(long i=0;i<nn;i++){
		pix_x[i] = (float) detector_x.data[i];
		pix_y[i] = (float) detector_y.data[i];
		pix_z[i] = (float) detector_z.data[i];
	}
	
	
	// Divide array (in m) by pixel size to get pixel location indicies (ijk)
	for(i=0;i<nn;i++){
		pix_x[i] /= pix_dx;
		pix_y[i] /= pix_dx;
		pix_z[i] /= pix_dx;
	}
	
	
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
	//xmax = ceil(xmax);
	//xmin = floor(xmin);
	//ymax = ceil(ymax);
	//ymin = floor(ymin);

	fesetround(1);
	xmax = lrint(xmax);
	xmin = lrint(xmin);
	ymax = lrint(ymax);
	ymin = lrint(ymin);
	printf("\tImage bounds:\n");
	printf("\tx range %f to %f\n",xmin,xmax);
	printf("\ty range %f to %f\n",ymin,ymax);
	
	
	// How big must the output image be?
	float max = xmax;
	if(ymax > max) max = ymax;
	if(fabs(xmin) > max) max = fabs(xmin);
	if(fabs(ymin) > max) max = fabs(ymin);
	image_nx = 2*(unsigned)max;
	image_nn = image_nx*image_nx;
	printf("\tImage output array will be %li x %li\n",image_nx,image_nx);
	
	// Compute radial distances
	pix_r = (float *) calloc(nn, sizeof(float));
	radial_max = 0.0;
	for(long i=0;i<nn;i++){
		pix_r[i] = sqrt(pix_x[i]*pix_x[i]+pix_y[i]*pix_y[i]);
		if(pix_r[i] > radial_max)
			radial_max = pix_r[i];
	}	
	radial_nn = (long int) ceil(radial_max)+1;
}

/*
 *  Update K-space variables
 *  (called whenever detector has moved)
 */
void cGlobal::updateKspace(float wavelengthA) {
    long i;
    float   x, y, z, r;
    float   kx,ky,kz,kr;
    float   res;

    printf("Recalculating K-space coordinates\n");

    for ( i=0; i<pix_nn; i++ ) {
        x = pix_x[i]*pixelSize;
        y = pix_y[i]*pixelSize;
        z = pix_z[i]*pixelSize + detectorZ*cameraLengthScale;
        
        r = sqrt(x*x + y*y + z*z);
        kx = x/r/wavelengthA;
        ky = y/r/wavelengthA;
        kz = (z/r - 1)/wavelengthA;                 // assuming incident beam is along +z direction
        kr = sqrt(kx*kx + ky*ky + kz*kz);
        res = 1/kr;
        
        pix_kx[i] = kx;
        pix_ky[i] = ky;
        pix_kz[i] = kz;
        pix_kr[i] = kr;
        pix_res[i] = res;
        
        
        // Check whether resolution limits still make sense.
        if ( hitfinderLimitRes == 1 ) {
            if ( ( res < hitfinderMinRes ) && (res > hitfinderMaxRes) ) {
                hitfinderResMask[i] = 1;
            } 
            else {
                hitfinderResMask[i] = 0;
            }
        }
    }
}


/*
 *	Read in darkcal file
 */
void cGlobal::readDarkcal(char *filename){	
	
	// Create memory space and pad with zeros
	darkcal = (int32_t*) calloc(pix_nn, sizeof(int32_t));
	memset(darkcal,0, pix_nn*sizeof(int32_t));

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
		darkcal[i] = (int32_t) temp2d.data[i];
	
}


/*
 *	Read in gaincal file
 */
void cGlobal::readGaincal(char *filename){
	

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
void cGlobal::readPeakmask(char *filename){
	

	
	// Create memory space and default to searching for peaks everywhere
	peakmask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0;i<pix_nn;i++)
		peakmask[i] = 1;
	
	// Do we even need a peakmask file?
	if ( hitfinderUsePeakmask == 0 ){
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
	
	
	// Copy into darkcal array
	for(long i=0;i<pix_nn;i++)
		peakmask[i] = (int16_t) temp2d.data[i];
}


/*
 *	Read in bad pixel mask
 */
void cGlobal::readBadpixelMask(char *filename){
	
	
	// Create memory space and default to searching for peaks everywhere
	badpixelmask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0;i<pix_nn;i++)
		badpixelmask[i] = 1;
	
	
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
	for(long i=0;i<pix_nn;i++)
		badpixelmask[i] = (int16_t) temp2d.data[i];
}

/*
 *	Read in wire mask
 */
void cGlobal::readWireMask(char *filename){
	

	// Create memory space and default to searching for peaks everywhere
	wiremask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0;i<pix_nn;i++)
		wiremask[i] = 1;
	
	// Do we need this file?
	if ( cmSubtractBehindWires == 0 ){
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
	for(long i=0;i<pix_nn;i++)
		wiremask[i] = (int16_t) temp2d.data[i];
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
	fprintf(fp, "detectorType=%s\n",detectorTypeName);
	fprintf(fp, "detectorName=%s\n",detectorName);
	fprintf(fp, "startAtFrame=%d\n",startAtFrame);
	fprintf(fp, "stopAtFrame=%d\n",stopAtFrame);
	fprintf(fp, "nThreads=%ld\n",nThreads);
	fprintf(fp, "useHelperThreads=%d\n",useHelperThreads);
	fprintf(fp, "ioSpeedTest=%d\n",ioSpeedTest);
	fprintf(fp, "threadPurge=%d\n",threadPurge);
	fprintf(fp, "geometry=%s\n",geometryFile);
	fprintf(fp, "darkcal=%s\n",darkcalFile);
	fprintf(fp, "gaincal=%s\n",gaincalFile);
	fprintf(fp, "peakmask=%s\n",peaksearchFile);
	fprintf(fp, "badPixelMap=%s\n",badpixelFile);
	fprintf(fp, "subtractcmModule=%d\n",cmModule);
	fprintf(fp, "cmModule=%d\n",cmModule);
	fprintf(fp, "subtractUnbondedPixels=%d\n",cmSubtractUnbondedPixels);
	fprintf(fp, "wiremaskFile=%s\n",wireMaskFile);
	fprintf(fp, "subtractBehindWires=%d\n",cmSubtractBehindWires);
	fprintf(fp, "useGaincal=%d\n",useGaincal);
	fprintf(fp, "invertGain=%d\n",invertGain);
	fprintf(fp, "generateDarkcal=%d\n",generateDarkcal);
	fprintf(fp, "generateGaincal=%d\n",generateGaincal);
	//fprintf(fp, "subtractBg=%d\n",subtractBg);
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
	//fprintf(fp, "useSelfDarkcal=%d\n",useSubtractPersistentBackground);
	fprintf(fp, "useSubtractPersistentBackground=%d\n",useSubtractPersistentBackground);
	fprintf(fp, "useLocalBackgroundSubtraction=%d\n",useLocalBackgroundSubtraction);
	fprintf(fp, "localBackgroundRadius=%ld\n",localBackgroundRadius);
	fprintf(fp, "tofName=%s\n",tofName);
	fprintf(fp, "tofChannel=%d\n",TOFchannel);
	fprintf(fp, "hitfinderUseTOF=%d\n",hitfinderUseTOF);
	fprintf(fp, "hitfinderTOFMinSample=%d\n",hitfinderTOFMinSample);
	fprintf(fp, "hitfinderTOFMaxSample=%d\n",hitfinderTOFMaxSample);
	fprintf(fp, "hitfinderTOFThresh=%f\n",hitfinderTOFThresh);
	fprintf(fp, "cmFloor=%f\n",cmFloor);
	fprintf(fp, "pixelSize=%f\n",pixelSize);
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
	fprintf(framefp, "# FrameNumber, UnixTime, EventName, npeaks, nPixels, totalIntensity, peakResolution, peakDensity, hit, photonEnergyeV, EVR41\n");
	

	
	sprintf(cleanedfile,"cleaned.txt");
	cleanedfp = fopen (cleanedfile,"w");
	if(cleanedfp == NULL) {
		printf("Error: Can not open %s for writing\n",cleanedfile);
		printf("Aborting...");
		exit(1);
	}
	fprintf(cleanedfp, "# Filename, npeaks,, nPixels, totalIntensity, peakResolution, peakDensity\n");	
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
	mbs = fps*pix_nn*sizeof(uint16_t);
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

