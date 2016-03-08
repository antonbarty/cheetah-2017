/*
 *  dataVersion.cpp
 *  cheetah
 *
 *  Created by Max Felix Hantke on 3/11/14.
 *  Copyright 2014 LMB Uppsala University. All rights reserved.
 *
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
//#include <sys/time.h>
//#include <math.h>
//#include <limits>
//#include <hdf5.h>
//#include <fenv.h>
#include <stdlib.h>
#include <iostream>

#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "detectorObject.h"

void initRaw(cEventData *eventData, cGlobal *global){
	// Copy raw detector data into float array
	DETECTOR_LOOP {
		DEBUG3("Initializing raw data array (float). (detectorID=%ld)",global->detector[detIndex].detectorID);
		for(long i=0;i<global->detector[detIndex].pix_nn;i++){
			eventData->detector[detIndex].data_raw[i] = eventData->detector[detIndex].data_raw16[i];
		}
	}
}

void initDetectorCorrection(cEventData *eventData, cGlobal *global){
	// Copy raw detector data into detector corrected array as starting point for detector corrections
	DETECTOR_LOOP {
		DEBUG3("Initializing detector corrected data with raw data. (detectorID=%ld)",global->detector[detIndex].detectorID);
		for(long i=0;i<global->detector[detIndex].pix_nn;i++){
			eventData->detector[detIndex].data_detCorr[i] = eventData->detector[detIndex].data_raw[i];
		}
	}
}


void initPhotonCorrection(cEventData *eventData, cGlobal *global){
	// Copy detector corrected data into photon corrected array as starting point for photon corrections
	DETECTOR_LOOP {
		DEBUG3("Initialise photon corrected data with detector corrected data. (detectorID=%ld)",global->detector[detIndex].detectorID);										
		for(long i=0;i<global->detector[detIndex].pix_nn;i++){
			eventData->detector[detIndex].data_detPhotCorr[i] = eventData->detector[detIndex].data_detCorr[i];
		}
	}
}

const cDataVersion::dataFormat_t cDataVersion::DATA_FORMATS[4] = {DATA_FORMAT_NON_ASSEMBLED,
																  DATA_FORMAT_ASSEMBLED,
																  DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED,
																  DATA_FORMAT_RADIAL_AVERAGE};

cDataVersion::cDataVersion(cPixelDetectorEvent * detectorEvent0, cPixelDetectorCommon * detectorCommon0, const dataVersion_t dataVersion0, const dataFormat_t dataFormat0) {
	detectorCommon = detectorCommon0;
	detectorEvent = detectorEvent0;
	dataFormat = dataFormat0;
	dataFormatMain = detectorCommon->dataFormatMain;
	dataVersion = dataVersion0;
	dataVersionMain = detectorCommon->dataVersionMain;
	powderVersionMain = detectorCommon->powderVersionMain;
	dataVersionIndex = -1;

	clear();
	pixelmask = NULL;
	for (long powderClass=0; powderClass < MAX_POWDER_CLASSES; powderClass++) {
		powder_counter[powderClass]             = NULL;
		powder_raw[powderClass]                 = NULL;
		powder_raw_squared[powderClass]         = NULL;
		powder_detCorr[powderClass]             = NULL;
		powder_detCorr_squared[powderClass]     = NULL;
		powder_detPhotCorr[powderClass]         = NULL;
		powder_detPhotCorr_squared[powderClass] = NULL;
		powder_mutex[powderClass]               = &detectorCommon->null_mutex;
	}
	
	if (dataFormat == DATA_FORMAT_NON_ASSEMBLED) {
		sprintf(name_format,"non_assembled");
		// Event
		if (detectorEvent != NULL) {
			raw                   = detectorEvent->data_raw;
			detCorr               = detectorEvent->data_detCorr;
			detPhotCorr           = detectorEvent->data_detPhotCorr;
			pixelmask             = detectorEvent->pixelmask;
		}
		// Global
		memcpy(&(powder_raw[0]), &(detectorCommon->powderData_raw[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_raw_squared[0]), &(detectorCommon->powderData_raw_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_raw_counter[0]), &(detectorCommon->powderData_raw_counter[0]), sizeof(long*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr[0]), &(detectorCommon->powderData_detCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr_squared[0]), &(detectorCommon->powderData_detCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr_counter[0]), &(detectorCommon->powderData_detCorr_counter[0]), sizeof(long*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr[0]), &(detectorCommon->powderData_detPhotCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr_squared[0]), &(detectorCommon->powderData_detPhotCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr_counter[0]), &(detectorCommon->powderData_detPhotCorr_counter[0]), sizeof(long*)*detectorCommon->nPowderClasses);
		for (long i = 0; i < detectorCommon->nPowderClasses; i++) {
			powder_mutex[i] = &detectorCommon->powderData_mutex[i];
		}

		pix_nn                     = detectorCommon->pix_nn;
		pix_nx                     = detectorCommon->pix_nx;
		pix_ny                     = detectorCommon->pix_ny;
	}
	else if (dataFormat == DATA_FORMAT_ASSEMBLED) {
		sprintf(name_format,"assembled");
		if (detectorEvent != NULL) {
			// Event
			raw                   = detectorEvent->image_raw;
			detCorr               = detectorEvent->image_detCorr;
			detPhotCorr           = detectorEvent->image_detPhotCorr;
			pixelmask             = detectorEvent->image_pixelmask;
		}
		// Global
		memcpy(&(powder_raw[0]), &(detectorCommon->powderImage_raw[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_raw_squared[0]), &(detectorCommon->powderImage_raw_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr[0]), &(detectorCommon->powderImage_detCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr_squared[0]), &(detectorCommon->powderImage_detCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr[0]), &(detectorCommon->powderImage_detPhotCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr_squared[0]), &(detectorCommon->powderImage_detPhotCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		for (long i = 0; i < detectorCommon->nPowderClasses; i++) {
			powder_mutex[i] = &detectorCommon->powderImage_mutex[i];
		}

		pix_nn                    = detectorCommon->image_nn;
		pix_nx                    = detectorCommon->image_nx;
		pix_ny                    = detectorCommon->image_ny;
	}
	else if (dataFormat == DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED) {
		sprintf(name_format,"assembled_and_downsampled");
		if (detectorEvent != NULL) {
			// Event
			raw                   = detectorEvent->imageXxX_raw;
			detCorr               = detectorEvent->imageXxX_detCorr;
			detPhotCorr           = detectorEvent->imageXxX_detPhotCorr;
			pixelmask             = detectorEvent->imageXxX_pixelmask;
		}
		// Global
		memcpy(&(powder_raw[0]), &(detectorCommon->powderImageXxX_raw[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_raw_squared[0]), &(detectorCommon->powderImageXxX_raw_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr[0]), &(detectorCommon->powderImageXxX_detCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr_squared[0]), &(detectorCommon->powderImageXxX_detCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr[0]), &(detectorCommon->powderImageXxX_detPhotCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr_squared[0]), &(detectorCommon->powderImageXxX_detPhotCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		for (long i = 0; i < detectorCommon->nPowderClasses; i++) {
			powder_mutex[i] = &detectorCommon->powderImageXxX_mutex[i];
		}

		pix_nn                    = detectorCommon->imageXxX_nn;
		pix_nx                    = detectorCommon->imageXxX_nx;
		pix_ny                    = detectorCommon->imageXxX_ny;
	}
	else if (dataFormat == DATA_FORMAT_RADIAL_AVERAGE) {
		sprintf(name_format,"radial_average");
		if (detectorEvent != NULL) {
			// Event
			raw                     = detectorEvent->radialAverage_raw;
			detCorr                 = detectorEvent->radialAverage_detCorr;
			detPhotCorr             = detectorEvent->radialAverage_detPhotCorr;
			pixelmask               = detectorEvent->radialAverage_pixelmask;
		}
		// Global
		memcpy(&(powder_raw[0]), &(detectorCommon->powderRadialAverage_raw[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_raw_squared[0]), &(detectorCommon->powderRadialAverage_raw_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr[0]), &(detectorCommon->powderRadialAverage_detCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detCorr_squared[0]), &(detectorCommon->powderRadialAverage_detCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr[0]), &(detectorCommon->powderRadialAverage_detPhotCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		memcpy(&(powder_detPhotCorr_squared[0]), &(detectorCommon->powderRadialAverage_detPhotCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
		for (long i = 0; i < detectorCommon->nPowderClasses; i++) {
			powder_mutex[i] = &detectorCommon->powderRadialAverage_mutex[i];
		}

		pix_nn                  = detectorCommon->radial_nn;
		pix_nx                  = 0;
		pix_ny                  = 0;
	}
	else {
		ERROR("cDataVersion initialised with incorrect data format!");
	}
}

/*
void cDataVersion::testMemory() {
	if (detectorEvent != NULL) {
		float * temp = (float *) calloc(pix_nn,sizeof(float));
		for (long i = 0; i < pix_nn; i++) {
			temp[i]=raw[i];
			raw[i]=0;
		}
		for (long i = 0; i < pix_nn; i++) {
			raw[i]=temp[i];
		}
	}
	if (detectorEvent != NULL) {
		float * temp = (float *) calloc(pix_nn,sizeof(float));
		for (long i = 0; i < pix_nn; i++) {
			temp[i]=raw[i];
			raw[i]=0;
		}
		for (long i = 0; i < pix_nn; i++) {
			raw[i]=temp[i];
		}
	}
	if (detectorEvent != NULL) {
		float * temp = (float *) calloc(pix_nn,sizeof(float));
		for (long i = 0; i < pix_nn; i++) {
			temp[i]=raw[i];
			raw[i]=0;
		}
		for (long i = 0; i < pix_nn; i++) {
			raw[i]=temp[i];
		}
	}
}
*/
		
void cDataVersion::clear() {
	name[0] = 0;
	name_version[0] = 0;
	isMainDataset = 0;
	isMainVersion = 0;
	data      = NULL;
	//dataVersionIndex = -1;
	for (long powderClass=0; powderClass < MAX_POWDER_CLASSES; powderClass++) {		
		powder[powderClass]                     = NULL;
		powder_squared[powderClass]             = NULL;
		powder_counter[powderClass]             = NULL;
	}
}


bool cDataVersion::next(){
	while (dataVersionIndex < 3){
		dataVersionIndex++;
		if ((dataVersionIndex == 0) && isBitOptionSet(dataVersion,DATA_VERSION_RAW)) {
			data = raw;
			memcpy(&(powder[0]), &(powder_raw[0]), sizeof(double*)*detectorCommon->nPowderClasses);
			memcpy(&(powder_squared[0]), &(powder_raw_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
			memcpy(&(powder_counter[0]), &(powder_raw_counter[0]), sizeof(long*)*detectorCommon->nPowderClasses);
			sprintf(name_version,"raw");
			sprintf(name,"%s_%s",name_format,name_version);
			isMainVersion = (dataVersionMain == DATA_VERSION_RAW);
			isMainDataset = isMainVersion && (dataFormatMain == dataFormat);
			return true;
		}
		else if ((dataVersionIndex == 1) && isBitOptionSet(dataVersion,DATA_VERSION_DETECTOR_CORRECTED)) {
			data = detCorr;
			memcpy(&(powder[0]), &(powder_detCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
			memcpy(&(powder_squared[0]), &(powder_detCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
			memcpy(&(powder_counter[0]), &(powder_detCorr_counter[0]), sizeof(long*)*detectorCommon->nPowderClasses);
			sprintf(name_version,"detector_corrected");
			sprintf(name,"%s_%s",name_format,name_version);
			isMainVersion = (dataVersionMain == DATA_VERSION_DETECTOR_CORRECTED);
			isMainDataset = isMainVersion && (dataFormatMain == dataFormat);
			return true;
		}
		else if ((dataVersionIndex == 2) && isBitOptionSet(dataVersion,DATA_VERSION_DETECTOR_AND_PHOTON_CORRECTED)) {
			data = detPhotCorr;
			memcpy(&(powder[0]), &(powder_detPhotCorr[0]), sizeof(double*)*detectorCommon->nPowderClasses);
			memcpy(&(powder_squared[0]), &(powder_detPhotCorr_squared[0]), sizeof(double*)*detectorCommon->nPowderClasses);
			memcpy(&(powder_counter[0]), &(powder_detPhotCorr_counter[0]), sizeof(long*)*detectorCommon->nPowderClasses);
			sprintf(name_version,"detector_and_photon_corrected");
			sprintf(name,"%s_%s",name_format,name_version);
			isMainVersion = (dataVersionMain == DATA_VERSION_DETECTOR_AND_PHOTON_CORRECTED);
			isMainDataset = isMainVersion && (dataFormatMain == dataFormat);
			return true;
		}
		else {
			clear();
		}
	}
	//dataVersionIndex = -1;
	return false;
}


float * cDataVersion::getData() {
	if (data == NULL) {
		ERROR("Trying to access data that does not exist!");
	} 
	return data;
}

uint16_t * cDataVersion::getPixelmask() {
	if (pixelmask == NULL) {
		ERROR("Trying to access pixelmask that does not exist!");
	}
	return pixelmask;
}

double * cDataVersion::getPowder(long powderClass) {
	if (powder == NULL) {
		ERROR("Trying to access powder data that does not exist!");
	}
	return powder[powderClass];
}

double * cDataVersion::getPowderSquared(long powderClass) {
	if (powder_squared[powderClass] == NULL) {
		ERROR("Trying to access squared powder data that does not exist!");
	} 
	return powder_squared[powderClass]; 
}


long * cDataVersion::getPowderCounter(long powderClass) {
	if (powder_counter[powderClass] == NULL) {
		ERROR("Trying to access powder counter that does not exist");
	}
	return powder_counter[powderClass];
}


pthread_mutex_t *cDataVersion::getPowderMutex(long powderClass) {
	if (powder_mutex[powderClass] == &detectorCommon->null_mutex) {
		ERROR("Trying to access powder mutex that does not exist!");
	} 
	return powder_mutex[powderClass];
}

