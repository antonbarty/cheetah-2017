/*
 *  worker.cpp
 *  cspad_cryst
 *
 *  Created by Anton Barty on 6/2/11.
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



/*
 *	Worker thread function for processing each cspad data frame
 */
void *worker(void *threadarg) {

	/*
	 *	Turn threadarg into a more useful form
	 */
	tGlobal			*global;
	tThreadInfo		*threadInfo;
	threadInfo = (tThreadInfo*) threadarg;
	global = threadInfo->pGlobal;

	
	printf("%i: Started worker thread\n", threadInfo->threadNum);
	
	
	
	/*
	 *	Basic test - write out raw Quadrant info
	 */
	FILE* fp;
	char filename[1024];
	int fiducial = threadInfo->fiducial;
	/*	Not needed any more - left in place in case needed for debugging
	 for(int quadrant=0; quadrant<4; quadrant++) {
		sprintf(filename,"%x-q%i.h5",fiducial,quadrant);
		writeSimpleHDF5(filename, threadInfo->quad_data[quadrant], 2*ROWS, 8*COLS, H5T_STD_U16LE);		
	}
	 */

	
	
	/*
	 *	Make one large array out of raw data 
	 */
	threadInfo->raw_data = (uint16_t*) calloc(8*ROWS*8*COLS,sizeof(uint16_t));
	for(int quadrant=0; quadrant<4; quadrant++) {
		long	i,j,ii;
		for(long k=0; k<2*ROWS*8*COLS; k++) {
			i = k % (2*ROWS) + quadrant*(2*ROWS);
			j = k / (2*ROWS);
			ii  = i+(8*ROWS)*j;
			threadInfo->raw_data[ii] = threadInfo->quad_data[quadrant][k];
		}
	}
	// Write out for diagnostics
	sprintf(filename,"%x.h5",fiducial);
	//writeSimpleHDF5(filename, threadInfo->raw_data, 8*ROWS, 8*COLS, H5T_STD_U16LE);		
	
	
	
	/*
	 *	Assemble quadrants into a 'realistic' 2D image
	 */
	assemble2Dimage(threadInfo, global);
	sprintf(filename,"%x-image.h5",fiducial);
	writeSimpleHDF5(filename, threadInfo->image, global->image_nx, global->image_nx, H5T_STD_U16LE);		
	
	
	/*
	 *	Write out to our favourite HDF5 format
	 */
	//writeHDF5(threadInfo, global);
	
	
	/*
	 *	Cleanup and exit
	 */
	printf("%i: Cleaning up and exiting\n",threadInfo->threadNum);

	// Decrement thread pool counter by one
	pthread_mutex_lock(&global->nActiveThreads_mutex);
	global->nActiveThreads -= 1;
	pthread_mutex_unlock(&global->nActiveThreads_mutex);
	
	// Free memory
	for(int quadrant=0; quadrant<4; quadrant++) 
		free(threadInfo->quad_data[quadrant]);	
	free(threadInfo->raw_data);
	free(threadInfo->image);
	free(threadInfo);

	// Exit thread
	pthread_exit(NULL);
}



/*
 *	Interpolate raw (corrected) cspad data into a physical 2D image
 *	using pre-defined pixel mapping (as loaded from .h5 file)
 */
void assemble2Dimage(tThreadInfo *threadInfo, tGlobal *global){
	
	
	// Allocate temporary arrays for pixel interpolation (needs to be floating point)
	float	*data = (float*) calloc(global->image_nn,sizeof(float));
	float	*weight = (float*) calloc(global->image_nn,sizeof(float));
	for(long i=0; i<global->image_nn; i++){
		data[i] = 0;
		weight[i]= 0;
	}
	
	
	// Loop through all pixels and interpolate onto regular grid
	float	x, y;
	float	pixel_value, w;
	long	ix, iy;
	float	fx, fy;
	long	image_index;
	

	for(long i=0;i<global->pix_nn;i++){
		// Pixel location with (0,0) at array element (0,0) in bottom left corner
		x = global->pix_x[i] + global->image_nx/2;
		y = global->pix_y[i] + global->image_nx/2;
		pixel_value = threadInfo->raw_data[i];
		
		// Split coordinate into integer and fractional parts
		ix = (long) floor(x);
		iy = (long) floor(y);
		fx = x - ix;
		fy = y - iy;
		
		//printf("%i\t%i\n",ix,iy);
		
		// Interpolate intensity over adjacent 4 pixels using fractional overlap as the weighting factor
		// (0,0)
		if(ix>=0 && iy>=0 && ix<global->image_nx && iy<global->image_nx) {
			w = (1-fx)*(1-fy);
			image_index = ix + global->image_nx*iy;
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (+1,0)
		if((ix+1)>=0 && iy>=0 && (ix+1)<global->image_nx && iy<global->image_nx) {
			w = (fx)*(1-fy);
			image_index = (ix+1) + global->image_nx*iy;
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (0,+1)
		if(ix>=0 && (iy+1)>=0 && ix<global->image_nx && (iy+1)<global->image_nx) {
			w = (1-fx)*(fy);
			image_index = ix + global->image_nx*(iy+1);
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (+1,+1)
		if((ix+1)>=0 && (iy+1)>=0 && (ix+1)<global->image_nx && (iy+1)<global->image_nx) {
			w = (fx)*(fy);
			image_index = (ix+1) + global->image_nx*(iy+1);
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
	}
	
	
	// Reweight pixel interpolation
	for(long i=0; i<global->image_nn; i++){
		if(weight[i] < 0.1)
			data[i] = 0;
		else
			data[i] /= weight[i];
	}

	
	// Allocate memory for output image
	threadInfo->image = (uint16_t*) calloc(global->image_nn,sizeof(uint16_t));

	// Copy interpolated image across into image array
	for(long i=0;i<global->image_nn;i++){
		threadInfo->image[i] = (uint16_t) data[i];
	}	
	
	
	// Free temporary arrays
	free(data);
	free(weight);
	
}



void writeHDF5(tThreadInfo *info, tGlobal *global){
	
	/*
	 * Copied from the way LCLS formats its time string 
	 *	localtime_r is supposed to be thread safe (!)
	 */
	//static const char timeFormatStr[40] = "%04Y-%02m-%02d %02H:%02M:%02S"; /* Time format string */    
	//static char sTimeText[40];
	//int seconds = clockTimeCurDatagram.seconds();
	//struct tm tmTimeStamp;
	//localtime_r( (const time_t*) (void*) &seconds, &tmTimeStamp );    
	//strftime(sTimeText, sizeof(sTimeText), timeFormatStr, &tmTimeStamp );

	
	/*
	 *	Create filename based on date, time and LCLS fiducial for this image
	 */

	
	int			unixtime;
	struct tm	tmTimeStamp;
	char buffer1[80];
	char buffer2[80];
	char outfile[1024];
	
	unixtime = info->seconds;
	printf("Time: %i\n",info->seconds);
	localtime_r( (const time_t*)(void*)&unixtime, &tmTimeStamp );    
	strftime(buffer1, 80, "%Y_%b%d", &tmTimeStamp);
	strftime(buffer2, 80, "%H%M%S", &tmTimeStamp);
	sprintf(outfile,"LCLS_%s_r%04u_%s_%x_cspad.h5",buffer1,info->runNumber,buffer2,info->fiducial);
	printf("%i: Writing data to: %s\n",info->threadNum, outfile);

		
	
	
}





/*
 *	Write test data to a simple HDF5 file
 */

void writeSimpleHDF5(const char *filename, const void *data, int width, int height, int type) 
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
	
	size[0] = height;
	size[1] = width;
	max_size[0] = height;
	max_size[1] = width;
	sh = H5Screate_simple(2, size, max_size);
	
	dh = H5Dcreate(gh, "data", type, sh,
	               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( dh < 0 ) {
		ERROR("Couldn't create dataset\n");
		H5Fclose(fh);
	}
	
	/* Muppet check */
	H5Sget_simple_extent_dims(sh, size, max_size);
	
	r = H5Dwrite(dh, type, H5S_ALL,
	             H5S_ALL, H5P_DEFAULT, data);
	if ( r < 0 ) {
		ERROR("Couldn't write data\n");
		H5Dclose(dh);
		H5Fclose(fh);
	}
	
	H5Gclose(gh);
	H5Dclose(dh);
	H5Fclose(fh);
}
