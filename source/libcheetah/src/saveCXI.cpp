/*

 *  saveCXI.cpp
 *  cheetah
 *
 *  Created by Jing Liu on 05/11/12.
 *  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
 *
 */

#include <string>
#include <vector>
#include <pthread.h>
#include <math.h>

#include "saveCXI.h"


herr_t
cheetahHDF5ErrorHandler(hid_t,void *)
{
	// print the error message
	//H5Eprint1(stderr);
	H5Eprint(H5E_DEFAULT, stderr);
	// abort such that we get a stack trace to debug
	abort();
}


template <class T>
hid_t get_datatype(const T * foo){
	hid_t datatype = 0;
	if(typeid(T) == typeid(bool) && sizeof(bool) == 1){
		datatype = H5T_NATIVE_INT8;
	}else if(typeid(T) == typeid(short)){
		datatype = H5T_NATIVE_INT16;
	}else if(typeid(T) == typeid(unsigned short)){
		datatype = H5T_NATIVE_UINT16;
	}else if((typeid(T) == typeid(int))) {
		datatype = H5T_NATIVE_INT32;
	}else if(typeid(T) == typeid(unsigned int)){
		datatype = H5T_NATIVE_UINT32;
	}else if(typeid(T) == typeid(long)){
		datatype = H5T_NATIVE_LONG;
	}else if(typeid(T) == typeid(unsigned long)){
		datatype = H5T_NATIVE_ULONG;
	}else if(typeid(T) == typeid(float)){
		datatype = H5T_NATIVE_FLOAT;
	}else if(typeid(T) == typeid(double)){
		datatype = H5T_NATIVE_DOUBLE;
	}else if(typeid(T) == typeid(char)){
		datatype = H5T_NATIVE_CHAR;
	}else{
		ERROR("Do not understand type: %s",typeid(T).name());
	}
	return datatype;
}

template <class T>
static T * generateThumbnail(const T * src,const int srcWidth, const int srcHeight, const int scale)
{
	int dstWidth = srcWidth/scale;
	int dstHeight = srcHeight/scale;
	T * dst = new T [srcWidth*srcHeight];
	for(int x = 0; x <dstWidth; x++){
		for(int y = 0; y<dstHeight; y++){
			double res=0;
			for (int xx = x*scale; xx <x*scale+scale; xx++){
				for(int yy = y*scale; yy <y*scale+scale; yy++){
					res += src[yy*srcWidth+xx];
				} 
			}
			dst[y*dstWidth+x] = (short int) res/(scale*scale);
		}
	}
	return dst;
}

static uint getStackSlice(CXI::File * cxi){
#ifdef __GNUC__
	return __sync_fetch_and_add(&(cxi->stackCounter),1);
#else
	pthread_mutex_lock(&global->framefp_mutex);
	uint ret = cxi->stackCounter;
	cxi->stackCounter++;
	pthread_mutex_unlock(&global->framefp_mutex);
	return ret;
#endif
}

static hid_t createScalarStack(const char * name, hid_t loc, hid_t dataType){
	hsize_t dims[1] = {CXI::chunkSize1D/H5Tget_size(dataType)};
	hsize_t maxdims[1] = {H5S_UNLIMITED};
	hid_t cparms = H5Pcreate(H5P_DATASET_CREATE);
	hid_t dataspace = H5Screate_simple(1, dims, maxdims);
	if( dataspace<0 ) {ERROR("Cannot create dataspace.\n");}
	/* Modify dataset creation properties, i.e. enable chunking  */
	H5Pset_chunk(cparms, 1, dims);
	//  H5Pset_deflate (cparms, 2);
	hid_t dataset = H5Dcreate(loc, name, dataType, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
	if( dataset<0 ) {ERROR("Cannot create dataset.\n");}

	const char * axis = "experiment_identifier";
	hsize_t one = 1;
	hid_t datatype = H5Tcopy(H5T_C_S1);
	H5Tset_size(datatype, strlen(axis));
	hid_t memspace = H5Screate_simple(1,&one,NULL);
	hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
	H5Awrite(attr,datatype,axis);
	H5Tclose(datatype);
	H5Aclose(attr);

	attr = H5Acreate(dataset,CXI::ATTR_NAME_NUM_EVENTS,H5T_NATIVE_INT32,memspace,H5P_DEFAULT,H5P_DEFAULT);
	int zero = 0;
	H5Awrite(attr,H5T_NATIVE_INT32,&zero);
	H5Aclose(attr);

	H5Sclose(memspace);
	H5Sclose(dataspace);
	H5Pclose(cparms);
	return dataset;
}

template <class T> 
static void writeScalarToStack(hid_t dataset, uint stackSlice, T value){
	hid_t hs,w;
	hsize_t count[1] = {1};
	hsize_t offset[1] = {stackSlice};
	hsize_t stride[1] = {1};
	hsize_t block[1] = {1};
	/* dummy */
	hsize_t mdims[1];
 
	hid_t dataspace = H5Dget_space (dataset);
	if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	H5Sget_simple_extent_dims(dataspace, block, mdims);
	/* check if we need to extend the dataset */
	if(block[0] <= stackSlice){
		while(block[0] <= stackSlice){
			block[0] *= 2;
		}
		H5Dset_extent(dataset, block);
		/* get enlarged dataspace */
		H5Sclose(dataspace);
		dataspace = H5Dget_space (dataset);
		if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	}
	block[0] = 1;
	hid_t memspace = H5Screate_simple (1, block, NULL);
	hid_t type = get_datatype(&value);

	hs = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
	if( hs<0 ) {ERROR("Cannot select hyperslab.\n");}
	w = H5Dwrite(dataset, type, memspace, dataspace, H5P_DEFAULT, &value) < 0;
	if( w<0 ){
		ERROR("Cannot write to file.\n");
		abort();
	}

	hid_t a = H5Aopen(dataset, CXI::ATTR_NAME_NUM_EVENTS, H5P_DEFAULT);
	// Silently ignore failure to write, this attribute is non-essential
	if(a>=0) {
		uint oldVal;
		w = H5Aread(a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to read back size attribute");
		}
		if (oldVal < stackSlice + 1) {
			oldVal = stackSlice + 1;
		}
		w = H5Awrite (a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to write size attribute");
		}
		H5Aclose(a);
	}

	H5Sclose(memspace);
	H5Sclose(dataspace);
}


/* Create a 2D stack. The fastest changing dimension is along the width */
static hid_t create2DStack(const char *name, hid_t loc, int width, int height, hid_t dataType){
	hsize_t dims[3] = {lrintf(((float)CXI::chunkSize2D)/H5Tget_size(dataType)/width/height),
					   static_cast<hsize_t>(height),static_cast<hsize_t>(width)};
	hsize_t maxdims[3] = {H5S_UNLIMITED,static_cast<hsize_t>(height),static_cast<hsize_t>(width)};
	hid_t dataspace = H5Screate_simple(3, dims, maxdims);
	if( dataspace<0 ) {ERROR("Cannot create dataspace.\n");}
	hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
	H5Pset_chunk(cparms, 3, dims);
	//  H5Pset_deflate (cparms, 2);
	hid_t dataset = H5Dcreate(loc, name, dataType, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
	if( dataset<0 ) {ERROR("Cannot create dataset.\n");}
	H5Pset_chunk_cache(H5Dget_access_plist(dataset),H5D_CHUNK_CACHE_NSLOTS_DEFAULT,1024*1024*16,1);

	const char * axis = "experiment_identifier:y:x";
	hsize_t one = 1;
	hid_t datatype = H5Tcopy(H5T_C_S1);
	H5Tset_size(datatype, strlen(axis));
	hid_t memspace = H5Screate_simple(1,&one,NULL);
	hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
	H5Awrite(attr,datatype,axis);
	H5Aclose(attr);
	attr = H5Acreate(dataset,CXI::ATTR_NAME_NUM_EVENTS,H5T_NATIVE_INT32,memspace,H5P_DEFAULT,H5P_DEFAULT);
	int zero = 0;
	H5Awrite(attr,H5T_NATIVE_INT32,&zero);
	H5Tclose(datatype);
	H5Aclose(attr);
	H5Sclose(memspace);
	H5Sclose(dataspace);
	H5Pclose(cparms);
	return dataset;    
}

template <class T> 
static void write2DToStack(hid_t dataset, uint stackSlice, T * data){  
	hid_t hs,w;
	hsize_t count[3] = {1,1,1};
	hsize_t offset[3] = {stackSlice,0,0};
	/* stride is irrelevant in this case */
	hsize_t stride[3] = {1,1,1};
	hsize_t block[3];
	/* dummy */
	hsize_t mdims[3];
	/* Use the existing dimensions as block size */
	hid_t dataspace = H5Dget_space (dataset);
	if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	H5Sget_simple_extent_dims(dataspace, block, mdims);
	/* check if we need to extend the dataset */
	if(block[0] <= stackSlice){
		while(block[0] <= stackSlice){
			block[0] *= 2;
		}
		H5Dset_extent (dataset, block);
		/* get enlarged dataspace */
		H5Sclose(dataspace);
		dataspace = H5Dget_space (dataset);
		if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	}
	block[0] = 1;
	hid_t memspace = H5Screate_simple (3, block, NULL);
	hid_t type = get_datatype(data);

	hs = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
	if( hs<0 ) {
		ERROR("Cannot select hyperslab.\n");
	}
	w = H5Dwrite (dataset, type, memspace, dataspace, H5P_DEFAULT, data);
	if( w<0 ){
		ERROR("Cannot write to file.\n");
	}
	hid_t a = H5Aopen(dataset, CXI::ATTR_NAME_NUM_EVENTS, H5P_DEFAULT);
	// Silently ignore failure to write, this attribute is non-essential
	if(a>=0) {
		uint oldVal;
		w = H5Aread(a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to read back size attribute");
		}
		if (oldVal < stackSlice + 1) {
			oldVal = stackSlice + 1;
		}
		w = H5Awrite (a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to write size attribute");
		}
		H5Aclose(a);
	}
	H5Sclose(memspace);
	H5Sclose(dataspace);
}


/* Create a 1D stack. */
static hid_t create1DStack(const char *name, hid_t loc, int size, hid_t dataType){
	hsize_t dims[2] = {lrintf(((float)CXI::chunkSize2D)/H5Tget_size(dataType)/size),
					   static_cast<hsize_t>(size)};
	hsize_t maxdims[2] = {H5S_UNLIMITED,static_cast<hsize_t>(size)};
	hid_t dataspace = H5Screate_simple(2, dims, maxdims);
	if( dataspace<0 ) {ERROR("Cannot create dataspace.\n");}
	hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
	H5Pset_chunk(cparms, 2, dims);
	//  H5Pset_deflate (cparms, 2);
	hid_t dataset = H5Dcreate(loc, name, dataType, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
	if( dataset<0 ) {ERROR("Cannot create dataset.\n");}
	H5Pset_chunk_cache(H5Dget_access_plist(dataset),H5D_CHUNK_CACHE_NSLOTS_DEFAULT,1024*16,1);

	const char * axis = "experiment_identifier:coordinate";
	hsize_t one = 1;
	hid_t datatype = H5Tcopy(H5T_C_S1);
	H5Tset_size(datatype, strlen(axis));
	hid_t memspace = H5Screate_simple(1,&one,NULL);
	hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
	H5Awrite(attr,datatype,axis);
	H5Aclose(attr);
	attr = H5Acreate(dataset,CXI::ATTR_NAME_NUM_EVENTS,H5T_NATIVE_INT32,memspace,H5P_DEFAULT,H5P_DEFAULT);
	int zero = 0;
	H5Awrite(attr,H5T_NATIVE_INT32,&zero);
	H5Tclose(datatype);
	H5Aclose(attr);
	H5Sclose(memspace);
	H5Sclose(dataspace);
	H5Pclose(cparms);
	return dataset;    
}

template <class T> 
static void write1DToStack(hid_t dataset, uint stackSlice, T * data){  
	hid_t hs,w;
	hsize_t count[2] = {1,1};
	hsize_t offset[2] = {stackSlice,0};
	/* stride is irrelevant in this case */
	hsize_t stride[2] = {1,1};
	hsize_t block[2];
	/* dummy */
	hsize_t mdims[2];
	/* Use the existing dimensions as block size */
	hid_t dataspace = H5Dget_space (dataset);
	if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	H5Sget_simple_extent_dims(dataspace, block, mdims);
	/* check if we need to extend the dataset */
	if(block[0] <= stackSlice){
		while(block[0] <= stackSlice){
			block[0] *= 2;
		}
		H5Dset_extent (dataset, block);
		/* get enlarged dataspace */
		H5Sclose(dataspace);
		dataspace = H5Dget_space (dataset);
		if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	}
	block[0] = 1;
	hid_t memspace = H5Screate_simple (2, block, NULL);
	hid_t type = get_datatype(data);

	hs = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
	if( hs<0 ) {
		ERROR("Cannot select hyperslab.\n");
	}
	w = H5Dwrite (dataset, type, memspace, dataspace, H5P_DEFAULT, data);
	if( w<0 ){
		ERROR("Cannot write to file.\n");
	}
	hid_t a = H5Aopen(dataset, CXI::ATTR_NAME_NUM_EVENTS, H5P_DEFAULT);
	// Silently ignore failure to write, this attribute is non-essential
	if(a>=0) {
		uint oldVal;
		w = H5Aread(a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to read back size attribute");
		}
		if (oldVal < stackSlice + 1) {
			oldVal = stackSlice + 1;
		}
		w = H5Awrite (a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to write size attribute");
		}
		H5Aclose(a);
	}
	H5Sclose(memspace);
	H5Sclose(dataspace);
}

template <class T> 
static hid_t createDataset(const char *name, hid_t loc, T *data,int width=1, int height=0, int length=0){
	hid_t datatype;
	int ndims;
	hid_t dataspace;
	hsize_t dims1[1] = {static_cast<hsize_t>(static_cast<hsize_t>(width))};
	hsize_t dims2[2] = {static_cast<hsize_t>(width),static_cast<hsize_t>(height)};
	hsize_t dims3[3] = {static_cast<hsize_t>(width),static_cast<hsize_t>(height),static_cast<hsize_t>(length)};
	datatype = get_datatype(data);
	if(typeid(T) == typeid(char)){
		datatype = H5Tcopy(H5T_C_S1);
		H5Tset_size(datatype, width);    
		dims1[0] = 1;
		ndims = 1;
		dataspace = H5Screate_simple(ndims, dims1, dims1);
	}else{
		if(height == 0 && length==0){
			ndims = 1;
			dataspace = H5Screate_simple(ndims, dims1, dims1);
		}else if(length==0){
			ndims = 2; 
			dataspace = H5Screate_simple(ndims, dims2, dims2);
		}else{
			ndims = 3;
			dataspace = H5Screate_simple(ndims, dims3, dims3);
		}
	}
	if( dataspace<0 ) {ERROR("Cannot create dataspace.\n");}
	hid_t dataset = H5Dcreate(loc, name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(dataspace);
	if(typeid(T) == typeid(char)){
		H5Tclose(datatype);
	}
	return dataset;
}



template <class T> 
static void writeDataset(hid_t dataset, T *data,int width=1, int height=0, int length=0){
	hid_t datatype = get_datatype(data);
	if(typeid(T) == typeid(char)){
		datatype = H5Tcopy(H5T_C_S1);
		H5Tset_size(datatype, width);    
	}
	hid_t w = H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);  
	if( w<0 ){
		ERROR("Cannot write to file.\n");
	}
	H5Dclose(dataset);
	if(typeid(T) == typeid(char)){
		H5Tclose(datatype);
	}
}

template <class T> 
static void createAndWriteDataset(const char *name, hid_t loc, T *data,int width=1, int height=0, int length=0){
	hid_t dataset = createDataset(name,loc,data,width,height,length);
	writeDataset(dataset, data, width, height, length);
}


template <class T> 
static void openAndWriteDataset(const char *name, hid_t loc, T *data,int width=1, int height=0, int length=0){
	hid_t dataset = H5Dopen(loc,name,H5P_DEFAULT);
	writeDataset(dataset, data, width, height, length);
}

static hid_t createStringStack(const char * name, hid_t loc, int maxSize = 128){
	/* FM: This is probably wrong */
	hid_t datatype = H5Tcopy(H5T_C_S1);
	if(H5Tset_size(datatype, maxSize) < 0){
		ERROR("Cannot set type size.\n");
	}
	hsize_t dims[1] = {CXI::chunkSize1D/H5Tget_size(datatype)};
	hsize_t maxdims[1] = {H5S_UNLIMITED};
	hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
	hid_t dataspace = H5Screate_simple(1, dims, maxdims);
	H5Pset_chunk (cparms, 1, dims);
	//  H5Pset_deflate(cparms, 2);
	hid_t dataset = H5Dcreate(loc, name, datatype, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
	if( dataset<0 ){
		ERROR("Cannot create dataset.\n");
	}
	H5Tclose(datatype);
	const char * axis = "experiment_identifier";
	hsize_t one = 1;
	datatype = H5Tcopy(H5T_C_S1);
	H5Tset_size(datatype, strlen(axis));
	hid_t memspace = H5Screate_simple(1,&one,NULL);
	hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
  
	H5Awrite(attr,datatype,axis);
	H5Aclose(attr);

	attr = H5Acreate(dataset,CXI::ATTR_NAME_NUM_EVENTS,H5T_NATIVE_INT32,memspace,H5P_DEFAULT,H5P_DEFAULT);
	int zero = 0;
	H5Awrite(attr,H5T_NATIVE_INT32,&zero);
	H5Tclose(datatype);
	H5Aclose(attr);

	H5Sclose(memspace);
	H5Sclose(dataspace);
	H5Pclose(cparms);
	return dataset;    
}

static void writeStringToStack(hid_t dataset, uint stackSlice, const char * value){
	hid_t sh,w;
	hsize_t count[1] = {1};
	hsize_t offset[1] = {stackSlice};
	hsize_t stride[1] = {1};
	hsize_t block[1] = {1};
	/* dummy */
	hsize_t mdims[1];

  
	hid_t dataspace = H5Dget_space (dataset);
	if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	H5Sget_simple_extent_dims(dataspace, block, mdims);
	/* check if we need to extend the dataset */
	if(block[0] <= stackSlice){
		while(block[0] <= stackSlice){
			block[0] *= 2;
		}
		H5Dset_extent(dataset, block);
		/* get enlarged dataspace */
		H5Sclose(dataspace);
		dataspace = H5Dget_space (dataset);
		if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
	}
	block[0] = 1;
	hid_t memspace = H5Screate_simple (1, block, NULL);

	hid_t type = H5Tcopy(H5T_C_S1);
	H5Tset_size(type, strlen(value));

	sh = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
	if( sh<0 ) {ERROR("Cannot select hyperslab.\n");}
	w = H5Dwrite(dataset, type, memspace, dataspace, H5P_DEFAULT, value);
	if( w<0 ){
		ERROR("Cannot write to file.\n");
		abort();
	}
	H5Tclose(type);

	hid_t a = H5Aopen(dataset, CXI::ATTR_NAME_NUM_EVENTS, H5P_DEFAULT);
	// Silently ignore failure to write, this attribute is non-essential
	if(a>=0) {
		uint oldVal;
		w = H5Aread(a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to read back size attribute");
		}
		if (oldVal < stackSlice + 1) {
			oldVal = stackSlice + 1;
		}
		w = H5Awrite (a, H5T_NATIVE_INT32, &oldVal);
		if (w < 0)
		{
			ERROR("Failure to write size attribute");
		}
		H5Aclose(a);
	}
	H5Sclose(memspace);
	H5Sclose(dataspace);
}

/*

  CXI file skeleton

  |
  LCLS
  |  |
  |  |-... (just copied from XTC)
  |
  entry_1
  |
  |- data_1 ---------
  |                  |
  |- instrument_1    | symlink
  |   |              |
  |   |-detector_1 <-----------------------------
  |   |   |                                      |
  |   |   |- data [raw data, 3D array]           |
  |   |   |- (mask) [raw masks, 3D array]        |
  |   |   |- mask_shared [raw mask, 2D array]    | 
  |   |   |- ...                                 | symlink
  |   .   .                                      |
  |                                              |
  |- (image_1)                                   |
  |    |                                         |
  |    |- detector_1 ----------------------------|
  |    |- data [assembled data, 3D array]        |
  |    |- (mask) [assembled masks, 3D array]     |
  |    |- mask_shared [assembled mask, 2D array] |
  |    |- ...                                    |
  |    .                                         |
  |                                              |
  |- (image_2)                                   |
  |    |                                         |
  |    |- detector_1 ----------------------------
  |    |- data [downsampled assembled data, 3D array]
  |    |- (mask) [downsampled assembled masks, 3D array]
  |    |- mask_shared [downsampled assembled mask, 2D array] 
  |    |- ...
  .    .
 
*/

static CXI::File * createCXISkeleton(const char * filename,cGlobal *global){
	/* Creates the initial skeleton for the CXI file.
	   We'll rely on HDF5 automatic error reporting. It's usually loud enough.
	*/

	puts("Creating Skeleton");
	CXI::File * cxi = new CXI::File;
	hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
	if(fapl_id < 0 || H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG) < 0){
		ERROR("Cannot set file access properties.\n");
	}
#ifdef H5F_ACC_SWMR_WRITE
	if(H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0){
		ERROR("Cannot set file access properties.\n");
	}
#endif

	hid_t fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
	if( fid<0 ) {ERROR("Cannot create file.\n");}
	cxi->self = fid;
	cxi->stackCounter = 0;
	hsize_t dims[3];

	/* Create /cxi_version */
	dims[0] = 1;
	hid_t dataspace = H5Screate_simple(1, dims, dims);
	if( dataspace<0 ) {ERROR("Cannot create dataspace.\n");}
	hid_t dataset = H5Dcreate(cxi->self, "cxi_version", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite (dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &CXI::version);

	/* /entry_1 */

	// /entry_1
	cxi->entry.self = H5Gcreate(cxi->self,"/entry_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	// /entry_1/experiment_identifier
	cxi->entry.experimentIdentifier = createStringStack("experiment_identifier",cxi->entry.self);

	// /entry_1/instrument_1
	cxi->entry.instrument.self = H5Gcreate(cxi->self, "/entry_1/instrument_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	// /entry_1/instrument_1/source_1
	cxi->entry.instrument.source.self = H5Gcreate(cxi->self,"/entry_1/instrument_1/source_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	// /entry_1/instrument_1/source_1/energy
	cxi->entry.instrument.source.energy = createScalarStack("energy",cxi->entry.instrument.source.self,H5T_NATIVE_DOUBLE);
	// /entry_1/instrument_1/experiment_identifier -> /entry_1/experiment_identifier
	H5Lcreate_soft("/entry_1/experiment_identifier",cxi->entry.instrument.source.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);

	DETECTOR_LOOP{
		char detectorPath[1024];
		char dataName[1024];
		char imageName[1024];
    
		// /entry_1/instrument_1/detector_i
		sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld",detID+1);
		CXI::Detector d;
		d.self = H5Gcreate(cxi->entry.instrument.self, detectorPath, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		// /entry_1/data_i -> /entry_1/instrument_1/detector_i
		sprintf(dataName,"data_%ld",detID+1);
		H5Lcreate_soft(detectorPath,cxi->entry.self,dataName,H5P_DEFAULT,H5P_DEFAULT);

		d.distance = createScalarStack("distance", d.self,H5T_NATIVE_DOUBLE);
		d.description = createStringStack("description",d.self);
		d.xPixelSize = createScalarStack("x_pixel_size",d.self,H5T_NATIVE_DOUBLE);
		d.yPixelSize = createScalarStack("y_pixel_size",d.self,H5T_NATIVE_DOUBLE);
    
		/* Raw images */
		if(global->saveRaw){
			// /entry_1/instrument_1/detector_i/data
			d.data = create2DStack("data", d.self, global->detector[detID].pix_nx, global->detector[detID].pix_ny, H5T_STD_I16LE);
			if(global->savePixelmask){
				// /entry_1/instrument_1/detector_i/mask
				d.mask = create2DStack("mask", d.self, global->detector[detID].pix_nx, global->detector[detID].pix_ny, H5T_NATIVE_UINT16);
			}
			// /entry_1/instrument_1/detector_i/mask_shared
			createAndWriteDataset("mask_shared", d.self, global->detector[detID].pixelmask_shared, global->detector[detID].pix_nx, global->detector[detID].pix_ny);
			// /entry_1/instrument_1/detector_i/mask_shared_max
			createAndWriteDataset("mask_shared_max", d.self, global->detector[detID].pixelmask_shared_max, global->detector[detID].pix_nx, global->detector[detID].pix_ny);
			// /entry_1/instrument_1/detector_i/mask_shared_min
			createAndWriteDataset("mask_shared_min", d.self, global->detector[detID].pixelmask_shared_min, global->detector[detID].pix_nx, global->detector[detID].pix_ny);
			// /entry_1/instrument_1/detector_i/thumbnail
			d.thumbnail = create2DStack("thumbnail", d.self, global->detector[detID].pix_nx/CXI::thumbnailScale, global->detector[detID].pix_ny/CXI::thumbnailScale, H5T_STD_I16LE);
			// /entry_1/instrument_1/detector_i/experiment_identifier -> /entry_1/experiment_identifier
			H5Lcreate_soft("/entry_1/experiment_identifier",d.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);
		}
		cxi->entry.instrument.detectors.push_back(d);

		cxi->entry.sample.geometry.translation = 0;
		/* Assembled images */
		if(global->saveAssembled){
			// /entry_1/image_i
			sprintf(imageName,"/entry_1/image_%ld",detID+1);
			CXI::Image img;
			img.self = H5Gcreate(cxi->entry.self, imageName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
			// /entry_1/image_i/data
			img.data = create2DStack("data", img.self, global->detector[detID].image_nx, global->detector[detID].image_ny, H5T_NATIVE_FLOAT);
			if(global->savePixelmask){
				// /entry_1/image_i/mask
				img.mask = create2DStack("mask", img.self, global->detector[detID].image_nx, global->detector[detID].image_ny, H5T_NATIVE_UINT16);
			}
			// /entry_1/image_i/mask_shared
			uint16_t *image_pixelmask_shared = (uint16_t*) calloc(global->detector[detID].image_nn,sizeof(uint16_t));
			assemble2Dmask(image_pixelmask_shared, global->detector[detID].pixelmask_shared, global->detector[detID].pix_x, global->detector[detID].pix_y, global->detector[detID].pix_nn, global->detector[detID].image_nx, global->detector[detID].image_nn, global->assembleInterpolation);
			createAndWriteDataset("mask_shared", img.self, image_pixelmask_shared, global->detector[detID].image_nx, global->detector[detID].image_ny);
			// /entry_1/image_i/detector_1
			H5Lcreate_soft(detectorPath,img.self,"detector_1",H5P_DEFAULT,H5P_DEFAULT);
			// /entry_1/image_i/source_1
			H5Lcreate_soft("/entry_1/instrument_1/source_1",img.self,"source_1",H5P_DEFAULT,H5P_DEFAULT);
			// /entry_1/image_i/data_type
			img.dataType = createStringStack("data_type",img.self);
			// /entry_1/image_i/data_space
			img.dataSpace = createStringStack("data_space",img.self);
			// /entry_1/image_i/thumbnail
			img.thumbnail = create2DStack("thumbnail", img.self, global->detector[detID].image_nx/CXI::thumbnailScale, global->detector[detID].image_nx/CXI::thumbnailScale, H5T_NATIVE_FLOAT);
			// /entry_1/image_i/experiment_identifier
			H5Lcreate_soft("/entry_1/experiment_identifier",img.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);
			cxi->entry.images.push_back(img);

			// If we have sample translation configured, write it out to file
			if(global->samplePosXPV[0] || global->samplePosYPV[0] || 
			   global->samplePosZPV[0]){
				cxi->entry.sample.self = H5Gcreate(cxi->entry.self, "sample_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
				cxi->entry.sample.geometry.self = H5Gcreate(cxi->entry.sample.self, "geometry_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				cxi->entry.sample.geometry.translation = create1DStack("translation", cxi->entry.sample.geometry.self, 3, H5T_NATIVE_FLOAT);				
			}
			

			if(global->detector[detID].downsampling > 1){
				// /entry_1/image_j
				sprintf(imageName,"/entry_1/image_%ld",global->nDetectors+detID+1);
				CXI::Image imgXxX;
				imgXxX.self = H5Gcreate(cxi->entry.self, imageName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				// /entry_1/image_j/data
				imgXxX.data = create2DStack("data", imgXxX.self, global->detector[detID].imageXxX_nx, global->detector[detID].imageXxX_ny, H5T_NATIVE_FLOAT);
				if(global->savePixelmask){
					// /entry_1/image_j/mask
					imgXxX.mask = create2DStack("mask", imgXxX.self, global->detector[detID].imageXxX_nx, global->detector[detID].imageXxX_ny, H5T_NATIVE_UINT16);
				}
				// /entry_1/image_j/mask_shared
				uint16_t *imageXxX_pixelmask_shared = (uint16_t*) calloc(global->detector[detID].imageXxX_nn,sizeof(uint16_t));
				if(global->detector[detID].downsamplingConservative==1){
					downsampleMaskConservative(image_pixelmask_shared,imageXxX_pixelmask_shared,global->detector[detID].image_nn,global->detector[detID].image_nx,global->detector[detID].imageXxX_nn,global->detector[detID].imageXxX_nx,global->detector[detID].downsampling);
				} else {
					downsampleMaskNonConservative(image_pixelmask_shared,imageXxX_pixelmask_shared,global->detector[detID].image_nn,global->detector[detID].image_nx,global->detector[detID].imageXxX_nn,global->detector[detID].imageXxX_nx,global->detector[detID].downsampling);
				}
				createAndWriteDataset("mask_shared", imgXxX.self, imageXxX_pixelmask_shared, global->detector[detID].imageXxX_nx, global->detector[detID].imageXxX_ny);
				// /entry_1/image_j/detector_1
				H5Lcreate_soft(detectorPath,imgXxX.self,"detector_1",H5P_DEFAULT,H5P_DEFAULT);
				// /entry_1/image_j/source_1
				H5Lcreate_soft("/entry_1/instrument_1/source_1",imgXxX.self,"source_1",H5P_DEFAULT,H5P_DEFAULT);
				// /entry_1/image_j/data_type
				imgXxX.dataType = createStringStack("data_type",imgXxX.self);
				// /entry_1/image_j/data_space
				imgXxX.dataSpace = createStringStack("data_space",imgXxX.self);
				// /entry_1/image_j/thumbnail
				imgXxX.thumbnail = create2DStack("thumbnail", imgXxX.self, global->detector[detID].imageXxX_nx/CXI::thumbnailScale, global->detector[detID].imageXxX_ny/CXI::thumbnailScale, H5T_NATIVE_FLOAT);
				// /entry_1/image_j/experiment_identifier
				H5Lcreate_soft("/entry_1/experiment_identifier",imgXxX.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);
				cxi->entry.images.push_back(imgXxX);
				free(imageXxX_pixelmask_shared);
			}
			free(image_pixelmask_shared);

      
		}
	}

	cxi->lcls.self = H5Gcreate(cxi->self, "LCLS", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	cxi->lcls.machineTime = createScalarStack("machineTime", cxi->lcls.self, H5T_NATIVE_INT32);
	cxi->lcls.fiducial = createScalarStack("fiducial", cxi->lcls.self, H5T_NATIVE_INT32);
	cxi->lcls.ebeamCharge = createScalarStack("ebeamCharge", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.ebeamL3Energy = createScalarStack("ebeamL3Energy", cxi->lcls.self, H5T_NATIVE_DOUBLE);
	cxi->lcls.ebeamPkCurrBC2 = createScalarStack("ebeamPkCurrBC2", cxi->lcls.self, H5T_NATIVE_DOUBLE);
	cxi->lcls.ebeamLTUPosX = createScalarStack("ebeamLTUPosX", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.ebeamLTUPosY = createScalarStack("ebeamLTUPosY", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.ebeamLTUAngX = createScalarStack("ebeamLTUAngX", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.ebeamLTUAngY = createScalarStack("ebeamLTUAngY", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.phaseCavityTime1 = createScalarStack("phaseCavityTime1", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.phaseCavityTime2 = createScalarStack("phaseCavityTime2", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.phaseCavityCharge1 = createScalarStack("phaseCavityCharge1", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.phaseCavityCharge2 = createScalarStack("phaseCavityCharge2", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.photon_energy_eV = createScalarStack("photon_energy_eV", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.photon_wavelength_A = createScalarStack("photon_wavelength_A", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.f_11_ENRC = createScalarStack("f_11_ENRC", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.f_12_ENRC = createScalarStack("f_12_ENRC", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.f_21_ENRC = createScalarStack("f_21_ENRC", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.f_22_ENRC = createScalarStack("f_22_ENRC", cxi->lcls.self,H5T_NATIVE_DOUBLE);
	cxi->lcls.evr41 = createScalarStack("evr41", cxi->lcls.self,H5T_NATIVE_INT);
	cxi->lcls.eventTimeString = createStringStack("eventTimeString", cxi->lcls.self);
	if(global->TOFPresent){
		cxi->lcls.tofTime = create2DStack("tofTime", cxi->lcls.self, 1, global->AcqNumSamples, H5T_NATIVE_DOUBLE);
		cxi->lcls.tofVoltage = create2DStack("tofVoltage", cxi->lcls.self, 1, global->AcqNumSamples, H5T_NATIVE_DOUBLE);
	}
	H5Lcreate_soft("/LCLS/eventTimeString", cxi->self, "/LCLS/eventTime",H5P_DEFAULT,H5P_DEFAULT);
	H5Lcreate_soft("/entry_1/experiment_identifier",cxi->lcls.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);

	DETECTOR_LOOP{
		char buffer[1024];
		sprintf(buffer,"detector%li-position", detID +1);
		cxi->lcls.detector_positions.push_back(createScalarStack(buffer, cxi->lcls.self,H5T_NATIVE_DOUBLE));
		sprintf(buffer,"detector%li-EncoderValue", detID +1);
		cxi->lcls.detector_EncoderValues.push_back(createScalarStack(buffer, cxi->lcls.self,H5T_NATIVE_DOUBLE));
	}

	// Save cheetah variables  
	cxi->cheetahVal.self = H5Gcreate(cxi->self, "cheetah", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	cxi->cheetahVal.unsharedVal.self = H5Gcreate(cxi->cheetahVal.self, "unshared", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/* For some reason the swmr version of hdf5 can't cope with string stacks larger than 255 characters */
	cxi->cheetahVal.unsharedVal.eventName = createStringStack("eventName", cxi->cheetahVal.unsharedVal.self,255);
	cxi->cheetahVal.unsharedVal.frameNumber = createScalarStack("frameNumber", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_LONG);
	cxi->cheetahVal.unsharedVal.frameNumberIncludingSkipped = createScalarStack("frameNumberIncludingSkipped", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_LONG);
	cxi->cheetahVal.unsharedVal.threadID = createScalarStack("threadID", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_LONG);
	cxi->cheetahVal.unsharedVal.gmd1 = createScalarStack("gmd1", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_DOUBLE);
	cxi->cheetahVal.unsharedVal.gmd2 = createScalarStack("gmd2", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_DOUBLE);
	cxi->cheetahVal.unsharedVal.energySpectrumExist = createScalarStack("energySpectrumExist", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_INT);
	cxi->cheetahVal.unsharedVal.nPeaks = createScalarStack("nPeaks", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_INT);  
	cxi->cheetahVal.unsharedVal.peakNpix = createScalarStack("peakNpix", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_FLOAT);  
	cxi->cheetahVal.unsharedVal.peakTotal = createScalarStack("peakTotal", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_FLOAT);  
	cxi->cheetahVal.unsharedVal.peakResolution = createScalarStack("peakResolution", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_FLOAT);  
	cxi->cheetahVal.unsharedVal.peakResolutionA = createScalarStack("peakResolutionA", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_FLOAT);  
	cxi->cheetahVal.unsharedVal.peakDensity = createScalarStack("peakDensity", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_FLOAT);  
	cxi->cheetahVal.unsharedVal.laserEventCodeOn = createScalarStack("laserEventCodeOn", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_INT);
	cxi->cheetahVal.unsharedVal.laserDelay = createScalarStack("laserDelay", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_DOUBLE);
	cxi->cheetahVal.unsharedVal.hit = createScalarStack("hit", cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_INT);
	DETECTOR_LOOP{
		char buffer[1024];
		sprintf(buffer,"detector%li-sum", detID +1);
		cxi->cheetahVal.unsharedVal.sums.push_back(createScalarStack(buffer, cxi->cheetahVal.unsharedVal.self,H5T_NATIVE_FLOAT));
	}



	cxi->cheetahVal.sharedVal.self = H5Gcreate(cxi->cheetahVal.self, "shared", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	cxi->cheetahVal.sharedVal.hit = createScalarStack("hit", cxi->cheetahVal.sharedVal.self,H5T_NATIVE_INT);
	cxi->cheetahVal.sharedVal.nPeaks = createScalarStack("nPeaks", cxi->cheetahVal.sharedVal.self,H5T_NATIVE_INT);

	CXI::ConfValues confVal;
	cxi->cheetahVal.confVal.self = H5Gcreate(cxi->cheetahVal.self, "configuration", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	confVal = cxi->cheetahVal.confVal;

	DETECTOR_LOOP{
		char buffer[1024];
		sprintf(buffer,"detector%ld_lastBgUpdate",detID);
		cxi->cheetahVal.sharedVal.lastBgUpdate[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_nHot",detID);
		cxi->cheetahVal.sharedVal.nHot[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_lastHotPixUpdate",detID);
		cxi->cheetahVal.sharedVal.lastHotPixUpdate[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_hotPixCounter",detID);
		cxi->cheetahVal.sharedVal.hotPixCounter[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_nHalo",detID);
		cxi->cheetahVal.sharedVal.nHalo[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_lastHaloPixUpdate",detID);
		cxi->cheetahVal.sharedVal.lastHaloPixUpdate[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_haloPixCounter",detID);
		cxi->cheetahVal.sharedVal.haloPixCounter[detID] = createScalarStack(buffer, cxi->cheetahVal.sharedVal.self,H5T_NATIVE_LONG);
		sprintf(buffer,"detector%ld_detectorName",detID);
		createAndWriteDataset(buffer,confVal.self,global->detector[detID].detectorName,MAX_FILENAME_LENGTH);
		sprintf(buffer,"detector%ld_geometryFile",detID);
		createAndWriteDataset(buffer,confVal.self,global->detector[detID].geometryFile,MAX_FILENAME_LENGTH);
		sprintf(buffer,"detector%ld_darkcalFile",detID);
		createAndWriteDataset(buffer,confVal.self,global->detector[detID].darkcalFile,MAX_FILENAME_LENGTH);
		sprintf(buffer,"detector%ld_gaincalFile",detID);
		createAndWriteDataset(buffer,confVal.self,global->detector[detID].gaincalFile,MAX_FILENAME_LENGTH);
		sprintf(buffer,"detector%ld_badpixelFile",detID);
		createAndWriteDataset(buffer,confVal.self,global->detector[detID].badpixelFile,MAX_FILENAME_LENGTH);
		sprintf(buffer,"detector%ld_wireMaskFile",detID);
		createAndWriteDataset(buffer,confVal.self,global->detector[detID].wireMaskFile,MAX_FILENAME_LENGTH);
		sprintf(buffer,"detector%ld_pixelSize",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].pixelSize);
		sprintf(buffer,"detector%ld_cmModule",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].cmModule);
		sprintf(buffer,"detector%ld_cspadSubtractUnbondedPixels",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].cspadSubtractUnbondedPixels);
		sprintf(buffer,"detector%ld_cspadSubtractBehindWires",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].cspadSubtractBehindWires);
		sprintf(buffer,"detector%ld_invertGain",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].invertGain);
		sprintf(buffer,"detector%ld_saveDetectorCorrectedOnly",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].saveDetectorCorrectedOnly);
		sprintf(buffer,"detector%ld_saveDetectorRaw",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].saveDetectorRaw);
		sprintf(buffer,"detector%ld_useAutoHotpixel",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].useAutoHotpixel);
		sprintf(buffer,"detector%ld_maskSaturatedPixels",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].maskSaturatedPixels);
		sprintf(buffer,"detector%ld_pixelSaturationADC",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].pixelSaturationADC);
		sprintf(buffer,"detector%ld_useSubtractPersistentBackground",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].useSubtractPersistentBackground);
		sprintf(buffer,"detector%ld_useBackgroundBufferMutex",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].useBackgroundBufferMutex);
		sprintf(buffer,"detector%ld_useLocalBackgroundSubtraction",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].useLocalBackgroundSubtraction);
		sprintf(buffer,"detector%ld_cmFloor",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].cmFloor);
		sprintf(buffer,"detector%ld_hotpixFreq",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].hotpixFreq);
		sprintf(buffer,"detector%ld_hotpixADC",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].hotpixADC);
		sprintf(buffer,"detector%ld_hotpixMemory",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].hotpixMemory);
		sprintf(buffer,"detector%ld_bgMemory",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].bgMemory);
		sprintf(buffer,"detector%ld_bgRecalc",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].bgRecalc);
		sprintf(buffer,"detector%ld_bgMedian",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].bgMedian);
		sprintf(buffer,"detector%ld_bgIncludeHits",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].bgIncludeHits);
		sprintf(buffer,"detector%ld_bgNoBeamReset",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].bgNoBeamReset);
		sprintf(buffer,"detector%ld_scaleBackground",detID);
		createAndWriteDataset(buffer,confVal.self,&global->detector[detID].scaleBackground);
		//sprintf(buffer,"detector%ld_X",detID);
		//createAndWriteDataset(buffer,confVal.self,&global->detector[detID].X);
	}

	createAndWriteDataset("defaultPhotonEnergyeV",confVal.self,&global->defaultPhotonEnergyeV);
	createAndWriteDataset("startAtFrame",confVal.self,&global->startAtFrame);
	createAndWriteDataset("stopAtFrame",confVal.self,&global->stopAtFrame);
	createAndWriteDataset("nThreads",confVal.self,&global->nThreads);
	createAndWriteDataset("useHelperThreads",confVal.self,&global->useHelperThreads);
	createAndWriteDataset("ioSpeedTest",confVal.self,&global->ioSpeedTest);
	createAndWriteDataset("threadPurge",confVal.self,&global->threadPurge);
	createAndWriteDataset("peaksearchFile",confVal.self,global->peaksearchFile,MAX_FILENAME_LENGTH);
	createAndWriteDataset("generateDarkcal",confVal.self,&global->generateDarkcal);
	createAndWriteDataset("generateGaincal",confVal.self,&global->generateGaincal);
	createAndWriteDataset("hitfinder",confVal.self,&global->hitfinder);
	createAndWriteDataset("savehits",confVal.self,&global->savehits);
	createAndWriteDataset("saveRaw",confVal.self,&global->saveRaw);
	createAndWriteDataset("saveAssembled",confVal.self,&global->saveAssembled);
	createAndWriteDataset("assembleInterpolation",confVal.self,&global->assembleInterpolation);
	createAndWriteDataset("hdf5dump",confVal.self,&global->hdf5dump);
	createAndWriteDataset("saveInterval",confVal.self,&global->saveInterval);
	createAndWriteDataset("savePixelmask",confVal.self,&global->savePixelmask);
	createAndWriteDataset("tofName",confVal.self,global->tofName);
	createAndWriteDataset("TOFchannel",confVal.self,&global->TOFchannel);
	createAndWriteDataset("hitfinderUseTOF",confVal.self,&global->hitfinderUseTOF);
	createAndWriteDataset("hitfinderTOFMinSample",confVal.self,&global->hitfinderTOFMinSample[0], global->hitfinderTOFMinSample.size());
	createAndWriteDataset("hitfinderTOFMaxSample",confVal.self,&global->hitfinderTOFMaxSample[0], global->hitfinderTOFMaxSample.size());
	createAndWriteDataset("hitfinderTOFMeanBackground",confVal.self,&global->hitfinderTOFMeanBackground[0], global->hitfinderTOFMeanBackground.size());
	createAndWriteDataset("hitfinderTOFThresh",confVal.self,&global->hitfinderTOFThresh[0], global->hitfinderTOFThresh.size());
	createAndWriteDataset("saveRadialStacks",confVal.self,&global->saveRadialStacks);
	createAndWriteDataset("radialStackSize",confVal.self,&global->radialStackSize);
	createAndWriteDataset("espectrum1D",confVal.self,&global->espectrum1D);
	createAndWriteDataset("espectrumTiltAng",confVal.self,&global->espectrumTiltAng);
	createAndWriteDataset("espectrumLength",confVal.self,&global->espectrumLength);
	createAndWriteDataset("espectrumSpreadeV",confVal.self,&global->espectrumSpreadeV);
	createAndWriteDataset("espectrumDarkSubtract",confVal.self,&global->espectrumDarkSubtract);
	createAndWriteDataset("espectrumDarkFile",confVal.self,global->espectrumDarkFile,MAX_FILENAME_LENGTH);
	createAndWriteDataset("espectrumScaleFile",confVal.self,global->espectrumScaleFile,MAX_FILENAME_LENGTH);
	createAndWriteDataset("debugLevel",confVal.self,&global->debugLevel);
	createAndWriteDataset("powderthresh",confVal.self,&global->powderthresh);
	createAndWriteDataset("powderSumHits",confVal.self,&global->powderSumHits);
	createAndWriteDataset("powderSumBlanks",confVal.self,&global->powderSumBlanks);
	createAndWriteDataset("hitfinderADC",confVal.self,&global->hitfinderADC);
	createAndWriteDataset("hitfinderCheckGradient",confVal.self,&global->hitfinderCheckGradient);
	createAndWriteDataset("hitfinderMinGradient",confVal.self,&global->hitfinderMinGradient);
	createAndWriteDataset("hitfinderCluster",confVal.self,&global->hitfinderCluster);
	createAndWriteDataset("hitfinderNpeaks",confVal.self,&global->hitfinderNpeaks);
	createAndWriteDataset("hitfinderAlgorithm",confVal.self,&global->hitfinderAlgorithm);
	createAndWriteDataset("hitfinderMinPixCount",confVal.self,&global->hitfinderMinPixCount);
	createAndWriteDataset("hitfinderMaxPixCount",confVal.self,&global->hitfinderMaxPixCount);
	createAndWriteDataset("hitfinderMinPeakSeparation",confVal.self,&global->hitfinderMinPeakSeparation);
	createAndWriteDataset("hitfinderSubtractLocalBG",confVal.self,&global->hitfinderSubtractLocalBG);
	createAndWriteDataset("hitfinderLocalBGRadius",confVal.self,&global->hitfinderLocalBGRadius);
	createAndWriteDataset("hitfinderLocalBGThickness",confVal.self,&global->hitfinderLocalBGThickness);
	createAndWriteDataset("hitfinderMinRes",confVal.self,&global->hitfinderMinRes);
	createAndWriteDataset("hitfinderMaxRes",confVal.self,&global->hitfinderMaxRes);
	createAndWriteDataset("hitfinderResolutionUnitPixel",confVal.self,&global->hitfinderResolutionUnitPixel);
	createAndWriteDataset("hitfinderMinSNR",confVal.self,&global->hitfinderMinSNR);
	createAndWriteDataset("saveCXI",confVal.self,&global->saveCXI);

	DETECTOR_LOOP{
		POWDER_LOOP{
			cPixelDetectorCommon * detector = &global->detector[detID];
			long pix_nx =  detector->pix_nx;
			long pix_ny =  detector->pix_ny;
			long imageXxX_nx = detector->imageXxX_nx;
			long imageXxX_ny = detector->imageXxX_ny;
			long image_nx = detector->image_nx;
			long image_ny = detector->image_ny;
			long radial_nn = detector->radial_nn;

			char buffer[1024];
			double * dummy = NULL;
			sprintf(buffer,"detector%li_class%li_mean_raw",detID,powID);
			createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,pix_nx,pix_ny);
			sprintf(buffer,"detector%li_class%li_mean_raw_radial",detID,powID);
			createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,radial_nn);
			sprintf(buffer,"detector%li_class%li_sigma_raw",detID,powID);
			createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,pix_nx,pix_ny);

			sprintf(buffer,"detector%li_class%li_mean_corrected",detID,powID);
			createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,pix_nx,pix_ny);
			sprintf(buffer,"detector%li_class%li_mean_corrected_radial",detID,powID);
			createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,radial_nn);
			sprintf(buffer,"detector%li_class%li_sigma_corrected",detID,powID);
			createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,pix_nx,pix_ny);
			if(global->assemblePowders && global->assemble2DImage) {
				sprintf(buffer,"detector%li_class%li_mean_assembled",detID,powID);
				createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,image_nx,image_ny);
				sprintf(buffer,"detector%li_class%li_sigma_assembled",detID,powID);
				createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,image_nx,image_ny);
			}
			if(global->assemblePowders && (global->detector[detID].downsampling > 1)){
				sprintf(buffer,"detector%li_class%li_mean_downsampled",detID,powID);
				createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,imageXxX_nx,imageXxX_ny);
				sprintf(buffer,"detector%li_class%li_sigma_downsampled",detID,powID);
				createDataset(buffer, cxi->cheetahVal.sharedVal.self,dummy,imageXxX_nx,imageXxX_ny);
			}
		}
	}

#if defined H5F_ACC_SWMR_READ
  
	int nobjs = H5Fget_obj_count( cxi->self, H5F_OBJ_DATASET | H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR);
	hid_t * obj_id_list = (hid_t *)malloc(sizeof(hid_t)*nobjs);
	H5Fget_obj_ids(cxi->self,H5F_OBJ_DATASET | H5F_OBJ_GROUP | H5F_OBJ_DATATYPE | H5F_OBJ_ATTR, nobjs, obj_id_list);
	for(int i = 0; i < nobjs;i++){
		hid_t id = obj_id_list[i];
		H5I_type_t type = H5Iget_type(id);
		if(type == H5I_DATASET){
			H5Dclose(id);
		}else if(type == H5I_GROUP){
			H5Gclose(id);
		}else if(type == H5I_DATATYPE){
			H5Tclose(id);
		}else if(type == H5I_DATASPACE){
			H5Sclose(id);
		}else if(type == H5I_ATTR){
			H5Aclose(id);
		}    
	}  
	free(obj_id_list);
	if(H5Fstart_swmr_write(cxi->self) < 0){
		ERROR("Cannot change to SWMR mode.\n");
	}

	/* Painfully reopen datasets.
	   This part of the code is a bit
	   too horrible to be true.
	*/

	cxi->entry.experimentIdentifier = H5Dopen(cxi->self,"/entry_1/experiment_identifier",H5P_DEFAULT);
	cxi->entry.instrument.source.energy = H5Dopen(cxi->self,"/entry_1/instrument_1/source_1/energy",H5P_DEFAULT);
	if(global->samplePosXPV[0] || global->samplePosYPV[0] || 
	   global->samplePosZPV[0]){		
		cxi->entry.sample.geometry.translation = H5Dopen(cxi->self,"/entry_1/sample_1/geometry_1/translation",H5P_DEFAULT);
	}
	int cxi_img_id = 0;
	DETECTOR_LOOP{
		char detectorPath[1024];
		sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/distance",detID+1);
		cxi->entry.instrument.detectors[detID].distance = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);
		sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/description",detID+1);
		cxi->entry.instrument.detectors[detID].description = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);
		sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/x_pixel_size",detID+1);
		cxi->entry.instrument.detectors[detID].xPixelSize = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);
		sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/y_pixel_size",detID+1);
		cxi->entry.instrument.detectors[detID].yPixelSize = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);
		if(global->saveRaw){
			sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/data",detID+1);
			cxi->entry.instrument.detectors[detID].data = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);
			if(global->savePixelmask){
				sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/mask",detID+1);
				cxi->entry.instrument.detectors[detID].mask = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);
			}
			sprintf(detectorPath,"/entry_1/instrument_1/detector_%ld/thumbnail",detID+1);
			cxi->entry.instrument.detectors[detID].thumbnail = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
		}
		if(global->saveAssembled){
			sprintf(detectorPath,"/entry_1/image_%ld/data",detID+1);
			cxi->entry.images[cxi_img_id].data = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
			if(global->savePixelmask){
				sprintf(detectorPath,"/entry_1/image_%ld/mask",detID+1);
				cxi->entry.images[cxi_img_id].mask = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
			}
			sprintf(detectorPath,"/entry_1/image_%ld/data_type",detID+1);
			cxi->entry.images[cxi_img_id].dataType = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
			sprintf(detectorPath,"/entry_1/image_%ld/data_space",detID+1);
			cxi->entry.images[cxi_img_id].dataSpace = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
			sprintf(detectorPath,"/entry_1/image_%ld/thumbnail",detID+1);
			cxi->entry.images[cxi_img_id].thumbnail = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
			cxi_img_id++;
			if(global->detector[detID].downsampling > 1){
				sprintf(detectorPath,"/entry_1/image_%ld/data",global->nDetectors+detID+1);
				cxi->entry.images[cxi_img_id].data = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
				if(global->savePixelmask){
					sprintf(detectorPath,"/entry_1/image_%ld/mask",global->nDetectors+detID+1);
					cxi->entry.images[cxi_img_id].mask = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
				}
				sprintf(detectorPath,"/entry_1/image_%ld/data_type",global->nDetectors+detID+1);
				cxi->entry.images[cxi_img_id].dataType = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
				sprintf(detectorPath,"/entry_1/image_%ld/data_space",global->nDetectors+detID+1);
				cxi->entry.images[cxi_img_id].dataSpace = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      
				sprintf(detectorPath,"/entry_1/image_%ld/thumbnail",global->nDetectors+detID+1);
				cxi->entry.images[cxi_img_id].thumbnail = H5Dopen(cxi->self,detectorPath,H5P_DEFAULT);      	
				cxi_img_id++;
			}
		}
	}
	cxi->lcls.machineTime = H5Dopen(cxi->self,"/LCLS/machineTime",H5P_DEFAULT);
	cxi->lcls.fiducial = H5Dopen(cxi->self,"/LCLS/fiducial",H5P_DEFAULT);
	cxi->lcls.ebeamCharge = H5Dopen(cxi->self,"/LCLS/ebeamCharge",H5P_DEFAULT);
	cxi->lcls.ebeamL3Energy = H5Dopen(cxi->self,"/LCLS/ebeamL3Energy",H5P_DEFAULT);
	cxi->lcls.ebeamPkCurrBC2 = H5Dopen(cxi->self,"/LCLS/ebeamPkCurrBC2",H5P_DEFAULT);
	cxi->lcls.ebeamLTUPosX = H5Dopen(cxi->self,"/LCLS/ebeamLTUPosX",H5P_DEFAULT);
	cxi->lcls.ebeamLTUPosY = H5Dopen(cxi->self,"/LCLS/ebeamLTUPosY",H5P_DEFAULT);
	cxi->lcls.ebeamLTUAngX = H5Dopen(cxi->self,"/LCLS/ebeamLTUAngX",H5P_DEFAULT);
	cxi->lcls.ebeamLTUAngY = H5Dopen(cxi->self,"/LCLS/ebeamLTUAngY",H5P_DEFAULT);
	cxi->lcls.phaseCavityTime1 = H5Dopen(cxi->self,"/LCLS/phaseCavityTime1",H5P_DEFAULT);
	cxi->lcls.phaseCavityTime2 = H5Dopen(cxi->self,"/LCLS/phaseCavityTime2",H5P_DEFAULT);
	cxi->lcls.phaseCavityCharge1 = H5Dopen(cxi->self,"/LCLS/phaseCavityCharge1",H5P_DEFAULT);
	cxi->lcls.phaseCavityCharge2 = H5Dopen(cxi->self,"/LCLS/phaseCavityCharge2",H5P_DEFAULT);
	cxi->lcls.photon_energy_eV = H5Dopen(cxi->self,"/LCLS/photon_energy_eV",H5P_DEFAULT);
	cxi->lcls.photon_wavelength_A = H5Dopen(cxi->self,"/LCLS/photon_wavelength_A",H5P_DEFAULT);
	cxi->lcls.f_11_ENRC = H5Dopen(cxi->self,"/LCLS/f_11_ENRC",H5P_DEFAULT);
	cxi->lcls.f_12_ENRC = H5Dopen(cxi->self,"/LCLS/f_12_ENRC",H5P_DEFAULT);
	cxi->lcls.f_21_ENRC = H5Dopen(cxi->self,"/LCLS/f_21_ENRC",H5P_DEFAULT);
	cxi->lcls.f_22_ENRC = H5Dopen(cxi->self,"/LCLS/f_22_ENRC",H5P_DEFAULT);
	cxi->lcls.evr41 = H5Dopen(cxi->self,"/LCLS/evr41",H5P_DEFAULT);
	cxi->lcls.eventTimeString = H5Dopen(cxi->self,"/LCLS/eventTimeString",H5P_DEFAULT);
	if(global->TOFPresent){
		cxi->lcls.tofTime = H5Dopen(cxi->self,"/LCLS/tofTime",H5P_DEFAULT);
		cxi->lcls.tofVoltage = H5Dopen(cxi->self,"/LCLS/tofVoltage",H5P_DEFAULT);    
	}
	DETECTOR_LOOP{
		char buffer[1024];
		sprintf(buffer,"/LCLS/detector%li-position",detID+1);    
		cxi->lcls.detector_positions[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
		sprintf(buffer,"/LCLS/detector%li-EncoderValue",detID+1);    
		cxi->lcls.detector_EncoderValues[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
	}

	cxi->cheetahVal.unsharedVal.eventName = H5Dopen(cxi->self,"/cheetah/unshared/eventName",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.frameNumber = H5Dopen(cxi->self,"/cheetah/unshared/frameNumber",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.frameNumberIncludingSkipped = H5Dopen(cxi->self,"/cheetah/unshared/frameNumberIncludingSkipped",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.threadID = H5Dopen(cxi->self,"/cheetah/unshared/threadID",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.gmd1 = H5Dopen(cxi->self,"/cheetah/unshared/gmd1",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.gmd2 = H5Dopen(cxi->self,"/cheetah/unshared/gmd2",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.energySpectrumExist = H5Dopen(cxi->self,"/cheetah/unshared/energySpectrumExist",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.nPeaks = H5Dopen(cxi->self,"/cheetah/unshared/nPeaks",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.peakNpix = H5Dopen(cxi->self,"/cheetah/unshared/peakNpix",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.peakTotal = H5Dopen(cxi->self,"/cheetah/unshared/peakTotal",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.peakResolution = H5Dopen(cxi->self,"/cheetah/unshared/peakResolution",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.peakResolutionA = H5Dopen(cxi->self,"/cheetah/unshared/peakResolutionA",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.peakDensity = H5Dopen(cxi->self,"/cheetah/unshared/peakDensity",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.laserEventCodeOn = H5Dopen(cxi->self,"/cheetah/unshared/laserEventCodeOn",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.laserDelay = H5Dopen(cxi->self,"/cheetah/unshared/laserDelay",H5P_DEFAULT);
	cxi->cheetahVal.unsharedVal.hit = H5Dopen(cxi->self,"/cheetah/unshared/hit",H5P_DEFAULT);

	DETECTOR_LOOP{
		char buffer[1024];
		sprintf(buffer,"/cheetah/unshared/detector%li-sum",detID+1);    
		cxi->cheetahVal.unsharedVal.sums[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
	}

	cxi->cheetahVal.sharedVal.hit = H5Dopen(cxi->self,"/cheetah/shared/hit",H5P_DEFAULT);
	cxi->cheetahVal.sharedVal.nPeaks = H5Dopen(cxi->self,"/cheetah/shared/nPeaks",H5P_DEFAULT);

	DETECTOR_LOOP{
		char buffer[1024];
		sprintf(buffer,"/cheetah/shared/detector%ld_lastBgUpdate",detID);    
		cxi->cheetahVal.sharedVal.lastBgUpdate[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
		sprintf(buffer,"/cheetah/shared/detector%ld_nHot",detID);    
		cxi->cheetahVal.sharedVal.nHot[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
		sprintf(buffer,"/cheetah/shared/detector%ld_lastHotPixUpdate",detID);    
		cxi->cheetahVal.sharedVal.lastHotPixUpdate[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
		sprintf(buffer,"/cheetah/shared/detector%ld_hotPixCounter",detID);    
		cxi->cheetahVal.sharedVal.hotPixCounter[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
		sprintf(buffer,"/cheetah/shared/detector%ld_nHalo",detID);    
		cxi->cheetahVal.sharedVal.nHalo[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT);
		sprintf(buffer,"/cheetah/shared/detector%ld_lastHaloPixUpdate",detID);    
		cxi->cheetahVal.sharedVal.lastHaloPixUpdate[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT); 
		sprintf(buffer,"/cheetah/shared/detector%ld_haloPixCounter",detID);    
		cxi->cheetahVal.sharedVal.haloPixCounter[detID] = H5Dopen(cxi->self,buffer,H5P_DEFAULT); 
	}

	cxi->cheetahVal.sharedVal.self = H5Gopen(cxi->self,"/cheetah/shared",H5P_DEFAULT);
#endif
	return cxi;
}

static std::vector<std::string> openFilenames = std::vector<std::string>();
static std::vector<CXI::File* > openFiles = std::vector<CXI::File *>();

static CXI::File * getCXIFileByName(cGlobal *global){
	char * filename = global->cxiFilename;
	pthread_mutex_lock(&global->framefp_mutex);
	/* search again to be sure */
	for(uint i = 0;i<openFilenames.size();i++){
		if(openFilenames[i] == std::string(filename)){
			pthread_mutex_unlock(&global->framefp_mutex);
			return openFiles[i];
		}
	}
	openFilenames.push_back(filename);
	CXI::File * cxi = createCXISkeleton(filename,global);
	openFiles.push_back(cxi);
	pthread_mutex_unlock(&global->framefp_mutex);
	return cxi;
}

void writeAccumulatedCXI(cGlobal * global){
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_lock(&global->swmr_mutex);
#endif
	CXI::File * cxi = getCXIFileByName(global);
	CXI::SharedValues sharedVal = cxi->cheetahVal.sharedVal;

	cPixelDetectorCommon *detector;
	long	radial_nn;
	long	pix_nn,pix_nx,pix_ny;
	long	image_nn,image_nx,image_ny;
	long	imageXxX_nn,imageXxX_nx,imageXxX_ny;

	DETECTOR_LOOP{
		POWDER_LOOP{
			detector = &global->detector[detID];
			radial_nn = detector->radial_nn;
			pix_nn =  detector->pix_nn;
			pix_nx =  detector->pix_nx;
			pix_ny =  detector->pix_ny;
			image_nn = detector->image_nn;
			image_nx = detector->image_nx;
			image_ny = detector->image_ny;
			imageXxX_nn = detector->imageXxX_nn;
			imageXxX_nx = detector->imageXxX_nx;
			imageXxX_ny = detector->imageXxX_ny;
			char buffer[1024];

			// Dereference/create arrays to be written to file
			// SUM(data)
			// SUMSQ(data*data)
			// MEAN(data)
			// SIGMA(data) = sqrt(SUM(data*data)-SUM(data)*SUM(data)) / N
			// ANG, ANGCNT: radial projection

			// raw
			double * sum_raw = detector->powderRaw[powID];
			double * sum_rawSq = detector->powderRawSquared[powID];     
			double * mean_raw = (double*) calloc(pix_nn, sizeof(double));
			double * sigma_raw = (double *) calloc(pix_nn,sizeof(double));
			for(long i = 0; i<pix_nn; i++){
				mean_raw[i] = sum_raw[i]/(1.*detector->nPowderFrames[powID]);
				sigma_raw[i] =
					sqrt( fabs(sum_rawSq[i] - sum_raw[i]*sum_raw[i]/(1.*detector->nPowderFrames[powID])) / (1.*detector->nPowderFrames[powID]) );
			}
			double * mean_raw_radial = (double*) calloc(radial_nn, sizeof(double));
			double * mean_raw_angCnt = (double*) calloc(radial_nn, sizeof(double));
			calculateRadialAverage(mean_raw,mean_raw_radial,mean_raw_angCnt,global,detID);
			sprintf(buffer,"detector%li_class%li_mean_raw",detID,powID);
			openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,mean_raw,pix_nx,pix_ny);
			sprintf(buffer,"detector%li_class%li_mean_raw_radial",detID,powID);
			openAndWriteDataset(buffer, sharedVal.self,mean_raw_radial,radial_nn);
			sprintf(buffer,"detector%li_class%li_sigma_raw",detID,powID);
			openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,sigma_raw,pix_nx,pix_ny);

			// corrected
			double * sum_corrected = detector->powderCorrected[powID];
			double * sum_correctedSq = detector->powderCorrectedSquared[powID];
			double * mean_corrected = (double*) calloc(pix_nn, sizeof(double));
			double * sigma_corrected = (double *) calloc(pix_nn,sizeof(double));
			for(long i = 0; i<pix_nn; i++){
				mean_corrected[i] = sum_corrected[i]/(1.*detector->nPowderFrames[powID]);
				sigma_corrected[i] =
					sqrt( fabs(sum_correctedSq[i] - sum_corrected[i]*sum_corrected[i]/(1.*detector->nPowderFrames[powID])) / (1.*detector->nPowderFrames[powID]) );
			}      
			double * mean_corrected_radial = (double*) calloc(radial_nn, sizeof(double));
			double * mean_corrected_angCnt = (double*) calloc(radial_nn, sizeof(double));
			calculateRadialAverage(mean_corrected,mean_corrected_radial,mean_corrected_angCnt,global,detID);
			sprintf(buffer,"detector%li_class%li_mean_corrected",detID,powID);
			openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,mean_corrected,pix_nx,pix_ny);
			sprintf(buffer,"detector%li_class%li_mean_corrected_radial",detID,powID);
			openAndWriteDataset(buffer, sharedVal.self,mean_corrected_radial,radial_nn);
			sprintf(buffer,"detector%li_class%li_sigma_corrected",detID,powID);
			openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,sigma_corrected,pix_nx,pix_ny);

			// assembled
			double * sum_assembled = detector->powderAssembled[powID];
			double * sum_assembledSq = detector->powderAssembledSquared[powID];
			double * mean_assembled = (double*) calloc(image_nn, sizeof(double));
			double * sigma_assembled = (double*) calloc(image_nn, sizeof(double));
			if(global->assemblePowders && global->assemble2DImage) {
				for(long i = 0; i<image_nn; i++){
					mean_assembled[i] = sum_assembled[i]/(1.*detector->nPowderFrames[powID]);
					sigma_assembled[i] =
						sqrt( fabs(sum_assembledSq[i] - sum_assembled[i]*sum_assembled[i]/(1.*detector->nPowderFrames[powID])) / (1.*detector->nPowderFrames[powID]) );
				}
				sprintf(buffer,"detector%li_class%li_mean_assembled",detID,powID);
				openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,mean_assembled,image_nx,image_ny);
				sprintf(buffer,"detector%li_class%li_sigma_assembled",detID,powID);
				openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,sigma_assembled,image_nx,image_ny);
			}

			// downsampled
			double * sum_downsampled = detector->powderDownsampled[powID];
			double * sum_downsampledSq = detector->powderDownsampledSquared[powID];
			double * mean_downsampled = (double*) calloc(imageXxX_nn, sizeof(double));
			double * sigma_downsampled = (double*) calloc(imageXxX_nn, sizeof(double));
			if(global->assemblePowders && (global->detector[detID].downsampling > 1)){
				for(long i = 0; i<imageXxX_nn; i++){
					mean_downsampled[i] = sum_downsampled[i]/(1.*detector->nPowderFrames[powID]);
					sigma_downsampled[i] =
						sqrt( fabs(sum_downsampledSq[i] - sum_downsampled[i]*sum_downsampled[i]/(1.*detector->nPowderFrames[powID])) / (1.*detector->nPowderFrames[powID]) );
				}
				sprintf(buffer,"detector%li_class%li_mean_downsampled",detID,powID);
				openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,mean_downsampled,imageXxX_nx,imageXxX_ny);
				sprintf(buffer,"detector%li_class%li_sigma_downsampled",detID,powID);
				openAndWriteDataset(buffer, cxi->cheetahVal.sharedVal.self,sigma_downsampled,imageXxX_nx,imageXxX_ny);
			}

			free(mean_corrected_radial);
			free(mean_corrected_angCnt);
			free(mean_raw_radial);
			free(mean_raw_angCnt);
			free(mean_raw);
			free(mean_corrected);
			free(sigma_raw);
			free(sigma_corrected);
			if(global->assemblePowders){
				if(global->assemble2DImage) {
					free(mean_assembled);
					free(sigma_assembled);
				}
				if(global->detector[detID].downsampling > 1){
					free(mean_downsampled);
					free(sigma_downsampled);
				}
			}
		}      
	}
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_unlock(&global->swmr_mutex);
#endif
}

static void  closeCXI(CXI::File * cxi){
	hid_t ids[256];
	int n_ids = H5Fget_obj_ids(cxi->self, H5F_OBJ_DATASET, 256, ids);
	for (int i=0; i<n_ids; i++){
		//H5I_type_t type;
		hsize_t block[3];
		hsize_t mdims[3];
		hid_t attr_id;
		hid_t dataspace = H5Dget_space (ids[i]);
		int size;
		if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
		H5Sget_simple_extent_dims(dataspace, block, mdims);
		if(mdims[0] == H5S_UNLIMITED){
			attr_id = H5Aopen_name(ids[i],CXI::ATTR_NAME_NUM_EVENTS);
			H5Aread(attr_id,H5T_NATIVE_INT32,&size); 
			H5Aclose(attr_id);
			block[0] = size;
			H5Dset_extent(ids[i], block);
		}
	}
	H5Fflush(cxi->self, H5F_SCOPE_GLOBAL);
	H5Fclose(cxi->self);
	delete cxi;
}

void closeCXIFiles(cGlobal * global){
#if H5_VERS_MAJOR == 1 && H5_VERS_MINOR == 8 && H5_VERS_RELEASE < 9
#warning "HDF5 < 1.8.9 contains a bug which makes it impossible to shrink certain datasets.\n"
#warning "Please update your HDF5 to get properly truncated output files.\n"
	fprintf(stderr,"HDF5 < 1.8.9 contains a bug which makes it impossible to shrink certain datasets.\n");
	fprintf(stderr,"Please update your HDF5 to get properly truncated output files.\n");
#else
	pthread_mutex_lock(&global->framefp_mutex);
	/* Go through each file and resize them to their right size */
	for(uint i = 0;i<openFilenames.size();i++){
		closeCXI(openFiles[i]);    
	}
	openFiles.clear();
	openFilenames.clear();
	pthread_mutex_unlock(&global->framefp_mutex);
#endif
	H5close();
}

void writeCXIHitstats(cEventData *info, cGlobal *global ){
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_lock(&global->swmr_mutex);
#endif
	/* Get the existing CXI file or open a new one */
	CXI::File * cxi = getCXIFileByName(global);

	writeScalarToStack(cxi->cheetahVal.sharedVal.hit,global->nCXIEvents,info->hit);
	writeScalarToStack(cxi->cheetahVal.sharedVal.nPeaks,global->nCXIEvents,info->nPeaks);
	global->nCXIEvents += 1;
#ifdef H5F_ACC_SWMR_WRITE  
	pthread_mutex_unlock(&global->swmr_mutex);
#endif
}


void writeCXI(cEventData *info, cGlobal *global ){
#ifdef H5F_ACC_SWMR_WRITE
	bool didDecreaseActive = false;
	pthread_mutex_lock(&global->nActiveThreads_mutex);
	if (global->nActiveThreads) {
		global->nActiveThreads--;
		didDecreaseActive = true;
	}
	pthread_mutex_unlock(&global->nActiveThreads_mutex);
	pthread_mutex_lock(&global->swmr_mutex);
#endif
	/* Get the existing CXI file or open a new one */
	CXI::File * cxi = getCXIFileByName(global);

	uint stackSlice = getStackSlice(cxi);
	info->stackSlice = stackSlice;

	global->nCXIHits += 1;
	double en = info->photonEnergyeV * 1.60217646e-19;
	writeScalarToStack(cxi->entry.instrument.source.energy,stackSlice,en);
	// remove the '.h5' from eventname
	info->eventname[strlen(info->eventname) - 3] = 0;
	writeStringToStack(cxi->entry.experimentIdentifier,stackSlice,info->eventname);
	// put it back
	info->eventname[strlen(info->eventname)] = '.';
  
	if(cxi->entry.sample.geometry.translation){
		
		write1DToStack(cxi->entry.sample.geometry.translation,stackSlice,info->samplePos);
	}
	DETECTOR_LOOP {    
		/* Save assembled image under image groups */
		writeScalarToStack(cxi->entry.instrument.detectors[detID].distance,stackSlice,global->detector[detID].detectorZ/1000.0);
		writeScalarToStack(cxi->entry.instrument.detectors[detID].xPixelSize,stackSlice,global->detector[detID].pixelSize);
		writeScalarToStack(cxi->entry.instrument.detectors[detID].yPixelSize,stackSlice,global->detector[detID].pixelSize);

		long imgID = detID;
		if (global->detector[detID].downsampling > 1){
			imgID = detID * 2;
		}
		char buffer[1024];
		sprintf(buffer,"%s [%s]",global->detector[detID].detectorType,global->detector[detID].detectorName);
		writeStringToStack(cxi->entry.instrument.detectors[detID].description,stackSlice,buffer);
		if(global->saveAssembled){
			if (cxi->entry.images[imgID].data<0) {ERROR("No valid dataset.");}
			write2DToStack(cxi->entry.images[imgID].data,stackSlice,info->detector[detID].image);
			if(global->savePixelmask){
				if (cxi->entry.images[imgID].mask<0) {ERROR("No valid dataset.");}
				write2DToStack(cxi->entry.images[imgID].mask,stackSlice,info->detector[detID].image_pixelmask);
			}
			float * thumbnail = generateThumbnail(info->detector[detID].image,global->detector[detID].image_nx,global->detector[detID].image_ny,CXI::thumbnailScale);
			if (cxi->entry.images[imgID].thumbnail<0){ERROR("No valid dataset.");}
			write2DToStack(cxi->entry.images[imgID].thumbnail,stackSlice,thumbnail);
			writeStringToStack(cxi->entry.images[imgID].dataType,stackSlice,"intensities");
			writeStringToStack(cxi->entry.images[imgID].dataSpace,stackSlice,"diffraction");
			delete [] thumbnail;      
			if (global->detector[detID].downsampling > 1){
				imgID = detID * 2 + 1;
				if (cxi->entry.images[imgID].data<0) {ERROR("No valid dataset.");}
				write2DToStack(cxi->entry.images[imgID].data,stackSlice,info->detector[detID].imageXxX);
				if(global->savePixelmask){
					if (cxi->entry.images[imgID].mask<0) {ERROR("No valid dataset.");}
					write2DToStack(cxi->entry.images[imgID].mask,stackSlice,info->detector[detID].imageXxX_pixelmask);
				}
				float * thumbnail = generateThumbnail(info->detector[detID].imageXxX,global->detector[detID].imageXxX_nx,global->detector[detID].imageXxX_ny,CXI::thumbnailScale);
				if (cxi->entry.images[imgID].thumbnail<0){ERROR("No valid dataset.");}
				write2DToStack(cxi->entry.images[imgID].thumbnail,stackSlice,thumbnail);
				writeStringToStack(cxi->entry.images[imgID].dataType,stackSlice,"intensities");
				writeStringToStack(cxi->entry.images[imgID].dataSpace,stackSlice,"diffraction");
				delete [] thumbnail;
			}
		}
		if(global->saveRaw){
			if (cxi->entry.instrument.detectors[detID].data<0){ERROR("No valid dataset.");}
			int16_t* corrected_data_int16 = (int16_t*) calloc(global->detector[detID].pix_nn,sizeof(int16_t));
			for(long i=0;i<global->detector[detID].pix_nn;i++){
				corrected_data_int16[i] = (int16_t) lrint(info->detector[detID].corrected_data[i]);
			}
			write2DToStack(cxi->entry.instrument.detectors[detID].data,stackSlice,corrected_data_int16);
			if(global->savePixelmask){
				write2DToStack(cxi->entry.instrument.detectors[detID].mask,stackSlice,info->detector[detID].pixelmask);
			}
			int16_t * thumbnail = generateThumbnail(corrected_data_int16,global->detector[detID].pix_nx,global->detector[detID].pix_ny,CXI::thumbnailScale);
			write2DToStack(cxi->entry.instrument.detectors[detID].thumbnail,stackSlice,thumbnail);
			delete [] thumbnail;
			free(corrected_data_int16);
		}
	}
	/*Write LCLS informations*/
	DETECTOR_LOOP{
		writeScalarToStack(cxi->lcls.detector_positions[detID],stackSlice,global->detector[detID].detectorZ);
		writeScalarToStack(cxi->lcls.detector_EncoderValues[detID],stackSlice,detID);
	}
	writeScalarToStack(cxi->lcls.machineTime,stackSlice,info->seconds);
	writeScalarToStack(cxi->lcls.fiducial,stackSlice,info->fiducial);
	writeScalarToStack(cxi->lcls.ebeamCharge,stackSlice,info->fEbeamCharge);
	writeScalarToStack(cxi->lcls.ebeamL3Energy,stackSlice,info->fEbeamL3Energy);
	writeScalarToStack(cxi->lcls.ebeamLTUAngX,stackSlice,info->fEbeamLTUAngX);
	writeScalarToStack(cxi->lcls.ebeamLTUAngY,stackSlice,info->fEbeamLTUAngY);
	writeScalarToStack(cxi->lcls.ebeamLTUPosX,stackSlice,info->fEbeamLTUPosX);
	writeScalarToStack(cxi->lcls.ebeamLTUPosY,stackSlice,info->fEbeamLTUPosY);
	writeScalarToStack(cxi->lcls.ebeamPkCurrBC2,stackSlice,info->fEbeamPkCurrBC2);
	writeScalarToStack(cxi->lcls.phaseCavityTime1,stackSlice,info->phaseCavityTime1);
	writeScalarToStack(cxi->lcls.phaseCavityTime2,stackSlice,info->phaseCavityTime2);
	writeScalarToStack(cxi->lcls.phaseCavityCharge1,stackSlice,info->phaseCavityCharge1);
	writeScalarToStack(cxi->lcls.phaseCavityCharge2,stackSlice,info->phaseCavityCharge2);
	writeScalarToStack(cxi->lcls.photon_energy_eV,stackSlice,info->photonEnergyeV);
	writeScalarToStack(cxi->lcls.photon_wavelength_A,stackSlice,info->wavelengthA);
	writeScalarToStack(cxi->lcls.f_11_ENRC,stackSlice,info->gmd11);
	writeScalarToStack(cxi->lcls.f_12_ENRC,stackSlice,info->gmd12);
	writeScalarToStack(cxi->lcls.f_21_ENRC,stackSlice,info->gmd21);
	writeScalarToStack(cxi->lcls.f_22_ENRC,stackSlice,info->gmd22);
	if(info->TOFPresent){
		write2DToStack(cxi->lcls.tofVoltage, stackSlice, info->TOFVoltage);
		write2DToStack(cxi->lcls.tofTime, stackSlice, info->TOFTime);
	}
	int LaserOnVal = (info->laserEventCodeOn)?1:0;
	writeScalarToStack(cxi->lcls.evr41,stackSlice,LaserOnVal);
	char timestr[26];
	time_t eventTime = info->seconds;
	ctime_r(&eventTime,timestr);
	writeStringToStack(cxi->lcls.eventTimeString,stackSlice,timestr);

	writeStringToStack(cxi->cheetahVal.unsharedVal.eventName,stackSlice,info->eventname);
	writeScalarToStack(cxi->cheetahVal.unsharedVal.frameNumber,stackSlice,info->frameNumber);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.frameNumberIncludingSkipped,stackSlice,info->frameNumberIncludingSkipped);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.threadID,stackSlice,info->threadNum);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.gmd1,stackSlice,info->gmd1);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.gmd2,stackSlice,info->gmd2);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.energySpectrumExist,stackSlice,info->energySpectrumExist);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.nPeaks,stackSlice,info->nPeaks);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.peakNpix,stackSlice,info->peakNpix);  

	writeScalarToStack(cxi->cheetahVal.unsharedVal.peakTotal,stackSlice,info->peakTotal);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.peakResolution,stackSlice,info->peakResolution);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.peakResolutionA,stackSlice,info->peakResolutionA);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.peakDensity,stackSlice,info->peakDensity);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.laserEventCodeOn,stackSlice,info->laserEventCodeOn);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.laserDelay,stackSlice,info->laserDelay);  
	writeScalarToStack(cxi->cheetahVal.unsharedVal.hit,stackSlice,info->hit);
  
    
	DETECTOR_LOOP{
		writeScalarToStack(cxi->cheetahVal.sharedVal.lastBgUpdate[detID],stackSlice,global->detector[detID].bgLastUpdate);  
		writeScalarToStack(cxi->cheetahVal.sharedVal.nHot[detID],stackSlice,global->detector[detID].nhot);  
		writeScalarToStack(cxi->cheetahVal.sharedVal.lastHotPixUpdate[detID],stackSlice,global->detector[detID].hotpixLastUpdate);  
		writeScalarToStack(cxi->cheetahVal.sharedVal.hotPixCounter[detID],stackSlice,global->detector[detID].hotpixCounter);  
		writeScalarToStack(cxi->cheetahVal.sharedVal.nHalo[detID],stackSlice,global->detector[detID].nhalo);  
		writeScalarToStack(cxi->cheetahVal.sharedVal.lastHaloPixUpdate[detID],stackSlice,global->detector[detID].halopixLastUpdate);  
		writeScalarToStack(cxi->cheetahVal.sharedVal.haloPixCounter[detID],stackSlice,global->detector[detID].halopixCounter);  
		writeScalarToStack(cxi->cheetahVal.unsharedVal.sums[detID],stackSlice,info->detector[detID].sum);  
	}
#ifdef H5F_ACC_SWMR_WRITE  
	if(global->cxiFlushPeriod && (stackSlice % global->cxiFlushPeriod) == 0){
		H5Fflush(cxi->self,H5F_SCOPE_LOCAL);
	}

	if (didDecreaseActive) {
		pthread_mutex_lock(&global->nActiveThreads_mutex);
		global->nActiveThreads++;
		pthread_mutex_unlock(&global->nActiveThreads_mutex);
	}
	pthread_mutex_unlock(&global->swmr_mutex);
#endif
}


