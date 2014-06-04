/*
 *  data2d.cpp
 *
 *  Created by Anton Barty on 4/9/10.
 *  Copyright 2010 all rights reserved.
 *
 */

#include "data2d.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <hdf5.h>


cData2d::cData2d(){
	nx = 0;
	ny = 0;
	nn = 0;
	data = NULL;
}


cData2d::cData2d(long n){
	nx = n;
	ny = n;
	nn = n*n;
	free(data); data = NULL;
	data = (tData2d *) calloc(nn, sizeof(tData2d));
}


cData2d::~cData2d(){
	free(data); data = NULL;
}	


void cData2d::create(long n){
	nx = n;
	ny = n;
	nn = n*n;
	free(data); data = NULL;
	data = (tData2d *) calloc(nn, sizeof(tData2d));
}

void cData2d::create(long nnx, long nny){
	nx = nnx;
	ny = nny;
	nn = nx*ny;
	free(data); data = NULL;
	data = (tData2d *) calloc(nn, sizeof(tData2d));
}


void cData2d::readHDF5(char* filename){
	char fieldname[100];
	strcpy(fieldname, "/data/data");
	readHDF5(filename, fieldname);
}

void cData2d::readHDF5(char* filename, char* fieldname){
	
	// Open the file
	hid_t file_id;
	file_id = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
	if(file_id < 0){
		printf("ERROR: Could not open file %s\n",filename);
		return;
	}
	
	// Open the dataset 
	hid_t dataset_id;
	hid_t dataspace_id;
	dataset_id = H5Dopen1(file_id, fieldname);
	dataspace_id = H5Dget_space(dataset_id);
	
	
	// Test if 2D data
	int ndims;
	ndims = H5Sget_simple_extent_ndims(dataspace_id);
	if(ndims != 2) {
		printf("2dData::readHDF5: Not 2D data set, ndims=%i\n",ndims);
		exit(1);
	}
	

	// Get dimensions of data set (nx, ny, nn)
	hsize_t dims[ndims];
	H5Sget_simple_extent_dims(dataspace_id,dims,NULL);
	ny = dims[0];
	nx = dims[1];
	nn = 1;
	for(int i = 0;i<ndims;i++)
		nn *= dims[i];
	
	
	// Create space for the new data
	free(data); data = NULL;
	data = (tData2d *) calloc(nn, sizeof(tData2d));

	
	// Read in data after setting up a temporary buffer of the appropriate variable type
	// Somehow this works best when split out accordint to different data types 
	// Fix into general form later
	hid_t		datatype_id;
	H5T_class_t dataclass;
	size_t size;
	datatype_id =  H5Dget_type(dataset_id);
	dataclass = H5Tget_class(datatype_id);
	size = H5Tget_size(datatype_id);

	if(dataclass == H5T_FLOAT){
		if (size == sizeof(float)) {
			float* buffer = (float *) calloc(nn, sizeof(float));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				data[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(double)) {
			double* buffer = (double *) calloc(nn, sizeof(double));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				data[i] = buffer[i];
			free(buffer);
		}
		else {
			printf("2dData::readHDF5: unknown floating point type, size=%i\n",(int) size);
			return;
		}
	} 
	else if(dataclass == H5T_INTEGER){
		if (size == sizeof(char)) {
			char* buffer = (char*) calloc(nn, sizeof(char)); 
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				data[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(short)) {
			short* buffer = (short*) calloc(nn, sizeof(short)); 
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				data[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(int)) {
			int* buffer = (int *) calloc(nn, sizeof(int)); 
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				data[i] = buffer[i];
			free(buffer);
		}
		else if (size == sizeof(long)) {
			long* buffer = (long *) calloc(nn, sizeof(long));
			H5Dread(dataset_id, datatype_id, H5S_ALL,H5S_ALL, H5P_DEFAULT, buffer);
			for(long i=0; i<nn; i++)
				data[i] = buffer[i];
			free(buffer);
		}
		else {
			printf("2dData::readHDF5: unknown integer type, size=%lu\n",size);
			exit(1);
		}
	}
	else {
		printf("2dData::readHDF5: unknown HDF5 data type\n");	
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


/*
 *	Write 2D data to HDF5 file
 */
void cData2d::writeHDF5(char* filename){
	
	// Figure out the HDF5 data type
	hid_t out_type_id = 0;
	if(sizeof(tData2d) == sizeof(float))
		out_type_id = H5T_NATIVE_FLOAT;
	else if(sizeof(tData2d) == sizeof(double))
		out_type_id = H5T_NATIVE_DOUBLE;
	else {
		printf("2dData::writeHDF5: unsuppoted data type\n");	
		exit(1);
	}
	
	// Create the file and data group
	hid_t file_id;
	
	file_id = H5Fcreate(filename,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	H5Gcreate1(file_id,"data",0);
	
	// Data space dimensions
	int	ndims = 2;
	hsize_t dims[ndims];
	dims[0] = ny;
	dims[1] = nx;
	
	//  Write the data
	hid_t dataspace_id;
	hid_t  dataset_id;
	dataspace_id = H5Screate_simple(ndims, dims, NULL);
	dataset_id = H5Dcreate1(file_id, "/data/data", out_type_id, dataspace_id, H5P_DEFAULT);
	if(H5Dwrite(dataset_id,out_type_id , H5S_ALL, H5S_ALL,H5P_DEFAULT, data)< 0){
		printf("2dData::writeHDF5: Error writing data to file\n");	
		exit(1);
	}
	
	
	// Close and exit
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

