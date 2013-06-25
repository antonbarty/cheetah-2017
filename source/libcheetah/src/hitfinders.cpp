
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "median.h"
#include "hitfinders.h"
#include "peakfinders.h"
#include "data2d.h"


/*
 *	Various flavours of hitfinder
 *		0 - Everything is a hit
 *		1 - Number of pixels above ADC threshold
 *		2 - Total intensity above ADC threshold
 *		3 - Count Bragg peaks
 *		4 - Use TOF
 *		5 - Depreciated and no longer exists
 *		6 - Experimental - find peaks by SNR criteria
 *              7 - Laser on event code (usually EVR41)
 */
int  hitfinder(cEventData *eventData, cGlobal *global){
	
	// Dereference stuff
	int	    detID = global->hitfinderDetector;
	int		hit=0;
	
	/*
	 *	Default values for some metrics
	 */
	eventData->peakNpix = 0;
	eventData->peakTotal = 0;
	eventData->peakResolution = 0;
	eventData->peakDensity = 0;
      
 	/*
	 *	Use one of various hitfinder algorithms
	 */
 	switch(global->hitfinderAlgorithm) {
		
	case 0 :	// Everything is a hit. Used for converting xtc to hdf
	  hit = 1;
	  break;	

	case 1 :	// Count the number of pixels above ADC threshold
	  hit = hitfinder1(global,eventData,detID);
	  break;	
	  
	case 2 :	// Integrated intensity above ADC threshold
	  hit = hitfinder2(global,eventData,detID);
	  break;
			
	case 3 : 	// Count number of Bragg peaks
	  hit = peakfinder3(global,eventData, detID);			
	  break;	

	case 4 :	// Use TOF signal to find hits
	  hit = hitfinder4(global,eventData,detID);
	  break;
						
	case 6 : 	// Count number of Bragg peaks
	  hit = peakfinder6(global,eventData, detID);
	  break;
            
	case 7 : 	// Return laser on event code
	  hit = eventData->laserEventCodeOn;
	  eventData->nPeaks = eventData->laserEventCodeOn;
	  break;
			
	case 8 :	// Arbitrary threshold map
		hit = hitfinder8(global, eventData, detID);
		break;

	case 9 :	// Histogram analysis
		hit = hitfinder9(global, eventData, detID);
		break;

			
	default :
	  printf("Unknown hit finding algorithm selected: %i\n", global->hitfinderAlgorithm);
	  printf("Stopping in confusion.\n");
	  exit(1);
	  break;
			
	}
	
	// Statistics on the peaks, for certain hitfinders
	if( eventData->nPeaks > 1 &&
	   ( global->hitfinderAlgorithm == 3 || global->hitfinderAlgorithm == 5 || global->hitfinderAlgorithm == 6 ) ) {
		   
		long	np;
		long  kk;
		float	resolution;
		float	resolutionA;	
		float	cutoff = 0.95;
		long		pix_nn = global->detector[detID].pix_nn;
		long		asic_nx = global->detector[detID].asic_nx;
		long		asic_ny = global->detector[detID].asic_ny;
		long	*inx = (long *) calloc(pix_nn, sizeof(long));
		long	*iny = (long *) calloc(pix_nn, sizeof(long));


		np = eventData->nPeaks;
		if(np >= global->hitfinderNpeaksMax) 
		   np = global->hitfinderNpeaksMax; 
		kk = (long) floor(cutoff*np);
	
	

		// Pixel radius resolution (bigger is better)
		float *buffer1 = (float*) calloc(global->hitfinderNpeaksMax, sizeof(float));
		for(long k=0; k<np; k++) 
			buffer1[k] = eventData->peak_com_r_assembled[k];
		resolution = kth_smallest(buffer1, np, kk);		   
		eventData->peakResolution = resolution;
		free(buffer1);
	
		// Resolution to real space (in Angstrom)
		// Crystallographic resolution d = lambda/sin(theta)
		float z = global->detector[0].detectorZ;
		float dx = global->detector[0].pixelSize;
		double r = sqrt(z*z+dx*dx*resolution*resolution);
		double sintheta = dx*resolution/r;
		resolutionA = eventData->wavelengthA/sintheta;
		eventData->peakResolutionA = resolutionA;

	
		if(resolution > 0) {
			float	area = (3.141*resolution*resolution)/(asic_ny*asic_nx);
			eventData->peakDensity = (cutoff*np)/area;
		}
		free(inx); 			
		free(iny);	

	   
	}
	
	// Update central hit counter
	if(hit) {
		pthread_mutex_lock(&global->nhits_mutex);
		global->nhits++;
		global->nrecenthits++;
		pthread_mutex_unlock(&global->nhits_mutex);
	}
	

	
	return(hit);
	
}


void integratePixAboveThreshold(float *data, uint16_t *mask, long pix_nn, float ADC_threshold, uint16_t pixel_options, long *nat, float *tat){

  *nat = 0;
  *tat = 0.0;

  for(long i=0;i<pix_nn;i++){
    if(isNoneOfBitOptionsSet(mask[i],pixel_options) && data[i] > ADC_threshold){
      *tat += data[i];
      *nat += 1;
    }
  }
}


/*
 *	Hitfinder #1
 *	Hit if number of pixels above ADC threshold exceeds MinPixCount
 */

int hitfinder1(cGlobal *global, cEventData *eventData, long detID){

  int       hit = 0;
  long      nat = 0;
  float     tat = 0.;
  uint16_t  *mask = eventData->detector[detID].pixelmask;
  float     *data = eventData->detector[detID].corrected_data;
  long	    pix_nn = global->detector[detID].pix_nn;  
  float     ADC_threshold = global->hitfinderADC;
	
  // Combine pixel options for pixels to be ignored
  uint16_t  pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_MISSING;
  
  integratePixAboveThreshold(data,mask,pix_nn,ADC_threshold,pixel_options,&nat,&tat);
  eventData->peakTotal = tat;
  eventData->peakNpix = nat;
  eventData->nPeaks = nat;

  if(nat >= global->hitfinderMinPixCount){
    hit = 1;
  }
  return hit;
}


/*
 *	Hitfinder #2
 *	Hit if integrated value of pixels above ADC threshold exceeds TAT threshold
 */

int hitfinder2(cGlobal *global, cEventData *eventData, long detID){
  
  int       hit = 0;
  long      nat = 0;
  float     tat = 0.;
  uint16_t  *mask = eventData->detector[detID].pixelmask;
  float     *data = eventData->detector[detID].corrected_data;
  long	    pix_nn = global->detector[detID].pix_nn;  
  float     ADC_threshold = global->hitfinderADC;
  // Combine pixel options for pixels to be ignored
  uint16_t  pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED | PIXEL_IS_MISSING;
  
  integratePixAboveThreshold(data,mask,pix_nn,ADC_threshold,pixel_options,&nat,&tat);
  eventData->peakTotal = tat;
  eventData->peakNpix = nat;
  eventData->nPeaks = nat;

  if(tat >= global->hitfinderTAT){
    hit = 1;
  }
  return hit;
}

int hitfinder4(cGlobal *global,cEventData *eventData,long detID){
	int hit = 0;
	long		pix_nn = global->detector[detID].pix_nn;
	uint16_t      *mask = eventData->detector[detID].pixelmask;
	long	nat = 0;
	long	counter;
	float	total;
	float	mingrad = global->hitfinderMinGradient*2;
	mingrad *= mingrad;
	
	nat = 0;
	counter = 0;
	total = 0.0;
	
	/*
	 *	Create a buffer for image data so we don't nuke the main image by mistake
	 */
	float *temp = (float*) calloc(pix_nn, sizeof(float));
	memcpy(temp, eventData->detector[detID].corrected_data, pix_nn*sizeof(float));
	
	
	// combine pixelmask bits
	uint16_t combined_pixel_options = PIXEL_IS_IN_PEAKMASK | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_HOT | PIXEL_IS_BAD | PIXEL_IS_SATURATED;
	
	/*
	 *	Apply masks
	 *	(multiply data by 0 to ignore regions)
	 */
	for(long i=0;i<pix_nn;i++){
		temp[i] *= isNoneOfBitOptionsSet(mask[i], combined_pixel_options);
	}
	
	if ((global->hitfinderUseTOF==1) && (eventData->TOFPresent==1)){
		double total_tof = 0.;
		for(int i=global->hitfinderTOFMinSample; i<global->hitfinderTOFMaxSample; i++){
			total_tof += eventData->TOFVoltage[i];
		}
		if (total_tof > global->hitfinderTOFThresh)
			hit = 1;
	}
	// Use cspad threshold if TOF is not present
	else {
		for(long i=0;i<pix_nn;i++){
			if(temp[i] > global->hitfinderADC){
				nat++;
			}
		}
		if(nat >= global->hitfinderMinPixCount)
			hit = 1;
	}
	free(temp);
	return hit;
}



/*
 *	Variable ADC threshold per pixel
 */
int hitfinder8(cGlobal *global, cEventData *eventData, long detID){
}

/*
 *	Histogram wizardry
 */
int hitfinder9(cGlobal *global, cEventData *eventData, long detID){
}


/*
 *	Read in pixel threshold map
 *  (Per-pixel static ADC threshold)
 */
void readHitfinderThresholdMap(cGlobal *global) {
	
	// Map only needed for Hitfinder 8
	if ( global->hitfinder != 8 ){
		return;
	}
    
	char *filename = global->hitfinderThresholdMapFile;
	
	// Check if filename is blank
	if ( strcmp(filename,"") == 0 ){
		printf("Hitfinder threshold map was not specified.\n");
		printf("Aborting...\n");
		exit(1);
	}
    
	printf("Reading Hitfinder threshold map:\n");
	printf("\t%s\n",filename);
    
	// Check whether file exists!
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tHitfinder threshold map does not exist: %s\n",filename);
		printf("\tPlease specify a file using the keyword: hitfinderThresholdMap= \n");
		printf("\tAborting...\n");
		exit(1);
	}
	
	
	// Read data from file
	cData2d		temp2d;
	temp2d.readHDF5(filename);
	
	// Correct geometry? Check against detector 0
	if(temp2d.nx != global->detector[0].pix_nx || temp2d.ny != global->detector[0].pix_ny) {
		printf("\tGeometry mismatch: %lix%li != %lix%li\n",temp2d.nx, temp2d.ny, global->detector[0].pix_nx, global->detector[0].pix_ny);
		printf("\tAborting...\n");
		exit(1);
	}
	
	
	
	// Copy data into threhsold array
	global->hitfinderThresholdMap = (float*) calloc(temp2d.nn, sizeof(float));
	
	for(long i=0;i<temp2d.nn;i++){
		global->hitfinderThresholdMap[i] = temp2d.data[i];
	}
}


/*
 *	Read in histogram for statistical hitfinding
 *  This is a big 3D array!
 */
void readHitfinderHistogram(cGlobal *global){
	
	// Only needed for Hitfinder 9
	if ( global->hitfinder != 9 ){
		return;
	}

	char *filename = global->hitfinderHistogramFile;
	
	// Check if filename is blank
	if ( strcmp(filename,"") == 0 ){
		printf("Hitfinder histogram map was not specified.\n");
		printf("Aborting...\n");
		exit(1);
	}
    
	printf("Reading Hitfinder histogram map:\n");
	printf("\t%s\n",filename);
    
	// Check whether file exists
	FILE* fp = fopen(filename, "r");
	if (fp) 	// file exists
		fclose(fp);
	else {		// file doesn't exist
		printf("\tHitfinder histogram map does not exist: %s\n",filename);
		printf("\tPlease specify a file using the keyword: hitfinderHistogramFile= \n");
		printf("\tAborting...\n");
		exit(1);
	}

	
	// Open the HDF5 file
	hid_t file_id;
	file_id = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
	if(file_id < 0){
		printf("ERROR: H5Fopen could not open file %s\n",filename);
		exit(1);
	}
	
	// Open the dataset
	char fieldname[100];
	strcpy(fieldname, "/data/data");
	hid_t dataset_id;
	hid_t dataspace_id;
	dataset_id = H5Dopen1(file_id, fieldname);
	dataspace_id = H5Dget_space(dataset_id);
	
	
	// Test if 2D data
	int ndims;
	ndims = H5Sget_simple_extent_ndims(dataspace_id);
	if(ndims != 3) {
		printf("2dData::readHDF5: Not 2D data set, ndims=%i\n",ndims);
		exit(0);
	}
	
	
	// Get dimensions of data set (nx, ny, nn)
	hsize_t dims[ndims];
	H5Sget_simple_extent_dims(dataspace_id,dims,NULL);
	
	long	hist_ny = dims[0];
	long	hist_nx = dims[1];
	long	hist_depth = dims[2];
	long	hist_nn = 1;
	for(int i = 0;i<ndims;i++)
		hist_nn *= dims[i];
	
	printf("\tHistogram size is %lix%lix%li\n",hist_nx, hist_ny, hist_depth);
	
	
	// Just how big will this array be?
	float	histogramMemory;
	float	histogramMemoryGb;
	float	histogramMaxMemoryGb = 8;
	histogramMemory = (hist_nn * sizeof(float));
	histogramMemoryGb = histogramMemory / (1024LL*1024LL*1024LL);
	if (histogramMemoryGb > histogramMaxMemoryGb) {
		printf("Size of histogram buffer would exceed allowed size:\n");
		printf("Histogram depth: %li\n", hist_depth);
		printf("Histogram buffer size (GB): %f\n", histogramMemoryGb);
		printf("Maximum histogram buffer size (GB): %f\n", histogramMaxMemoryGb);
		printf("Set histogramMaxMemoryGb to a larger value and recompile if you really want to use a bigger array\n");
		exit(1);
	}
	printf("Histogram buffer size (GB): %f\n", histogramMemoryGb);

	
	
	// Create space for the new data
	global->hitfinderHistogramDepth = hist_depth;
	global->hitfinderHistogramNx = hist_nx;
	global->hitfinderHistogramNy = hist_ny;
	global->hitfinderHistogram = (float*) calloc(hist_nn, sizeof(float));
	float *hist = global->hitfinderHistogram;

	
	/*
	 *	Read in data after setting up a temporary buffer of the appropriate variable type
	 *	Somehow this works best when split out according to different data types
	 */
	hid_t		datatype_id;
	H5T_class_t dataclass;
	size_t size;
	datatype_id =  H5Dget_type(dataset_id);
	dataclass = H5Tget_class(datatype_id);
	size = H5Tget_size(datatype_id);
	long	nn = hist_nn;
	if(dataclass == H5T_FLOAT){
		if (size == sizeof(float)) {
			float* buffer = (float *) calloc(nn, sizeof(float));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				hist[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(double)) {
			double* buffer = (double *) calloc(nn, sizeof(double));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				hist[i] = buffer[i];
			free(buffer);
		}
		else {
			printf("readHitfinderHistogram: unknown floating point type, size=%i\n",(int) size);
			return;
		}
	}
	else if(dataclass == H5T_INTEGER){
		if (size == sizeof(char)) {
			char* buffer = (char*) calloc(nn, sizeof(char));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				hist[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(short)) {
			short* buffer = (short*) calloc(nn, sizeof(short));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				hist[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(int)) {
			int* buffer = (int *) calloc(nn, sizeof(int));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				hist[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(long)) {
			long* buffer = (long *) calloc(nn, sizeof(long));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				hist[i] = buffer[i];
			free(buffer);
		}
		else {
			printf("readHitfinderHistogram: unknown integer type, size=%lu\n",size);
			exit(1);
		}
	}
	else {
		printf("readHitfinderHistogram: unknown HDF5 data type\n");
		return;
	}
	
	
	// Close and cleanup
	H5Dclose(dataset_id);
	
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

}



