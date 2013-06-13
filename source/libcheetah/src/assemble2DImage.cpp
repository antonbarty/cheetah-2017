//
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


void assemble2Dmask(cEventData *eventData, cGlobal *global) {

  if(global->assemble2DMask) {
    DETECTOR_LOOP {
      long		pix_nn = global->detector[detID].pix_nn;
      long		image_nx = global->detector[detID].image_nx;
      long		image_nn = global->detector[detID].image_nn;
      float		*pix_x = global->detector[detID].pix_x;
      float		*pix_y = global->detector[detID].pix_y;
      uint16_t        *pixelmask = eventData->detector[detID].pixelmask;
      uint16_t	*image_pixelmask = eventData->detector[detID].image_pixelmask;
      int             assembleInterpolation = global->assembleInterpolation;
      assemble2Dmask(image_pixelmask,pixelmask, pix_x, pix_y, pix_nn, image_nx, image_nn, assembleInterpolation);
    }
  }
}



/*
 *	Interpolate raw (corrected) cspad data into a physical 2D image
 *	input data: float
 *	output data: int16_t
 */
void assemble2Dimage(int16_t *image, float *corrected_data, float *pix_x, float *pix_y, long pix_nn, long image_nx, long image_nn,int assembleInterpolation) {

  if(assembleInterpolation == ASSEMBLE_INTERPOLATION_LINEAR){
  
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
    
	
    
    // Check for int16 overflow
    for(long i=0;i<image_nn;i++){
      if(lrint(data[i]) > 32767) 
	data[i]=32767;
      if(lrint(data[i]) < -32767) 
	data[i]=-32767;
    }
	
	
    // Copy interpolated image across into int_16 image array
    for(long i=0;i<image_nn;i++){
      image[i] = (int16_t) lrint(data[i]);
    }
	
	
    // Free temporary arrays
    free(data);
    free(weight);
  }
  else if(assembleInterpolation == ASSEMBLE_INTERPOLATION_NEAREST){

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
      if (image_index >= image_nn || image_index < 0)
	{
	  printf("Image index over/underflow %d %d %d\n", ix, iy, image_index);
	  continue;
	}
      // Check for int16 overflow
      if(lrint(corrected_data[i]) > 32767) 
	image[image_index]=32767;
      else if(lrint(corrected_data[i]) < -32767) 
	image[image_index]=-32767;
      else
	image[image_index]= (int16_t) lrint(corrected_data[i]);
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

void downsample(cEventData *eventData, cGlobal *global){

  DETECTOR_LOOP {
    if(global->detector[detID].downsampling > 1){

      long		image_nx = global->detector[detID].image_nx;
      long		image_nn = global->detector[detID].image_nn;
      int16_t		*image = eventData->detector[detID].image;
      uint16_t	        *image_pixelmask = eventData->detector[detID].image_pixelmask;
      long		imageXxX_nx = global->detector[detID].imageXxX_nx;
      long		imageXxX_nn = global->detector[detID].imageXxX_nn;
      int16_t		*imageXxX = eventData->detector[detID].imageXxX;
      uint16_t	        *imageXxX_pixelmask = eventData->detector[detID].imageXxX_pixelmask;
      float             rescale = global->detector[detID].downsamplingRescale;

      downsampleImage(image,imageXxX,image_nn,image_nx,imageXxX_nn,imageXxX_nx,rescale);
      downsampleMask(image_pixelmask,imageXxX_pixelmask,image_nn,image_nx,imageXxX_nn,imageXxX_nx);
    }
  }
}

template <class T>
void downsampleImage(T *img,T *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, float rescale = 1.){
  long x0,y0;
  long x1,y1;
  long downsampling = img_nx/imgXxX_nx;
  long i0,i1;
  double int16_t_max = 32767.;

  double *temp = calloc(imgXxX_nn,sizeof(double));
  
  for(i0 = 0;i0<img_nn;i0++){
    x0 = i0%img_nx;
    y0 = i0/img_nx;
    x1 = x0/downsampling;
    y1 = y0/downsampling;
    i1 = y1*imgXxX_nx + x1;
    temp[i1] += (double) img[i0];
  }

  for(i1 = 0;i1<imgXxX_nn;i1++){
    if (typeid(T) == typeid(int16_t)){
      // Check for overflow and clamp in case
      if(temp[i1]>int16_t_max){
	temp[i1] = int16_t_max;
      }
    }
    // cast to type
    imgXxX[i1] = (T) temp[i1];
  }

  free(temp);
}

void downsampleMask(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx){
  long x0,y0;
  long x1,y1;
  long downsampling = img_nx/imgXxX_nx;
  long i0,i1;

  for(i1 = 0;i1<imgXxX_nn;i1++){
    mskXxX[i1] = 0;
  }
  for(i0 = 0;i0<img_nn;i0++){
    x0 = i0%img_nx;
    y0 = i0/img_nx;
    x1 = x0/downsampling;
    y1 = y0/downsampling;
    i1 = y1*imgXxX_nx + x1;
    mskXxX[i1] |= msk[i0];
  }
}
