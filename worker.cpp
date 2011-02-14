/*
 *  worker.cpp
 *  cspad_cryst
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad-gjw/CspadTemp.hh"
#include "cspad-gjw/CspadCorrector.hh"
#include "cspad-gjw/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>

#include "setup.h"
#include "worker.h"



/*
 *	Worker thread function for processing each cspad data frame
 */
void *worker(void *threadarg) {

	/*
	 *	Turn threadarg into a more useful form
	 */
	cGlobal			*global;
	tThreadInfo		*threadInfo;
	threadInfo = (tThreadInfo*) threadarg;
	global = threadInfo->pGlobal;

	

	/*
	 *	Assemble all four quadrants into one large array 
	 */
	threadInfo->raw_data = (uint16_t*) calloc(8*ROWS*8*COLS,sizeof(uint16_t));
	for(int quadrant=0; quadrant<4; quadrant++) {
		long	i,j,ii;
		for(long k=0; k<2*ROWS*8*COLS; k++) {
			i = k % (2*ROWS) + quadrant*(2*ROWS);
			j = k / (2*ROWS);
			ii  = i+(8*ROWS)*j;
			threadInfo->raw_data[ii] = threadInfo->quad_data[quadrant][k];
		}
	}
	threadInfo->corrected_data = (uint16_t*) calloc(8*ROWS*8*COLS,sizeof(uint16_t));
	memcpy(threadInfo->corrected_data, threadInfo->raw_data, 8*ROWS*8*COLS*sizeof(uint16_t));
	

	
	/*
	 *	Subtract common mode offsets
	 */
	if(global->cmModule) {
		cmModuleSubtract(threadInfo, global);
	}
	else if(global->cmColumn) {
		// Blank for now
	}
	
	
	/*
	 *	Subtract darkcal image
	 */
	if(global->subtractDarkcal) {
		subtractDarkcal(threadInfo, global);
	}
	

	/*
	 *	Identify and remove hot pixels
	 */
	if(global->autohotpixel){
		killHotpixels(threadInfo, global);
	}
	
	/*
	 *	Hitfinding
	 */
	int	hit = 0;
	
	// Everything is a hit if we are in hdf5dump mode! 
	if(global->hdf5dump)
		hit = 1;
	
	/*
	 *	Assemble quadrants into a 'realistic' 2D image
	 */
	assemble2Dimage(threadInfo, global);
	
	
	/*
	 *	Maintain a running sum of data
	 */
	addToPowder(threadInfo, global);
	
	
	
	/*
	 *	If this is a hit, write out to our favourite HDF5 format
	 */
	if(hit)
		writeHDF5(threadInfo, global);
	else
		printf("%i (%2.1fHz): Processed\n", threadInfo->threadNum,global->datarate);

	
	
	
	/*
	 *	Cleanup and exit
	 */
	// Decrement thread pool counter by one
	pthread_mutex_lock(&global->nActiveThreads_mutex);
	global->nActiveThreads -= 1;
	pthread_mutex_unlock(&global->nActiveThreads_mutex);
	
	// Free memory
	for(int quadrant=0; quadrant<4; quadrant++) 
		free(threadInfo->quad_data[quadrant]);	
	free(threadInfo->raw_data);
	free(threadInfo->image);
	free(threadInfo);

	// Exit thread
	pthread_exit(NULL);
}


/*
 *	Subtract common mode on each module
 *	This is done in a very slow way now - speed up later once we know it works!
 */
void cmModuleSubtract(tThreadInfo *threadInfo, cGlobal *global){

	DEBUGL2_ONLY printf("cmModuleSubtract\n");
	
	long		e;
	long		counter;
	uint16_t	value;
	uint16_t	median;
	
	// Create histogram array
	int			nhist = 65535;
	uint16_t	*histogram;
	histogram = (uint16_t*) calloc(nhist, sizeof(uint16_t));
	
	// Loop over modules (8x8 array)
	for(long mi=0; mi<8; mi++){
		for(long mj=0; mj<8; mj++){

			// Zero histogram
			memset(histogram, 0, nhist*sizeof(uint16_t));
			
			
			// Loop over pixels within a module
			for(long i=0; i<ROWS; i++){
				for(long j=0; j<COLS; j++){
					e = (j + mj*COLS) * (8*ROWS);
					e += i + mi*ROWS;
					histogram[threadInfo->corrected_data[e]] += 1;
				}
			}
			
			// Find median value
			counter = 0;
			for(long i=0; i<nhist; i++){
				counter += histogram[i];
				if(counter > (global->cmFloor*ROWS*COLS)) {
					median = i;
					break;
				}
			}
			//DEBUGL2_ONLY printf("Median of module (%i,%i) = %i\n",mi,mj,median);

			// Subtract median value
			for(long i=0; i<ROWS; i++){
				for(long j=0; j<COLS; j++){
					e = (j + mj*COLS) * (8*ROWS);
					e += i + mi*ROWS;
					value = threadInfo->corrected_data[e];
					if(value > median)
						threadInfo->corrected_data[e] -= median;
					else
						threadInfo->corrected_data[e] = 0;
				}
			}
		}
	}
}


/*
 *	Subtract pre-loaded darkcal file
 */
void subtractDarkcal(tThreadInfo *threadInfo, cGlobal *global){

	// Make sure subtraction bottoms out at 0 and does not go 'negative'
	for(long i=0;i<global->pix_nn;i++){
		if(threadInfo->corrected_data[i] > global->darkcal[i])
			threadInfo->corrected_data[i] -= global->darkcal[i];
		else
			threadInfo->corrected_data[i] = 0;
	}
}

/*
 *	Identify and kill hot pixels
 */
void killHotpixels(tThreadInfo *threadInfo, cGlobal *global){
	
	pthread_mutex_lock(&global->hotpixel_mutex);
	for(long i=0;i<global->pix_nn;i++){
		global->hotpixelmask[i] = ( ((threadInfo->corrected_data[i]>global->hotpixADC)?(1.0):(0.0)) + (global->hotpixMemory-1)*global->hotpixelmask[i]) / global->hotpixMemory;
	}
	pthread_mutex_unlock(&global->hotpixel_mutex);

	
	long	nhot = 0;
	for(long i=0;i<global->pix_nn;i++){
		if(global->hotpixelmask[i] > global->hotpixFreq) {
			threadInfo->corrected_data[i] = 0;
			//nhot++;
		}
	}	
	//printf("%i\n",nhot);
}
	

/*
 *	Maintain running powder patterns
 */
void addToPowder(tThreadInfo *threadInfo, cGlobal *global){
	
	// Sum raw format data
	pthread_mutex_lock(&global->powdersum1_mutex);
	global->npowder += 1;
	for(long i=0; i<global->pix_nn; i++)
		global->powderRaw[i] += threadInfo->corrected_data[i];
	pthread_mutex_unlock(&global->powdersum1_mutex);

	
	// Sum assembled data
	pthread_mutex_lock(&global->powdersum2_mutex);
	for(long i=0; i<global->image_nn; i++)
		global->powderAssembled[i] += threadInfo->image[i];
	pthread_mutex_unlock(&global->powdersum2_mutex);
	
}


/*
 *	Interpolate raw (corrected) cspad data into a physical 2D image
 *	using pre-defined pixel mapping (as loaded from .h5 file)
 */
void assemble2Dimage(tThreadInfo *threadInfo, cGlobal *global){
	
	
	// Allocate temporary arrays for pixel interpolation (needs to be floating point)
	float	*data = (float*) calloc(global->image_nn,sizeof(float));
	float	*weight = (float*) calloc(global->image_nn,sizeof(float));
	for(long i=0; i<global->image_nn; i++){
		data[i] = 0;
		weight[i]= 0;
	}
	
	
	// Loop through all pixels and interpolate onto regular grid
	float	x, y;
	float	pixel_value, w;
	long	ix, iy;
	float	fx, fy;
	long	image_index;

	for(long i=0;i<global->pix_nn;i++){
		// Pixel location with (0,0) at array element (0,0) in bottom left corner
		x = global->pix_x[i] + global->image_nx/2;
		y = global->pix_y[i] + global->image_nx/2;
		pixel_value = threadInfo->corrected_data[i];
		
		// Split coordinate into integer and fractional parts
		ix = (long) floor(x);
		iy = (long) floor(y);
		fx = x - ix;
		fy = y - iy;
		
		//printf("%i\t%i\n",ix,iy);
		
		// Interpolate intensity over adjacent 4 pixels using fractional overlap as the weighting factor
		// (0,0)
		if(ix>=0 && iy>=0 && ix<global->image_nx && iy<global->image_nx) {
			w = (1-fx)*(1-fy);
			image_index = ix + global->image_nx*iy;
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (+1,0)
		if((ix+1)>=0 && iy>=0 && (ix+1)<global->image_nx && iy<global->image_nx) {
			w = (fx)*(1-fy);
			image_index = (ix+1) + global->image_nx*iy;
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (0,+1)
		if(ix>=0 && (iy+1)>=0 && ix<global->image_nx && (iy+1)<global->image_nx) {
			w = (1-fx)*(fy);
			image_index = ix + global->image_nx*(iy+1);
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
		// (+1,+1)
		if((ix+1)>=0 && (iy+1)>=0 && (ix+1)<global->image_nx && (iy+1)<global->image_nx) {
			w = (fx)*(fy);
			image_index = (ix+1) + global->image_nx*(iy+1);
			data[image_index] += w*pixel_value;
			weight[image_index] += w;
		}
	}
	
	
	// Reweight pixel interpolation
	for(long i=0; i<global->image_nn; i++){
		if(weight[i] < 0.1)
			data[i] = 0;
		else
			data[i] /= weight[i];
	}

	
	// Allocate memory for output image
	threadInfo->image = (uint16_t*) calloc(global->image_nn,sizeof(uint16_t));

	// Copy interpolated image across into image array
	for(long i=0;i<global->image_nn;i++){
		threadInfo->image[i] = (uint16_t) data[i];
	}	
	
	
	// Free temporary arrays
	free(data);
	free(weight);
	
}


/*
 *	Write out processed data to our 'standard' HDF5 format
 */
void writeHDF5(tThreadInfo *info, cGlobal *global){
	/*
	 *	Create filename based on date, time and fiducial for this image
	 */
	char outfile[1024];
	char buffer1[80];
	char buffer2[80];	
	time_t eventTime = info->seconds;
	
	//setenv("TZ","US/Pacific",1);		// <--- Dangerous (not thread safe!)
	struct tm *timestatic, timelocal;
	timestatic=localtime_r( &eventTime, &timelocal );	
	strftime(buffer1,80,"%Y_%b%d",&timelocal);
	strftime(buffer2,80,"%H%M%S",&timelocal);
	sprintf(outfile,"LCLS_%s_r%04u_%s_%x_cspad.h5",buffer1,getRunNumber(),buffer2,info->fiducial);
	printf("%i (%2.1f Hz): Writing data to: %s\n",info->threadNum,global->datarate, outfile);


		
	/* 
 	 *  HDF5 variables
	 */
	hid_t		hdf_fileID;
	hid_t		dataspace_id;
	hid_t		dataset_id;
	hid_t		datatype;
	hsize_t 	size[2],max_size[2];
	herr_t		hdf_error;
	hid_t   	gid;
	char 		fieldname[100]; 
	
	
	/*
	 *	Create the HDF5 file
	 */
	hdf_fileID = H5Fcreate(outfile,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	
	
	/*
	 *	Save image data into '/data' part of HDF5 file
	 */
	gid = H5Gcreate(hdf_fileID, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( gid < 0 ) {
		ERROR("%i: Couldn't create group\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}
	
	// Assembled image
	size[0] = global->image_nx;	// size[0] = height
	size[1] = global->image_nx;	// size[1] = width
	max_size[0] = global->image_nx;
	max_size[1] = global->image_nx;
	dataspace_id = H5Screate_simple(2, size, max_size);
	dataset_id = H5Dcreate(gid, "data", H5T_STD_U16LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( dataset_id < 0 ) {
		ERROR("%i: Couldn't create dataset\n", info->threadNum);
		H5Fclose(hdf_fileID);
		return;
	}
	hdf_error = H5Dwrite(dataset_id, H5T_STD_U16LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, info->image);
	if ( hdf_error < 0 ) {
		ERROR("%i: Couldn't write data\n", info->threadNum);
		H5Dclose(dataspace_id);
		H5Fclose(hdf_fileID);
		return;
	}
	H5Dclose(dataset_id);
	H5Sclose(dataspace_id);
	

	// Save raw data?
	if(global->saveRaw) {
		size[0] = 8*COLS;	// size[0] = height
		size[1] = 8*ROWS;	// size[1] = width
		max_size[0] = 8*COLS;
		max_size[1] = 8*ROWS;
		dataspace_id = H5Screate_simple(2, size, max_size);
		dataset_id = H5Dcreate(gid, "rawdata", H5T_STD_U16LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if ( dataset_id < 0 ) {
			ERROR("%i: Couldn't create dataset\n", info->threadNum);
			H5Fclose(hdf_fileID);
			return;
		}
		hdf_error = H5Dwrite(dataset_id, H5T_STD_U16LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, info->raw_data);
		if ( hdf_error < 0 ) {
			ERROR("%i: Couldn't write data\n", info->threadNum);
			H5Dclose(dataspace_id);
			H5Fclose(hdf_fileID);
			return;
		}
		H5Dclose(dataset_id);
		H5Sclose(dataspace_id);
	}

	
	// Done with this group
	H5Gclose(gid);
	
	
	double		phaseCavityTime1;
	double		phaseCavityTime2;
	double		phaseCavityCharge1;
	double		phaseCavityCharge2;
	
	/*
	 *	Write LCLS event information
	 */
	gid = H5Gcreate1(hdf_fileID,"LCLS",0);
	size[0] = 1;
	dataspace_id = H5Screate_simple( 1, size, NULL );
	//dataspace_id = H5Screate(H5S_SCALAR);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/machineTime", H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->seconds );
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/fiducial", H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fiducial );
	H5Dclose(dataset_id);
		
	// Electron beam data
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamCharge", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamCharge );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamL3Energy", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamL3Energy );
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamPkCurrBC2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamPkCurrBC2 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUPosX", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUPosX );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUPosY", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUPosY );
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUAngX", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUAngX );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/ebeamLTUAngY", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->fEbeamLTUAngY );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityTime1", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityTime1 );
	H5Dclose(dataset_id);
	
	// Phase cavity information
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityTime2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityTime2 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityCharge1", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityCharge1 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/phaseCavityCharge2", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->phaseCavityCharge2 );
	H5Dclose(dataset_id);
	
	// Calculated photon energy
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_energy_eV", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->photonEnergyeV);
	H5Dclose(dataset_id);
	
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/photon_wavelength_A", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->wavelengthA);
	H5Dclose(dataset_id);
	
	
	// Gas detector values
	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_11_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd11 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_12_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd12 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_21_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd21 );
	H5Dclose(dataset_id);

	dataset_id = H5Dcreate1(hdf_fileID, "/LCLS/f_22_ENRC", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info->gmd22 );	
	H5Dclose(dataset_id);

	// Finished with scalar dataset ID
	H5Sclose(dataspace_id);
	
	
	// Time in human readable format
	// Writing strings in HDF5 is a little tricky --> this could be improved!
	char* timestr;
	timestr = ctime(&eventTime);
	dataspace_id = H5Screate(H5S_SCALAR);
	datatype = H5Tcopy(H5T_C_S1);  
	H5Tset_size(datatype,strlen(timestr)+1);
	dataset_id = H5Dcreate1(hdf_fileID, "LCLS/eventTimeString", datatype, dataspace_id, H5P_DEFAULT);
	H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, timestr );
	H5Dclose(dataset_id);
	H5Sclose(dataspace_id);
	hdf_error = H5Lcreate_soft( "/LCLS/eventTimeString", hdf_fileID, "/LCLS/eventTime",0,0);
	
	
	// Close group and flush buffers
	H5Gclose(gid);
	H5Fflush(hdf_fileID,H5F_SCOPE_LOCAL);

	
	/*
	 *	Clean up stale HDF5 links
	 *		(thanks Tom/Filipe)
	 */
	int n_ids;
	hid_t ids[256];
	n_ids = H5Fget_obj_ids(hdf_fileID, H5F_OBJ_ALL, 256, ids);
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
	
	H5Fclose(hdf_fileID); 
}





/*
 *	Write test data to a simple HDF5 file
 */

void writeSimpleHDF5(const char *filename, const void *data, int width, int height, int type) 
{
	hid_t fh, gh, sh, dh;	/* File, group, dataspace and data handles */
	herr_t r;
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
	
	size[0] = height;
	size[1] = width;
	max_size[0] = height;
	max_size[1] = width;
	sh = H5Screate_simple(2, size, max_size);
	
	dh = H5Dcreate(gh, "data", type, sh,
	               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if ( dh < 0 ) {
		ERROR("Couldn't create dataset\n");
		H5Fclose(fh);
	}
	
	/* Muppet check */
	H5Sget_simple_extent_dims(sh, size, max_size);
	
	r = H5Dwrite(dh, type, H5S_ALL,
	             H5S_ALL, H5P_DEFAULT, data);
	if ( r < 0 ) {
		ERROR("Couldn't write data\n");
		H5Dclose(dh);
		H5Fclose(fh);
	}
	
	H5Gclose(gh);
	H5Dclose(dh);
	H5Fclose(fh);
}


void saveRunningSums(cGlobal *global) {
	char	filename[1024];

	/*
	 *	Save raw data
	 */
	printf("Saving raw sum data to file\n");
	sprintf(filename,"r%04u-RawSum.h5",global->runNumber);
	pthread_mutex_lock(&global->powdersum1_mutex);
	writeSimpleHDF5(filename, global->powderRaw, global->pix_nx, global->pix_ny, H5T_STD_U32LE);	
	pthread_mutex_unlock(&global->powdersum1_mutex);
	

	printf("Saving assembled sum data to file\n");
	sprintf(filename,"r%04u-AssembledSum.h5",global->runNumber);
	pthread_mutex_lock(&global->powdersum2_mutex);
	writeSimpleHDF5(filename, global->powderAssembled, global->image_nx, global->image_nx, H5T_STD_U32LE);	
	pthread_mutex_unlock(&global->powdersum2_mutex);
	
	
	/*
	 *	Compute and save darkcal
	 */
	printf("Processing darkcal\n");
	sprintf(filename,"r%04u-darkcal.h5",global->runNumber);
	uint16_t *data = (uint16_t*) calloc(global->pix_nn, sizeof(uint16_t));
	pthread_mutex_lock(&global->powdersum1_mutex);
	for(long i=0; i<global->pix_nn; i++)
		data[i] = (uint16_t) (global->powderRaw[i]/global->npowder);
	pthread_mutex_unlock(&global->powdersum1_mutex);
	
	printf("Saving darkcal to file\n");
	writeSimpleHDF5(filename, data, global->pix_nx, global->pix_ny, H5T_STD_U16LE);	
	free(data);
	
}

