/*
 *  setup.cpp
 *  cheetah
 *
 *  created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <limits>
#include <hdf5.h>
#include <fenv.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream> 
#include <errno.h>

#include "data2d.h"
#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "tofDetector.h"

/*
 *	Default settings/configuration
 */
cGlobal::cGlobal(void) {

	// ini file to use
	strcpy(configFile, "cheetah.ini");
	strcpy(configOutFile, "cheetah.out");

	// Default experiment info (in case beamline data is missing...)
	defaultPhotonEnergyeV = 0;

	// Detector info
	nDetectors = 0;
	for(long i=0; i<MAX_DETECTORS; i++) {
		//strcpy(detector[i].detectorConfigFile, "No_file_specified");
		strcpy(detector[i].configGroup,"none");
		detector[i].detectorID = i;
	}

	// Statistics
	summedPhotonEnergyeV = 0;
	meanPhotonEnergyeV = 0;
	datarateWorker = 1.;
	datarateWorkerMemory = 0.95;
	datarateWorkerSkipCounter = 0;
	lastTimingFrame = 0;

	// Pv values
	strcpy(pumpLaserDelayPV, "LAS:FS5:Angle:Shift:Ramp:rd");
	pumpLaserDelay = std::numeric_limits<float>::quiet_NaN();
	pumpLaserDelay = 0;
	samplePosXPV[0] = 0;
	samplePosYPV[0] = 0;
	samplePosZPV[0] = 0;
	sampleVoltage[0] = 0;

	// Misc. PV values
	nEpicsPvFloatValues = 0;

	// Start and stop frames
	startAtFrame = 0;
	stopAtFrame = 0;
	skipFract = 0.;

	// Calibrations
	generateDarkcal = 0;
	generateGaincal = 0;

	// Hitfinding
	hitfinder = 0;
	hitfinderForInitials = 1;
    hitfinderInvertHit = 0;
	hitfinderDetectorID = 0;
	hitfinderADC = 100;
	hitfinderTAT = 1e3;

	hitfinderNpeaks = 50;
	hitfinderNpeaksMax = 100000;
	hitfinderAlgorithm = 8;
	hitfinderMinPixCount = 3;
	// hitfinderMaxPixCount is a new feature. For backwards compatibility it should be neutral by default, therefore hitfinderMaxPixCount = 0
	hitfinderMaxPixCount = 0;
	hitfinderUsePeakmask = 0;
	hitfinderCheckGradient = 0;
	hitfinderMinGradient = 0;
	strcpy(peaksearchFile, "No_file_specified");
	savePeakInfo = 0;
	hitfinderCheckPeakSeparation = 0;
	hitfinderMinPeakSeparation = 0;
	hitfinderSubtractLocalBG = 0;
	hitfinderLocalBGRadius = 4;
    hitfinderLocalBGThickness = 4;
	hitfinderMinRes = 0;
	hitfinderMaxRes = 1e6;
	hitfinderResolutionUnitPixel = 1;
	hitfinderMinSNR = 40;
	hitfinderIgnoreHaloPixels = 0;
	hitfinderDownsampling = 0;
	hitfinderOnDetectorCorrectedData = 0;
	hitfinderFastScan = 0;

	// Sorting (eg: pump laser on/off)
	sortPumpLaserOn = 0;
    strcpy(pumpLaserScheme,"evr41");
    

	// TOF (Aqiris)
	hitfinderUseTOF = 0;
	hitfinderTOFMinCount = 0;
    hitfinderTOFMaxCount = 1000;
	hitfinderTOFWindow = 3;

	// TOF configuration
	TOFPresent = 0;
	
	// FEE spectrum
	useFEEspectrum = 0;
	FEEspectrumStackSize = 200000;
	FEEspectrumWidth = 1024;

	
    // CXI downstream energy spectrum default configuration
    espectrum = 0;
    espectrum1D = 0;
	espectrumTiltAng = 0;
	espectrumLength = 1080;
	espectrumWidth = 900;
	espectrumDarkSubtract = 0;
	espectrumSpreadeV = 40;
	espectrumStackSize = 10000;
	strcpy(espectrumDarkFile, "No_file_specified");
	strcpy(espectrumScaleFile, "No_file_specified");

	// Powder pattern generation
	nPowderClasses = 2;
	usePowderThresh = 0;
	powderthresh = 0.0;
	powderSumHits = 1;
	powderSumBlanks = 0;
	powderSumWithBackgroundSubtraction = 1;
	assemblePowders = 0;

	// Radial average stacks
	radialStackSize=10000;

	// Assemble options
	assembleInterpolation = ASSEMBLE_INTERPOLATION_DEFAULT;
    assemble2DImage = 0;
    assemble2DMask = 0;

	// Saving options
	saveHits = 0;
	saveBlanks = 0;
	saveAssembled = 1;
	saveNonAssembled = 0;
	saveRadialAverage = 1;
	h5compress = 5;
	hdf5dump = 0;
	saveInterval = 1000;
	savePixelmask = 1;
    // Do not output 1 HDF5 per image by default
	saveCXI = 0;
	// Flush after every image by default
	cxiFlushPeriod = 1;
	// Save data in modular stack (see CXI version 1.4)
	saveModular=0;

	strcpy(dataSaveFormat, "INT16");

	// Do not use SWMR mode by default
	cxiSWMR = 0;

	// Visualization
	pythonFile[0] = 0;

	// Peak lists
	savePeakList = 1;

	// Verbosity
	// !!! Why such a high verbosity by default? /Max
	debugLevel = 2;

	// I/O speed test?
	ioSpeedTest = 0;

	// Default to only a few threads
	nThreads = 16;
	// depreciated?
	useHelperThreads = 0;
	// depreciated?
	threadPurge = 10000;

	anaModThreads = 32;
	
	// Saving to subdirectories
	subdirFileCount = -1;
	subdirNumber = 0;
	strcpy(subdirName, "");


	// Log files
	strcpy(logfile, "log.txt");
	strcpy(framefile, "frames.txt");
	strcpy(cleanedfile, "cleaned.txt");
	strcpy(peaksfile, "peaks.txt");

	// Fudge EVR41 (modify EVR41 according to the Acqiris trace)...
	fudgeevr41 = 0; // this means no fudge by default
	lasttime = 0;

	// By default do not profile code
	profilerDiagnostics = false;

	// Only one thread during calibration
	useSingleThreadCalibration = 0;
	strcpy(cxiFilename, "");


}




/*
 * Setup stuff to do with thread management, settings, etc.
 */
void cGlobal::setup() {

	/*
	 *  Init timing
	 */
	gettimeofday(&datarateWorkerTimevalLast,NULL);
  

	/*
	 *	Configure detectors
	 */
	hitfinderDetIndex = -1;
	for(long detIndex=0; detIndex < nDetectors; detIndex++){
		detector[detIndex].configure();
		detector[detIndex].readDetectorGeometry(detector[detIndex].geometryFile);
		detector[detIndex].readDarkcal(detector[detIndex].darkcalFile);
		detector[detIndex].readGaincal(detector[detIndex].gaincalFile);
		detector[detIndex].pixelmask_shared = (uint16_t*) calloc(detector[detIndex].pix_nn,sizeof(uint16_t));
		detector[detIndex].pixelmask_shared_max = (uint16_t*) calloc(detector[detIndex].pix_nn,sizeof(uint16_t));
		detector[detIndex].pixelmask_shared_min = (uint16_t*) malloc(detector[detIndex].pix_nn*sizeof(uint16_t));
		for(long j=0; j<detector[detIndex].pix_nn; j++){
			detector[detIndex].pixelmask_shared_min[j] = PIXEL_IS_ALL;
		}
		detector[detIndex].readPeakmask(self, peaksearchFile);
		detector[detIndex].readBadpixelMask(detector[detIndex].badpixelFile);
		detector[detIndex].readBaddataMask(detector[detIndex].baddataFile);
		detector[detIndex].readWireMask(detector[detIndex].wireMaskFile);
		if (detector[detIndex].detectorID == hitfinderDetectorID){
			hitfinderDetIndex = detIndex;
		}
	}

	/*
	 * Configure corrections
	 */
	for (int verIndex=0; verIndex < nVersions; verIndex++){
		
	}

	// Check whether the detector chosen for hitfinding is configured
	if ((hitfinderDetIndex == -1) && (hitfinderAlgorithm != 0)) {
		printf("ERROR: hitfinderDetectorID is not listed among the configured detectors:\n");
		for(long detIndex=0; detIndex < nDetectors; detIndex++){
			printf("\t%s with detectorID=%li configured.\n",detector[detIndex].detectorName,detector[detIndex].detectorID);
		}
		printf("hitfinderDetectorID=%i\n", hitfinderDetectorID);
		printf("This doesn't make sense.\n");
		printf("in void cGlobal::setup()\n");
		printf("Quitting...\n");
		exit(1);
	}
	
	// Read hits from list if used as hitfinder
	if (hitfinder == 1 && hitfinderAlgorithm == 12) {
		readHits(hitlistFile);
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
     *  Pump laser logic
     *  Search for 'Pump laser logic' to find all places in which code needs to be changed to implement a new schema
     */
	if(sortPumpLaserOn) {
        if(strcmp(pumpLaserScheme, "evr41") == 0) {
            nPowderClasses *= 2;
        }
        else if(strcmp(pumpLaserScheme, "LD57") == 0) {
            nPowderClasses = 6;
        }
        else {
            printf("Error: Unknown pump laser scheme\n");
            printf("pumpLaserScheme = %s\n", pumpLaserScheme);
            printf("Known schemes are: evr41, LD57\n");
            exit(1);
        }
    }
    
	
	for(int i = 0; i<nPowderClasses; i++){
		nPeaksMin[i] = 1000000000;
		nPeaksMax[i] = 0;
	}

	/*
	 * Set up thread management
	 */
	nActiveThreads = 0;
	threadCounter = 0;
	pthread_mutex_init(&hitclass_mutex, NULL);
	pthread_mutex_init(&process_mutex, NULL);
	pthread_mutex_init(&nActiveThreads_mutex, NULL);
	pthread_mutex_init(&hotpixel_mutex, NULL);
	pthread_mutex_init(&halopixel_mutex, NULL);
	pthread_mutex_init(&selfdark_mutex, NULL);
	pthread_mutex_init(&bgbuffer_mutex, NULL);
	pthread_mutex_init(&nhits_mutex, NULL);
	pthread_mutex_init(&framefp_mutex, NULL);
	pthread_mutex_init(&powderfp_mutex, NULL);
	pthread_mutex_init(&peaksfp_mutex, NULL);
	pthread_mutex_init(&subdir_mutex, NULL);
	pthread_mutex_init(&nespechits_mutex, NULL);
	pthread_mutex_init(&espectrumRun_mutex, NULL);
	pthread_mutex_init(&espectrumBuffer_mutex, NULL);
	pthread_mutex_init(&datarateWorker_mutex, NULL);  
	pthread_mutex_init(&saveCXI_mutex, NULL);  
	pthread_mutex_init(&pixelmask_shared_mutex, NULL);  
	threadID = (pthread_t*) calloc(nThreads, sizeof(pthread_t));
	pthread_mutex_init(&gmd_mutex, NULL);  

	// Set number of frames for initial calibrations
	nInitFrames = 0;
	long temp;
	for (long detIndex=0; detIndex<MAX_DETECTORS; detIndex++){
		temp = detector[detIndex].startFrames;
		nInitFrames = std::max(nInitFrames,temp);
		detector[detIndex].halopixCalibrated = 0;
		detector[detIndex].hotpixCalibrated = 0;
		detector[detIndex].bgCalibrated = 0;
	}
	calibrated = 0;

	/*
	 * Trap specific configurations and mutually incompatible options
	 */
	if(generateDarkcal) {

		printf("******************************************************************\n");
		printf("keyword generatedarkcal set: this overrides some keyword values!!!\n");
		printf("******************************************************************\n");

		hitfinder = 0;
		saveHits = 0;
		saveBlanks = 0;
		hdf5dump = 0;
		saveNonAssembled = 0;
		nInitFrames = 0;
		hitfinderFastScan = 0;
		powderSumHits = 0;
		powderSumBlanks = 0;
		powderthresh = -30000;
		for(long i=0; i<nDetectors; i++) {
			detector[i].cmModule = 0;
			detector[i].cspadSubtractUnbondedPixels = 0;
			detector[i].useDarkcalSubtraction = 0;
			detector[i].useGaincal=0;
			detector[i].useAutoHotpixel = 0;
			detector[i].useSubtractPersistentBackground = 0;
			detector[i].useLocalBackgroundSubtraction = 0;
			detector[i].startFrames = 0;
			detector[i].saveDetectorRaw = 1;
			detector[i].saveDetectorCorrectedOnly = 0;
		}
	}

	if(generateGaincal) {

		printf("******************************************************************\n");
		printf("keyword generategaincal set: this overrides some keyword values!!!\n");
		printf("******************************************************************\n");

		hitfinder = 0;
        hitfinderFastScan = 0;
		saveHits = 0;
		saveBlanks = 0;
		hdf5dump = 0;
		saveNonAssembled = 0;

		nInitFrames = 0;

		powderSumHits = 0;
		powderSumBlanks = 0;
		powderthresh = -30000;
		for(long i=0; i<nDetectors; i++) {
			detector[i].cmModule = 0;
			detector[i].cspadSubtractUnbondedPixels = 0;
			detector[i].useDarkcalSubtraction = 1;
			detector[i].useAutoHotpixel = 0;
			detector[i].useSubtractPersistentBackground = 0;
            detector[i].useLocalBackgroundSubtraction = 0;
			detector[i].useGaincal=0;
			detector[i].startFrames = 0;
			detector[i].saveDetectorRaw = 1;
			detector[i].saveDetectorCorrectedOnly = 1;
		}
	}

	// Make sure to save something...
	// !!! I would rather like to throw an error here instead of silently changing the configuration. /Max
	if(saveNonAssembled==0 && saveAssembled == 0) {
		saveAssembled = 1;
	}

	/* Only save peak info for certain hitfinders */
	if (( hitfinderAlgorithm == 3 ) ||
		( hitfinderAlgorithm == 5 ) ||
		( hitfinderAlgorithm == 6 ) ||
		( hitfinderAlgorithm == 8 ))
		savePeakInfo = 1; 

    
    
    
	/*
	 * Other stuff
	 */
	npowderHits = 0;
	npowderBlanks = 0;
	nhits = 0;
	nrecenthits = 0;
	nespechits = 0;
	nprocessedframes = 0;
	nrecentprocessedframes = 0;
	lastclock = clock()-10;
	datarate = 1;
	runNumber = 0;
	time(&tstart);
	avgGMD = 0;

	for(long i=0; i<MAX_DETECTORS; i++) {
		detector[i].bgCounter = 0;
		detector[i].bgLastUpdate = 0;
		detector[i].hotpixCounter = 0;
		detector[i].hotpixLastUpdate = 0;
		detector[i].hotpixRecalc = detector[i].bgRecalc;
		detector[i].nhot = 0;
		detector[i].halopixCounter = 0;
		detector[i].halopixLastUpdate = 0;
		detector[i].halopixRecalc = detector[i].bgRecalc;
		detector[i].halopixMemory = detector[i].bgRecalc;
		detector[i].nhalo = 0;
		detector[i].detectorZprevious = 0;
		detector[i].detectorZ = 0;
		detector[i].detectorEncoderValue = 0;
	}  

	// Make sure to use SLAC timezone!
	setenv("TZ","US/Pacific",1);

	/*
	 * Set up arrays for hot pixels, running backround, etc.
	 */
	for(long i=0; i<nDetectors; i++) {
		detector[i].allocatePowderMemory(self);
	}

	
	/*
	 *	Energy spectrum stuff
	 */
	// Set up array for run integrated energy spectrum
	espectrumRun = (double *) calloc(espectrumLength, sizeof(double));
	for(long i=0; i<espectrumLength; i++) {
		espectrumRun[i] = 0;
	}
	// Set up buffer array for calculation of energy spectrum background
	espectrumBuffer = (double *) calloc(espectrumLength*espectrumWidth, sizeof(double));
	for(long i=0; i<espectrumLength*espectrumWidth; i++) {
		espectrumBuffer[i] = 0;
	}
	// Set up Darkcal array for holding energy spectrum background
	espectrumDarkcal = (double *) calloc(espectrumLength*espectrumWidth, sizeof(double));
	for(long i=0; i<espectrumLength*espectrumWidth; i++) {
		espectrumDarkcal[i] = 0;
	}
	//Set up array for holding energy spectrum scale
	espectrumScale = (double *) calloc(espectrumLength, sizeof(double));
	for(long i=0; i<espectrumLength; i++) {
		espectrumScale[i] = 0;
	}
	readSpectrumDarkcal(self, espectrumDarkFile);
	readSpectrumEnergyScale(self, espectrumScaleFile);
	
	/*
	 *	Energy spectrum stacks
	 */
	if (espectrum) {
		printf("Allocating spectral stacks\n");
		int spectrumLength = espectrumLength;
		
		for(long i=0; i<nPowderClasses; i++) {
			espectrumStackCounter[i] = 0;
			espectrumStack[i] = (float *) calloc(espectrumStackSize*spectrumLength, sizeof(float));
			for(long j=0; j<espectrumStackSize*spectrumLength; j++) {
				espectrumStack[i][j] = 0;
			}
		}
		printf("Spectral stack allocated\n");
	}
	
	if (useFEEspectrum) {
		printf("Allocating FEE spectrum stacks\n");
		for(long i=0; i<nPowderClasses; i++) {
			FEEspectrumStackCounter[i] = 0;
			FEEspectrumStack[i] = (float *) calloc(FEEspectrumStackSize*FEEspectrumWidth, sizeof(float));
		}
	}

	for(long i=0; i<nPowderClasses; i++) {
		pthread_mutex_init(&espectrumStack_mutex[i], NULL);
		pthread_mutex_init(&FEEspectrumStack_mutex[i], NULL);
	}


	/*
	 * Set up arrays for powder classes and radial stacks
	 * Currently only tracked for detector[0]  (generalise this later)
	 */
    pthread_mutex_lock(&powderfp_mutex);
	for(long i=0; i<nPowderClasses; i++) {
		char  filename[1024];
		powderlogfp[i] = NULL;
		FEElogfp[i] = NULL;
		if(runNumber > 0) {
			sprintf(filename,"r%04u-class%ld-log.txt",runNumber,i);
			powderlogfp[i] = fopen(filename, "w");
            sprintf(filename,"r%04u-FEEspectrum-class%ld-index.txt",runNumber,i);
			powderlogfp[i] = fopen(filename, "w");
		}
	}
    pthread_mutex_unlock(&powderfp_mutex);


}

void cGlobal::freeMutexes(void) {
	pthread_mutex_unlock(&hitclass_mutex);
	pthread_mutex_unlock(&process_mutex);
	pthread_mutex_unlock(&nActiveThreads_mutex);
	pthread_mutex_unlock(&hotpixel_mutex);
	pthread_mutex_unlock(&halopixel_mutex);
	pthread_mutex_unlock(&selfdark_mutex);
	pthread_mutex_unlock(&bgbuffer_mutex);
	pthread_mutex_unlock(&nhits_mutex);
	pthread_mutex_unlock(&framefp_mutex);
	pthread_mutex_unlock(&powderfp_mutex);
	pthread_mutex_unlock(&peaksfp_mutex);
	pthread_mutex_unlock(&subdir_mutex);
	pthread_mutex_unlock(&nespechits_mutex);
	pthread_mutex_unlock(&espectrumRun_mutex);
	pthread_mutex_unlock(&espectrumBuffer_mutex);
	pthread_mutex_unlock(&datarateWorker_mutex);
	pthread_mutex_unlock(&saveCXI_mutex);
	pthread_mutex_unlock(&pixelmask_shared_mutex);
	
	for(long i=0; i<nDetectors; i++) {
		for(long j=0; j<nPowderClasses; j++) {
			pthread_mutex_unlock(&detector[i].powderRaw_mutex[j]);
			pthread_mutex_unlock(&detector[i].powderRawSquared_mutex[j]);
			pthread_mutex_unlock(&detector[i].powderCorrected_mutex[j]);
			pthread_mutex_unlock(&detector[i].powderCorrectedSquared_mutex[j]);
			pthread_mutex_unlock(&detector[i].powderAssembled_mutex[j]);
			pthread_mutex_unlock(&detector[i].radialStack_mutex[j]);
			pthread_mutex_unlock(&detector[i].correctedMin_mutex[j]);
			pthread_mutex_unlock(&detector[i].correctedMax_mutex[j]);
			pthread_mutex_unlock(&detector[i].assembledMin_mutex[j]);
			pthread_mutex_unlock(&detector[i].assembledMax_mutex[j]);
			pthread_mutex_unlock(&detector[i].radialStack_mutex[j]);
		}
		pthread_mutex_unlock(&detector[i].histogram_mutex);
	}

	nCXIEvents = 0;
	nCXIHits = 0;
	for(long i=0; i<nPowderClasses; i++) {
		pthread_mutex_unlock(&espectrumStack_mutex[i]);
		pthread_mutex_unlock(&FEEspectrumStack_mutex[i]);
	}
}




/*
 * Parse command line arguments
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
				parseConfigTag(argv[i]+1, argv[i+1]);
				i++;
			}
		}
	}
}


/*
 *	Read and process configuration file
 */
void cGlobal::parseConfigFile(char* filename) {
	
	char  cbuf[cbufsize] = "";
	char  tag[cbufsize] = "";
	char  value[cbufsize] = "";
	char  group[cbufsize] = "";
	char  groupPrepend[cbufsize] = "";
	char  ts[cbufsize] = "";
	char  *cp;
	FILE  *fp;
	int cnt,fail;
	int exitCheetah = 0;

	/*
	 * Open configuration file for reading
	 */
	printf("Parsing cheetah configuration file: %s\n",filename);
	printf("\t%s\n",filename);

	fp = fopen(filename,"r");
	if (fp == NULL) {
		printf("\tCould not open configuration file \"%s\"\n",filename);
		printf("\tExiting\n");
		exit(1);
	}

	/*
	 * Loop through configuration file until EOF
	 * Ignore lines beginning with a '#' (comments)
	 * Split each line into tag and value at the '=' sign
	 */	

	printf("\t------ Start cheetah configuration keywords ------\n");
	while (feof(fp) == 0) {

		fail = 0;

		cp = fgets(cbuf, cbufsize, fp);
		if (cp == NULL)
			break;

		/* strip whitespace */
		cnt=0;
		for (uint i=0; i<cbufsize; i++){
			if (cbuf[i] == ' ') continue;
			cbuf[cnt] = cbuf[i];
			cnt++;
		}

		/* strip comments */
		for (uint i=0; i<cbufsize-1; i++){
			if (cbuf[i] == '#'){
				cbuf[i] = '\n';
				cbuf[i+1] = '\0';
				break;
			}
		}


		/* skip empty lines */
		if ( strlen(cbuf) <= 1) continue;

		/* Print our for sanity */
		printf("\t%s",cbuf);

		
		/* check for string prepend */
		cp = strrchr(cbuf,']');
		if (cp != NULL){
			*(cp) = '\0';
			strncpy(groupPrepend,cbuf+1,cbufsize);
			groupPrepend[cbufsize-1] = '\0';
			if (strlen(groupPrepend) != 0) 
				strcat(groupPrepend,"/");
			continue;
		}

		/* prepend string */
		if (strcmp(groupPrepend, "")) {
			strncpy(ts,groupPrepend,cbufsize);
			strcat(ts,cbuf);
			strncpy(cbuf,ts,cbufsize);
		}
	
		/* get the value */
		cp = strpbrk(cbuf, "=");
		if (cp == NULL)
			continue;
		*(cp) = '\0';
		sscanf(cp+1,"%s",value);
		
		/* get the tag and group */
		cp = strrchr(cbuf,'/');
		if (cp == NULL){
			sscanf(cbuf,"%s",tag);
			sscanf("default","%s",group);
		} else {
			*(cp) = '\0';
			sscanf(cp+1,"%s",tag);
			sscanf(cbuf,"%s",group);
		}

		//printf("group=%s, tag=%s, value=%s\n",group,tag,value);

		/* Try to set global configuration */
		fail = parseConfigTag(tag, value);
		
		/* Not a global keyword?  Then it must be detector-specific. */
		if (fail != 0 && strcmp(group,"default") != 0){

			int matched=0;

			/* First check if group is an existing  detector */

			/* Check if it's an existing pixel detector */
			for (long i=0; i<nDetectors && !matched; i++){
				if (!strcmp(group,detector[i].configGroup)){
					matched = 1;
					fail = detector[i].parseConfigTag(tag,value);
					break;
				}

			}
			/* Check if it's an existing TOF detector */
			for (long i=0; i<nTOFDetectors && !matched; i++){
				if (strcmp(group,tofDetector[i].configGroup) == 0){
					matched = 1;
					fail = tofDetector[i].parseConfigTag(tag,value);
					break;
				}
	
			}

			/* If it didn't match try to create a new group */

			/* Check if it's a new TOF detector */
			/* Only consider a new TOF detector if the group starts with "detectortype = tof" */
			if(!matched && strcasecmp(tag,"detectortype") == 0 && 
			   strcasecmp(value,"tof") == 0 &&
			   nTOFDetectors < MAX_TOF_DETECTORS){
				/* new TOF detector*/
				strncpy(tofDetector[nTOFDetectors].configGroup,group,cbufsize);
				matched = 1;
				fail = tofDetector[nTOFDetectors].parseConfigTag(tag,value);
				nTOFDetectors++;
			}

			if(!matched && nDetectors < MAX_DETECTORS){
				/* new pixel Detector */
				strncpy(detector[nDetectors].configGroup,group,cbufsize);
				matched = 1;
				fail = detector[nDetectors].parseConfigTag(tag,value);
				nDetectors++;
			}
				
			if (matched == 0){
				printf("ERROR: Only %i detectors allowed at this time... fix your config file.\n",MAX_DETECTORS);
				exit(1);
			}

		}

		if (fail != 0){
			printf("ERROR: The keyword %s is not recognized.\n",tag);
			exitCheetah = 1;
		}

	}
	printf("\t------ End cheetah configuration keywords ------\n");


	if (exitCheetah != 0){
		printf("ERROR: Exiting Cheetah due to unknown configuration keywords.\n");
		exit(1);
	}

	printf("Configured %d detectors\n",nDetectors);
	for (long i=0;i<nDetectors;i++){
		printf("detector %li: %s\n",i,detector[i].configGroup);
	}
	printf("Configured %d TOF detectors\n",nTOFDetectors);
	for (long i=0;i<nTOFDetectors;i++){
		TOFPresent = 1;
		printf("TOF detector %li: Group - %s Source - %s Channel - %d\n",i,
			   tofDetector[i].configGroup,tofDetector[i].sourceIdentifier,
			   tofDetector[i].channel);
	}
	fclose(fp);
}


/*
 * Process tags for both configuration file and command line options
 */
int cGlobal::parseConfigTag(char *tag, char *value) {

	int fail = 0;

	/*
	 * Convert to lowercase
	 */
	for(uint i=0; i<strlen(tag); i++)
		tag[i] = tolower(tag[i]);

	/*
	 * Parse known tags
	 */
	if (!strcmp(tag, "defaultphotonenergyev")) {
		defaultPhotonEnergyeV = atof(value);
	}
	else if (!strcmp(tag, "fixedphotonenergyev")){
		fixedPhotonEnergyeV = atof(value);
	}
	else if (!strcmp(tag, "saveepicspvfloat")) {
		strcpy(&epicsPvFloatAddresses[nEpicsPvFloatValues][0],value);
		nEpicsPvFloatValues++;
	}
	else if (!strcmp(tag, "startatframe")) {
		startAtFrame = atoi(value);
	}
	else if (!strcmp(tag, "skipfract")) {
		skipFract = atof(value);
	}
	else if (!strcmp(tag, "stopatframe")) {
		stopAtFrame = atoi(value);
	}
	else if (!strcmp(tag, "nthreads")) {
		nThreads = atoi(value);
	}
	else if (!strcmp(tag, "anamodthreads")) {
		anaModThreads = atoi(value);
	}
	else if (!strcmp(tag, "usehelperthreads")) {
		useHelperThreads = atoi(value);
	}
	else if (!strcmp(tag, "iospeedtest")) {
		ioSpeedTest = atoi(value);
	}
	else if (!strcmp(tag, "profilerdiagnostics")) {
		profilerDiagnostics = atoi(value);
	}
	else if (!strcmp(tag, "threadpurge")) {
		threadPurge = atoi(value);
	}
	else if (!strcmp(tag, "peakmask")) {
		strcpy(peaksearchFile, value);
		hitfinderUsePeakmask = 1;
	}
	else if (!strcmp(tag, "hitlist")) {
		strcpy(hitlistFile, value);
	}
	// Processing options
	else if (!strcmp(tag, "subtractcmmodule")) {
		printf("The keyword subtractcmModule is depreciated.\n"
			   "Modify your ini file and try again...\n");
		fail = 1;
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
		fail = 1;
	}
	else if (!strcmp(tag, "hitfinder")) {
		hitfinder = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderforinitials")) {
		hitfinderForInitials = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderinverthit")) {
		hitfinderInvertHit = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderdetector")) {
		printf("The keyword hitfinderDetector is depreciated.\n"
			   "Please specify the detector that shall be used for hitfinding by defining hitfinderDetectorID instead.\n"
			   "For choosing the front detector set hitfinderDetectorID=0, for choosing the back detector set hitfinderDetectorID=1.\n"
			   "Modify your ini file and try again...\n");
		fail = 1;
	}
	else if (!strcmp(tag, "hitfinderdetectorid")) {
		hitfinderDetectorID = atoi(value);
	}
	else if (!strcmp(tag, "savehits")) {
		saveHits = atoi(value);
	}
	else if (!strcmp(tag, "saveblanks")) {
		saveBlanks = atoi(value);
	}
	else if (!strcmp(tag, "powdersum")) {
		printf("The keyword powdersum has been changed.  It is\n"
			   "now known as powderSumHits and powderSumBlanks.\n"
			   "Modify your ini file and try again...\n");
		fail = 1;
	}
	else if ((!strcmp(tag, "saveraw")) || (!strcmp(tag, "savenonassembled"))) {
		saveNonAssembled = atoi(value);
	}
	else if (!strcmp(tag, "savemodular")) {
		saveModular = atoi(value);
	}
	else if (!strcmp(tag, "saveassembled")) {
		saveAssembled = atoi(value);
		assemble2DImage = saveAssembled;
		assemble2DMask = saveAssembled;
	}
	else if (!strcmp(tag, "assembleinterpolation")) {
		assembleInterpolation = atoi(value);
	}
	else if (!strcmp(tag, "saveassembled")) {
		saveRadialAverage = atoi(value);
	}
	else if (!strcmp(tag, "savepixelmask")) {
		savePixelmask = atoi(value);
	}
	else if (!strcmp(tag, "h5compress")) {
		h5compress = atoi(value);
	}
	else if (!strcmp(tag, "hdf5dump")) {
		hdf5dump = atoi(value);
	}
	else if (!strcmp(tag, "saveinterval")) {
		saveInterval = atoi(value);
	}
	// Time-of-flight
	else if (!strcmp(tag, "hitfinderusetof")) {
		hitfinderUseTOF = atoi(value);
	}
	else if (!strcmp(tag, "hitfindertofmincount")) {
		hitfinderTOFMinCount = atoi(value);
	}
    else if (!strcmp(tag, "hitfindertofmaxcount")) {
		hitfinderTOFMaxCount = atoi(value);
	}
	else if (!strcmp(tag, "hitfindertofwindow")) {
		hitfinderTOFWindow = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderignorehalopixels")) {
		hitfinderIgnoreHaloPixels = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderondetectorcorrecteddata")) {
		hitfinderOnDetectorCorrectedData = atoi(value);
	}


	// Sorting
	else if (!strcmp(tag, "sortpumplaseron")) {
		sortPumpLaserOn = atoi(value);
	}
    else if (!strcmp(tag, "pumplaserscheme")) {
		strcpy(pumpLaserScheme, value);
	}

	

	// Energy spectrum parameters
	else if (!strcmp(tag, "usefeespectrum")) {
		useFEEspectrum = atoi(value);
	}
	else if (!strcmp(tag, "espectrum")) {
		espectrum = atoi(value);
	}
	else if (!strcmp(tag, "espectrum1d")) {
		espectrum1D = atoi(value);
	}
	else if (!strcmp(tag, "espectrumtiltang")) {
		espectrumTiltAng = atoi(value);
	}
	else if (!strcmp(tag, "espectrumlength")) {
		espectrumLength = atoi(value);
	}
	else if (!strcmp(tag, "espectrumdarksubtract")) {
		espectrumDarkSubtract = atoi(value);
	}
	else if (!strcmp(tag, "espectrumspreadev")) {
		espectrumSpreadeV = atoi(value);
	}
	else if (!strcmp(tag, "espectrumdarkfile")) {
		strcpy(espectrumDarkFile, value);
	}
	else if (!strcmp(tag, "espectrumscalefile")) {
		strcpy(espectrumScaleFile, value);
	}

	// Radial average stacks
	else if ((!strcmp(tag, "saveradialstacks")) || (!strcmp(tag, "radialstacksize")))  {
		printf("The keywords saveradialstacks and radialstacksize have been removed.\n"
			   "Please set saveradialaverage=1 in combination with savecxi=1.\n"
			   "Modify your ini file and try again...\n");
		fail = 1;
	}
	// Power user settings
	else if (!strcmp(tag, "debuglevel")) {
		debugLevel = atoi(value);
	}
	else if (!strcmp(tag, "powderthresh")) {
		powderthresh = atof(value);
		usePowderThresh	= 1;
	}
	else if (!strcmp(tag, "powdersumhits")) {
		powderSumHits = atoi(value);
	}
	else if (!strcmp(tag, "powdersumblanks")) {
		powderSumBlanks = atoi(value);
	}
	else if (!strcmp(tag, "powdersumwithbackgroundsubtraction")) {
		powderSumWithBackgroundSubtraction = atoi(value);
	}
	else if (!strcmp(tag, "assemblepowders")){
		assemblePowders = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderadc")) {
		hitfinderADC = atof(value);
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
	// depreciated?
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
	else if (!strcmp(tag, "hitfinderminpeakseparation")) {
		hitfinderMinPeakSeparation = atof(value);
		hitfinderCheckPeakSeparation = 1;
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
	else if (!strcmp(tag, "hitfinderminres")) {
		hitfinderMinRes = atof(value);
	}
	else if (!strcmp(tag, "hitfindermaxres")) {
		hitfinderMaxRes = atof(value);
	}
	else if (!strcmp(tag, "hitfinderresolutionunitpixel")) {
		hitfinderResolutionUnitPixel = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderminsnr")) {
		hitfinderMinSNR = atof(value);
	}
	else if (!strcmp(tag, "hitfinderdownsampling")) {
		hitfinderDownsampling = (long) atoi(value);
	}
	else if (!strcmp(tag, "hitfinderfastscan")) {
		hitfinderFastScan = atoi(value);
	}
	else if (!strcmp(tag, "selfdarkmemory")) {
		printf("The keyword selfDarkMemory has been changed.  It is\n"
			   "now known as bgMemory.\n"
			   "Modify your ini file and try again...\n");
		fail = 1;
	}
	// depreciated?
	else if (!strcmp(tag, "fudgeevr41")) {
		fudgeevr41 = atoi(value);
	}
	else if (!strcmp(tag, "pumplaserscheme")) {
		strcpy(pumpLaserScheme, value); 
	}
	else if (!strcmp(tag, "savecxi")) {
		saveCXI = atoi(value);
	}
	else if (!strcmp(tag, "datasaveformat")) {
		strcpy(dataSaveFormat, value);
	}
    else if (!strcmp(tag, "cxiflushperiod")) {
		cxiFlushPeriod = atoi(value);
	} else if (!strcmp(tag, "cxiswmr")) {
		cxiSWMR = atoi(value);
	} else if (!strcmp(tag, "pythonfile")) {
		strcpy(pythonFile, value);
	}
    else if (!strcmp(tag, "usesinglethreadcalibration")) {
		useSingleThreadCalibration = atoi(value);
	} 
	else if (!strcmp(tag, "sampleposxpv")) {
		strcpy(samplePosXPV,value);
	} 
	else if (!strcmp(tag, "sampleposypv")) {
		strcpy(samplePosYPV,value);
	} 
	else if (!strcmp(tag, "sampleposzpv")) {
		strcpy(samplePosZPV,value);
	}
 	else if (!strcmp(tag, "samplevoltage")) {
		strcpy(sampleVoltage,value);
	}
	
	// Unknown tags
	else {
		//printf("\tUnknown tag: %s = %s\n",tag,value);
		//printf("Aborting...\n");
		fail = 1;
	}

	return fail;

}

int cGlobal::validateConfiguration(void){
	int fail = 0;
#ifndef H5_HAVE_THREADSAFE
	if (nThreads > 1){
		ERROR("Configuration with nThreads=%d is incompatible with your HDF5 installation (no thread safety). "
			  "Either run in single-threaded mode (nThreads=1) or add thread safety by reconfiguring "
			  "your HDF5 installation (./configure --enable-threadsafe --with-pthread; make install).",nThreads);
		fail = 1;
	}
#endif
    
    /* Do we know this data format */
    if (strcmp(dataSaveFormat, "INT16") && strcmp(dataSaveFormat, "float") && strcmp(dataSaveFormat, "INT32")) {
        printf("Error: Unknown data format type specified:");
        printf("dataSaveFormat = %s", dataSaveFormat);
        fail = 1;
    };
    
    

    
	return fail;
}


/*
 *	Write all configuration parameters to log file
 *	(in a format that can be used as a .ini file if needed)
 */
void cGlobal::writeConfigurationLog(void){
	FILE *fp;

	
	
	// Logfile name
	printf("Writing configuration file: %s\n", configOutFile);

	// Open file
	fp = fopen(configOutFile,"w");
	if(fp == NULL) {
		printf("Error: Can not open %s for writing\n",configOutFile);
		printf("Aborting...");
		exit(1);
	}
	
	fprintf(fp, "# Automatic output of all Cheetah configurations settings\n");
	fprintf(fp, "# (if not set in your .ini, these are the default values used in calculation)\n");
    
    fprintf(fp, "# --------------------------------------\n");
    fprintf(fp, "# GLOBAL PARAMETERS\n");
    fprintf(fp, "# --------------------------------------\n");
    fprintf(fp, "defaultPhotonEnergyeV=%f\n",defaultPhotonEnergyeV);
    fprintf(fp, "fixedPhotonEnergyeV=%f\n",fixedPhotonEnergyeV);
    fprintf(fp, "generateDarkcal=%d\n",generateDarkcal);
    fprintf(fp, "generateGaincal=%d\n",generateGaincal);
    fprintf(fp, "hitfinder=%d\n",hitfinder);
    fprintf(fp, "hitfinderDetectorID=%d\n",hitfinderDetectorID);
    fprintf(fp, "hitfinderAlgorithm=%d\n",hitfinderAlgorithm);
    fprintf(fp, "hitfinderADC=%f\n",hitfinderADC);
    fprintf(fp, "hitfinderTAT=%f\n",hitfinderTAT);
    //fprintf(fp, "hitfinderCluster=%d\n",hitfinderCluster);
    fprintf(fp, "hitfinderCheckGradient=%d\n",hitfinderCheckGradient);
    fprintf(fp, "hitfinderMinGradient=%f\n",hitfinderMinGradient);
    fprintf(fp, "hitfinderNPeaks=%d\n",hitfinderNpeaks);
    fprintf(fp, "hitfinderNPeaksMax=%d\n",hitfinderNpeaksMax);
    fprintf(fp, "hitfinderMinPixCount=%d\n",hitfinderMinPixCount);
    fprintf(fp, "hitfinderMaxPixCount=%d\n",hitfinderMaxPixCount);
    fprintf(fp, "hitfinderMinPeakSeparation=%f\n",hitfinderMinPeakSeparation);
    fprintf(fp, "hitfinderSubtractLocalBG=%d\n",hitfinderSubtractLocalBG);
    fprintf(fp, "hitfinderLocalBGRadius=%d\n",hitfinderLocalBGRadius);
    fprintf(fp, "hitfinderLocalBGThickness=%d\n",hitfinderLocalBGThickness);
    fprintf(fp, "hitfinderMinRes=%f\n",hitfinderMinRes);
    fprintf(fp, "hitfinderMaxRes=%f\n",hitfinderMaxRes);
    fprintf(fp, "hitfinderResolutionUnitPixel=%i\n",hitfinderResolutionUnitPixel);
    fprintf(fp, "hitfinderMinSNR=%f\n",hitfinderMinSNR);
    fprintf(fp, "hitlist=%s\n",hitlistFile);
    fprintf(fp, "peakmask=%s\n",peaksearchFile);
    fprintf(fp, "powderThresh=%f\n",powderthresh);
    fprintf(fp, "powderSumHits=%d\n",powderSumHits);
    fprintf(fp, "powderSumBlanks=%d\n",powderSumBlanks);
    fprintf(fp, "assemblePowders=%d\n",assemblePowders);
    fprintf(fp, "saveInterval=%d\n",saveInterval);
    fprintf(fp, "saveRadialStacks=%d\n",saveRadialStacks);
    fprintf(fp, "radialStackSize=%ld\n",radialStackSize);
    fprintf(fp, "saveHits=%d\n",saveHits);
    fprintf(fp, "saveBlanks=%d\n",saveBlanks);
    fprintf(fp, "saveNonAssembled=%d\n",saveNonAssembled);
    //fprintf(fp, "saveRawInt16=%d\n",saveRawInt16);
    fprintf(fp, "saveModular=%d\n",saveModular);
    fprintf(fp, "saveAssembled=%d\n",saveAssembled);
    fprintf(fp, "saveRadialAverage=%d\n",saveRadialAverage);	
    fprintf(fp, "assembleInterpolation=%d\n",assembleInterpolation);
    fprintf(fp, "savePixelmask=%d\n",savePixelmask);
    fprintf(fp, "saveCXI=%d\n",saveCXI);
    fprintf(fp, "hdf5dump=%d\n",hdf5dump);
    fprintf(fp, "pythonfile=%s\n",pythonFile);
    fprintf(fp, "debugLevel=%d\n",debugLevel);
    fprintf(fp, "nThreads=%ld\n",nThreads);
    fprintf(fp, "useHelperThreads=%d\n",useHelperThreads);
    fprintf(fp, "threadPurge=%ld\n",threadPurge);
    fprintf(fp, "ioSpeedTest=%d\n",ioSpeedTest);
    //fprintf(fp, "tofName=%s\n",tofName);
    //fprintf(fp, "tofChannel=%d\n",TOFchannel);
    fprintf(fp, "hitfinderUseTOF=%d\n",hitfinderUseTOF);
    //fprintf(fp, "hitfinderTOFMinSample=%d\n",hitfinderTOFMinSample[0]);
    //fprintf(fp, "hitfinderTOFMaxSample=%d\n",hitfinderTOFMaxSample[0]);
    //fprintf(fp, "hitfinderTOFMeanBackground=%f\n",hitfinderTOFMeanBackground[0]);
    //fprintf(fp, "hitfinderTOFThresh=%f\n",hitfinderTOFThresh[0]);
    fprintf(fp, "espectrum=%d\n",espectrum);
    fprintf(fp, "espectrum1D=%d\n",espectrum1D);
    fprintf(fp, "espectrumTiltAng=%f\n",espectrumTiltAng);
    fprintf(fp, "espectrumLength=%d\n",espectrumLength);
    fprintf(fp, "espectrumWidth=%d\n",espectrumWidth);
    fprintf(fp, "espectrumSpreadeV=%d\n",espectrumSpreadeV);
    fprintf(fp, "espectrumDarkSubtract=%d\n",espectrumDarkSubtract);
    fprintf(fp, "espectrumDarkFile=%s\n",espectrumDarkFile);
    fprintf(fp, "espectrumScaleFile=%s\n",espectrumScaleFile);
    //fprintf(fp, "skipFract=%f\n",skipFract);
    //fprintf(fp, "startAtFrame=%ld\n",startAtFrame);
    //fprintf(fp, "stopAtFrame=%ld\n",stopAtFrame);
    
    for (int i=0; i<nDetectors; i++) {
        fprintf(fp, "# --------------------------------------\n");
        fprintf(fp, "# DETECTOR %d PARAMETERS\n",i);
        fprintf(fp, "# --------------------------------------\n");
        fprintf(fp, "detectorType=%s\n",detector[i].detectorType);
        fprintf(fp, "detectorName=%s\n",detector[i].detectorName);
        fprintf(fp, "geometry=%s\n",detector[i].geometryFile);
        fprintf(fp, "pixelSize=%f\n",detector[i].pixelSize);
        fprintf(fp, "beamCenterX=%f\n",detector[i].beamCenterPixX);
        fprintf(fp, "beamCenterY=%f\n",detector[i].beamCenterPixY);
        fprintf(fp, "defaultCameraLengthMm=%f\n",detector[i].defaultCameraLengthMm);
        fprintf(fp, "cameraLengthOffset=%f\n",detector[i].cameraLengthOffset);
        fprintf(fp, "cameraLengthScale=%f\n",detector[i].cameraLengthScale);
        fprintf(fp, "fixedCameraLengthMm=%f\n",detector[i].fixedCameraLengthMm);
        fprintf(fp, "badPixelMap=%s\n",detector[i].badpixelFile);
        fprintf(fp, "applyBadPixelMap=%d\n",detector[i].applyBadPixelMask);
        fprintf(fp, "badDataMap=%s\n",detector[i].baddataFile);
        fprintf(fp, "wiremaskFile=%s\n",detector[i].wireMaskFile);
        fprintf(fp, "darkcal=%s\n",detector[i].darkcalFile);
        fprintf(fp, "cmModule=%d\n",detector[i].cmModule);
        fprintf(fp, "cmFloor=%f\n",detector[i].cmFloor);
        fprintf(fp, "subtractBehindWires=%d\n",detector[i].cspadSubtractBehindWires);
        fprintf(fp, "subtractUnbondedPixels=%d\n",detector[i].cspadSubtractUnbondedPixels);
        fprintf(fp, "gaincal=%s\n",detector[i].gaincalFile);
        fprintf(fp, "invertGain=%d\n",detector[i].invertGain);
        fprintf(fp, "usePolarizationCorrection=%d\n",detector[i].usePolarizationCorrection);
        fprintf(fp, "horizontalFractionOfPolarization=%f\n",detector[i].horizontalFractionOfPolarization);
        fprintf(fp, "useSolidAngleCorrection=%d\n",detector[i].useSolidAngleCorrection);
        fprintf(fp, "solidAngleAlgorithm=%d\n",detector[i].solidAngleAlgorithm);
        fprintf(fp, "useSubtractPersistentBackground=%d\n",detector[i].useSubtractPersistentBackground);
        fprintf(fp, "useBackgroundBufferMutex=%d\n",detector[i].useBackgroundBufferMutex);
        fprintf(fp, "scaleBackground=%d\n",detector[i].scaleBackground);
        fprintf(fp, "bgMemory=%li\n",detector[i].bgMemory);
        fprintf(fp, "bgRecalc=%ld\n",detector[i].bgRecalc);
        fprintf(fp, "bgMedian=%f\n",detector[i].bgMedian);
        fprintf(fp, "bgIncludeHits=%d\n",detector[i].bgIncludeHits);
        fprintf(fp, "bgNoBeamReset=%d\n",detector[i].bgNoBeamReset);
        fprintf(fp, "bgFiducialGlitchReset=%d\n",detector[i].bgFiducialGlitchReset);
        fprintf(fp, "startFrames=%d\n",detector[i].startFrames);
        fprintf(fp, "useLocalBackgroundSubtraction=%d\n",detector[i].useLocalBackgroundSubtraction);
        fprintf(fp, "localBackgroundRadius=%ld\n",detector[i].localBackgroundRadius);
        fprintf(fp, "#useAutoHotPixel=%d\n",detector[i].useAutoHotpixel);
        fprintf(fp, "applyAutoHotPixel=%d\n",detector[i].applyAutoHotpixel);
        fprintf(fp, "hotpixFreq=%f\n",detector[i].hotpixFreq);
        fprintf(fp, "hotpixADC=%d\n",detector[i].hotpixADC);
        fprintf(fp, "hotpixMemory=%d\n",detector[i].hotpixMemory);
        fprintf(fp, "maskSaturatedPixels=%d\n",detector[i].maskSaturatedPixels);
        fprintf(fp, "pixelSaturationADC=%ld\n",detector[i].pixelSaturationADC);
        fprintf(fp, "useAutoHalopixel=%d\n",detector[i].useAutoHalopixel);
        fprintf(fp, "halopixMinDeviation=%f\n",detector[i].halopixMinDeviation);
        fprintf(fp, "halopixelMemory=%li\n",detector[i].halopixMemory);
        fprintf(fp, "halopixelRecalc=%ld\n",detector[i].halopixRecalc);
        fprintf(fp, "histogram=%d\n",detector[i].histogram);
        fprintf(fp, "histogramMin=%ld\n",detector[i].histogramMin);
        fprintf(fp, "histogramNbins=%ld\n",detector[i].histogramNbins);
        fprintf(fp, "histogramBinSize=%f\n",detector[i].histogramBinSize);
        fprintf(fp, "histogram_fs_min=%ld\n",detector[i].histogram_fs_min);
        fprintf(fp, "histogram_fs_max=%ld\n",detector[i].histogram_fs_max);
        fprintf(fp, "histogram_ss_min=%ld\n",detector[i].histogram_ss_min);
        fprintf(fp, "histogram_ss_max=%ld\n",detector[i].histogram_ss_max);
        fprintf(fp, "histogramMaxMemoryGb=%f\n",detector[i].histogramMaxMemoryGb);
        fprintf(fp, "downsampling=%ld\n",detector[i].downsampling);
        fprintf(fp, "saveDetectorRaw=%d\n",detector[i].saveDetectorRaw);
        fprintf(fp, "saveDetectorCorrectedOnly=%d\n",detector[i].saveDetectorCorrectedOnly);
    }
    
	// CLose file
	fclose (fp);

	
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

	fprintf(framefp, "# eventData->Filename, eventData->frameNumber, eventData->threadNum, eventData->hit, eventData->powderClass, eventData->photonEnergyeV, eventData->wavelengthA, eventData->gmd1, eventData->gmd2, eventData->detector[0].detectorZ, eventData->energySpectrumExist,  eventData->nPeaks, eventData->peakNpix, eventData->peakTotal, eventData->peakResolution, eventData->peakDensity, eventData->pumpLaserCode, eventData->pumpLaserDelay, eventData->pumpLaserOn\n");

	sprintf(cleanedfile,"cleaned.txt");
	cleanedfp = fopen (cleanedfile,"w");
	if(cleanedfp == NULL) {
		printf("Error: Can not open %s for writing\n",cleanedfile);
		printf("Aborting...");
		exit(1);
	}
	fprintf(cleanedfp, "# Filename, frameNumber, nPeaks, nPixels, totalIntensity, peakResolution, peakResolutionA, peakDensity\n");
	pthread_mutex_unlock(&framefp_mutex);

	pthread_mutex_lock(&peaksfp_mutex);
	sprintf(peaksfile,"peaks.txt");
	peaksfp = fopen (peaksfile,"w");
	if(peaksfp == NULL) {
		printf("Error: Can not open %s for writing\n",peaksfile);
		printf("Aborting...");
		exit(1);
	}
	fprintf(peaksfp, "# frameNumber, eventName, photonEnergyEv, wavelengthA, GMD, peak_index, peak_x_raw, peak_y_raw, peak_r_assembled, peak_q, peak_resA, nPixels, totalIntensity, maxIntensity, sigmaBG, SNR\n");
	pthread_mutex_unlock(&peaksfp_mutex);

}

void cGlobal::writeHitClasses(FILE* to) {
	fprintf(to, "Hitclasses:\n");
	for (int coord = 0; coord < 3; coord++) {
		int lastFirst = 1 << 30;
		int lastVal = 0;
		for (std::map<std::pair<int, int>, int>::iterator i = hitClasses[coord].begin(); i != hitClasses[coord].end(); i++) {
			fprintf(to, "Coord %d: %05d %d %d\n", coord, i->first.first, i->first.second, i->second);
			if (i->first.second)
			{
				if (lastFirst != i->first.first) {
					lastVal = 0;
				}
				double sum = lastVal + i->second;
				fprintf(to, "\t%0.03lf %%\n", i->second / sum * 100);
			} else {
				lastFirst = i->first.first;
				lastVal = i->second;
			}
		}
	}
	fprintf(to, "\n\n");
}

/*
 * Update log file
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
    writeHitClasses(::stdout);
    writeHitClasses(fp);
	fprintf(fp, "nFrames: %li,  nHits: %li (%2.2f%%), recentHits: %li (%2.2f%%), wallTime: %ihr %imin %isec (%2.1f fps)\n", nprocessedframes, nhits, hitrate, nrecenthits, recenthitrate, hrs, mins, secs, fps);
	fclose (fp);

	nrecenthits = 0;
	nrecentprocessedframes = 0;


    // Flush frame file buffers
	fflush(framefp);
	fflush(cleanedfp);
    fflush(peaksfp);
	for(long i=0; i<nPowderClasses; i++) {
		fflush(powderlogfp[i]);
        fflush(FEElogfp[i]);
	}

}

/*
 *	Write (and keep over-writing) a little status file
 */
void cGlobal::writeStatus(const char* message) {
	
	// Current time
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

	// Now write it to file
    FILE *fp;
    fp = fopen ("status.txt","w");
	fprintf(fp, "# Cheetah status\n");
	fprintf(fp, "Update time: %s\n",timestr);
	fprintf(fp, "Elapsed time: %ihr %imin %isec\n",hrs,mins,secs);
    fprintf(fp, "Status: %s\n", message);
	fprintf(fp, "Frames processed: %li\n",nprocessedframes);
	fprintf(fp, "Number of hits: %li\n",nhits);
    fclose (fp);


}

void cGlobal::updateCalibrated(void){
	int temp = 1;
	for(long detIndex=0; detIndex<MAX_DETECTORS; detIndex++) {
		temp *= ((detector[detIndex].useAutoHotpixel == 0) || detector[detIndex].hotpixCalibrated);
		temp *= ((detector[detIndex].useAutoHalopixel == 0) || detector[detIndex].halopixCalibrated);
		temp *= ((detector[detIndex].useSubtractPersistentBackground == 0) || detector[detIndex].bgCalibrated);
		/* FOR TESTING
		   printf("detector[%i].useAutoHotpixel=%i,calibrated=%i\n",detIndex,detector[detIndex].useAutoHotpixel,detector[detIndex].hotpixCalibrated);
		   printf("detector[%i].useAutoHalopixel=%i,calibrated=%i\n",detIndex,detector[detIndex].useAutoHalopixel,detector[detIndex].halopixCalibrated);
		   printf("detector[%i].useSubtractPersistentBackground=%i,calibrated=%i\n",detIndex,detector[detIndex].useSubtractPersistentBackground,detector[detIndex].bgCalibrated);
		*/
	}
	calibrated = temp;
}


/*
 *	Write final log file
 */
void cGlobal::writeFinalLog(void){

	FILE *fp;

	// Logfile name
	printf("Writing log file: %s\n", logfile);
	fp = fopen (logfile,"a");

    writeHitClasses(::stdout);
    writeHitClasses(fp);
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
	fprintf(fp, "Number of hits: %li\n",nhits);
	fprintf(fp, "Average hit rate: %2.2f %%\n",hitrate);
	fprintf(fp, "nFrames in powder patterns:\n");
	for(long i=0; i<nPowderClasses; i++) {
		fprintf(fp, "\tclass%ld: %li\n", i, nPowderFrames[i]);
	}
	fprintf(fp, "Number of energy spectra collected: %li\n",nespechits);
	fprintf(fp, "Average frame rate: %2.2f fps\n",fps);
	fprintf(fp, "Average data rate: %2.2f MB/sec\n",mbs);
	fprintf(fp, "Average photon energy: %7.2f	eV\n",meanPhotonEnergyeV);
	fprintf(fp, "Photon energy sigma: %5.2f eV\n",photonEnergyeVSigma);
	fprintf(fp, "Cheetah clean exit\n");
	fprintf(fp, ">-------- Cheetah exit --------<\n");
	fclose (fp);


	// Close frame buffers
	if(framefp != NULL)
		fclose(framefp);
	if(cleanedfp != NULL)
		fclose(cleanedfp);
    if(peaksfp != NULL)
        fclose(peaksfp);

	for(long i=0; i<nPowderClasses; i++) {
        if(powderlogfp[i] != NULL)
			fclose(powderlogfp[i]);
        if(FEElogfp[i] != NULL)
            fclose(FEElogfp[i]);
	}


}



/*
 *	Read in list of hits from text file
 */
void cGlobal::readHits(char *filename) {
	
	printf("Reading list of hits:\n");
	printf("\t%s\n",filename);
	
	std::ifstream infile;
	infile.open(filename);
	if (infile.fail()) {
		std::cout << "\tUnable to open " << filename << std::endl;
		infile.clear();
		printf("\tDisabling the hitfinder\n");
		hitfinder = 0;
		return;
	}
	
	std::string line;
	while (true) {
		std::getline(infile, line);
		if (infile.fail()) break;
		if (line[0] != '#') {
			hitlist.push_back(line);
		}
	}
	
	std::cout << "\tList contained " << hitlist.size() << " hits." << std::endl;
}

template<typename T>
void cGlobal::splitList(char * values, std::vector<T> & elems) {
	char delim = ',';
    std::stringstream ss(values);
    std::string item;
	elems.clear();
    while (std::getline(ss, item, delim)) {
		std::stringstream innerss(item);
		
		T elem;
		innerss >> elem;
		elems.push_back(elem);
    }
}
