/*
 *  dataVersion.h
 *  cheetah
 *
 *  Created by Max Felix Hantke on 3/11/14.
 *  Copyright 2014 LMB Uppsala University. All rights reserved.
 *
 */

#ifndef DATAVERSION_H
#define DATAVERSION_H

#include <stdint.h>

#define MAX_POWDER_CLASSES 16

class cPixelDetectorEvent;
class cPixelDetectorCommon;

class cDataVersion {
 public:
    // Data versions and formats as bit flag options
	typedef enum {DATA_VERSION_RAW = 1,
				  DATA_VERSION_DETECTOR_CORRECTED = 2,
				  DATA_VERSION_DETECTOR_AND_PHOTON_CORRECTED = 4,
				  DATA_VERSION_NONE = 0,
				  DATA_VERSION_ALL = 1|2|4} dataVersion_t;
	typedef enum {DATA_FORMAT_NON_ASSEMBLED = 1,
				  DATA_FORMAT_ASSEMBLED = 2,
				  DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED = 4,
				  DATA_FORMAT_RADIAL_AVERAGE = 16,
				  DATA_FORMAT_NONE = 0,
				  DATA_FORMAT_ALL = 1|2|4|16} dataFormat_t;
	static const dataFormat_t DATA_FORMATS[4];
	cDataVersion(cPixelDetectorEvent *detectorEvent, cPixelDetectorCommon *detectorCommon, const dataVersion_t dataVersion, const dataFormat_t dataFormat);
	bool next();
	float * getData();
	uint16_t * getPixelmask();
	double * getPowder(long powderClass);
	double * getPowderSquared(long powderClass);
	pthread_mutex_t * getPowderMutex(long powderClass);
	char name[1024];
	char name_format[1024];
	char name_version[1024];
	int isMainDataset,isMainVersion;
	long pix_nn,pix_nx,pix_ny;		

 private:
	cPixelDetectorEvent *detectorEvent;
	cPixelDetectorCommon *detectorCommon;

	float *data;
	uint16_t *pixelmask;
	double *powder[MAX_POWDER_CLASSES];
	double *powder_squared[MAX_POWDER_CLASSES];
	pthread_mutex_t * powder_mutex[MAX_POWDER_CLASSES];	

	float *raw;
	float *detCorr;
	float *detPhotCorr;
	double *powder_raw[MAX_POWDER_CLASSES];
	double *powder_raw_squared[MAX_POWDER_CLASSES];
	double *powder_detCorr[MAX_POWDER_CLASSES];
	double *powder_detCorr_squared[MAX_POWDER_CLASSES];
	double *powder_detPhotCorr[MAX_POWDER_CLASSES];
	double *powder_detPhotCorr_squared[MAX_POWDER_CLASSES];

	int dataVersionIndex;

	uint16_t dataLoopMode;
	//uint16_t *pixelmask_shared;

	dataFormat_t dataFormat;
	dataFormat_t dataFormatMain;
	dataVersion_t dataVersion;
	dataVersion_t dataVersionMain;

	int get();
	void clear();
};


#define FOREACH_DATAFORMAT_T( intpvar, intary ) cDataVersion::dataFormat_t* intpvar; for( intpvar= (cDataVersion::dataFormat_t*) intary; intpvar < (intary + (sizeof(intary)/sizeof(intary[0]))) ; intpvar++)

const int DATA_VERSION_N = 3;

#endif
