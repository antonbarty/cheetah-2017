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
	tThreadInfo *threadInfo;
	tGlobal		*global;
	threadInfo = (tThreadInfo*) threadarg;
	global = threadInfo->pGlobal;
	
	
	
	/*
	 *	Basic test - write out raw Quadrant info
	 */
	FILE* fp;
	char filename[1024];
	int fiducial = threadInfo->fiducial;
	/*	Not needed any more - left in place in case needed for debugging
	 for(int quadrant=0; quadrant<4; quadrant++) {
		sprintf(filename,"%x-q%i.h5",fiducial,quadrant);
		hdf5_write(filename, threadInfo->quad_data[quadrant], 2*ROWS, 8*COLS, H5T_STD_U16LE);		
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
	hdf5_write(filename, threadInfo->raw_data, 8*ROWS, 8*COLS, H5T_STD_U16LE);		
	
	
	
	/*
	 *	Assemble quadrants into a 'realistic' 2D image
	 */
	assemble2Dimage(threadInfo, global);
	sprintf(filename,"%x-image.h5",fiducial);
	hdf5_write(filename, threadInfo->image, global->image_nx, global->image_nx, H5T_STD_U16LE);		
	
	
	
	
	/*
	 *	This bit currently copied verbatim from Garth's myana code
	 *	Commented out for now for debugging purposes
	if (!threadInfo->cspad_fail) {
		nevents++;
		const Pds::CsPad::ElementHeader* element;
		
		// loop over elements (quadrants)
		while(( element=iter.next() )) {  
			//gjw:  only quad 3 has data during commissioning
			if(element->quad()==2){
				//gjw:  check that we are not on a new fiducial
				if (fiducials != element->fiducials())
					printf("Fiducials %x/%d:%x\n",fiducials,element->quad(),element->fiducials());
				//gjw:  get temp on strong back 2 
				printf("Temperature: %3.1fC\n",CspadTemp::instance().getTemp(element->sb_temp(2)));
				
				//gjw:  read 2x1 "sections"
				const Pds::CsPad::Section* s;
				unsigned section_id;
				
				
				// loop over sections and copy into data (each section is a "two by one")
				uint16_t data[COLS*ROWS*16];
				while(( s=iter.next(section_id) )) {  
					//gjw:  read out data in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
					memcpy(&data[section_id*2*ROWS*COLS],s->pixel[0],2*ROWS*COLS*sizeof(uint16_t));
				}
				
				// ROWS = 194;  COLS = 185;
				sprintf(filename,"%x.h5",element->fiducials());
				hdf5_write(filename, data, 2*ROWS, 8*COLS, H5T_STD_U16LE);
				
				
				//gjw:  split 2x8 array into 2x2 modules (detector unit for rotation), 4 in a quad and write to file
				uint16_t tbt[4][2*COLS][2*ROWS];
				for(int g=0;g<4;g++){
					// Copy memory into buffers
					memcpy(&tbt[g],&data[g*2*2*ROWS*COLS],2*2*COLS*ROWS*sizeof(uint16_t));
					
					// Write to file
					sprintf(filename,"%x-q%d.h5",element->fiducials(),g);
					hdf5_write(filename, data, ROWS*2, COLS*2, H5T_STD_U16LE);
				}
				
				
				// Geometrical corrections
				//gjw:  rotate/reflect 2x2s into consistent orientations
				uint16_t buff[2*COLS][2*ROWS];
				uint16_t buff1[2*ROWS][2*COLS];
				uint16_t buff2[2*COLS][2*ROWS];
				uint16_t buff3[2*ROWS][2*COLS];
				FILE* fp1;
				
				//quad 0 (asics 0-3) 370x388 pixels
				// Reflect X
				for(unsigned int g=0;g<2*COLS;g++) for(unsigned int j=0;j<2*ROWS;j++)
					buff[g][j]=tbt[0][2*COLS-1-g][j];	
				// Rotate 90
				for(unsigned int g=0;g<2*ROWS;g++) for(unsigned int j = 0;j<2*COLS;j++)
					buff1[g][j]=buff[2*ROWS-1-j+2*(COLS-ROWS)][g];
				
				// Write to file						
				sprintf(filename,"%x-q%d-corrected.h5",element->fiducials(),0);
				hdf5_write(filename, buff1[0], ROWS*2, COLS*2, H5T_STD_U16LE);
				
				
				//quad 1 (asics 4-7) 388x370
				//reflect X
				for(unsigned int g=0;g<2*COLS;g++)for(unsigned int j=0;j<2*ROWS;j++)
					buff[g][j]=tbt[1][2*COLS-1-g][j];	
				//reflect Y
				for(unsigned int g=0;g<2*COLS;g++)for(unsigned int j=0;j<2*ROWS;j++)
					buff3[g][j]=buff[g][2*ROWS-1-j];	
				// Write to file
				sprintf(filename,"%x-q%d-corrected.h5",element->fiducials(),1);
				hdf5_write(filename, buff1[0], ROWS*2, COLS*2, H5T_STD_U16LE);
				
				
				//quad 2 (asics 8-11) 370x388
				//rotate -90
				for(unsigned int g=0;g<2*ROWS;g++)for(unsigned int j = 0;j<2*COLS;j++)
					buff1[g][j]=tbt[2][j][2*COLS-1-g+2*(ROWS-COLS)];
				// Write to file
				sprintf(filename,"%x-q%d-corrected.h5",element->fiducials(),2);
				hdf5_write(filename, buff1[0], ROWS*2, COLS*2, H5T_STD_U16LE);
				
				
				//quad 3 (asics 12-15) 388x370
				//reflect X
				for(unsigned int g=0;g<2*COLS;g++)for(unsigned int j=0;j<2*ROWS;j++)
					buff[g][j]=tbt[3][2*COLS-1-g][j];	
				//reflect Y
				for(unsigned int g=0;g<2*COLS;g++)for(unsigned int j=0;j<2*ROWS;j++)
					buff2[g][j]=buff[g][2*ROWS-1-j];	
				// Write to file
				sprintf(filename,"%x-q%d-corrected.h5",element->fiducials(),3);
				hdf5_write(filename, buff1[0], ROWS*2, COLS*2, H5T_STD_U16LE);
			}
		}
	}
	 
	 * End of Garth's geometrical corrections
	 */
	
	
	
	/*
	 *	Cleanup and exit
	 */
	printf("Cleaning up thread\n");

	// Free memory used to store this data frame
	for(int jj=0; jj<4; jj++) {
		free(threadInfo->quad_data[jj]);	
		threadInfo->quad_data[jj] = NULL;;	
	}
	free(threadInfo->raw_data);
	free(threadInfo->image);

	
	// Decrement thread pool counter by one
	pthread_mutex_lock(&global->nActiveThreads_mutex);
	global->nActiveThreads -= 1;
	pthread_mutex_unlock(&global->nActiveThreads_mutex);
	
	
	// Free memory used by threadInfo structure and exit
	//free(threadInfo);
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
	for(long i=0;i<global->image_nn;i++){
		data[i] = 0;
		weight[i]= 0;
	}
	
	
	// Loop through all pixels and interpolate onto regular grid
	float	x,y;
	float	pixel_value,w;
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
		
		// Interpolate intensity over nearest 4 pixels using fractional overlap as the weighting factor
		// (0,0)
		w = (1-fx)*(1-fy);
		image_index = ix + global->image_nx*iy;
		data[image_index] += w*pixel_value;
		weight[image_index] += w;
		// (+1,0)
		w = (fx)*(1-fy);
		image_index = (ix+1) + global->image_nx*iy;
		data[image_index] += w*pixel_value;
		weight[image_index] += w;
		// (0,+1)
		w = (1-fx)*(fy);
		image_index = ix + global->image_nx*(iy+1);
		data[image_index] += w*pixel_value;
		weight[image_index] += w;
		// (+1,+1)
		w = (fx)*(fy);
		image_index = (ix+1) + global->image_nx*(iy+1);
		data[image_index] += w*pixel_value;
		weight[image_index] += w;
	}
	
	
	// Reweight pixel interpolation
	for(long i=0;i<global->pix_nn;i++){
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




/*
 *	Write data to HDF5 file
 */

int hdf5_write(const char *filename, const void *data, int width, int height, int type) 
{
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	herr_t r;
	hsize_t size[2];
	hsize_t max_size[2];
	
	fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	if ( fh < 0 ) {
		ERROR("Couldn't create file: %s\n", filename);
		return 1;
	}
	
	gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gh < 0 ) {
		ERROR("Couldn't create group\n");
		H5Fclose(fh);
		return 1;
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
		return 1;
	}
	
	/* Muppet check */
	H5Sget_simple_extent_dims(sh, size, max_size);
	
	r = H5Dwrite(dh, type, H5S_ALL,
	             H5S_ALL, H5P_DEFAULT, data);
	if ( r < 0 ) {
		ERROR("Couldn't write data\n");
		H5Dclose(dh);
		H5Fclose(fh);
		return 1;
	}
	
	H5Gclose(gh);
	H5Dclose(dh);
	H5Fclose(fh);
	
	return 0;
}
