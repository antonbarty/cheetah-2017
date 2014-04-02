//
//  downsample.cpp
//  cheetah
//
//  Created by Max Hantke on 13/06/13.
//  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>

#include "cheetahGlobal.h"
#include "cheetahEvent.h"

void downsampleImageConservative(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, float rescale, long downsampling){
	long x0,y0;
	long x1,y1;
	long i0,i1;

	for(i0 = 0;i0<img_nn;i0++){
		x0 = i0%img_nx;
		y0 = i0/img_nx;
		x1 = x0/downsampling;
		y1 = y0/downsampling;
		i1 = y1*imgXxX_nx + x1;
		imgXxX[i1] += rescale*img[i0];
	}
}

void downsampleImageConservative(int16_t *img,int16_t *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, float rescale, long downsampling){
	long i;
	float int16_t_max = 32767.;

	float *  temp = (float *) calloc(img_nn,sizeof(float));
	for(i = 0;i<img_nn;i++){
		temp[i] = (float) img[i];
	}
	float *  tempXxX = (float *) calloc(imgXxX_nn,sizeof(float));
	downsampleImageConservative(temp,tempXxX,img_nn,img_nx,imgXxX_nn,imgXxX_nx,rescale,downsampling);
	for(i = 0;i<imgXxX_nn;i++){
		// Check for overflow and clamp in case
		if(tempXxX[i]>int16_t_max){
			imgXxX[i] = 32767;
		} else {
			// cast to type
			imgXxX[i] = (int16_t)roundf(tempXxX[i]);
		}
	}
	free(temp);
	free(tempXxX);
}


// sub-pixels that have any of the mask_out_bits set are disregarded. New super-pixels are rescaled accordingly. Super-pixels with all sub-pixels masked out are set to 0
void downsampleImageNonConservative(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, float rescale, uint16_t *msk, long downsampling){
	long x0,y0;
	long x1,y1;
	long i0,i1,i;
	uint16_t mask_out_bits = PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_MISSING;
	bool good_pixel;
	float *tempN;
	tempN = (float *) calloc(imgXxX_nn,sizeof(float));

	for(i0 = 0;i0<img_nn;i0++){
		x0 = i0%img_nx;
		y0 = i0/img_nx;
		x1 = x0/downsampling;
		y1 = y0/downsampling;
		i1 = y1*imgXxX_nx + x1;
		good_pixel = (float) isNoneOfBitOptionsSet(msk[i0],mask_out_bits);
		imgXxX[i1] += good_pixel*rescale*img[i0];
		tempN[i1] += good_pixel;
	}
	for(i = 0;i<imgXxX_nn;i++){
		if (tempN[i] != 0.){
			imgXxX[i] *= downsampling*downsampling/tempN[i];
		}
	}
	free(tempN);
}


void downsampleMaskConservative(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, long downsampling){
	long x0,y0;
	long x1,y1;
	long imgXxX_ny = imgXxX_nn/imgXxX_nx;
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

// downsample mask by only masking if all sub-pixels share a bit, if all pixels are having any of the critical bits set they are masked too 
void downsampleMaskNonConservative(uint16_t *msk,uint16_t *mskXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, long downsampling){
	long x0,y0;
	long x1,y1;
	long imgXxX_ny = imgXxX_nn/imgXxX_nx;
	long i0,i1;
	long *tempN;
	uint16_t *tempM;
	uint16_t mask_out_bits = PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_MISSING;
	tempN = (long *) calloc(imgXxX_nn,sizeof(long));
	tempM = (uint16_t *) calloc(imgXxX_nn,sizeof(uint16_t));

	for(i1 = 0;i1<imgXxX_nn;i1++){
		mskXxX[i1] = 0;
	}
	for(i0 = 0;i0<img_nn;i0++){
		x0 = i0%img_nx;
		y0 = i0/img_nx;
		x1 = x0/downsampling;
		y1 = y0/downsampling;
		i1 = y1*imgXxX_nx + x1;
		if (isAnyOfBitOptionsSet(msk[i0],mask_out_bits)){
			// at least one mask-out-bit is set
			tempM[i1] |= msk[i0];
		} else {
			// no mask-out-bit is set
			tempN[i1] += 1;
			mskXxX[i1] |= msk[i0];
		}
	}
	for(i1 = 0;i1<imgXxX_nn;i1++){
		// pixels with all subpixels crucially bad will be masked out conservatively
		if (tempN[i1] == 0){
			mskXxX[i1] = tempM[i1];
		}
	}
	free(tempN);
	free(tempM);
}

void downsample(cEventData *eventData, cGlobal *global){

	DETECTOR_LOOP {
		if(global->detector[detID].downsampling > 1){

			long              downsampling = global->detector[detID].downsampling;
			long		image_nx = global->detector[detID].image_nx;
			long		image_nn = global->detector[detID].image_nn;
			float		*image = eventData->detector[detID].image;
			uint16_t	        *image_pixelmask = eventData->detector[detID].image_pixelmask;
			long		imageXxX_nx = global->detector[detID].imageXxX_nx;
			long		imageXxX_nn = global->detector[detID].imageXxX_nn;
			float		*imageXxX = eventData->detector[detID].imageXxX;
			uint16_t	        *imageXxX_pixelmask = eventData->detector[detID].imageXxX_pixelmask;
			float             rescale = global->detector[detID].downsamplingRescale;

			if (global->detector[detID].downsamplingConservative == 1){
				downsampleImageConservative(image,imageXxX,image_nn,image_nx,imageXxX_nn,imageXxX_nx,rescale,downsampling);
				downsampleMaskConservative(image_pixelmask,imageXxX_pixelmask,image_nn,image_nx,imageXxX_nn,imageXxX_nx,downsampling);
			} else {
				downsampleImageNonConservative(image,imageXxX,image_nn,image_nx,imageXxX_nn,imageXxX_nx,rescale,image_pixelmask,downsampling);
				downsampleMaskNonConservative(image_pixelmask,imageXxX_pixelmask,image_nn,image_nx,imageXxX_nn,imageXxX_nx,downsampling);	
			}
      
		}
	}
}
