/*
 *  setup.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
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

#include "data2d.h"
#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"


/*
 *	Default settings/configuration
 */
void cGlobal::defaultConfiguration(void) {

	// ini file to use
	strcpy(configFile, "cheetah.ini");

	// Default experiment info (in case beamline data is missing...)
	defaultPhotonEnergyeV = 0;

	// Detector info
	nDetectors = 0;
	for(long i=0; i<MAX_DETECTORS; i++) {
		strcpy(detector[i].detectorConfigFile, "No_file_specified");
		strcpy(detector[i].configGroup,"none");
	}

	// Statistics
	summedPhotonEnergyeV = 0;
	meanPhotonEnergyeV = 0;

	// Pv values
	strcpy(laserDelayPV, "LAS:FS5:Angle:Shift:Ramp:rd");
	laserDelay = std::numeric_limits<float>::quiet_NaN();
	laserDelay = 0;

	// Start and stop frames
	startAtFrame = 0;
	stopAtFrame = 0;

	// Calibrations
	generateDarkcal = 0;
	generateGaincal = 0;

	// Hitfinding
	hitfinder = 0;
	hitfinderADC = 100;
	hitfinderTAT = 1e3;
	hitfinderNpeaks = 50;
	hitfinderNpeaksMax = 100000;
	hitfinderAlgorithm = 3;
	hitfinderMinPixCount = 3;
	hitfinderMaxPixCount = 20;
	hitfinderUsePeakmask = 0;
	hitfinderCheckGradient = 0;
	hitfinderMinGradient = 0;
	strcpy(peaksearchFile, "No_file_specified");
	savePeakInfo = 0;
	hitfinderCheckPeakSeparation = 0;
	hitfinderMinPeakSeparation = 50;
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
	//tofType = Pds::DetInfo::Acqiris;
	//tofPdsDetInfo = Pds::DetInfo::CxiSc1;

	// Powder pattern generation
	nPowderClasses = 2;
	powderthresh = -100000;
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
	saveInterval = 1000;

	// Peak lists
	savePeakList = 1;

	// Verbosity
	debugLevel = 2;

	// I/O speed test?
	ioSpeedTest = 0;

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
 * Setup stuff to do with thread management, settings, etc.
 */
void cGlobal::setup() {

	/*
	 *	Determine detector type
	 *	This section of code possibly redundant if we know detector type from the address
	 *	(eg: CxiDs1 is always a cspad)
	 */
    /*
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
     */

	/*
	 *	Determine detector address
	 *	A list of addresses can be found in:
	 *		release/pdsdata/xtc/Detinfo.hh
	 *		release/pdsdata/xtc/src/Detinfo.cc
	 */
    /*
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
     */

	/*
	 * Detector parameters
	 */
	for(long i=0; i<nDetectors; i++) {
		if(strcmp(detector[i].detectorName, "CxiDs1") == 0 ||
			strcmp(detector[i].detectorName, "CxiDs2") == 0 ||
			strcmp(detector[i].detectorName, "CxiDsd") == 0 ||
			strcmp(detector[i].detectorName, "XppGon") == 0) {
			detector[i].asic_nx = CSPAD_ASIC_NX;
			detector[i].asic_ny = CSPAD_ASIC_NY;
			detector[i].asic_nn = CSPAD_ASIC_NX * CSPAD_ASIC_NY;
			detector[i].nasics_x = CSPAD_nASICS_X;
			detector[i].nasics_y = CSPAD_nASICS_Y;
		} else {
			printf("Error: unknown detector %s\n", detector[i].detectorName);
			printf("Quitting\n");
			exit(1);
			break;
		}
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
	 * Set up arrays for hot pixels, running backround, etc.
	 */
	for(long i=0; i<nDetectors; i++) {
		detector[i].allocatePowderMemory(self);
	}

	hitfinderResMask = (int	*) calloc(detector[0].pix_nn, sizeof(int));
	for(long j=0; j<detector[0].pix_nn; j++) {
		hitfinderResMask[j] = 1;
	}


	/*
	 * Set up arrays for powder classes and radial stacks
	 * Currently only tracked for detector[0]  (generalise this later)
	 */
	for(long i=0; i<nPowderClasses; i++) {
		char  filename[1024];
		powderlogfp[i] = NULL;
		if(runNumber > 0) {
			sprintf(filename,"r%04u-class%ld-log.txt",runNumber,i);
			powderlogfp[i] = fopen(filename, "w");
		}
	}


	/*
	 * Set up thread management
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
	 * Trap specific configurations and mutually incompatible options
	 */
	if(generateDarkcal) {

		printf("keyword generatedarkcal set: overriding some keyword values!!!");

		hitfinder = 0;
		savehits = 0;
		hdf5dump = 0;
		saveRaw = 0;
		powderSumHits = 0;
		powderSumBlanks = 0;
		powderthresh = -30000;
		for(long i=0; i<nDetectors; i++) {
			detector[i].cmModule = 0;
			detector[i].cmSubtractUnbondedPixels = 0;
			detector[i].useDarkcalSubtraction = 0;
			detector[i].useGaincal=0;
			detector[i].useAutoHotpixel = 0;
			detector[i].useSubtractPersistentBackground = 0;
			detector[i].startFrames = 0;
			detector[i].saveDetectorRaw = 1;
			detector[i].saveDetectorCorrectedOnly = 1;
		}
	}

	if(generateGaincal) {

		printf("keyword generategaincal set: overriding some keyword values!!!");

		hitfinder = 0;
		savehits = 0;
		hdf5dump = 0;
		saveRaw = 0;
		powderSumHits = 0;
		powderSumBlanks = 0;
		powderthresh = -30000;
		for(long i=0; i<nDetectors; i++) {
			detector[i].cmModule = 0;
			detector[i].cmSubtractUnbondedPixels = 0;
			detector[i].useDarkcalSubtraction = 1;
			detector[i].useAutoHotpixel = 0;
			detector[i].useSubtractPersistentBackground = 0;
			detector[i].useGaincal=0;
			detector[i].startFrames = 0;
			detector[i].saveDetectorRaw = 1;
			detector[i].saveDetectorCorrectedOnly = 1;
		}
	}

	if(saveRaw==0 && saveAssembled == 0) {
		saveAssembled = 1;
	}

	/* Only save peak info for certain hitfinders */
	if (( hitfinderAlgorithm == 3 ) ||
	    ( hitfinderAlgorithm == 5 ) ||
	    ( hitfinderAlgorithm == 6 ))
		savePeakInfo = 1; 

	/*
	 * Other stuff
	 */
	npowderHits = 0;
	npowderBlanks = 0;
	nhits = 0;
	nrecenthits = 0;
	nprocessedframes = 0;
	nrecentprocessedframes = 0;
	lastclock = clock()-10;
	datarate = 1;
	runNumber = 0;
	time(&tstart);
	avgGMD = 0;

	for(long i=0; i<MAX_DETECTORS; i++) {
		detector[i].bgCounter = 0;
		detector[i].last_bg_update = 0;
		detector[i].hotpixCounter = 0;
		detector[i].last_hotpix_update = 0;
		detector[i].hotpixRecalc = detector[i].bgRecalc;
		detector[i].nhot = 0;
		detector[i].detectorZprevious = 0;
		detector[i].detectorZ = 0;
		detector[i].detectorEncoderValue = 0;
	}

	time(&tlast);
	lastTimingFrame=0;

	// Make sure to use SLAC timezone!
	setenv("TZ","US/Pacific",1);

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
				parseConfigTag(argv[i]+1, argv[++i]);
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
	int i,cnt;

	/*
	 * Open configuration file for reading
	 */
	printf("Parsing cheetah configuration file: %s\n",filename);
	printf("\t%s\n",filename);

	fp = fopen(filename,"r");
	if (fp == NULL) {
		printf("\tCould not open configuration file \"%s\"\n",filename);
		printf("\tUsing default values\n");
		return;
	}

	/*
	 * Loop through configuration file until EOF
	 * Ignore lines beginning with a '#' (comments)
	 * Split each line into tag and value at the '=' sign
	 */	

	while (feof(fp) == 0) {

		int fail = 0;

		cp = fgets(cbuf, cbufsize, fp);
		if (cp == NULL)
			break;

		/* strip whitespace */
		cnt=0;
		for (i=0; i<cbufsize; i++){
			if (cbuf[i] == ' ') continue;
			cbuf[cnt] = cbuf[i];
			cnt++;
		}

		/* strip comments */
		for (i=0; i<cbufsize-1; i++){
			if (cbuf[i] == '#'){
				cbuf[i] = '\n';
				cbuf[i+1] = '\0';
			}
		}

		/* skip empty lines */
		if ( strlen(cbuf) <= 1) continue;

		/* check for string prepend */
		cp = strrchr(cbuf,']');
		if (cp != NULL){
			*(cp) = '\0';
			strcpy(groupPrepend,cbuf+1);
			if (strlen(groupPrepend) != 0) 
				strcat(groupPrepend,"/");
			continue;
		}

		/* prepend string */
		if (strcmp(groupPrepend, "")) {
			strcpy(ts,groupPrepend);
			strcat(ts,cbuf);
			strcpy(cbuf,ts);
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

		if (!strcmp(group,"default")) {

			/* set global configuration */
			fail = parseConfigTag(tag, value);
			
			/* tag doesn't belog to global?  Then by default we will pass 
			 * it to the first detector. */
			fail = detector[0].parseConfigTag(tag,value);

		} else {

			int matched=0;

			/* set detector-specific configuration */
			for (i=0; i<MAX_DETECTORS; i++){

				/* new group? */
				if (!strcmp("none",detector[i].configGroup)){
					strcpy(detector[i].configGroup,group);
					nDetectors++;
				}

				/* try to match group to detector */
				if (!strcmp(group,detector[i].configGroup)){
					matched = 1;
					fail = detector[i].parseConfigTag(tag,value);
					break;
				}

			}
		
			if (matched == 0){
				printf("ERROR: Only %i detectors allowed at this time... fix your config file.\n",MAX_DETECTORS);
				exit(0);
			}

			if (fail != 0){
				printf("The tag %s is not recognized.\n",tag);
				exit(0);
			}

		}


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
	for(int i=0; i<strlen(tag); i++)
		tag[i] = tolower(tag[i]);

	/*
	 * Parse known tags
	 */
//	if (!strcmp(tag, "ndetectors")) {
//		nDetectors = atoi(value);
//	}
//	else if (!strcmp(tag, "detector0")) {
//		strcpy(detector[0].detectorConfigFile, value);
//	}
//	else if (!strcmp(tag, "detector1")) {
//		strcpy(detector[1].detectorConfigFile, value);
//	}
//	else if (!strcmp(tag, "detector2")) {
//		strcpy(detector[2].detectorConfigFile, value);
//	}
	if (!strcmp(tag, "defaultphotonenergyev")) {
		defaultPhotonEnergyeV = atof(value);
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
	else if (!strcmp(tag, "peakmask")) {
		strcpy(peaksearchFile, value);
		hitfinderUsePeakmask = 1;
	}
	// Processing options
	else if (!strcmp(tag, "subtractcmmodule")) {
		printf("The keyword subtractcmModule has been changed. It is\n"
		       "now known as cmModule.\n"
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
	else if (!strcmp(tag, "savehits")) {
		savehits = atoi(value);
	}
	else if (!strcmp(tag, "powdersum")) {
		printf("The keyword powdersum has been changed.  It is\n"
		       "now known as powderSumHits and powderSumBlanks.\n"
		       "Modify your ini file and try again...\n");
		fail = 1;
    }
	else if (!strcmp(tag, "saveraw")) {
		saveRaw = atoi(value);
	}
	else if (!strcmp(tag, "saveassembled")) {
		saveAssembled = atoi(value);
	}
	else if (!strcmp(tag, "hdf5dump")) {
		hdf5dump = atoi(value);
	}
	else if (!strcmp(tag, "saveinterval")) {
		saveInterval = atoi(value);
	}
	// Time-of-flight
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
	else if (!strcmp(tag, "debuglevel")) {
		debugLevel = atoi(value);
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
		hitfinderLimitRes = 1;
	}
	else if (!strcmp(tag, "hitfindermaxres")) {
		hitfinderMaxRes = atof(value);
		hitfinderLimitRes = 1;
	}
	else if (!strcmp(tag, "hitfinderminsnr")) {
		hitfinderMinSNR = atof(value);
	}
	else if (!strcmp(tag, "selfdarkmemory")) {
		printf("The keyword selfDarkMemory has been changed.  It is\n"
             "now known as bgMemory.\n"
             "Modify your ini file and try again...\n");
		fail = 1;
	}
	else if (!strcmp(tag, "fudgeevr41")) {
		fudgeevr41 = atoi(value);
	}
	// Unknown tags
	else {
		//printf("\tUnknown tag: %s = %s\n",tag,value);
		//printf("Aborting...\n");
		fail = 1;
	}

	return fail;

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
	//fprintf(fp, "defaultCameraLengthMm=%f\n",defaultCameraLengthMm);
	//fprintf(fp, "detectorType=%s\n",detector[0].detectorTypeName);
	//fprintf(fp, "detectorName=%s\n",detector[0].detectorName);
	fprintf(fp, "startAtFrame=%ld\n",startAtFrame);
	fprintf(fp, "stopAtFrame=%ld\n",stopAtFrame);
	fprintf(fp, "nThreads=%ld\n",nThreads);
	fprintf(fp, "useHelperThreads=%d\n",useHelperThreads);
	fprintf(fp, "ioSpeedTest=%d\n",ioSpeedTest);
	fprintf(fp, "threadPurge=%ld\n",threadPurge);
	//fprintf(fp, "geometry=%s\n",detector[0].geometryFile);
	//fprintf(fp, "darkcal=%s\n",detector[0].darkcalFile);
	//fprintf(fp, "gaincal=%s\n",detector[0].gaincalFile);
	fprintf(fp, "peakmask=%s\n",peaksearchFile);
	//fprintf(fp, "badPixelMap=%s\n",detector[0].badpixelFile);
	//fprintf(fp, "subtractcmModule=%d\n",cmModule);
	//fprintf(fp, "cmModule=%d\n",cmModule);
	//fprintf(fp, "subtractUnbondedPixels=%d\n",cmSubtractUnbondedPixels);
	//fprintf(fp, "wiremaskFile=%s\n",detector[0].wireMaskFile);
	//fprintf(fp, "subtractBehindWires=%d\n",cmSubtractBehindWires);
	//fprintf(fp, "invertGain=%d\n",invertGain);
	fprintf(fp, "generateDarkcal=%d\n",generateDarkcal);
	fprintf(fp, "generateGaincal=%d\n",generateGaincal);
	fprintf(fp, "hitfinder=%d\n",hitfinder);
	fprintf(fp, "saveHits=%d\n",savehits);
	fprintf(fp, "saveRaw=%d\n",saveRaw);
	fprintf(fp, "saveAssembled=%d\n",saveAssembled);
	//fprintf(fp, "saveDetectorCorrectedOnly=%d\n",saveDetectorCorrectedOnly);
	//fprintf(fp, "saveDetectorRaw=%d\n",saveDetectorRaw);
	fprintf(fp, "hdf5dump=%d\n",hdf5dump);
	fprintf(fp, "saveInterval=%d\n",saveInterval);
	//fprintf(fp, "useAutoHotPixel=%d\n",useAutoHotpixel);
	//fprintf(fp, "maskSaturatedPixels=%d\n",maskSaturatedPixels);
	//fprintf(fp, "pixelSaturationADC=%ld\n",pixelSaturationADC);
	//fprintf(fp, "maskSaturatedPixels=%d\n",maskSaturatedPixels);
	//fprintf(fp, "pixelSaturationADC=%d\n",pixelSaturationADC);
	//fprintf(fp, "useSubtractPersistentBackground=%d\n",useSubtractPersistentBackground);
	//fprintf(fp, "useBackgroundBufferMutex=%d\n",useBackgroundBufferMutex);
	//fprintf(fp, "useLocalBackgroundSubtraction=%d\n",useLocalBackgroundSubtraction);
	//fprintf(fp, "localBackgroundRadius=%ld\n",localBackgroundRadius);
	fprintf(fp, "tofName=%s\n",tofName);
	fprintf(fp, "tofChannel=%d\n",TOFchannel);
	fprintf(fp, "hitfinderUseTOF=%d\n",hitfinderUseTOF);
	fprintf(fp, "hitfinderTOFMinSample=%d\n",hitfinderTOFMinSample);
	fprintf(fp, "hitfinderTOFMaxSample=%d\n",hitfinderTOFMaxSample);
	fprintf(fp, "hitfinderTOFThresh=%f\n",hitfinderTOFThresh);
	fprintf(fp, "saveRadialStacks=%d\n",saveRadialStacks);
	fprintf(fp, "radialStackSize=%ld\n",radialStackSize);
	//fprintf(fp, "cmFloor=%f\n",cmFloor);
	//fprintf(fp, "pixelSize=%f\n",detector[0].pixelSize);
	fprintf(fp, "debugLevel=%d\n",debugLevel);
	//fprintf(fp, "hotpixFreq=%f\n",hotpixFreq);
	//fprintf(fp, "hotpixADC=%d\n",hotpixADC);
	//fprintf(fp, "hotpixMemory=%d\n",hotpixMemory);
	fprintf(fp, "powderThresh=%d\n",powderthresh);
	fprintf(fp, "powderSumHits=%d\n",powderSumHits);
	fprintf(fp, "powderSumBlanks=%d\n",powderSumBlanks);
	fprintf(fp, "hitfinderADC=%d\n",hitfinderADC);
	fprintf(fp, "hitfinderTIT=%f\n",hitfinderTAT);
	fprintf(fp, "hitfinderCheckGradient=%d\n",hitfinderCheckGradient);
	fprintf(fp, "hitfinderMinGradient=%f\n",hitfinderMinGradient);
	fprintf(fp, "hitfinderCluster=%d\n",hitfinderCluster);
	fprintf(fp, "hitfinderNPeaks=%d\n",hitfinderNpeaks);
	fprintf(fp, "hitfinderNPeaksMax=%d\n",hitfinderNpeaksMax);
	fprintf(fp, "hitfinderAlgorithm=%d\n",hitfinderAlgorithm);
	fprintf(fp, "hitfinderMinPixCount=%d\n",hitfinderMinPixCount);
	fprintf(fp, "hitfinderMaxPixCount=%d\n",hitfinderMaxPixCount);
	fprintf(fp, "hitfinderMinPeakSeparation=%f\n",hitfinderMinPeakSeparation);
	fprintf(fp, "hitfinderSubtractLocalBG=%d\n",hitfinderSubtractLocalBG);
	fprintf(fp, "hitfinderLocalBGRadius=%d\n",hitfinderLocalBGRadius);
	fprintf(fp, "hitfinderLocalBGThickness=%d\n",hitfinderLocalBGThickness);
	fprintf(fp, "hitfinderLimitRes=%d\n",hitfinderLimitRes);
	fprintf(fp, "hitfinderMinRes=%f\n",hitfinderMinRes);
	fprintf(fp, "hitfinderMaxRes=%f\n",hitfinderMaxRes);
	fprintf(fp, "hitfinderMinSNR=%f\n",hitfinderMinSNR);
	//fprintf(fp, "selfdarkMemory=%li\n",bgMemory);
	//fprintf(fp, "bgMemory=%li\n",bgMemory);
	//fprintf(fp, "bgRecalc=%ld\n",bgRecalc);
	//fprintf(fp, "bgMedian=%f\n",bgMedian);
	//fprintf(fp, "bgIncludeHits=%d\n",bgIncludeHits);
	//fprintf(fp, "bgNoBeamReset=%d\n",bgNoBeamReset);
	//fprintf(fp, "bgFiducialGlitchReset=%d\n",bgFiducialGlitchReset);
	//fprintf(fp, "scaleBackground=%d\n",scaleBackground);
	//fprintf(fp, "scaleDarkcal=%d\n",scaleBackground);
	//fprintf(fp, "startFrames=%d\n",startFrames);
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

	fprintf(framefp, "# eventData->eventname, eventData->threadNum, eventData->photonEnergyeV, eventData->wavelengthA, eventData->detector[0].detectorZ, eventData->gmd1, eventData->gmd2, eventData->nPeaks, eventData->peakNpix, eventData->peakTotal, eventData->peakResolution, eventData->peakDensity, eventData->laserEventCodeOn, eventData->laserDelay\n");

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
	pthread_mutex_unlock(&peaksfp_mutex);

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
