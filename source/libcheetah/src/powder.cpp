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
	int	powderClass = eventData->powderClass;
    
	DETECTOR_LOOP {
		if(global->generateDarkcal || global->generateGaincal) {
			addToPowder(eventData, global, 0, detIndex);
		} else if (hit && global->powderSumHits) {
			addToPowder(eventData, global, powderClass, detIndex);
		} else if (!hit && global->powderSumBlanks) {
			addToPowder(eventData, global, powderClass, detIndex);
		}
	}
}


void addToPowder(cEventData *eventData, cGlobal *global, int powderClass, long detIndex){

	// Increment counter of number of powder patterns
	pthread_mutex_lock(&global->detector[detIndex].powderData_mutex[powderClass]);
	global->detector[detIndex].nPowderFrames[powderClass] += 1;
	if(detIndex == 0)
		global->nPowderFrames[powderClass] += 1;
	pthread_mutex_unlock(&global->detector[detIndex].powderData_mutex[powderClass]);
	
    double  *buffer;	
	
	FOREACH_DATAFORMAT_T(i_f, cDataVersion::DATA_FORMATS) {
		if (isBitOptionSet(global->detector[detIndex].powderFormat,*i_f)) {
			cDataVersion dataV(&eventData->detector[detIndex], &global->detector[detIndex], global->detector[detIndex].powderVersion, *i_f);
			while (dataV.next()) {
				pthread_mutex_t mutex = dataV.getPowderMutex(powderClass);
				float * data = dataV.getData();
				double * powder = dataV.getPowder(powderClass);
				double * powder_squared = dataV.getPowderSquared(powderClass);
				// Powder
				pthread_mutex_lock(&mutex);
				for(long i=0; i<dataV.pix_nn; i++) {
					powder[i] += data[i];
				}			
				pthread_mutex_unlock(&mutex);
				// Powder squared
				buffer = (double*) calloc(dataV.pix_nn, sizeof(double));
				if(!global->usePowderThresh) {
					for(long i=0; i<dataV.pix_nn; i++)
						buffer[i] = data[i]*data[i];
				}
				else {
					for(long i=0; i<dataV.pix_nn; i++){
						if(data[i] > global->powderthresh)
							buffer[i] = data[i]*data[i];
						else
							buffer[i] = 0;
					}
				}
				pthread_mutex_lock(&mutex);
				for(long i=0; i<dataV.pix_nn; i++){
					powder_squared[i] += buffer[i];
				}
				pthread_mutex_unlock(&mutex);
				free(buffer);
			}				
		}
	}
	
	/*
     *  Sum of peaks centroids
     */
	if (eventData->nPeaks > 0) {
		pthread_mutex_lock(&global->detector[detIndex].powderPeaks_mutex[powderClass]);
		long	ci, cx, cy,  e;
		double  val;

		long	pix_nn = global->detector[detIndex].pix_nn;
		long	pix_nx = global->detector[detIndex].pix_nx;
		long	pix_ny = global->detector[detIndex].pix_ny;
		
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
			global->detector[detIndex].powderPeaks[powderClass][e] += val;
			//global->detector[detIndex].powderPeaks[powderClass][ci] += val;
		}
		pthread_mutex_unlock(&global->detector[detIndex].powderPeaks_mutex[powderClass]);
	}

    // Min nPeaks
    if(eventData->nPeaks < global->nPeaksMin[powderClass]){
        pthread_mutex_lock(&global->nPeaksMin_mutex[powderClass]);
        global->nPeaksMin[powderClass] = eventData->nPeaks;
        //memcpy(global->detector[detIndex].correctedMin[powderClass],eventData->detector[detIndex].corrected_data,sizeof(float)*pix_nn);
        pthread_mutex_unlock(&global->nPeaksMin_mutex[powderClass]);
    }

    // Max nPeaks
    if(eventData->nPeaks > global->nPeaksMax[powderClass]){
        pthread_mutex_lock(&global->nPeaksMax_mutex[powderClass]);
        global->nPeaksMax[powderClass] = eventData->nPeaks;
        //memcpy(global->detector[detIndex].correctedMax[powderClass],eventData->detector[detIndex].corrected_data,sizeof(float)*pix_nn);
        pthread_mutex_unlock(&global->nPeaksMax_mutex[powderClass]);
    } 
}


void saveRunningSums(cGlobal *global, int detIndex) {
	//	Save powder patterns from different classes
    printf("Writing intermediate powder patterns to file\n");
    for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
        if(global->powderSumBlanks && powderType == 0)
            savePowderPattern(global, detIndex, powderType);
        else if (global->powderSumHits && powderType > 0)
            savePowderPattern(global, detIndex, powderType);
    }
	
    // Compute and save darkcal
    if(global->generateDarkcal) {
        saveDarkcal(global, detIndex);
        savePowderPattern(global, detIndex, 0);
    }
	
    // Compute and save gain calibration
    if(global->generateGaincal) {
        saveGaincal(global, detIndex);
        savePowderPattern(global, detIndex, 0);
    }
}

/*
 *	Wrapper for saving all powder patterns for a detector
 *  Also for deciding whether to calculate gain, darkcal, etc.
 */
void saveRunningSums(cGlobal *global) {
    for(int detIndex=0; detIndex<global->nDetectors; detIndex++) {
        saveRunningSums(global, detIndex);
    }
}

/*
 *  Actually save the powder pattern to file
 */
void savePowderPattern(cGlobal *global, int detIndex, int powderClass) {
	
    // Dereference common variables
    cPixelDetectorCommon     *detector = &(global->detector[detIndex]);
	long	nframes = detector->nPowderFrames[powderClass];

	// Define buffer variables
	double  *powderBuffer;	
	double  *powderSquaredBuffer;
	double  *powderSigmaBuffer;
	double  *bufferPeaks;
	char    sBuffer[1024];

    /*
     *	Filename
     */
    char	filename[1024];
    char	filenamebase[1024];
    sprintf(filenamebase,"r%04u-detector%ld-class%d", global->runNumber, global->detector[detIndex].detectorID, powderClass);
    sprintf(filename,"%s-sum.h5",filenamebase);
    printf("%s\n",filename);
	
    /*
     *	Mess of stuff for writing the compound HDF5 file
     */
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_lock(&global->swmr_mutex);
#endif
    hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
    //herr_t r;
    hsize_t		size[2];
    //hsize_t		max_size[2];
	hid_t		h5compression;

	// Create file
    fh = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if ( fh < 0 ) {
        ERROR("Couldn't create HDF5 file: %s\n", filename);
    }
    gh = H5Gcreate(fh, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if ( gh < 0 ) {
        ERROR("Couldn't create HDF5 group\n");
        H5Fclose(fh);
    }
	
	// Setting compression level
	if (global->h5compress) {
		h5compression = H5Pcreate(H5P_DATASET_CREATE);
	}
	else {
		h5compression = H5P_DEFAULT;
	}

	FOREACH_DATAFORMAT_T(i_f, cDataVersion::DATA_FORMATS) {
		if (isBitOptionSet(global->detector[detIndex].powderFormat, *i_f)) {
			cDataVersion dataV(NULL, &global->detector[detIndex], global->detector[detIndex].powderVersion, *i_f);
			while (dataV.next()) {
				if (*i_f != cDataVersion::DATA_FORMAT_RADIAL_AVERAGE) {
					// size for 2D data
					size[0] = dataV.pix_ny;
					size[1] = dataV.pix_nx;
					sh = H5Screate_simple(2, size, NULL);
					// Compression for 1D
					if (global->h5compress) {
						H5Pset_chunk(h5compression, 2, size);
						H5Pset_deflate(h5compression, global->h5compress);		// Compression levels are 0 (none) to 9 (max)
					}			
				} else {
					// size for 1D data (radial average)
					size[0] = dataV.pix_nn;
					size[1] = 0;
					sh = H5Screate_simple(1, size, NULL);
					// Compression for 1D
					h5compression = H5P_DEFAULT;
				}
				double * powder = dataV.getPowder(powderClass);
				double * powder_squared = dataV.getPowderSquared(powderClass);
				pthread_mutex_t mutex = dataV.getPowderMutex(powderClass);
				// Powder
				powderBuffer = (double*) calloc(dataV.pix_nn, sizeof(double));
				pthread_mutex_lock(&mutex);
				memcpy(powderBuffer, powder, dataV.pix_nn*sizeof(double));
				pthread_mutex_unlock(&mutex);			
				// Write to dataset
				dh = H5Dcreate(gh, dataV.name, H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
				if (dh < 0) ERROR("Could not create dataset.\n");
				H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, powderBuffer);
				H5Dclose(dh);
				free(powderBuffer);
				if (dataV.isMainDataset) {
					// Create symbolic link if this is the main dataset
					sprintf(sBuffer,"/data/%s",dataV.name);
					H5Lcreate_soft(sBuffer, fh, "/data/data",0,0);
				}
				// Fluctuations (sigma)
				powderSquaredBuffer = (double*) calloc(dataV.pix_nn, sizeof(double));
				powderSigmaBuffer = (double*) calloc(dataV.pix_nn, sizeof(double));
				pthread_mutex_lock(&mutex);
				memcpy(powderSquaredBuffer, powder_squared, dataV.pix_nn*sizeof(double));
				pthread_mutex_unlock(&mutex);
				for(long i=0; i<dataV.pix_nn; i++){
					powderSigmaBuffer[i] = sqrt( fabs(powderSquaredBuffer[i]/nframes - (powderBuffer[i]/nframes)*(powderBuffer[i]/nframes)));
				}
				sprintf(sBuffer,"%s_sigma",dataV.name);
				dh = H5Dcreate(gh, sBuffer, H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
				if (dh < 0) ERROR("Could not create dataset.\n");
				H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, powderSigmaBuffer);
				H5Dclose(dh);
				free(powderSquaredBuffer);
				free(powderSigmaBuffer);
				H5Sclose(sh);
			}				
		}
	}

    // Peak powder
	size[0] = detector->pix_ny;
	size[1] = detector->pix_nx;
	sh = H5Screate_simple(2, size, NULL);
    bufferPeaks = (double*) calloc(detector->pix_nn, sizeof(double));
    pthread_mutex_lock(&detector->powderPeaks_mutex[powderClass]);
    memcpy(bufferPeaks, detector->powderPeaks[powderClass], detector->pix_nn*sizeof(double));
    pthread_mutex_unlock(&detector->powderPeaks_mutex[powderClass]);
	dh = H5Dcreate(gh, "peakpowder", H5T_NATIVE_DOUBLE, sh, H5P_DEFAULT, h5compression, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, bufferPeaks);
    H5Dclose(dh);
    H5Sclose(sh);
	free(bufferPeaks);
    
    // Save frame count
    size[0] = 1;
    sh = H5Screate_simple(1, size, NULL );
    dh = H5Dcreate(gh, "nframes", H5T_NATIVE_LONG, sh, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dh < 0) ERROR("Could not create dataset.\n");
    H5Dwrite(dh, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &detector->nPowderFrames[powderClass] );
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
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_unlock(&global->swmr_mutex);
#endif    
	
    
    /*
     *	Clean up stuff
     */
    for(long i=0; i<global->nPowderClasses; i++) {
        fflush(global->powderlogfp[i]);
    }

}



/*
 *	Compute and save dark calibration
 */
void saveDarkcal(cGlobal *global, int detIndex) {
	
	// Dereference common variables
	cPixelDetectorCommon     *detector = &(global->detector[detIndex]);
	long	pix_nn = detector->pix_nn;
	char	filename[1024];
	
	printf("Processing darkcal\n");
	//sprintf(filename,"r%04u-%s-%li-darkcal.h5",global->runNumber,detector->detectorName,detector->detectorID);
	sprintf(filename,"r%04u-detector%li-darkcal.h5",global->runNumber,detector->detectorID);
	float *buffer = (float*) calloc(pix_nn, sizeof(float));
	pthread_mutex_lock(&detector->powderData_mutex[0]);
	for(long i=0; i<pix_nn; i++)
		buffer[i] = detector->powderData_raw[0][i]/detector->nPowderFrames[0];
	pthread_mutex_unlock(&detector->powderData_mutex[0]);
	printf("Saving darkcal to file: %s\n", filename);
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_lock(&global->swmr_mutex);
#endif
	writeSimpleHDF5(filename, buffer, detector->pix_nx, detector->pix_ny, H5T_NATIVE_FLOAT,detector->detectorName,detector->detectorID);	
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_unlock(&global->swmr_mutex);
#endif
	free(buffer);
}



/*
 *	Compute and save gain calibration
 */
void saveGaincal(cGlobal *global, int detIndex) {
	
	printf("Processing gaincal\n");

	// Dereference common variables
	cPixelDetectorCommon     *detector = &(global->detector[detIndex]);
	long	pix_nn = detector->pix_nn;
	
	// Grab a snapshot of the current running sum
	pthread_mutex_lock(&detector->powderData_mutex[0]);
	float *buffer = (float*) calloc(pix_nn, sizeof(float));
	for(long i=0; i<pix_nn; i++)
		buffer[i] = detector->powderData_raw[0][i];
    for(long i=0; i<pix_nn; i++)
		buffer[i] /= detector->nPowderFrames[0];
	pthread_mutex_unlock(&detector->powderData_mutex[0]);

    
    
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
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_lock(&global->swmr_mutex);
#endif
	writeSimpleHDF5(filename, buffer, detector->pix_nx, detector->pix_ny, H5T_NATIVE_FLOAT);
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_unlock(&global->swmr_mutex);
#endif
	free(buffer);
}


