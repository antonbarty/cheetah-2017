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
	uint64_t	hist_nn = global->detector[detID].histogram_nn;
	long		hist_count;


    
    /*
	 *	Mess of stuff for writing the HDF5 file
     *  (OK to open HDF5 file outside the mutex lock)
	 */
	char	filename[1024];
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	hsize_t size[3];
	hsize_t max_size[3];
    
	sprintf(filename,"r%04u-detector%d-histogram.h5", global->runNumber, detID);
	printf("%s\n",filename);
	
	fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if ( fh < 0 ) {
		ERROR("Couldn't create HDF5 file: %s\n", filename);
	}
	gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gh < 0 ) {
		ERROR("Couldn't create HDF5 group\n");
		H5Fclose(fh);
	}
	

    
    
	/*
	 *	Lock the histogram 
     *  Keep this bit as short as possible because it locks up the rest of the code.
	 */
	pthread_mutex_lock(&global->detector[detID].histogram_mutex);
	hist_count = global->detector[detID].histogram_count;
	

    // Write histogram data
	size[0] = hist_ny;
	size[1] = hist_nx;
	size[2] = hist_depth;
	sh = H5Screate_simple(3, size, NULL);
	
	dh = H5Dcreate(gh, "data", H5T_NATIVE_UINT16, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if (dh < 0) ERROR("Could not create dataset.\n");
	H5Dwrite(dh, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, histogramData);
	H5Dclose(dh);
    

	
	/*
	 *	Perform some statistical analysis
	 */
	float	n;
	float	mean;
	float	var;
	float	rVar;
	float	cSq;
	float	kld;
	float	count;
	float	temp1, temp2, temp3, temp4;
	float	*mean_arr = (float*) calloc(pix_nn, sizeof(float));
	float	*var_arr = (float*) calloc(pix_nn, sizeof(float));
	float	*rVar_arr = (float*) calloc(pix_nn, sizeof(float));
	float	*cSq_arr = (float*) calloc(pix_nn, sizeof(float));
	float	*kld_arr = (float*) calloc(pix_nn, sizeof(float));
	float	*count_arr = (float*) calloc(pix_nn, sizeof(float));
	float	*hist = (float*) calloc(hist_depth, sizeof(float));
	uint64_t	offset;
	
	
	for(long i=0; i<pix_nn; i++) {
		offset = i*hist_depth;

		// Extract a temporary copy of the histogram for this pixel
		count = 0;
		for(long j=0; j<hist_depth; j++) {
			hist[j] = (float) histogramData[offset+j];
			count += hist[j];
		}

		
		// Normalise the histogram to total count of 1
		for(long j=0; j<hist_depth; j++)
			hist[j] /= count;


		// Calculate mean and variance
		count = 0;
		mean = 0;
		var = 0;
		for(long j=0; j<hist_depth; j++) {
			count += hist[j];
			mean += j*hist[j];
			var += j*j*hist[j];
		}
		var -= (mean*mean);
		
		
		// Calculate Chi-Squared and KL-divergence
		rVar = 0;
		cSq = 0;
		kld = 0;
		n = 0;
		for(long j=0; j<hist_depth; j++) {
			if(hist[j] > 1e-10 && var > 1e-10) {
				temp1 = (j - mean);
				temp2 = temp1*temp1;
				temp3 = expf(-0.5 * temp2 / var);
                temp2 *= hist[j];
				rVar += temp2;
				cSq += temp2;
				if(temp3 > 1e-10 && hist[j] > 1e-10) {
					temp4 = hist[j] / temp3;
					if (temp4 > 1e-10) {
						kld += hist[j] * logf(temp4);
						n += temp3;
					}
				}
			}
		}
		if(var > 1e-7)
			rVar /= var;
		if(mean > 1e-7)
			cSq /= (mean*mean);
		if(n > 1e-10)
			kld += logf(n);
		
		mean_arr[i] = mean;
		var_arr[i] = var;
		rVar_arr[i] = rVar;
		cSq_arr[i] = cSq;
		kld_arr[i] = kld;
		count_arr[i] = n;
	}
	
    
	// Unlock the histogram
    pthread_mutex_unlock(&global->detector[detID].histogram_mutex);

    
    /*
     *  Back outside of mutex lock - keep writing the rest of the data
     */
    
	
	// Write offsets for each pixel (darkcal)
	size[0] = hist_ny;
	size[1] = hist_nx;
	max_size[0] = hist_ny;
	max_size[1] = hist_nx;
	sh = H5Screate_simple(2, size, max_size);
	dh = H5Dcreate(gh, "offset", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, darkcal);
	H5Dclose(dh);

		
	// Write arrays of statistics for each pixel
	dh = H5Dcreate(gh, "mean", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, mean_arr);

	dh = H5Dcreate(gh, "variance", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, var_arr);

	dh = H5Dcreate(gh, "reduced-variance", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rVar_arr);

	dh = H5Dcreate(gh, "chi-squared", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, cSq_arr);

	dh = H5Dcreate(gh, "kl-divergence", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, kld_arr);

	dh = H5Dcreate(gh, "n", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, count_arr);

	
	H5Sclose(sh);
	
	
	
	
	
	// Write a bunch of other useful information
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
	
	
	
	/*
	 *	Release memory (very important here!)
	 */
	free(mean_arr);
	free(var_arr);
	free(rVar_arr);
	free(cSq_arr);
	free(kld_arr);
	free(count_arr);
	free(hist);

	
}
