//
//  spectrum.cpp
//  cheetah
//
//  Created by Richard Bean on 2/27/13.
//
//


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
#include "data2d.h"



/*
 *	Wrapper for saving all FEE spectral stacks
 */
void saveSpectrumStacks(cGlobal *global) {
	
    if(global->useFEEspectrum) {
		printf("Saving FEE spectral stacks\n");
		for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
			saveFEEspectrumStack(global, powderType);
		}
	}
	
	if(global->espectrum) {
		printf("Saving CXI spectral stacks\n");
		for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
			saveEspectrumStack(global, powderType);
		}
	}
}




/*
 *	FEE spectrometer
 */

void addFEEspectrumToStack(cEventData *eventData, cGlobal *global, int powderClass){
	
    float   *stack = global->FEEspectrumStack[powderClass];
    uint32_t  *spectrum = eventData->FEEspec_hproj;
    long	speclength = global->FEEspectrumWidth;
    long    stackCounter = global->FEEspectrumStackCounter[powderClass];
    long    stackSize = global->FEEspectrumStackSize;

	// No FEE data means go home
	if(!eventData->FEEspec_present)
		return;
		
	
    // Lock
	pthread_mutex_lock(&global->FEEspectrumStack_mutex[powderClass]);
	
    // Data offsets
    long stackoffset = stackCounter % stackSize;
    long dataoffset = stackoffset*speclength;
	
    // Copy data and increment counter
    for(long i=0; i<speclength; i++) {
        stack[dataoffset+i] = (float) spectrum[i];
    }
	
	
	// Write filename to log file in sync with stack positions (** Important for being able to index the patterns!)
	fprintf(global->FEElogfp[powderClass], "%li, %li, %s/%s\n", stackCounter, eventData->frameNumber, eventData->eventSubdir, eventData->eventname);


    // Increment counter
    global->FEEspectrumStackCounter[powderClass] += 1;
	
	
    // Save data once stack is full
    if((global->FEEspectrumStackCounter[powderClass] % stackSize) == 0) {
        printf("Saving FEE spectrum stack: %i\n", powderClass);
        saveFEEspectrumStack(global, powderClass);
        
        for(long j=0; j<speclength*stackSize; j++)
            global->FEEspectrumStack[powderClass][j] = 0;
    }
	
    pthread_mutex_unlock(&global->FEEspectrumStack_mutex[powderClass]);
}


/*
 *  Save FEE spectral stacks
 */
void saveFEEspectrumStack(cGlobal *global, int powderClass) {
	
    if(!global->useFEEspectrum)
        return;
	
	char	filename[1024];
    float   *stack = global->FEEspectrumStack[powderClass];
    long	speclength = global->FEEspectrumWidth;
    long    stackCounter = global->FEEspectrumStackCounter[powderClass];
    long    stackSize = global->FEEspectrumStackSize;
    pthread_mutex_t mutex = global->FEEspectrumStack_mutex[powderClass];
	
	if(global->FEEspectrumStackCounter[powderClass]==0)
		return;
	
    // Lock
	pthread_mutex_lock(&mutex);
	
	
	// We re-use stacks, what is this number?
	long	stackNum = stackCounter / stackSize;
 	if(stackNum == 0) stackNum = 1;
	
	// If stack is not full, how many rows are full?
    long    nRows = stackSize;
    if(stackCounter % stackSize != 0)
        nRows = (stackCounter % stackSize);
	
    sprintf(filename,"r%04u-FEEspectrum-class%i-stack%li.h5", global->runNumber, powderClass, stackNum);
    printf("Saving FEE spectral stack: %s\n", filename);
    writeSimpleHDF5(filename, stack, speclength, nRows, H5T_NATIVE_FLOAT);
	
	
	// Flush stack index buffer
	if(global->FEElogfp[powderClass] != NULL)
		fflush(global->FEElogfp[powderClass]);
	
	pthread_mutex_unlock(&mutex);
	
}




/*
 *	CXI spectrometer
 */

void integrateSpectrum(cEventData *eventData, cGlobal *global) {
	// proceed if event is a 'hit', spectrum data exists & spectrum required
	int hit = eventData->hit;
	int opalfail = eventData->specFail;
	int specWidth = eventData->specWidth;
	int specHeight = eventData->specHeight;
	int spectra = global->espectrum;
	
	if(global->generateDarkcal && !opalfail && spectra){
		eventData->energySpectrumExist = 1;
		genSpectrumBackground(eventData,global,specWidth,specHeight);
	}
	if(hit && !opalfail && spectra){
		eventData->energySpectrumExist = 1;
		integrateSpectrum(eventData,global,specWidth,specHeight);
		addToSpectrumStack(eventData,global, hit);
	}
	return;
}


void integrateSpectrum(cEventData *eventData, cGlobal *global, int specWidth,int specHeight) {
	// integrate spectrum into single line and output to event data
	float PIE = 3.141;
	float ttilt = tanf(global->espectrumTiltAng*PIE/180);
	int opalindex;
	int newind;

	for (long i=0; i<specHeight; i++) {
		for (long j=0; j<specWidth; j++) {
			newind = i + (int) ceilf(j*ttilt);        // index of the integrated array, must be integer,!
			if (newind >= 0 && newind < specHeight) {
				opalindex = i*specWidth + j;   // index of the 2D camera array
				eventData->energySpectrum1D[newind]+=eventData->specImage[opalindex];
				if (global->espectrumDarkSubtract) {
					eventData->energySpectrum1D[newind]-=global->espectrumDarkcal[opalindex];
				}
			}
		}
	}
	return;
}


void addToSpectrumStack(cEventData *eventData, cGlobal *global, int powderClass){
	
    float   *stack = global->espectrumStack[powderClass];
    double  *spectrum = eventData->energySpectrum1D;
    long	speclength = global->espectrumLength;
    long    stackCounter = global->espectrumStackCounter[powderClass];
    long    stackSize = global->espectrumStackSize;
    pthread_mutex_t mutex = global->espectrumStack_mutex[powderClass];

    // Lock
	pthread_mutex_lock(&mutex);
	
    // Data offsets
    long stackoffset = stackCounter % stackSize;
    long dataoffset = stackoffset*speclength;
	
    // Copy data and increment counter
    for(long i=0; i<speclength; i++) {
        stack[dataoffset+i] = (float) spectrum[i];
    }
	
    // Increment counter
    global->espectrumStackCounter[powderClass] += 1;
	
	
    // Save data once stack is full
    if((stackCounter % stackSize) == 0) {
        
        printf("Saving espectrum stack: %i\n", powderClass);
        saveEspectrumStack(global, powderClass);
        
        for(long j=0; j<speclength*stackSize; j++)
            global->espectrumStack[powderClass][j] = 0;
    }
	
    pthread_mutex_unlock(&mutex);
	
}

/*
 *	Wrapper for saving all radial stacks
 */
void saveEspectrumStacks(cGlobal *global) {

    if(!global->espectrum)
        return;
    
    printf("Saving spectral stacks\n");
	
	for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
		saveEspectrumStack(global, powderType);
	}
}



/*
 *  Save radial average stack
 */
void saveEspectrumStack(cGlobal *global, int powderClass) {
	
	char	filename[1024];
    float   *stack = global->espectrumStack[powderClass];
    long	speclength = global->espectrumLength;
    long    stackCounter = global->espectrumStackCounter[powderClass];
    long    stackSize = global->espectrumStackSize;
    pthread_mutex_t mutex = global->espectrumStack_mutex[powderClass];
	

    if(!global->espectrum)
        return;

	if(global->espectrumStackCounter[powderClass]==0)
		return;
	
    // Lock
	pthread_mutex_lock(&mutex);

	
	// We re-use stacks, what is this number?
	long	stackNum = stackCounter / stackSize;
 	if(stackNum == 0) stackNum = 1;
	
	// If stack is not full, how many rows are full?
    long    nRows = stackSize;
    if(stackCounter % stackSize != 0)
        nRows = (stackCounter % stackSize);
	
	
    sprintf(filename,"r%04u-espectrumstack-class%i-stack%li.h5", global->runNumber, powderClass, stackNum);
    printf("Saving spectral stack: %s\n", filename);
    writeSimpleHDF5(filename, stack, speclength, nRows, H5T_NATIVE_FLOAT);
	
	pthread_mutex_unlock(&mutex);
	
}


void integrateRunSpectrum(cEventData *eventData, cGlobal *global) {
	
	// Update integrated run spectrum
	if(eventData->hit && eventData->energySpectrumExist) {
		pthread_mutex_lock(&global->espectrumRun_mutex);
		for (long i=0; i<global->espectrumLength; i++) {
			global->espectrumRun[i] += eventData->energySpectrum1D[i];
		}
		pthread_mutex_unlock(&global->espectrumRun_mutex);
	}

	// Update spectrum hit counter
	if(eventData->energySpectrumExist && !global->generateDarkcal) {
		pthread_mutex_lock(&global->nespechits_mutex);
		global->nespechits++;
		pthread_mutex_unlock(&global->nespechits_mutex);
	}
	return;
}


void genSpectrumBackground(cEventData *eventData, cGlobal *global, int specWidth, int specHeight) {
	// Generate background for spectrum detector
	int spectrumpix = specWidth*specHeight;

	pthread_mutex_lock(&global->espectrumBuffer_mutex);
	for (int i=0; i<spectrumpix; i++) {
		global->espectrumBuffer[i]+=eventData->specImage[i];
	}
	pthread_mutex_unlock(&global->espectrumBuffer_mutex);
	pthread_mutex_lock(&global->nespechits_mutex);
	global->nespechits++;
	pthread_mutex_unlock(&global->nespechits_mutex);
	return;
}


void saveIntegratedRunSpectrum(cGlobal *global) {
    
    // Simply return if spectrum is not asked for
    if(global->espectrum == 0)
        return;

    
	int     spectrumpix = global->espectrumWidth*global->espectrumLength;
	double  *espectrumDark = (double*) calloc(spectrumpix, sizeof(double));
	double  *espectrumScale = (double*) calloc(global->espectrumLength, sizeof(double));
	char	filename[1024];
	int     maxindex = 0;
	int     evspread = global->espectrumSpreadeV;
	double  pixincrement = (double) evspread/global->espectrumLength;
	double  beamAveV = global->meanPhotonEnergyeV;
	double  eVoffset;

	// compute spectrum camera darkcal and save to HDF5
	if(global->generateDarkcal){
		pthread_mutex_lock(&global->espectrumRun_mutex);
		pthread_mutex_lock(&global->nespechits_mutex);
		for(int i=0; i<spectrumpix; i++) {
			espectrumDark[i] = global->espectrumBuffer[i]/global->nespechits;
		}

		sprintf(filename,"r%04u-energySpectrum-darkcal.h5", global->runNumber);
		printf("Saving energy spectrum darkcal to file: %s\n", filename);
        
		writeSimpleHDF5(filename, espectrumDark, global->espectrumWidth, global->espectrumLength, H5T_NATIVE_DOUBLE);

		pthread_mutex_unlock(&global->espectrumRun_mutex);
		pthread_mutex_unlock(&global->nespechits_mutex);
		free(espectrumDark);
		return;
	}

	// find maximum of run integrated spectum array and save both to HDF5
	pthread_mutex_lock(&global->espectrumRun_mutex);
	pthread_mutex_lock(&global->nespechits_mutex);

	for (int i=0; i<global->espectrumLength; i++) {
		if (global->espectrumRun[i] > global->espectrumRun[maxindex]) {
			maxindex = i;
		}
	}
	eVoffset = beamAveV - maxindex*pixincrement;
	for (int i=0; i<global->espectrumLength; i++) {
		espectrumScale[i]=i*pixincrement + eVoffset;
	}

	sprintf(filename,"r%04u-integratedEnergySpectrum.h5", global->runNumber);
	printf("Saving run-integrated energy spectrum: %s\n", filename);

	writeSpectrumInfoHDF5(filename, espectrumScale, global->espectrumRun, global->espectrumLength, H5T_NATIVE_DOUBLE, &maxindex, 1, H5T_NATIVE_INT);

	pthread_mutex_unlock(&global->espectrumRun_mutex);
	pthread_mutex_unlock(&global->nespechits_mutex);
	return;
}


void readSpectrumDarkcal(cGlobal *global, char *filename) {

	int spectrumpix = global->espectrumLength*global->espectrumWidth;

	// Do we need a darkcal file?
	if (global->espectrumDarkSubtract == 0){
		return;
	}
	
	// Check if a darkcal file has been specified
	if ( strcmp(filename,"") == 0 ){
		printf("spectrum camera Darkcal file path was not specified.\n");
		exit(1);
	}
	
	// Check whether file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tspecified energy spectrum Darkcal file does not exist: %s\n",filename);
		printf("\tAborting...\n");
		exit(1);
	}

	printf("Reading energy spectrum Darkcal file:\n");
	printf("\t%s\n",filename);
	
	// Read darkcal data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	// Copy into darkcal array
	for(long i=0; i<spectrumpix; i++) {
		global->espectrumDarkcal[i] =  temp2d.data[i];
	}
	return;
}

void readSpectrumEnergyScale(cGlobal *global, char *filename) {
	
	char        groupname[1024];
	char        fieldname[1024];
	hid_t       file_id;
	hid_t       datagroup_id;
	hid_t       dataset_id;
	hid_t       dataspace_id;
	hid_t       datatype_id;
	H5T_class_t dataclass;
	size_t      size;
	
	int ndims;
	
	sprintf(groupname, "energySpectrum");
	sprintf(fieldname, "runIntegratedEnergyScale");
	// Check if an energy scale calibration file has been specified
	if ( strcmp(filename,"") == 0 ){
		printf("spectrum energy scale calibration file path was not specified\n");
		printf("spectra will be output with default (0) energy scale\n");
		return;
	}
	
	// Check whether file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("specified energy scale calibration file does not exist: %s\n",filename);
		printf("spectra will be output with default (0) energy scale\n");
		return;
	}
	
	printf("Reading energy spectrum scale calibration file:\n");
	printf("\t%s\n",filename);
	
	// Open the file
	file_id = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
	if(file_id < 0){
		printf("ERROR: Could not open file %s\n",filename);
		printf("spectra will be output with default (0) energy scale\n");
		return;
	}
	
	// Open the dataset
	datagroup_id = H5Gopen1(file_id, groupname);
	dataset_id = H5Dopen1(datagroup_id, fieldname);
	dataspace_id = H5Dget_space(dataset_id);
	
	// Test if correct dimensions / size
	ndims = H5Sget_simple_extent_ndims(dataspace_id);
	if(ndims != 1) {
		printf("the specified file does not have the correct dimensions for energy scale calibration, ndims=%i\n",ndims);
		printf("spectra will be output with default (0) energy scale\n");
		return;
	}
	hsize_t dims[ndims];
	H5Sget_simple_extent_dims(dataspace_id,dims,NULL);
	if (!dims[0]==1 || !dims[1]==global->espectrumLength) {
		printf("the specified file does not have the correct dimensions for energy scale calibration\n");
		printf("spectra will be output with default (0) energy scale\n");
		return;
	}
	
	datatype_id =  H5Dget_type(dataset_id);
	dataclass = H5Tget_class(datatype_id);
	size = H5Tget_size(datatype_id);
		
	double*     energyscale = (double *) calloc(global->espectrumLength, sizeof(double));
	H5Dread(dataset_id, datatype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, energyscale);
	for(int i=0; i<global->espectrumLength; i++) {
		global->espectrumScale[i] = energyscale[i];
	}
	free(energyscale);
	
	// Close and cleanup
	H5Dclose(dataset_id);
	H5Gclose(datagroup_id);
	
	// Cleanup stale IDs
	hid_t ids[256];
	int n_ids = H5Fget_obj_ids(file_id, H5F_OBJ_ALL, 256, ids);
	for (long i=0; i<n_ids; i++ ) {
		
		hid_t id;
		H5I_type_t type;
		id = ids[i];
		type = H5Iget_type(id);
		if ( type == H5I_GROUP )
			H5Gclose(id);
		if ( type == H5I_DATASET )
			H5Dclose(id);
		if ( type == H5I_DATASPACE )
			H5Sclose(id);
		//if ( type == H5I_DATATYPE )
		//	H5Dclose(id);
	}
	
	H5Fclose(file_id);
	printf("energy spectrum scale calibration file read successful:\n");
	return;
}
