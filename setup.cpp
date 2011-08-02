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
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad-gjw/CspadTemp.hh"
#include "cspad-gjw/CspadCorrector.hh"
#include "cspad-gjw/CspadGeometry.hh"

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
	strcpy(configFile, "cspad-cryst.ini");

	// Geometry
	strcpy(geometryFile, "geometry/cspad_pixelmap.h5");
	pixelSize = 110e-6;
	
	// Bad pixel mask
	strcpy(badpixelFile, "badpixels.h5");
	useBadPixelMask = 0;

	// Static dark calibration (electronic offsets)
	strcpy(darkcalFile, "darkcal.h5");
	useDarkcalSubtraction = 1;
	generateDarkcal = 0;
	
	// Common mode subtraction from each ASIC
	cmModule = 0;
	cmFloor = 0.1;

	// Gain calibration correction
	strcpy(gaincalFile, "gaincal.h5");
	useGaincal = 0;
	invertGain = 0;
	
	// Subtraction of running background (persistent photon background) 
	useSubtractPersistentBackground = 0;
	subtractBg = 0;
	bgMemory = 50;
	startFrames = 0;
	scaleBackground = 0;
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
	hitfinderNpeaks = 50;
	hitfinderNpeaksMax = 100000;
	hitfinderAlgorithm = 3;
	hitfinderMinPixCount = 3;
	hitfinderMaxPixCount = 20;
	hitfinderUsePeakmask = 0;
	strcpy(peaksearchFile, "peakmask.h5");
	savePeakInfo = 1;

	// Powder pattern generation
	powdersum = 1;
	powderthresh = -20000;
	powderSumHits = 1;
	powderSumBlanks = 0;

	
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

	
	// Default to only a few threads
	nThreads = 16;
	useHelperThreads = 0;
	threadPurge = 10000;
	
	// Log files
	strcpy(logfile, "log.txt");
	strcpy(framefile, "frames.txt");
	strcpy(cleanedfile, "cleaned.txt");
	strcpy(peaksfile, "peaks.txt");
	
	
}



/*
 *	Setup stuff to do with thread management, settings, etc.
 */
void cGlobal::setup() {
	/*
	 *	Set up arrays for remembering powder data, background, etc.
	 */
	selfdark = (float*) calloc(pix_nn, sizeof(float));
	powderHitsRaw = (double*) calloc(pix_nn, sizeof(double));
	powderBlanksRaw = (double*) calloc(pix_nn, sizeof(double));
	powderHitsAssembled = (double*) calloc(image_nn, sizeof(double));
	powderBlanksAssembled = (double*) calloc(image_nn, sizeof(double));
	bg_buffer = (int16_t*) calloc(bgMemory*pix_nn, sizeof(int16_t)); 
	hotpix_buffer = (int16_t*) calloc(hotpixMemory*pix_nn, sizeof(int16_t)); 
	hotpixelmask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0; i<pix_nn; i++) {
		selfdark[i] = 0;
		powderHitsRaw[i] = 0;
		powderBlanksRaw[i] = 0;
		hotpixelmask[i] = 1;
	}
	for(long i=0; i<image_nn; i++) {
		powderHitsAssembled[i] = 0;
		powderBlanksAssembled[i] = 0;
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
	pthread_mutex_init(&powderHitsRaw_mutex, NULL);
	pthread_mutex_init(&powderHitsAssembled_mutex, NULL);
	pthread_mutex_init(&powderBlanksRaw_mutex, NULL);
	pthread_mutex_init(&powderBlanksAssembled_mutex, NULL);
	pthread_mutex_init(&nhits_mutex, NULL);
	pthread_mutex_init(&framefp_mutex, NULL);
	pthread_mutex_init(&peaksfp_mutex, NULL);
	threadID = (pthread_t*) calloc(nThreads, sizeof(pthread_t));

	
	/*
	 *	Trap specific configurations and mutually incompatible options
	 */
	if(generateDarkcal) {
		cmModule = 0;
		subtractBg = 0;
		useDarkcalSubtraction = 0;
		useSubtractPersistentBackground = 0;
		hitfinder = 0;
		savehits = 0;
		hdf5dump = 0;
		saveRaw = 0;
		saveDetectorRaw = 1;
		powderSumHits = 1;
		powderSumBlanks = 1;
		powderthresh = -20000;
		useAutoHotpixel = 0;
		startFrames = 0;
		powderthresh = 0;
	}
	
	if(saveRaw==0 && saveAssembled == 0) {
		saveAssembled = 1;
	}
	
	
	
	
	/*
	 *	Other stuff
	 */
	npowderHits = 0;
	npowderBlanks = 0;
	nprocessedframes = 0;
	nhits = 0;
	lastclock = clock()-10;
	datarate = 1;
	detectorZ = 0;
	runNumber = getRunNumber();
	time(&tstart);
	avgGMD = 0;
	bgCounter = 0;
	last_bg_update = 0;
	hotpixCounter = 0;
	last_hotpix_update = 0;
	hotpixRecalc = bgRecalc;
	nhot = 0;
	
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
	printf("Parsing input configuration file:\n",filename);
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
	if (!strcmp(tag, "nthreads")) {
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
	else if (!strcmp(tag, "usegaincal")) {
		useGaincal = atoi(value);
	}
	else if (!strcmp(tag, "invertgain")) {
		invertGain = atoi(value);
	}
	else if (!strcmp(tag, "generatedarkcal")) {
		generateDarkcal = atoi(value);
	}
	else if (!strcmp(tag, "subtractbg")) {
		subtractBg = atoi(value);
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
		powdersum = atoi(value);
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
	else if (!strcmp(tag, "useselfdarkcal")) {
		useSubtractPersistentBackground = atoi(value);
	}
	else if (!strcmp(tag, "usesubtractpersistentbackground")) {
		useSubtractPersistentBackground = atoi(value);
	}
	
	// Local background subtraction
	else if (!strcmp(tag, "uselocalbackgroundsubtraction")) {
		useLocalBackgroundSubtraction = atoi(value);
	}
	else if (!strcmp(tag, "localbackgroundradius")) {
		localBackgroundRadius = atoi(value);
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
	
	
	
	else if (!strcmp(tag, "hitfinderusepeakmask")) {
		hitfinderUsePeakmask = atoi(value);
	}

	// Backgrounds
	else if (!strcmp(tag, "selfdarkmemory")) {
		bgMemory = atof(value);
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
		scaleBackground = atoi(value);
	}
	else if (!strcmp(tag, "startframes")) {
		startFrames = atoi(value);
	}
	
	
	
	// Unknown tags
	else {
		printf("\tUnknown tag (ignored): %s = %s\n",tag,value);
	}
}



/*
 *	Read in detector configuration
 */
void cGlobal::readDetectorGeometry(char* filename) {
	

	// Pixel size (measurements in geometry file are in m)
	module_rows = ROWS;
	module_cols = COLS;	
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
	if (detector_x.nx != 8*ROWS || detector_x.ny != 8*COLS) {
		printf("readDetectorGeometry: array size mismatch\n");
		printf("%ix%i != %ix%i\n", 8*ROWS, 8*COLS, detector_x.nx, detector_x.ny);
		exit(1);
	}
	
	
	// Create local arrays for detector pixel locations
	long	nx = 8*ROWS;
	long	ny = 8*COLS;
	long	nn = nx*ny;
	pix_nx = nx;
	pix_ny = ny;
	pix_nn = nn;
	pix_x = (float *) calloc(nn, sizeof(float));
	pix_y = (float *) calloc(nn, sizeof(float));
	pix_z = (float *) calloc(nn, sizeof(float));
	printf("\tPixel map is %li x %li pixel array\n",nx,ny);
	
	
	// Copy values from 2D array
	for(long i=0;i<nn;i++){
		pix_x[i] = (float) detector_x.data[i];
		pix_y[i] = (float) detector_y.data[i];
		pix_z[i] = (float) detector_z.data[i];
	}
	
	
	// Divide array (in m) by pixel size to get pixel location indicies (ijk)
	for(long i=0;i<nn;i++){
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
	printf("\tImage output array will be %i x %i\n",image_nx,image_nx);
	
}


/*
 *	Read in darkcal file
 */
void cGlobal::readDarkcal(char *filename){
	
	printf("Reading darkcal configuration:\n");
	printf("\t%s\n",filename);
	
	
	// Create memory space and pad with zeros
	darkcal = (int32_t*) calloc(pix_nn, sizeof(int32_t));
	memset(darkcal,0, pix_nn*sizeof(int32_t));
	
	
	
	// Check whether pixel map file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tDarkcal file does not exist: %s\n",filename);
		printf("\tDefaulting to all-zero darkcal\n");
		return;
	}
	
	
	// Read darkcal data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	
	// Correct geometry?
	if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
		printf("\tGeometry mismatch: %ix%x != %ix%i\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
		printf("\tDefaulting to all-zero darkcal\n");
		return;
	} 
	
	// Copy into darkcal array
	for(long i=0;i<pix_nn;i++)
		darkcal[i] = (int32_t) temp2d.data[i];
	
}


/*
 *	Read in gaincal file
 */
void cGlobal::readGaincal(char *filename){
	
	printf("Reading detector gain calibration:\n");
	printf("\t%s\n",filename);
	
	
	// Create memory space and set default gain to 1 everywhere
	gaincal = (float*) calloc(pix_nn, sizeof(float));
	for(long i=0;i<pix_nn;i++)
		gaincal[i] = 1;
	
		
	// Check whether gain calibration file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tGain calibration file does not exist: %s\n",filename);
		printf("\tDefaulting to uniform gaincal\n");
		return;
	}
	
	
	// Read darkcal data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	

	// Correct geometry?
	if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
		printf("\tGeometry mismatch: %ix%x != %ix%i\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
		printf("\tDefaulting to uniform gaincal\n");
		return;
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
	
	printf("Reading peak search mask:\n");
	printf("\t%s\n",filename);
	
	
	// Create memory space and default to searching for peaks everywhere
	peakmask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0;i<pix_nn;i++)
		peakmask[i] = 1;
	
	
	// Check whether file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tPeak search mask does not exist: %s\n",filename);
		printf("\tDefaulting to uniform search mask\n");
		return;
	}
	
	
	// Read darkcal data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	
	
	// Correct geometry?
	if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
		printf("\tGeometry mismatch: %ix%x != %ix%i\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
		printf("\tDefaulting to uniform peak search mask\n");
		return;
	} 
	
	
	// Copy into darkcal array
	for(long i=0;i<pix_nn;i++)
		peakmask[i] = (int16_t) temp2d.data[i];
}


/*
 *	Read in bad pixel mask
 */
void cGlobal::readBadpixelMask(char *filename){
	
	printf("Reading bad pixel mask:\n");
	printf("\t%s\n",filename);
	
	
	// Create memory space and default to searching for peaks everywhere
	badpixelmask = (int16_t*) calloc(pix_nn, sizeof(int16_t));
	for(long i=0;i<pix_nn;i++)
		badpixelmask[i] = 1;
	
	
	// Check whether file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tPeak search mask does not exist: %s\n",filename);
		printf("\tDefaulting to uniform search mask\n");
		return;
	}
	
	
	// Read darkcal data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	
	
	// Correct geometry?
	if(temp2d.nx != pix_nx || temp2d.ny != pix_ny) {
		printf("\tGeometry mismatch: %ix%x != %ix%i\n",temp2d.nx, temp2d.ny, pix_nx, pix_ny);
		printf("\tDefaulting to uniform peak search mask\n");
		return;
	} 
	
	
	// Copy back into array
	for(long i=0;i<pix_nn;i++)
		badpixelmask[i] = (int16_t) temp2d.data[i];
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

	fprintf(fp, "\nIni parameters:\n");
	fprintf(fp, "nthreads=%d\n",nThreads);
	fprintf(fp, "geometry=%s\n",geometryFile);
	fprintf(fp, "darkcal=%s\n",darkcalFile);
	fprintf(fp, "gaincal=%s\n",gaincalFile);
	fprintf(fp, "peakmask=%s\n",peaksearchFile);
	fprintf(fp, "badpixelmask=%s\n",badpixelFile);
	
	fprintf(fp, "\n#Processing options\n\n");
	fprintf(fp, "subtractcmmodule=%d\n",cmModule);
	fprintf(fp, "cmmodule=%d\n",cmModule);
	fprintf(fp, "usegaincal=%d\n",useGaincal);
	fprintf(fp, "invertgain=%d\n",invertGain);
	fprintf(fp, "generatedarkcal=%d\n",generateDarkcal);
	fprintf(fp, "subtractbg=%d\n",subtractBg);
	fprintf(fp, "usebadpixelmask=%d\n",useBadPixelMask);
	fprintf(fp, "usedarkcalsubtraction=%d\n",useDarkcalSubtraction);
	fprintf(fp, "hitfinder=%d\n",hitfinder);
	fprintf(fp, "savepeakinfo=%d\n",savePeakInfo);
	fprintf(fp, "savehits=%d\n",savehits);
	fprintf(fp, "powdersum=%d\n",powdersum);
	fprintf(fp, "saveraw=%d\n",saveRaw);
	fprintf(fp, "hdf5dump=%d\n",hdf5dump);
	fprintf(fp, "saveinterval=%d\n",saveInterval);
	fprintf(fp, "useautohotpixel=%d\n",useAutoHotpixel);
	fprintf(fp, "useselfdarkcal=%d\n",useSubtractPersistentBackground);
	fprintf(fp, "subtractpersistentbackground=%d\n",useSubtractPersistentBackground);
	
	fprintf(fp, "\n#Power user settings\n\n");
	fprintf(fp, "cmfloor=%f\n",cmFloor);
	fprintf(fp, "pixelsize=%f\n",pixelSize);
	fprintf(fp, "debuglevel=%d\n",debugLevel);
	fprintf(fp, "hotpixfreq=%f\n",hotpixFreq);
	fprintf(fp, "hotpixadc=%d\n",hotpixADC);
	fprintf(fp, "hotpixmemory=%d\n",hotpixMemory);
	fprintf(fp, "powderthresh=%d\n",powderthresh);
	fprintf(fp, "hitfinderadc=%d\n",hitfinderADC);
	fprintf(fp, "hitfindernat=%d\n",hitfinderNAT);
	fprintf(fp, "hitfindercluster=%d\n",hitfinderCluster);
	fprintf(fp, "hitfindernpeaks=%d\n",hitfinderNpeaks);
	fprintf(fp, "hitfindernpeaksmax=%d\n",hitfinderNpeaksMax);
	fprintf(fp, "hitfinderalgorithm=%d\n",hitfinderAlgorithm);
	fprintf(fp, "hitfinderminpixcount=%d\n",hitfinderMinPixCount);
	fprintf(fp, "hitfindermaxpixcount=%d\n",hitfinderMaxPixCount);
	fprintf(fp, "hitfinderusepeakmask=%d\n",hitfinderUsePeakmask);
	fprintf(fp, "selfdarkmemory=%f\n",bgMemory);
	fprintf(fp, "bgmemory=%f\n",bgMemory);
	fprintf(fp, "scaleBackground=%d\n",scaleBackground);
	fprintf(fp, "scaleDarkcal=%d\n",scaleBackground);
	fprintf(fp, "startframes=%d\n",startFrames);
	
	
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
	fprintf(framefp, "# FrameNumber, UnixTime, EventName, npeaks\n");
	
	sprintf(cleanedfile,"cleaned.txt");
	cleanedfp = fopen (cleanedfile,"w");
	if(cleanedfp == NULL) {
		printf("Error: Can not open %s for writing\n",cleanedfile);
		printf("Aborting...");
		exit(1);
	}
	fprintf(cleanedfp, "# Filename, npeaks\n");	
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
	
	// Calculate hit rate
	float hitrate;
	hitrate = 100.*( nhits / (float) nprocessedframes);
	
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
	fprintf(fp, "nFrames: %i,  nHits: %i (%2.2f%%), wallTime: %ihr %imin %isec (%2.1f fps)\n", nprocessedframes, nhits, hitrate, hrs, mins, secs, fps);
	fclose (fp);
	
	
	// Flush frame file buffer
	pthread_mutex_lock(&framefp_mutex);
	//fclose(framefp);
	//framefp = fopen (framefile,"a");
	//fclose(cleanedfp);
	//cleanedfp = fopen (cleanedfile,"a");
	fflush(framefp);
	fflush(cleanedfp);
	pthread_mutex_unlock(&framefp_mutex);
	
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
	fprintf(fp, "Frames processed: %i\n",nprocessedframes);
	fprintf(fp, "nFrames in hits powder pattern: %i\n",npowderHits);
	fprintf(fp, "nFrames in blanks powder pattern: %i\n",npowderBlanks);
	fprintf(fp, "Number of hits: %i\n",nhits);
	fprintf(fp, "Average hit rate: %2.2f %%\n",hitrate);
	fprintf(fp, "Average frame rate: %2.2f fps\n",fps);
	fprintf(fp, "Average data rate: %2.2f MB/sec\n",mbs);
	
	fclose (fp);

	
	// Flush frame file buffer
	pthread_mutex_lock(&framefp_mutex);
	fclose(framefp);
	fclose(cleanedfp);
	pthread_mutex_unlock(&framefp_mutex);
	
	pthread_mutex_lock(&peaksfp_mutex);
	fclose(peaksfp);
	pthread_mutex_unlock(&peaksfp_mutex);
	
	
}

