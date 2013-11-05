
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
void assemble2Dimage(cEventData *eventData, cGlobal *global) {

  if(global->assemble2DImage) {
    DETECTOR_LOOP {
      long		pix_nn = global->detector[detID].pix_nn;
      long		image_nx = global->detector[detID].image_nx;
      long		image_nn = global->detector[detID].image_nn;
      float		*pix_x = global->detector[detID].pix_x;
      float		*pix_y = global->detector[detID].pix_y;
      float		*corrected_data = eventData->detector[detID].corrected_data;
      int16_t		*image = eventData->detector[detID].image;
      int             assembleInterpolation = global->assembleInterpolation;
      assemble2Dimage(image, corrected_data, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);
    }
  }
}


/*
 *  Assemble 2D powder patterns into a realistic 2D image using geometry
 *  This is not called very often (once when powder patterns are about to be saved)
 */
void assemble2Dpowder(cGlobal *global) {

    DETECTOR_LOOP {
      long		pix_nn = global->detector[detID].pix_nn;
      long		image_nx = global->detector[detID].image_nx;
      long		image_nn = global->detector[detID].image_nn;
      float		*pix_x = global->detector[detID].pix_x;
      float		*pix_y = global->detector[detID].pix_y;
      int             assembleInterpolation = global->assembleInterpolation;
        cPixelDetectorCommon     *detector = &(global->detector[detID]);

        // Floating point buffer
        float   *fdata = (float*) calloc(pix_nn,sizeof(float));
        float   *fimage = (float*) calloc(image_nn,sizeof(float));
        
        
        // Assemble each powder type
        for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
            double  *data = detector->powderCorrected[powderType];
            double  *image = detector->powderAssembled[powderType];

            // Assembly is done using float; powder data is double (!!)
            for(long i=0; i<pix_nn; i++)
                fdata[i] = (float) data[i];
            
            // Assemble image
            assemble2Dimage(fimage, fdata, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);

            // Assembly is done using float; powder data is double (!!)
            for(long i=0; i<image_nn; i++)
                image[i] = (double) fimage[i];
    }
        
        // Cleanup
        free(fdata);
        free(fimage);
  }
}



/*
 *	Interpolate raw (corrected) cspad data into a physical 2D image
 *	input data: float
 *	output data: int16_t
 */
void  assemble2Dimage(int16_t *image, float *corrected_data, float *pix_x, float *pix_y, long pix_nn, long image_nx, long image_nn,int assembleInterpolation){

    // Assembly is done using floating point by default
    float	*temp = (float*) calloc(image_nn,sizeof(float));
    assemble2Dimage(temp, corrected_data, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);
    
    // Check for int16 overflow
    for(long i=0;i<image_nn;i++){
        if(lrint(temp[i]) > 32767)
            temp[i]=32767;
        if(lrint(temp[i]) < -32767)
            temp[i]=-32767;
    }
  
    // Copy interpolated image across into int_16 image array
    for(long i=0;i<image_nn;i++){
        image[i] = (int16_t) lrint(temp[i]);
    }

    free(temp);
}




    
    
void assemble2Dimage(float *image, float *corrected_data, float *pix_x, float *pix_y, long pix_nn, long image_nx, long image_nn,int assembleInterpolation) {
    
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
            image[image_index]= corrected_data[i];
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
      pixel_value = corrected_data[i];
		
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
void assemble2Dmask(cEventData *eventData, cGlobal *global) {
    
    if(global->assemble2DMask) {
        DETECTOR_LOOP {
            long		pix_nn = global->detector[detID].pix_nn;
            long		image_nx = global->detector[detID].image_nx;
            long		image_nn = global->detector[detID].image_nn;
            float		*pix_x = global->detector[detID].pix_x;
            float		*pix_y = global->detector[detID].pix_y;
            uint16_t  *pixelmask = eventData->detector[detID].pixelmask;
            uint16_t	*image_pixelmask = eventData->detector[detID].image_pixelmask;
            int       assembleInterpolation = global->assembleInterpolation;
            assemble2Dmask(image_pixelmask,pixelmask, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);
    }
  }	
}



/*
 *	Interpolate binary mask using pre-defined pixel mapping (as loaded from .h5 file)
 *      Options are dominant in united pixels
 *	input data: uint16_t
 *	output data: uint16_t
 */
void assemble2Dmask(uint16_t *assembled_mask, uint16_t *original_mask, float *pix_x, float *pix_y, long pix_nn, long image_nx, long image_nn,int assembleInterpolation) {
  
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

