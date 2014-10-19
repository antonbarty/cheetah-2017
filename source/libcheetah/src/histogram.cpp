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
		if (global->detector[detIndex].histogram) {
			addToHistogram(eventData, global, detIndex);
		}
	}
}


void addToHistogram(cEventData *eventData, cGlobal *global, int detIndex) {
	
	// Dereference common variables
	long		pix_nx = global->detector[detIndex].pix_nx;

	long		histMin = global->detector[detIndex].histogramMin;
	long		histNbins = global->detector[detIndex].histogramNbins;
	float       histBinSize = global->detector[detIndex].histogramBinSize;
	long		hist_fs_min = global->detector[detIndex].histogram_fs_min;
	long		hist_fs_max = global->detector[detIndex].histogram_fs_max;
	long		hist_ss_min = global->detector[detIndex].histogram_ss_min;
	long		hist_ss_max = global->detector[detIndex].histogram_ss_max;
	long		hist_nfs = global->detector[detIndex].histogram_nfs;
	long		hist_nn = global->detector[detIndex].histogram_nn;
	uint16_t	*histData = global->detector[detIndex].histogramData;

	
	// Figure out which bin should be filled
	// (done outside of mutex lock)
	long	bin;
	float	binf;
	long	i_hist, i_buffer;
	float	value;
	
	long	*buffer;
	buffer = (long *) calloc(hist_nn, sizeof(long));
	
	for(long ss=hist_ss_min; ss<hist_ss_max; ss++) {
		for(long fs=hist_fs_min; fs<hist_fs_max; fs++) {
			
			i_hist = fs + ss*pix_nx;
			i_buffer = (fs-hist_fs_min) + (ss-hist_ss_min)*hist_nfs;
			
			value = eventData->detector[detIndex].corrected_data[i_hist];
			binf = (value-histMin)/histBinSize;
			bin = (long) lrint(binf);
			
			if(bin < 0) bin = 0;
			if(bin >= histNbins) bin=histNbins-1;
			
			buffer[i_buffer] = bin;
		}
	}
	
	
	// Update histogram
	// This could be a little slow due to sparse memory access conflicting with predictive memory caching
	pthread_mutex_lock(&global->detector[detIndex].histogram_mutex);
	uint64_t	cell;
	for(long i=0; i<hist_nn; i++) {
		cell = i*histNbins;
		histData[cell+buffer[i]] += 1;
	}
	global->detector[detIndex].histogram_count += 1;
	pthread_mutex_unlock(&global->detector[detIndex].histogram_mutex);
	
	// Free temporary memory
	free(buffer);
}


/*
 *	Save histograms
 */
void saveHistograms(cGlobal *global) {

	printf("Writing histogram data \n");

	DETECTOR_LOOP {
		if (global->detector[detIndex].histogram) {
			saveHistogram(global, detIndex);
		}
	}

}


void saveHistogram(cGlobal *global, int detIndex) {
	
	// Dereference common variables
	
	long		histMin = global->detector[detIndex].histogramMin;
	long		histNbins = global->detector[detIndex].histogramNbins;
	float		histBinSize = global->detector[detIndex].histogramBinSize;
	long		hist_nfs = global->detector[detIndex].histogram_nfs;
	long		hist_nss = global->detector[detIndex].histogram_nss;
	long		hist_nn = global->detector[detIndex].histogram_nn;
	uint64_t	hist_nnn = global->detector[detIndex].histogram_nnn;
	uint16_t	*histData = global->detector[detIndex].histogramData;
	float		*darkcal = global->detector[detIndex].darkcal;
	
	long		hist_count;



    
	/*
     *  Copy histogram into a buffer so that the rest of the code can keep crunching in the meantime
     *  Keep this bit as short as possible because it locks up the rest of the code.
	 */
	
    // Create and allocate the buffer outside of mutex lock (memset forces allocation)
    uint16_t *histogramBuffer = (uint16_t*) calloc(hist_nnn, sizeof(uint16_t));
    memset(histogramBuffer, 0, hist_nnn*sizeof(uint16_t));
    
    // Copy histogram data inside mutex lock
	pthread_mutex_lock(&global->detector[detIndex].histogram_mutex);
	memcpy(histogramBuffer, histData, hist_nnn*sizeof(uint16_t));
	hist_count = global->detector[detIndex].histogram_count;
    pthread_mutex_unlock(&global->detector[detIndex].histogram_mutex);
    
    
	

    /*
	 *	Mess of stuff for writing the HDF5 file
     *  (OK to open HDF5 file outside the mutex lock)
	 */
	char	filename[1024];
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	hsize_t		size[3];
	hsize_t		max_size[3];
	hsize_t		chunk[3];
	hid_t		h5compression;

    
	sprintf(filename,"r%04u-detector%d-histogram.h5", global->runNumber, detIndex);
	printf("Writing histogram data to file: %s\n",filename);
	
	fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if ( fh < 0 ) {
		ERROR("Couldn't create HDF5 file: %s\n", filename);
	}
	gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gh < 0 ) {
		ERROR("Couldn't create HDF5 group\n");
		H5Fclose(fh);
	}
	
	if (global->h5compress) {
		h5compression = H5Pcreate(H5P_DATASET_CREATE);
		//H5Pset_chunk(h5compression, 2, chunksize);
		//H5Pset_deflate(h5compression, 3);		// Compression levels are 0 (none) to 9 (max)
		//H5Pset_chunk(h5compression, 2, size);
		//H5Pset_deflate(h5compression, 5);		// Compression levels are 0 (none) to 9 (max)

	}
	else {
		h5compression = H5P_DEFAULT;
	}

    

    /*
     *  Write histogram data
     */
	size[0] = hist_nss;
	size[1] = hist_nfs;
	size[2] = histNbins;
	sh = H5Screate_simple(3, size, NULL);

	chunk[0] = 1;
	chunk[1] = hist_nfs;
	chunk[2] = histNbins;
	if (global->h5compress) {
		H5Pset_chunk(h5compression, 3, chunk);
		//H5Pset_shuffle(h5compression);			// De-interlace bytes
		H5Pset_deflate(h5compression, 1);		// Compression levels are 0 (none) to 9 (max)
	}

	
	dh = H5Dcreate(gh, "histogram", H5T_NATIVE_UINT16, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	if (dh < 0) ERROR("Could not create dataset.\n");
	H5Dwrite(dh, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, histogramBuffer);
	H5Dclose(dh);
    
    // Create link from /data/histogram to /data/data (the default data locations)
    H5Lcreate_soft( "/data/histogram", fh, "/data/data",0,0);

	
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
	float	*mean_arr = (float*) calloc(hist_nn, sizeof(float));
	float	*var_arr = (float*) calloc(hist_nn, sizeof(float));
	float	*rVar_arr = (float*) calloc(hist_nn, sizeof(float));
	float	*cSq_arr = (float*) calloc(hist_nn, sizeof(float));
	float	*kld_arr = (float*) calloc(hist_nn, sizeof(float));
	float	*count_arr = (float*) calloc(hist_nn, sizeof(float));
	float	*hist = (float*) calloc(histNbins, sizeof(float));
	uint64_t	offset;
	
	
	for(long i=0; i<hist_nn; i++) {
		offset = i*histNbins;

		// Extract a temporary copy of the histogram for this pixel
		count = 0;
		for(long j=0; j<histNbins; j++) {
			hist[j] = (float) histogramBuffer[offset+j];
			count += hist[j];
		}

		
		// Normalise the histogram to total count of 1
		for(long j=0; j<histNbins; j++)
			hist[j] /= count;


		// Calculate mean and variance
		count = 0;
		mean = 0;
		var = 0;
		for(long j=0; j<histNbins; j++) {
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
		for(long j=0; j<histNbins; j++) {
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
	
    
    

	
	// Write offsets for each pixel (darkcal)
	size[0] = hist_nss;
	size[1] = hist_nfs;
	max_size[0] = hist_nss;
	max_size[1] = hist_nfs;
	sh = H5Screate_simple(2, size, max_size);

	if (global->h5compress) {
		H5Pset_chunk(h5compression, 2, size);
		//H5Pset_shuffle(h5compression);			// De-interlace bytes
		H5Pset_deflate(h5compression, 3);		// Compression levels are 0 (none) to 9 (max)
	}

	
	dh = H5Dcreate(gh, "offset", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, darkcal);
	H5Dclose(dh);

		
	// Write arrays of statistics for each pixel
	dh = H5Dcreate(gh, "mean", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, mean_arr);

	dh = H5Dcreate(gh, "variance", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, var_arr);

	dh = H5Dcreate(gh, "reduced-variance", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rVar_arr);

	dh = H5Dcreate(gh, "chi-squared", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, cSq_arr);

	dh = H5Dcreate(gh, "kl-divergence", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, kld_arr);

	dh = H5Dcreate(gh, "n", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
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
	dh = H5Dcreate(gh, "histogramNbins", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &histNbins );
	H5Dclose(dh);

	// Step size
	dh = H5Dcreate(gh, "histogramBinsize", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &histBinSize );
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
	 *	Release memory (very important because the histogram array is big!)
	 */
    free(histogramBuffer);
	free(mean_arr);
	free(var_arr);
	free(rVar_arr);
	free(cSq_arr);
	free(kld_arr);
	free(count_arr);
	free(hist);

	
}
