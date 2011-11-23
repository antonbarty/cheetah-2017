/*
 *  powder.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 23/11/11.
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
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "setup.h"
#include "worker.h"
#include "median.h"



/*
 *	Maintain running powder patterns
 *	(currently supports both old and new ways of writing out powder data)
 */
void addToPowder(tThreadInfo *threadInfo, cGlobal *global, int powderClass){
	
	double  *buffer;
	
	
	// Raw data
	pthread_mutex_lock(&global->powderRaw_mutex[powderClass]);
	for(long i=0; i<global->pix_nn; i++) {
		if(threadInfo->corrected_data[i] > global->powderthresh)
			global->powderRaw[powderClass][i] += threadInfo->corrected_data[i];
	}
	pthread_mutex_unlock(&global->powderRaw_mutex[powderClass]);			
	
	
	// Raw data squared (for calculating variance)
	buffer = (double*) calloc(global->pix_nn, sizeof(double));
	for(long i=0; i<global->pix_nn; i++) 
		buffer[i] = 0;
	for(long i=0; i<global->pix_nn; i++){
		if(threadInfo->corrected_data[i] > global->powderthresh)
			buffer[i] = (threadInfo->corrected_data[i])*(threadInfo->corrected_data[i]);
	}
	pthread_mutex_lock(&global->powderRawSquared_mutex[powderClass]);
	for(long i=0; i<global->pix_nn; i++) 
		global->powderRawSquared[powderClass][i] += buffer[i];
	pthread_mutex_unlock(&global->powderRawSquared_mutex[powderClass]);			
	free(buffer);
	
	
	// Assembled data
	pthread_mutex_lock(&global->powderAssembled_mutex[powderClass]);
	for(long i=0; i<global->image_nn; i++){
		if(threadInfo->image[i] > global->powderthresh)
			global->powderAssembled[powderClass][i] += threadInfo->image[i];
	}
	pthread_mutex_unlock(&global->powderAssembled_mutex[powderClass]);
	
	// Update frame counter
	global->nPowderFrames[powderClass] += 1;
	
	
}


void saveRunningSums(cGlobal *global) {
	char	filename[1024];
	char	filenamebase[1024];
	double	*radialAverage = (double*) calloc(global->radial_nn, sizeof(double));
	double	*radialAverageCounter = (double*) calloc(global->radial_nn, sizeof(double));
	
	
	/*
	 *	Save powder patterns from different classes
	 */
	printf("Writing intermediate powder patterns to file\n");
	for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
		double *buffer;
		
		sprintf(filenamebase,"r%04u-class%i", global->runNumber, powderType);
		//sprintf(filenamebase,"r%04u-class%i-%06i", global->runNumber, powderType, global->nprocessedframes);
		
		// Raw data
		sprintf(filename,"%s-sumRaw.h5",filenamebase);
		buffer = (double*) calloc(global->pix_nn, sizeof(double));
		pthread_mutex_lock(&global->powderRaw_mutex[powderType]);
		memcpy(buffer, global->powderRaw[powderType], global->pix_nn*sizeof(double));
		calculateRadialAverage(buffer, radialAverage, radialAverageCounter, global);
		pthread_mutex_unlock(&global->powderRaw_mutex[powderType]);
		writePowderData(filename, buffer, global->pix_nx, global->pix_ny, radialAverage, radialAverageCounter, global->radial_nn, global->nPowderFrames[powderType], H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		// Assembled sum
		sprintf(filename,"%s-sumAssembled.h5", filenamebase);
		buffer = (double*) calloc(global->image_nn, sizeof(double));
		pthread_mutex_lock(&global->powderAssembled_mutex[powderType]);
		memcpy(buffer, global->powderAssembled[powderType], global->image_nn*sizeof(double));
		pthread_mutex_unlock(&global->powderAssembled_mutex[powderType]);
		writePowderData(filename, buffer, global->image_nx, global->image_nx, radialAverage, radialAverageCounter, global->radial_nn, global->nPowderFrames[powderType], H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		// Blanks squared (for calculation of variance)
		sprintf(filename,"%s-sumRawSquared.h5",filenamebase);
		buffer = (double*) calloc(global->pix_nn, sizeof(double));
		pthread_mutex_lock(&global->powderRawSquared_mutex[powderType]);
		memcpy(buffer, global->powderRawSquared[powderType], global->pix_nn*sizeof(double));
		calculateRadialAverage(buffer, radialAverage, radialAverageCounter, global);
		pthread_mutex_unlock(&global->powderRawSquared_mutex[powderType]);
		writePowderData(filename, buffer, global->pix_nx, global->pix_ny, radialAverage, radialAverageCounter, global->radial_nn, global->nPowderFrames[powderType], H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		// Sigma (variance)
		sprintf(filename,"%s-sumRawSigma.h5",filenamebase);
		buffer = (double*) calloc(global->pix_nn, sizeof(double));
		pthread_mutex_lock(&global->powderRaw_mutex[powderType]);
		pthread_mutex_lock(&global->powderRawSquared_mutex[powderType]);
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = sqrt(global->powderRawSquared[powderType][i]/global->nPowderFrames[powderType] - (global->powderRaw[powderType][i]*global->powderRaw[powderType][i]/(global->nPowderFrames[powderType]*global->nPowderFrames[powderType])));
		calculateRadialAverage(buffer, radialAverage, radialAverageCounter, global);
		pthread_mutex_unlock(&global->powderRaw_mutex[powderType]);
		pthread_mutex_unlock(&global->powderRawSquared_mutex[powderType]);
		writePowderData(filename, buffer, global->pix_nx, global->pix_ny, radialAverage, radialAverageCounter, global->radial_nn, global->nPowderFrames[powderType], H5T_NATIVE_DOUBLE);	
		free(buffer);
		
	}	
	free(radialAverage);
	free(radialAverageCounter);
	
	
	/*
	 *	Compute and save darkcal
	 */
	if(global->generateDarkcal) {
		printf("Processing darkcal\n");
		sprintf(filename,"r%04u-darkcal.h5",global->runNumber);
		int16_t *buffer = (int16_t*) calloc(global->pix_nn, sizeof(int16_t));
		pthread_mutex_lock(&global->powderRaw_mutex[1]);
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = (int16_t) lrint(global->powderRaw[1][i]/global->nPowderFrames[1]);
		pthread_mutex_unlock(&global->powderRaw_mutex[1]);
		printf("Saving darkcal to file: %s\n", filename);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_STD_I16LE);	
		free(buffer);
	}
	
	/*
	 *	Compute and save gain calibration
	 */
	else if(global->generateGaincal) {
		printf("Processing gaincal\n");
		sprintf(filename,"r%04u-gaincal.h5",global->runNumber);
		// Calculate average intensity per frame
		pthread_mutex_lock(&global->powderRaw_mutex[1]);
		double *buffer = (double*) calloc(global->pix_nn, sizeof(double));
		for(long i=0; i<global->pix_nn; i++)
			buffer[i] = (global->powderRaw[1][i]/global->nPowderFrames[1]);
		pthread_mutex_unlock(&global->powderRaw_mutex[1]);
		
		// Find median value (this value will become gain=1)
		float *buffer2 = (float*) calloc(global->pix_nn, sizeof(float));
		for(long i=0; i<global->pix_nn; i++) {
			buffer2[i] = buffer[i];
		}
		float	dc;
		dc = kth_smallest(buffer2, global->pix_nn, lrint(0.5*global->pix_nn));
		printf("offset=%f\n",dc);
		free(buffer2);
		if(dc <= 0){
			printf("Error calculating gain, offset = %f\n",dc);
			return;
		}
		// gain=1 for a median value pixel, and is bounded between a gain of 0.1 and 10
		for(long i=0; i<global->pix_nn; i++) {
			buffer[i] /= (double) dc;
			if(buffer[i] < 0.1 || buffer[i] > 10)
				buffer[i]=0;
		}
		printf("Saving gaincal to file: %s\n", filename);
		writeSimpleHDF5(filename, buffer, global->pix_nx, global->pix_ny, H5T_NATIVE_DOUBLE);	
		free(buffer);
		
		
	}
}



void writePowderData(char *filename, void *data, int width, int height, void *radialAverage, void *radialAverageCounter, long radial_nn, long nFrames, int type) 
{
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	herr_t r;
	hsize_t size[2];
	hsize_t max_size[2];
	
	fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if ( fh < 0 ) {
		ERROR("Couldn't create file: %s\n", filename);
	}
	
	gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gh < 0 ) {
		ERROR("Couldn't create group\n");
		H5Fclose(fh);
	}
	
	// Write image data
	size[0] = height;
	size[1] = width;
	max_size[0] = height;
	max_size[1] = width;
	sh = H5Screate_simple(2, size, NULL);
	dh = H5Dcreate(gh, "data", type, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	
	H5Sget_simple_extent_dims(sh, size, max_size);
	H5Dwrite(dh, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	H5Dclose(dh);
	
	
	// Save radial averages
	size[0] = radial_nn;
	sh = H5Screate_simple(1, size, NULL);
	
	dh = H5Dcreate(gh, "radialAverage", type, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverage);
	H5Dclose(dh);
	
	dh = H5Dcreate(gh, "radialAverageCounter", type, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCounter);
	H5Dclose(dh);	
	H5Sclose(sh);
	
	
	// Save frame count
	size[0] = 1;
	sh = H5Screate_simple(1, size, NULL );
	//sh = H5Screate(H5S_SCALAR);
	
	dh = H5Dcreate(gh, "nframes", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &nFrames );
	H5Dclose(dh);
	H5Sclose(sh);
	
	
	// Close group
	H5Gclose(gh);
	
	
	// Clean up stale HDF5 links
	int n_ids;
	hid_t ids[256];
	n_ids = H5Fget_obj_ids(fh, H5F_OBJ_ALL, 256, ids);
	for ( int i=0; i<n_ids; i++ ) {
		hid_t id;
		H5I_type_t type;
		id = ids[i];
		type = H5Iget_type(id);
		if ( type == H5I_GROUP ) H5Gclose(id);
		if ( type == H5I_DATASET ) H5Dclose(id);
		if ( type == H5I_DATATYPE ) H5Tclose(id);
		if ( type == H5I_DATASPACE ) H5Sclose(id);
		if ( type == H5I_ATTR ) H5Aclose(id);
	}
	
	
	H5Fclose(fh);
}

