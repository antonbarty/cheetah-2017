/*
 *  setup.cpp
 *  cspad_cryst
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
#include <math.h>
#include <hdf5.h>
#include <fenv.h>

#include "setup.h"
#include "worker.h"
#include "data2d.h"


/*
 *	Default settings/configuration
 */
void cGlobal::defaultConfiguration(void) {

	// Default processing options
	cmModule = 1;
	cmSubModule = 0;
	subtractBg = 0;
	subtractDarkcal = 0;
	selfDarkcal = 0;
	hitfinder = 0;
	savehits = 0;
	hdf5dump = 0;
	powdersum = 1;
	saveRaw = 0;
	debugLevel = 2;
	autohotpixel = 1;
	startFrames = 0;
	
	// Power user settings
	cmFloor = 0.1;
	saveInterval = 0;
	powderthresh = 0;
	hitfinderADC = 100;
	hitfinderNAT = 100;
	hotpixFreq = 0.9;
	hotpixADC = 1000;
	hotpixMemory = 50;
	selfDarkMemory = 100;
	scaleDarkcal = 0;
	avgGMD = 0;
	

	// Default to single-threaded
	nThreads = 16;

	
	// Default configuration files and timezone
	strcpy(configFile, "cspad-cryst.ini");
	strcpy(geometryFile, "geometry/cspad_pixelmap.h5");
	strcpy(darkcalFile, "darkcal.h5");
	strcpy(logfile, "log.txt");
	setenv("TZ","US/Pacific",1);
	npowder = 0;
	nprocessedframes = 0;
	nhits = 0;
	lastclock = clock()-10;
	datarate = 1;
	time(&tstart);
	
	
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
 *	Setup stuff to do with thread management, settings, etc.
 */
void cGlobal::setupThreads() {
	
	nActiveThreads = 0;
	threadCounter = 0;
	
	pthread_mutex_init(&nActiveThreads_mutex, NULL);
	pthread_mutex_init(&hotpixel_mutex, NULL);
	pthread_mutex_init(&selfdark_mutex, NULL);
	pthread_mutex_init(&powdersum1_mutex, NULL);
	pthread_mutex_init(&powdersum2_mutex, NULL);
	pthread_mutex_init(&nhits_mutex, NULL);
	threadID = (pthread_t*) calloc(nThreads, sizeof(pthread_t));
	for(int i=0; i<nThreads; i++) 
		threadID[i] = -1;
	
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
	else if (!strcmp(tag, "geometry")) {
		strcpy(geometryFile, value);
	}
	else if (!strcmp(tag, "darkcal")) {
		strcpy(darkcalFile, value);
	}
	
	// Processing options
	else if (!strcmp(tag, "subtractcmmodule")) {
		cmModule = atoi(value);
	}
	else if (!strcmp(tag, "cmmodule")) {
		cmModule = atoi(value);
	}
	else if (!strcmp(tag, "subtractcmsubmodule")) {
		cmSubModule = atoi(value);
	}
	else if (!strcmp(tag, "subtractbg")) {
		subtractBg = atoi(value);
	}
	else if (!strcmp(tag, "subtractdarkcal")) {
		subtractDarkcal = atoi(value);
	}
	else if (!strcmp(tag, "hitfinder")) {
		hitfinder = atoi(value);
	}
	else if (!strcmp(tag, "savehits")) {
		savehits = atoi(value);
	}
	else if (!strcmp(tag, "powdersum")) {
		powdersum = atoi(value);
	}
	else if (!strcmp(tag, "saveraw")) {
		saveRaw = atoi(value);
	}
	else if (!strcmp(tag, "hdf5dump")) {
		hdf5dump = atoi(value);
	}
	else if (!strcmp(tag, "saveinterval")) {
		saveInterval = atoi(value);
	}
	else if (!strcmp(tag, "autohotpixel")) {
		autohotpixel = atoi(value);
	}
	else if (!strcmp(tag, "selfdarkcal")) {
		selfDarkcal = atoi(value);
	}
	

	// Power user settings
	else if (!strcmp(tag, "cmfloor")) {
		cmFloor = atof(value);
	}
	else if (!strcmp(tag, "debuglevel")) {
		debugLevel = atoi(value);
	}
	else if (!strcmp(tag, "hotpixfreq")) {
		hotpixFreq = atof(value);
	}
	else if (!strcmp(tag, "hotpixadc")) {
		hotpixADC = atof(value);
	}
	else if (!strcmp(tag, "hotpixmemory")) {
		hotpixMemory = atof(value);
	}
	else if (!strcmp(tag, "powderthresh")) {
		powderthresh = atoi(value);
	}
	else if (!strcmp(tag, "hitfinderadc")) {
		hitfinderADC = atoi(value);
	}
	else if (!strcmp(tag, "hitfindernat")) {
		hitfinderNAT = atoi(value);
	}
	else if (!strcmp(tag, "selfdarkmemory")) {
		selfDarkMemory = atof(value);
	}
	else if (!strcmp(tag, "scaledarkcal")) {
		scaleDarkcal = atoi(value);
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
	pix_dx = 110e-6;

	
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
	cData2d		dark2d;
	dark2d.readHDF5(filename);
	
	
	// Copy into darkcal array
	for(long i=0;i<pix_nn;i++)
		darkcal[i] = (int32_t) dark2d.data[i];
	
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

	fp = fopen (logfile,"w");
	fprintf(fp, "start time: %s\n",timestr);
	fprintf(fp, ">-------- Start of job --------<\n");

	fclose (fp);
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
	
	
	// Logfile name
	printf("Writing log file: %s\n", logfile);
	fp = fopen (logfile,"a");
	fprintf(fp, "nFrames: %i,  nHits: %i (%2.2f%%), wallTime: %ihr %imin %isec (%2.1fps)\n", nprocessedframes, nhits, hitrate, hrs, mins, secs, fps);
	fclose (fp);
	
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
	float	fps;
	fps = nprocessedframes / dtime;
				 
				 
	
	// Save log file
	fprintf(fp, ">-------- End of job --------<\n");
	fprintf(fp, "End time: %s\n",timestr);
	fprintf(fp, "Elapsed time: %ihr %imin %isec\n",hrs,mins,secs);
	fprintf(fp, "Frames processed: %i\n",nprocessedframes);
	fprintf(fp, "nFrames in powder pattern: %i\n",npowder);
	fprintf(fp, "Number of hits: %i\n",nhits);
	fprintf(fp, "Average hit rate: %2.2f %%\n",hitrate);
	fprintf(fp, "Average data rate: %2.2f fps\n",fps);

	fclose (fp);

	
}

