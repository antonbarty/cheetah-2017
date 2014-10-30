
//  assembleImage.cpp
//  cheetah
//
//  Created by Anton Barty on 14/08/12.
//  Copyright (c) 2012 CFEL. All rights reserved.
//


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <limits.h>

#include "cheetah.h"
#include "cheetahmodules.h"
#include "median.h"


/*
 *  Assemble data into a realistic 2d image using raw data and geometry
 */
void assemble2DImage(cEventData *eventData, cGlobal *global) {
	DETECTOR_LOOP {
		if (isAnyOfBitOptionsSet(global->detector[detIndex].saveFormat, DATA_FORMAT_ASSEMBLED | DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED)) {
			long		pix_nn = global->detector[detIndex].pix_nn;
			long		image_nx = global->detector[detIndex].image_nx;
			long		image_nn = global->detector[detIndex].image_nn;
			float		*pix_x = global->detector[detIndex].pix_x;
			float		*pix_y = global->detector[detIndex].pix_y;
			cDataVersion dataV(&eventData->detector[detIndex], &global->detector[detIndex], DATA_LOOP_MODE_POWDER, DATA_FORMAT_NON_ASSEMBLED);
			cDataVersion imageV(&eventData->detector[detIndex], &global->detector[detIndex], DATA_LOOP_MODE_POWDER, DATA_FORMAT_ASSEMBLED);
			while (dataV.next() == 0) {
				imageV.next();
				float		*data = dataV.data;
				float		*image = imageV.data;
				int         assembleInterpolation = global->assembleInterpolation;
				assemble2DImage(image, data, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);
			}
		}
	}
} 
    
void assemble2DImage(float *image, float *data, float *pix_x, float *pix_y, long pix_nn, long image_nx, long image_nn,int assembleInterpolation) {
 
    if(assembleInterpolation == ASSEMBLE_INTERPOLATION_NEAREST){
        // Loop through all pixels and interpolate onto regular grid
        float	x, y;
        long	ix, iy;
        long	image_index;
        
        for(long i=0;i<pix_nn;i++){
            x = pix_x[i] + image_nx/2.;
            y = pix_y[i] + image_nx/2.;
            // round to nearest neighbor
            ix = (long) (x+0.5);
            iy = (long) (y+0.5);
            
            image_index = ix + image_nx*iy;
            image[image_index]= data[i];
        }
    }

    else if(assembleInterpolation == ASSEMBLE_INTERPOLATION_LINEAR){
		// Allocate temporary arrays for pixel interpolation (needs to be floating point)
		float	*data = (float*) calloc(image_nn,sizeof(float));
		float	*weight = (float*) calloc(image_nn,sizeof(float));
		for(long i=0; i<image_nn; i++){
			data[i] = 0;
			weight[i]= 0;
		}
	
	
		// Loop through all pixels and interpolate onto regular grid
		float	x, y;
		float	pixel_value, w;
		long	ix, iy;
		float	fx, fy;
		long	image_index;
    
		for(long i=0;i<pix_nn;i++){
			// Pixel location with (0,0) at array element (0,0) in bottom left corner
			x = pix_x[i] + image_nx/2.;
			y = pix_y[i] + image_nx/2.;
			pixel_value = data[i];
		
			// Split coordinate into integer and fractional parts
			ix = (long) floor(x);
			iy = (long) floor(y);
			fx = x - ix;
			fy = y - iy;
		
			// Interpolate intensity over adjacent 4 pixels using fractional overlap as the weighting factor
			// (0,0)
			if(ix>=0 && iy>=0 && ix<image_nx && iy<image_nx) {
				w = (1-fx)*(1-fy);
				image_index = ix + image_nx*iy;
				data[image_index] += w*pixel_value;
				weight[image_index] += w;
			}
			// (+1,0)
			if((ix+1)>=0 && iy>=0 && (ix+1)<image_nx && iy<image_nx) {
				w = (fx)*(1-fy);
				image_index = (ix+1) + image_nx*iy;
				data[image_index] += w*pixel_value;
				weight[image_index] += w;
			}
			// (0,+1)
			if(ix>=0 && (iy+1)>=0 && ix<image_nx && (iy+1)<image_nx) {
				w = (1-fx)*(fy);
				image_index = ix + image_nx*(iy+1);
				data[image_index] += w*pixel_value;
				weight[image_index] += w;
			}
			// (+1,+1)
			if((ix+1)>=0 && (iy+1)>=0 && (ix+1)<image_nx && (iy+1)<image_nx) {
				w = (fx)*(fy);
				image_index = (ix+1) + image_nx*(iy+1);
				data[image_index] += w*pixel_value;
				weight[image_index] += w;
			}
		}
	
	
		// Reweight pixel interpolation
		for(long i=0; i<image_nn; i++){
			if(weight[i] < 0.05)
				data[i] = 0;
			else
				data[i] /= weight[i];
		}
    
        // Copy to output array
        for(long i=0; i<image_nn; i++){
            image[i] = data[i];
		}
	
	
		// Free temporary arrays
		free(data);
		free(weight);
	}
}

    


/*
 *  Assemble mask data into a realistic 2d image using raw data and geometry
 */
void assemble2DMask(cEventData *eventData, cGlobal *global) {   
    if(global->assemble2DMask) {
        DETECTOR_LOOP {
			if (isAnyOfBitOptionsSet(global->detector[detIndex].saveFormat, DATA_FORMAT_ASSEMBLED | DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED)) {
				long		pix_nn = global->detector[detIndex].pix_nn;
				long		image_nx = global->detector[detIndex].image_nx;
				long		image_nn = global->detector[detIndex].image_nn;
				float		*pix_x = global->detector[detIndex].pix_x;
				float		*pix_y = global->detector[detIndex].pix_y;
				uint16_t  *pixelmask = eventData->detector[detIndex].pixelmask;
				uint16_t	*image_pixelmask = eventData->detector[detIndex].image_pixelmask;
				int       assembleInterpolation = global->assembleInterpolation;
				assemble2DMask(image_pixelmask,pixelmask, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);
			}
		}	
	}
}



/*
 *	Interpolate binary mask using pre-defined pixel mapping (as loaded from .h5 file)
 *      Options are dominant in united pixels
 *	input data: uint16_t
 *	output data: uint16_t
 */
void assemble2DMask(uint16_t *assembled_mask, uint16_t *original_mask, float *pix_x, float *pix_y, long pix_nn, long image_nx, long image_nn,int assembleInterpolation) {
  
	float	x, y;
	long	ix, iy;
	long	image_index;
  
	for(long i=0;i<image_nn;i++){
		assembled_mask[i] = PIXEL_IS_MISSING;
	}
       		
	if(assembleInterpolation == ASSEMBLE_INTERPOLATION_LINEAR){
		// Loop through all pixels and interpolate onto regular grid
  
		for(long i=0;i<pix_nn;i++){
			// Pixel location with (0,0) at array element (0,0) in bottom left corner
			x = pix_x[i] + image_nx/2;
			y = pix_y[i] + image_nx/2;
		
			// Split coordinate into integer and fractional parts
			ix = (long) floor(x);
			iy = (long) floor(y);

			// Unite over adjacent 4 pixels
			// (0,0)
			if(ix>=0 && iy>=0 && ix<image_nx && iy<image_nx) {
				image_index = ix + image_nx*iy;
				assembled_mask[image_index] |= original_mask[i];
				assembled_mask[image_index] &= ~PIXEL_IS_MISSING;
			}
			// (+1,0)
			if((ix+1)>=0 && iy>=0 && (ix+1)<image_nx && iy<image_nx) {
				image_index = (ix+1) + image_nx*iy;
				assembled_mask[image_index] |= original_mask[i];
				assembled_mask[image_index] &= ~PIXEL_IS_MISSING;
			}
			// (0,+1)
			if(ix>=0 && (iy+1)>=0 && ix<image_nx && (iy+1)<image_nx) {
				image_index = ix + image_nx*(iy+1);
				assembled_mask[image_index] |= original_mask[i];
				assembled_mask[image_index] &= ~PIXEL_IS_MISSING;
			}
			// (+1,+1)
			if((ix+1)>=0 && (iy+1)>=0 && (ix+1)<image_nx && (iy+1)<image_nx) {
				image_index = (ix+1) + image_nx*(iy+1);
				assembled_mask[image_index] |= original_mask[i];
				assembled_mask[image_index] &= ~PIXEL_IS_MISSING;
			}
		}
	}
	else if(assembleInterpolation == ASSEMBLE_INTERPOLATION_NEAREST){
    
		for(long i=0;i<pix_nn;i++){
			x = pix_x[i] + image_nx/2.;
			y = pix_y[i] + image_nx/2.;
			ix = (long) (x+0.5);
			iy = (long) (y+0.5);

			image_index = ix + image_nx*iy;
			assembled_mask[image_index] = original_mask[i];
		}
	}
}

void assemble2D(cEventData *eventData, cGlobal *global) {
	assemble2DMask(eventData, global);
	assemble2DImage(eventData, global);
}



/*
 *  Assemble 2D powder patterns into a realistic 2D image using geometry
 *  This is not called very often (once when powder patterns are about to be saved)
 */
void assemble2DPowder(cGlobal *global) {

    DETECTOR_LOOP {
		if (isAnyOfBitOptionsSet(global->detector[detIndex].powderFormat, DATA_FORMAT_ASSEMBLED | DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED)) {
			long		pix_nn = global->detector[detIndex].pix_nn;
			long		image_nx = global->detector[detIndex].image_nx;
			long		image_nn = global->detector[detIndex].image_nn;
			float		*pix_x = global->detector[detIndex].pix_x;
			float		*pix_y = global->detector[detIndex].pix_y;
			int             assembleInterpolation = global->assembleInterpolation;
        
			// Assemble each powder type
			for(long powderClass=0; powderClass < global->nPowderClasses; powderClass++) {

				cDataVersion dataV(NULL, &global->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_NON_ASSEMBLED);
				cDataVersion imageV(NULL, &global->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_ASSEMBLED);
				while (dataV.next() == 0) {
					imageV.next();

					double * data = dataV.powder[powderClass];
					double * image = imageV.powder[powderClass];

					// Floating point buffer
					float   *fdata = (float*) calloc(pix_nn,sizeof(float));
					float   *fimage = (float*) calloc(image_nn,sizeof(float));

					// Assembly is done using float; powder data is double (!!)	
					for(long i=0; i<pix_nn; i++)
						fdata[i] = (float) data[i];

					// Assemble image
					assemble2DImage(fimage, fdata, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);

					// Assembly is done using float; powder data is double (!!)
					for(long i=0; i<image_nn; i++)
						image[i] = (double) fimage[i];
				
					// Cleanup
					free(fdata);
					free(fimage);
				}
			}
		}
	}
}
