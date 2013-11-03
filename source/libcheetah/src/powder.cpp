/*
 *  powder.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 23/11/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

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

void addToPowder(cEventData *eventData, cGlobal *global) {

  int hit = eventData->hit;
    
  DETECTOR_LOOP {
    if(global->generateDarkcal || global->generateGaincal){
		addToPowder(eventData, global, 0, detID);
    }
	else {
		if(!hit && global->powderSumBlanks){
			addToPowder(eventData, global, 0, detID);
		}
		if(hit && global->powderSumHits){
			addToPowder(eventData, global, 1, detID);
		}
	}
  }
}


void addToPowder(cEventData *eventData, cGlobal *global, int powderClass, int detID){
	
    // Dereference common variable
    //long	radial_nn = global->detector[detID].radial_nn;
    long	pix_nn = global->detector[detID].pix_nn;
    long	pix_nx = global->detector[detID].pix_nx;
    long	pix_ny = global->detector[detID].pix_ny;

    long	image_nn = global->detector[detID].image_nn;
    long	image_nx = global->detector[detID].image_nx;
    long	image_ny = global->detector[detID].image_ny;

    long	imageXxX_nn = global->detector[detID].imageXxX_nn;
    long	imageXxX_nx = global->detector[detID].imageXxX_nx;
    long	imageXxX_ny = global->detector[detID].imageXxX_ny;

    double  *buffer;
	
    
	// Increment counter of number of powder patterns
	pthread_mutex_lock(&global->detector[detID].powderCorrected_mutex[powderClass]);
	global->detector[detID].nPowderFrames[powderClass] += 1;
	if(detID == 0)
		global->nPowderFrames[powderClass] += 1;
	pthread_mutex_unlock(&global->detector[detID].powderCorrected_mutex[powderClass]);
	

	
    /*
     *  Sum of raw detector data (no thresholds)
     */
    pthread_mutex_lock(&global->detector[detID].powderRaw_mutex[powderClass]);
    for(long i=0; i<pix_nn; i++) 
        global->detector[detID].powderRaw[powderClass][i] += eventData->detector[detID].raw_data[i];
    pthread_mutex_unlock(&global->detector[detID].powderRaw_mutex[powderClass]);			

    
    /*
     *  Sum of raw data squared (no thresholds, for calculating variance)
     */
    buffer = (double*) calloc(pix_nn, sizeof(double));
    if(!global->usePowderThresh) {
        for(long i=0; i<pix_nn; i++)
            buffer[i] = (eventData->detector[detID].raw_data[i])*(eventData->detector[detID].raw_data[i]);
    }
    else {
        for(long i=0; i<pix_nn; i++){
            if(eventData->detector[detID].raw_data[i] > global->powderthresh)
                buffer[i] = (eventData->detector[detID].raw_data[i])*(eventData->detector[detID].raw_data[i]);
            else
                buffer[i] = 0;
        }
    }
    pthread_mutex_lock(&global->detector[detID].powderRawSquared_mutex[powderClass]);
    for(long i=0; i<pix_nn; i++){
        global->detector[detID].powderRawSquared[powderClass][i] += buffer[i];
    }
    pthread_mutex_unlock(&global->detector[detID].powderRawSquared_mutex[powderClass]);
    free(buffer);
    

    
    
    /*
     *  Sum of corrected data (with any powder threshold or masks)
     */
    pthread_mutex_lock(&global->detector[detID].powderCorrected_mutex[powderClass]);
    if(!global->usePowderThresh) {
        for(long i=0; i<pix_nn; i++)
            global->detector[detID].powderCorrected[powderClass][i] += eventData->detector[detID].corrected_data[i];
    }
    else {
        for(long i=0; i<pix_nn; i++) {
            if(eventData->detector[detID].corrected_data[i] > global->powderthresh)
                global->detector[detID].powderCorrected[powderClass][i] += eventData->detector[detID].corrected_data[i];
        }
    }
    pthread_mutex_unlock(&global->detector[detID].powderCorrected_mutex[powderClass]);

    


    /*
     *  Sum of corrected data squared (with any powder threshold or masks, for calculating variance)
     */
    buffer = (double*) calloc(pix_nn, sizeof(double));
    if(!global->usePowderThresh) {
        for(long i=0; i<pix_nn; i++)
            buffer[i] = (eventData->detector[detID].corrected_data[i])*(eventData->detector[detID].corrected_data[i]);
    }
    else {
        for(long i=0; i<pix_nn; i++){
            if(eventData->detector[detID].corrected_data[i] > global->powderthresh)
                buffer[i] = (eventData->detector[detID].corrected_data[i])*(eventData->detector[detID].corrected_data[i]);
            else
                buffer[i] = 0;
        }
    }
    pthread_mutex_lock(&global->detector[detID].powderCorrectedSquared_mutex[powderClass]);
    for(long i=0; i<pix_nn; i++){
        global->detector[detID].powderCorrectedSquared[powderClass][i] += buffer[i];
    }
    pthread_mutex_unlock(&global->detector[detID].powderCorrectedSquared_mutex[powderClass]);
    free(buffer);


    /*
     *  Sum of peaks centroids
     */
	if (eventData->nPeaks > 0) {
		pthread_mutex_lock(&global->detector[detID].powderAssembled_mutex[powderClass]);
		long	ci, cx, cy, val, e;

		for(long i=0; i<=eventData->peaklist.nPeaks && i<eventData->peaklist.nPeaks_max; i++) {
						
			// Peak position and value
			ci = eventData->peaklist.peak_com_index[i];
			cx = lrint(eventData->peaklist.peak_com_x[i]);
			cy = lrint(eventData->peaklist.peak_com_y[i]);
			val = eventData->peaklist.peak_totalintensity[i];
			
			// Bounds check
			if(cx < 0 || cx > (pix_nx-1) ) continue;
			if(cy < 0 || cy > (pix_ny-1) ) continue;
			if(ci < 0 || cx > (pix_nn-1) ) continue;
			
			// Element in 1D array
			e = cx + pix_nx*cy;
			if(e < 0 || e > (pix_nn-1) ) continue;
			global->detector[detID].powderPeaks[powderClass][e] += val;
			//global->detector[detID].powderPeaks[powderClass][ci] += val;
		}
		pthread_mutex_unlock(&global->detector[detID].powderAssembled_mutex[powderClass]);
	}

    // Min nPeaks: Pattern
    if(eventData->nPeaks < global->nPeaksMin[powderClass]){
        pthread_mutex_lock(&global->detector[detID].correctedMin_mutex[powderClass]);
        global->nPeaksMin[powderClass] = eventData->nPeaks;
        memcpy(global->detector[detID].correctedMin[powderClass],eventData->detector[detID].corrected_data,sizeof(float)*pix_nn);
        pthread_mutex_unlock(&global->detector[detID].correctedMin_mutex[powderClass]);
    }

    // Max nPeaks: Pattern
    if(eventData->nPeaks > global->nPeaksMax[powderClass]){
        pthread_mutex_lock(&global->detector[detID].correctedMax_mutex[powderClass]);
        global->nPeaksMax[powderClass] = eventData->nPeaks;
        memcpy(global->detector[detID].correctedMax[powderClass],eventData->detector[detID].corrected_data,sizeof(float)*pix_nn);
        pthread_mutex_unlock(&global->detector[detID].correctedMax_mutex[powderClass]);
    }
   
    // Only assemble data if explicitly specified by configuration
    if(global->assemblePowders) {
      if(global->assemble2DImage){
        // Assembled data
        pthread_mutex_lock(&global->detector[detID].powderAssembled_mutex[powderClass]);
        if(!global->usePowderThresh) {
	  for(long i=0; i<image_nn; i++){
	      global->detector[detID].powderAssembled[powderClass][i] += eventData->detector[detID].image[i];
	      global->detector[detID].powderAssembledSquared[powderClass][i] += eventData->detector[detID].image[i]*eventData->detector[detID].image[i];
	  }
        }
        else {
            for(long i=0; i<image_nn; i++){
                if(eventData->detector[detID].image[i] > global->powderthresh)
                    global->detector[detID].powderAssembled[powderClass][i] += eventData->detector[detID].image[i];
		    global->detector[detID].powderAssembledSquared[powderClass][i] += eventData->detector[detID].image[i]*eventData->detector[detID].image[i];
            }
        }
        pthread_mutex_unlock(&global->detector[detID].powderAssembled_mutex[powderClass]);
      }
      if(global->detector[detID].downsampling > 1){
	// Downsampled data
	pthread_mutex_lock(&global->detector[detID].powderDownsampled_mutex[powderClass]);
	if(!global->usePowderThresh) {
	  for(long i=0; i<imageXxX_nn; i++){
	      global->detector[detID].powderDownsampled[powderClass][i] += eventData->detector[detID].imageXxX[i];
	      global->detector[detID].powderDownsampledSquared[powderClass][i] += eventData->detector[detID].imageXxX[i]*eventData->detector[detID].imageXxX[i];
	  }
        }
        else {
            for(long i=0; i<imageXxX_nn; i++){
                if(eventData->detector[detID].image[i] > global->powderthresh)
                    global->detector[detID].powderDownsampled[powderClass][i] += eventData->detector[detID].imageXxX[i];
                    global->detector[detID].powderDownsampledSquared[powderClass][i] += eventData->detector[detID].imageXxX[i]*eventData->detector[detID].imageXxX[i];
            }
        }
        pthread_mutex_unlock(&global->detector[detID].powderDownsampled_mutex[powderClass]);
      }
    }
    
    

}


/*
 *	Wrapper for saving all powder patterns for a detector
 *  Also for deciding whether to calculate gain, darkcal, etc.
 */
void saveRunningSums(cGlobal *global) {
    for(int detID=0; detID<global->nDetectors; detID++) {
        saveRunningSums(global, detID);
    }
}


void saveRunningSums(cGlobal *global, int detID) {
    
    // Assemble 2D powder patterns using geometry (since we don't sum assembled patterns any more)
    assemble2Dpowder(global);


    //	Save powder patterns from different classes
    printf("Writing intermediate powder patterns to file\n");
    for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
        if(global->powderSumBlanks && powderType == 0)
            savePowderPattern(global, detID, powderType);
        else if (global->powderSumHits && powderType > 0)
            savePowderPattern(global, detID, powderType);
    }
	
    // Compute and save darkcal
    if(global->generateDarkcal) {
        saveDarkcal(global, detID);
        savePowderPattern(global, detID, 0);
    }
	
    // Compute and save gain calibration
    if(global->generateGaincal) {
        saveGaincal(global, detID);
        savePowderPattern(global, detID, 0);
    }
}



/*
 *  Actually save the powder pattern to file
 */
void savePowderPattern(cGlobal *global, int detID, int powderType) {
	
    // Dereference common variables
    cPixelDetectorCommon     *detector = &(global->detector[detID]);
    long	radial_nn = detector->radial_nn;
    long	pix_nn = detector->pix_nn;
    long	image_nn = detector->image_nn;
    double  *bufferAssembled;
    int16_t *bufferAssembledNPeaksMin;
    int16_t *bufferAssembledNPeaksMax;
	long	nframes = detector->nPowderFrames[powderType];

    /*
     *	Filename
     */
    char	filename[1024];
    char	filenamebase[1024];
    sprintf(filenamebase,"r%04u-detector%d-class%d", global->runNumber, detID, powderType);
    sprintf(filename,"%s-sum.h5",filenamebase);
    printf("%s\n",filename);
	
    /*
     *	Calculate powder patterns
     */
    // Raw data
    double *bufferRaw = (double*) calloc(pix_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderRaw_mutex[powderType]);
    memcpy(bufferRaw, detector->powderRaw[powderType], pix_nn*sizeof(double));
    pthread_mutex_unlock(&detector->powderRaw_mutex[powderType]);
    
    // Corrected data
    double *bufferCorrected = (double*) calloc(pix_nn, sizeof(double));
    double *radialAverageCorrected = (double*) calloc(radial_nn, sizeof(double));
    double *radialAverageCorrectedCounter = (double*) calloc(radial_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderCorrected_mutex[powderType]);
    memcpy(bufferCorrected, detector->powderCorrected[powderType], pix_nn*sizeof(double));
    pthread_mutex_unlock(&detector->powderCorrected_mutex[powderType]);
    calculateRadialAverage(bufferCorrected, radialAverageCorrected, radialAverageCorrectedCounter, global, detID);

    // Peak powder
    double *bufferPeaks = (double*) calloc(pix_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderAssembled_mutex[powderType]);
    memcpy(bufferPeaks, detector->powderPeaks[powderType], pix_nn*sizeof(double));
    pthread_mutex_unlock(&detector->powderAssembled_mutex[powderType]);
    calculateRadialAverage(bufferCorrected, radialAverageCorrected, radialAverageCorrectedCounter, global, detID);

    // Assembled image for viewing
    bufferAssembled = (double*) calloc(image_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderAssembled_mutex[powderType]);
    memcpy(bufferAssembled, detector->powderAssembled[powderType], image_nn*sizeof(double));
    pthread_mutex_unlock(&detector->powderAssembled_mutex[powderType]);
    
    // Data squared (for calculation of variance)
    double *bufferCorrectedSquared = (double*) calloc(pix_nn, sizeof(double));
    double *radialAverageCorrectedSquared = (double*) calloc(radial_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderCorrectedSquared_mutex[powderType]);
    memcpy(bufferCorrectedSquared, detector->powderCorrectedSquared[powderType], pix_nn*sizeof(double));
    pthread_mutex_unlock(&detector->powderCorrectedSquared_mutex[powderType]);
    calculateRadialAverage(bufferCorrectedSquared, radialAverageCorrectedSquared, radialAverageCorrectedCounter, global, detID);
	
    // Sigma (variance)
    double *bufferCorrectedSigma = (double*) calloc(pix_nn, sizeof(double));
    double *radialAverageCorrectedSigma = (double*) calloc(radial_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderCorrected_mutex[powderType]);
    pthread_mutex_lock(&detector->powderCorrectedSquared_mutex[powderType]);
    for(long i=0; i<pix_nn; i++){
        bufferCorrectedSigma[i] =
        sqrt( fabs(bufferCorrectedSquared[i]/nframes - (bufferCorrected[i]/nframes)*(bufferCorrected[i]/nframes)) );
    }
    pthread_mutex_unlock(&detector->powderCorrected_mutex[powderType]);
    pthread_mutex_unlock(&detector->powderCorrectedSquared_mutex[powderType]);
    calculateRadialAverage(bufferCorrectedSigma, radialAverageCorrectedSigma, radialAverageCorrectedCounter, global, detID);

    float *bufferCorrectedNPeaksMin = (float*) calloc(pix_nn,sizeof(float));
    pthread_mutex_lock(&detector->correctedMin_mutex[powderType]);
    memcpy(bufferCorrectedNPeaksMin, detector->correctedMin[powderType], pix_nn*sizeof(float));
    pthread_mutex_unlock(&detector->correctedMin_mutex[powderType]);

    float *bufferCorrectedNPeaksMax = (float*) calloc(pix_nn,sizeof(float));
    pthread_mutex_lock(&detector->correctedMax_mutex[powderType]);
    memcpy(bufferCorrectedNPeaksMax, detector->correctedMax[powderType], pix_nn*sizeof(float));
    pthread_mutex_unlock(&detector->correctedMax_mutex[powderType]);

    if(global->assemble2DImage) {
        bufferAssembledNPeaksMin = (int16_t*) calloc(image_nn,sizeof(int16_t));
        pthread_mutex_lock(&detector->assembledMin_mutex[powderType]);
        memcpy(bufferAssembledNPeaksMin, detector->assembledMin[powderType], image_nn*sizeof(int16_t));
        pthread_mutex_unlock(&detector->assembledMin_mutex[powderType]);
        
        bufferAssembledNPeaksMax = (int16_t*) calloc(image_nn,sizeof(int16_t));
        pthread_mutex_lock(&detector->assembledMax_mutex[powderType]);
        memcpy(bufferAssembledNPeaksMax, detector->assembledMax[powderType], image_nn*sizeof(int16_t));
        pthread_mutex_unlock(&detector->assembledMax_mutex[powderType]);
    }
  

    
    /*
     *	Mess of stuff for writing the compound HDF5 file
     */
    hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
    //herr_t r;
    hsize_t		size[2];
    hsize_t		max_size[2];
	hid_t		h5compression;

	
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
	}
	else {
		h5compression = H5P_DEFAULT;
	}

    // Write image data in Raw layout
    size[0] = detector->pix_ny;
    size[1] = detector->pix_nx;
    sh = H5Screate_simple(2, size, NULL);
    //H5Sget_simple_extent_dims(sh, size, max_size);
	if (global->h5compress) {
		H5Pset_chunk(h5compression, 2, size);
		H5Pset_deflate(h5compression, global->h5compress);		// Compression levels are 0 (none) to 9 (max)
	}
    dh = H5Dcreate(gh, "rawdata", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferRaw);
    H5Dclose(dh);
    
    dh = H5Dcreate(gh, "correcteddata", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferCorrected);
    H5Dclose(dh);

	dh = H5Dcreate(gh, "peakpowder", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferPeaks);
    H5Dclose(dh);
	
    //H5Sget_simple_extent_dims(sh, size, size);
    dh = H5Dcreate(gh, "correcteddatasquared", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferCorrectedSquared);
    H5Dclose(dh);
	
    //H5Sget_simple_extent_dims(sh, size, size);
    dh = H5Dcreate(gh, "correcteddatasigma", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferCorrectedSigma);
    H5Dclose(dh);
    
    //dh = H5Dcreate(gh, "correcteddatanpeaksmin", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //if (dh < 0) ERROR("Could not create dataset.\n");
    //H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferCorrectedNPeaksMin);
    //H5Dclose(dh);
    
    //dh = H5Dcreate(gh, "correcteddatanpeaksmax", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //if (dh < 0) ERROR("Could not create dataset.\n");
    //H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferCorrectedNPeaksMax);
    //H5Dclose(dh);
    H5Sclose(sh);
    
    if(global->assemble2DImage) {
        // Write assembled image data
        size[0] = detector->image_nn/detector->image_nx;
        size[1] = detector->image_nx;
        sh = H5Screate_simple(2, size, NULL);
		if (global->h5compress) {
			H5Pset_chunk(h5compression, 2, size);
			H5Pset_deflate(h5compression, global->h5compress);		// Compression levels are 0 (none) to 9 (max)
		}

        //H5Sget_simple_extent_dims(sh, size, max_size);
        dh = H5Dcreate(gh, "assembleddata", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
        if (dh < 0) ERROR("Could not create dataset.\n");
        H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferAssembled);
        H5Dclose(dh);
        
        //dh = H5Dcreate(gh, "assembleddatanpeaksmin", H5T_NATIVE_INT16, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        //if (dh < 0) ERROR("Could not create dataset.\n");
        //H5Dwrite(dh, H5T_NATIVE_INT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferAssembledNPeaksMin);
        //H5Dclose(dh);
        
        //dh = H5Dcreate(gh, "assembleddatanpeaksmax", H5T_NATIVE_INT16, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        //if (dh < 0) ERROR("Could not create dataset.\n");
        //H5Dwrite(dh, H5T_NATIVE_INT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferAssembledNPeaksMax);
        //H5Dclose(dh);
        H5Sclose(sh);
        
        H5Lcreate_soft( "/data/assembleddata", fh, "/data/data",0,0);
    }
    else {
        H5Lcreate_soft( "/data/correcteddata", fh, "/data/data",0,0);
    }

    // Save radial averages
    size[0] = radial_nn;
    max_size[0] = radial_nn;
    sh = H5Screate_simple(1, size, NULL);
    //H5Sget_simple_extent_dims(sh, size, max_size);
	
    dh = H5Dcreate(gh, "radialAverage", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCorrected);
    H5Dclose(dh);
    
    dh = H5Dcreate(gh, "radialAverageSquared", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCorrectedSquared);
    H5Dclose(dh);
    
    dh = H5Dcreate(gh, "radialAverageSigma", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCorrectedSigma);
    H5Dclose(dh);
    
    dh = H5Dcreate(gh, "radialAverageCounter", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCorrectedCounter);
    H5Dclose(dh);
    H5Sclose(sh);
	
	
    // Save frame count
    size[0] = 1;
    sh = H5Screate_simple(1, size, NULL );
    //H5Sget_simple_extent_dims(sh, size, size);
    //sh = H5Screate(H5S_SCALAR);
	
    dh = H5Dcreate(gh, "nframes", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &detector->nPowderFrames[powderType] );
    H5Dclose(dh);
    H5Sclose(sh);
	
	
    // Clean up stale HDF5 links
    H5Gclose(gh);
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
     *	Clean up stuff
     */
    free(bufferRaw);
    free(bufferCorrected);
    free(bufferCorrectedSquared);
    free(bufferCorrectedSigma);
    free(bufferCorrectedNPeaksMin);
    free(bufferCorrectedNPeaksMax);
	free(bufferPeaks);
    free(radialAverageCorrected);
    free(radialAverageCorrectedSquared);
    free(radialAverageCorrectedSigma);
    free(radialAverageCorrectedCounter);
    if(global->assemble2DImage) {
        free(bufferAssembled);
        free(bufferAssembledNPeaksMin);
        free(bufferAssembledNPeaksMax);
    }
    for(long i=0; i<global->nPowderClasses; i++) {
        fflush(global->powderlogfp[i]);
    }

}



/*
 *	Compute and save dark calibration
 */
void saveDarkcal(cGlobal *global, int detID) {
	
  // Dereference common variables
  cPixelDetectorCommon     *detector = &(global->detector[detID]);
  long	pix_nn = detector->pix_nn;
  char	filename[1024];
	
  printf("Processing darkcal\n");
  sprintf(filename,"r%04u-%s-darkcal.h5",global->runNumber, detector->detectorName);
  float *buffer = (float*) calloc(pix_nn, sizeof(float));
  pthread_mutex_lock(&detector->powderCorrected_mutex[0]);
  for(long i=0; i<pix_nn; i++)
    buffer[i] = detector->powderCorrected[0][i]/detector->nPowderFrames[0];
  pthread_mutex_unlock(&detector->powderCorrected_mutex[0]);
  printf("Saving darkcal to file: %s\n", filename);
  writeSimpleHDF5(filename, buffer, detector->pix_nx, detector->pix_ny, H5T_NATIVE_FLOAT);	
  free(buffer);
}



/*
 *	Compute and save gain calibration
 */
void saveGaincal(cGlobal *global, int detID) {
	
	printf("Processing gaincal\n");

	// Dereference common variables
	cPixelDetectorCommon     *detector = &(global->detector[detID]);
	long	pix_nn = detector->pix_nn;
    float   nframes;
	
	// Grab a snapshot of the current running sum
	pthread_mutex_lock(&detector->powderCorrected_mutex[0]);
	float *buffer = (float*) calloc(pix_nn, sizeof(float));
	for(long i=0; i<pix_nn; i++)
		buffer[i] = detector->powderCorrected[0][i];
    for(long i=0; i<pix_nn; i++)
		buffer[i] /= detector->nPowderFrames[0];
	pthread_mutex_unlock(&detector->powderCorrected_mutex[0]);

    
    
	// Find median value (this value will become gain=1)
	float	dc;
	float *buffer2 = (float*) calloc(pix_nn, sizeof(float));
	for(long i=0; i<pix_nn; i++) {
		buffer2[i] = buffer[i];
	}
	dc = kth_smallest(buffer2, pix_nn, lrint(0.5*pix_nn));
	printf("offset=%f\n",dc);
	free(buffer2);
    
    // Trap a potential error condition
	if(dc <= 0){
		printf("Error calculating gain, offset = %f\n",dc);
		return;
	}
    
    // Calculate gain
	// gain=1 for a median value pixel
    // Possibly bound gain between 0.1 and 10
	for(long i=0; i<pix_nn; i++) {
		buffer[i] /= (float) dc;
		//if(buffer[i] < 0.1 || buffer[i] > 10)
		//	buffer[i]=0;
	}

	char	filename[1024];
	sprintf(filename,"r%04u-%s-gaincal.h5",global->runNumber, detector->detectorName);
	printf("Saving gaincal to file: %s\n", filename);
	writeSimpleHDF5(filename, buffer, detector->pix_nx, detector->pix_ny, H5T_NATIVE_FLOAT);
	free(buffer);
}


/*
 *	Write a single powder pattern to file
 */
void writePowderData(char *filename, void *data, int width, int height, void *radialAverageCorrected, void *radialAverageCorrectedCounter, long radial_nn, long nFrames, int type) 
{
  hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
  //herr_t r;
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
	
  // Write image data
  size[0] = height;
  size[1] = width;
  max_size[0] = height;
  max_size[1] = width;
  sh = H5Screate_simple(2, size, NULL);
  dh = H5Dcreate(gh, "data", type, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	
  H5Sget_simple_extent_dims(sh, size, max_size);
  H5Dwrite(dh, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  H5Dclose(dh);
	
	
  // Save radial averages
  size[0] = radial_nn;
  sh = H5Screate_simple(1, size, NULL);
	
  dh = H5Dcreate(gh, "radialAverageCorrected", type, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCorrected);
  H5Dclose(dh);
	
  dh = H5Dcreate(gh, "radialAverageCorrectedCounter", type, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, radialAverageCorrectedCounter);
  H5Dclose(dh);	
  H5Sclose(sh);
	
	
  // Save frame count
  size[0] = 1;
  sh = H5Screate_simple(1, size, NULL );
  //sh = H5Screate(H5S_SCALAR);
	
  dh = H5Dcreate(gh, "nframes", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &nFrames );
  H5Dclose(dh);
  H5Sclose(sh);
	
	
  // Close group
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

