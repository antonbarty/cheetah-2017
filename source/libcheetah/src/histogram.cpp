//
//  histogram.cpp
//  cheetah
//
//  Created by Anton Barty on 20/5/13.
//  Copyright (c) 2013 Anton Barty. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"


/*
 *	Maintain running powder patterns
 */

void addToHistogram(cEventData *eventData, cGlobal *global) {
	   
	DETECTOR_LOOP {
		if (global->detector[detID].histogram) {
			addToHistogram(eventData, global, detID);
		}
	}
}


void addToHistogram(cEventData *eventData, cGlobal *global, int detID) {
	
	// Dereference common variables
	long		pix_nn = global->detector[detID].pix_nn;
	long		histMin = global->detector[detID].histogramMin;
	long		histMax = global->detector[detID].histogramMax;
	long		histStep = global->detector[detID].histogramBinSize;
	long		histDepth = global->detector[detID].histogram_depth;
	uint16_t	*histogramData = global->detector[detID].histogramData;

	
	// Figure out bin for each pixel outside of mutex lock
	long	*buffer;
	long	bin;
	float	value;
	buffer = (long *) calloc(pix_nn, sizeof(long));
	for(long i=0; i<pix_nn; i++) {
		value = eventData->detector[detID].corrected_data[i];
		bin = floor((value-histMin)/histStep);
		if(bin < 0) bin = 0;
		if(bin >= histDepth) bin=histDepth-1;
		buffer[i] = bin;
	}
	
	
	// Update histogram
	// This could be a little slow due to sparse memory access conflicting with predictive memory caching
	pthread_mutex_lock(&global->detector[detID].histogram_mutex);
	uint64_t	offset;
	for(long i=0; i<pix_nn; i++) {
		offset = i*histDepth;
		histogramData[offset+buffer[i]] += 1;
	}
	global->detector[detID].histogram_count += 1;
	pthread_mutex_unlock(&global->detector[detID].histogram_mutex);
	
	free(buffer);
}

/*
 *	Save histograms
 */
void saveHistograms(cGlobal *global) {
	DETECTOR_LOOP {
		if (global->detector[detID].histogram) {
			saveHistogram(global, detID);
		}
	}

}


void saveHistogram(cGlobal *global, int detID) {
	
	// Dereference common variables
	long		pix_nn = global->detector[detID].pix_nn;
	long		histMin = global->detector[detID].histogramMin;
	long		histMax = global->detector[detID].histogramMax;
	long		histBinsize = global->detector[detID].histogramBinSize;
	long		hist_nx = global->detector[detID].histogram_nx;
	long		hist_ny = global->detector[detID].histogram_ny;
	long		hist_depth = global->detector[detID].histogram_depth;
	float		*darkcal = global->detector[detID].darkcal;
	uint16_t	*histogramData = global->detector[detID].histogramData;


	/*
	 *	Filename
	 */
	char	filename[1024];
	sprintf(filename,"r%04u-detector%d-histogram.h5", global->runNumber, detID);
	printf("%s\n",filename);
	
	

	/*
	 *	Mess of stuff for writing the compound HDF5 file
	 */
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	//herr_t r;
	hsize_t size[3];
	hsize_t max_size[3];
	
	fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if ( fh < 0 ) {
		ERROR("Couldn't create HDF5 file: %s\n", filename);
	}
	gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gh < 0 ) {
		ERROR("Couldn't create HDF5 group\n");
		H5Fclose(fh);
	}
	

	// Write histogram data
	size[0] = hist_ny;
	size[1] = hist_nx;
	size[2] = hist_depth;
	sh = H5Screate_simple(3, size, NULL);
	
	dh = H5Dcreate(gh, "data", H5T_NATIVE_UINT16, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if (dh < 0) ERROR("Could not create dataset.\n");
	
	pthread_mutex_lock(&global->detector[detID].histogram_mutex);
	long		hist_count = global->detector[detID].histogram_count;
	H5Dwrite(dh, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, histogramData);
	pthread_mutex_unlock(&global->detector[detID].histogram_mutex);
	
	H5Dclose(dh);

	
	// Write offsets for each pixel (darkcal)
	size[0] = hist_ny;
	size[1] = hist_nx;
	max_size[0] = hist_ny;
	max_size[1] = hist_nx;
	sh = H5Screate_simple(2, size, max_size);
	dh = H5Dcreate(gh, "offset", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, darkcal);
	H5Dclose(dh);
	H5Sclose(sh);
	
	
	// Write other info
	size[0] = 1;
	sh = H5Screate_simple(1, size, NULL );
	//sh = H5Screate(H5S_SCALAR);
	
	// Number of frames in histogram
	dh = H5Dcreate(gh, "histogramCount", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &hist_count );
	H5Dclose(dh);
	
	// Histogram minimum
	dh = H5Dcreate(gh, "histogramMin", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &histMin );
	H5Dclose(dh);

	// Histogram maximum
	dh = H5Dcreate(gh, "histogramMax", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &histMax );
	H5Dclose(dh);

	// Step size
	dh = H5Dcreate(gh, "histogramBinsize", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &histBinsize );
	H5Dclose(dh);
	
	H5Sclose(sh);
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
