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

#include "worker.h"
#include "data2d.h"


/*
 *	Settings/configuration
 */
void globalConfiguration(tGlobal *global) {

	setenv("TZ","US/Pacific",1);
	global->nThreads = 1;

}



/*
 *	Setup stuff to do with thread management, settings, etc.
 */
void setupThreads(tGlobal *global) {

	global->nActiveThreads = 0;
	global->threadCounter = 0;

	pthread_mutex_init(&global->nActiveThreads_mutex, NULL);
	global->threadID = (pthread_t*) calloc(global->nThreads, sizeof(pthread_t));
	for(int i=0; i<global->nThreads; i++) 
		global->threadID[i] = -1;

}



/*
 *	Read in detector configuration
 */
void readDetectorGeometry(tGlobal *global) {
	

	// Pixel size (measurements in geometry file are in m)
	global->module_rows = ROWS;
	global->module_cols = COLS;	
	global->pix_dx = 100e-6;

	
	// Set filename here 
	char	detfile[1024];
	printf("\tReading detector configuration:\n");
	strcpy(detfile,"cspad_pixelmap.h5");
	printf("\t%s\n",detfile);
	
	
	// Check whether pixel map file exists!
	FILE* fp = fopen(detfile, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("Error: Detector configuration file does not exist: %s\n",detfile);
		exit(1);
	}
	
	
	// Read pixel locations from file
	cData2d		detector_x;
	cData2d		detector_y;
	cData2d		detector_z;
	detector_x.readHDF5(detfile, (char *) "x");
	detector_y.readHDF5(detfile, (char *) "y");
	detector_z.readHDF5(detfile, (char *) "z");
	
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
	global->pix_nx = nx;
	global->pix_ny = ny;
	global->pix_nn = nn;
	global->pix_x = (float *) calloc(nn, sizeof(float));
	global->pix_y = (float *) calloc(nn, sizeof(float));
	global->pix_z = (float *) calloc(nn, sizeof(float));
	printf("\tPixel map is %li x %li pixel array\n",nx,ny);
	
	
	// Copy values from 2D array
	for(long i=0;i<nn;i++){
		global->pix_x[i] = (float) detector_x.data[i];
		global->pix_y[i] = (float) detector_y.data[i];
		global->pix_z[i] = (float) detector_z.data[i];
	}
	
	
	// Divide array (in m) by pixel size to get pixel location indicies (ijk)
	for(long i=0;i<nn;i++){
		global->pix_x[i] /= global->pix_dx;
		global->pix_y[i] /= global->pix_dx;
		global->pix_z[i] /= global->pix_dx;
	}
	
	
	// Find bounds of image array
	float	xmax = -1e9;
	float	xmin =  1e9;
	float	ymax = -1e9;
	float	ymin =  1e9;
	for(long i=0;i<nn;i++){
		if (global->pix_x[i] > xmax) xmax = global->pix_x[i];
		if (global->pix_x[i] < xmin) xmin = global->pix_x[i];
		if (global->pix_y[i] > ymax) ymax = global->pix_y[i];
		if (global->pix_y[i] < ymin) ymin = global->pix_y[i];
	}
	xmax = ceil(xmax);
	xmin = floor(xmin);
	ymax = ceil(ymax);
	ymin = floor(ymin);
	printf("\tImage bounds:\n");
	printf("\tx range %f to %f\n",xmin,xmax);
	printf("\ty range %f to %f\n",ymin,ymax);
	
	
	// How big must the output image be?
	float max = xmax;
	if(ymax > max) max = ymax;
	if(fabs(xmin) > max) max = fabs(xmin);
	if(fabs(ymin) > max) max = fabs(ymin);
	global->image_nx = 2*(unsigned)max;
	global->image_nn = global->image_nx*global->image_nx;
	printf("\tImage output array will be %i x %i\n",global->image_nx,global->image_nx);
	
}
