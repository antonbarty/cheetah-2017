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
#include <hdf5.h>

#include "worker.h"
#include "data2d.h"



/*
 *	Setup stuff to do with thread management, settings, etc.
 */
void setupThreads(tGlobal *global) {

	global->nThreads = 2;
	global->nActiveThreads = 0;

	global->module_rows = ROWS;
	global->module_cols = COLS;	

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
	global->pix_dx = 100e-6;

	
	// Set filename here 
	char	filename[1024];
	printf("\tReading detector configuration:\n");
	strcpy(filename,"cspad_pixelmap.h5");
	printf("\t%s\n",filename);
	
	
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
	global->pix_nx = nx;
	global->pix_ny = ny;
	global->pix_nn = nn;
	global->pix_x = (float *) calloc(nn, sizeof(float));
	global->pix_y = (float *) calloc(nn, sizeof(float));
	global->pix_z = (float *) calloc(nn, sizeof(float));
	printf("\t%li x %li pixel array\n",nx,ny);
	
	
	// Copy values from 2D array
	for(long i=0;i<nn;i++){
		global->pix_x[i] = (float) detector_x.data[i];
		global->pix_y[i] = (float) detector_y.data[i];
		global->pix_z[i] = (float) detector_z.data[i];
	}
}
