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


void downsampleImage(int16_t *img,int16_t *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, float rescale){
  long x0,y0;
  long x1,y1;
  long downsampling = img_nx/imgXxX_nx;
  long i0,i1;
  double int16_t_max = 32767.;

  double *temp;
  temp = (double *) calloc(imgXxX_nn,sizeof(double));

  for(i0 = 0;i0<img_nn;i0++){
    x0 = i0%img_nx;
    y0 = i0/img_nx;
    x1 = x0/downsampling;
    y1 = y0/downsampling;
    i1 = y1*imgXxX_nx + x1;
    temp[i1] += rescale*img[i0];
  }

  for(i1 = 0;i1<imgXxX_nn;i1++){
    // Check for overflow and clamp in case
    if(temp[i1]>int16_t_max){
      temp[i1] = int16_t_max;
    }
    // cast to type
    imgXxX[i1] = temp[i1];
  }

  free(temp);
}

void downsampleImage(float *img,float *imgXxX,long img_nn, long img_nx, long imgXxX_nn, long imgXxX_nx, float rescale){
  long x0,y0;
  long x1,y1;
  long downsampling = img_nx/imgXxX_nx;
  long i0,i1;

  for(i0 = 0;i0<img_nn;i0++){
    x0 = i0%img_nx;
    y0 = i0/img_nx;
    x1 = x0/downsampling;
    y1 = y0/downsampling;
    i1 = y1*imgXxX_nx + x1;
    img[i1] += rescale*img[i0];
  }
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
