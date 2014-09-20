
/*
 *  saveCXI.cpp
 *  cheetah
 *
 *  Create by Jing Liu on 05/11/12.
 *  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
 *
 */
										
#include <string>
#include <vector>
#include <pthread.h>
#include <math.h>
#include <fstream> 

#include <saveCXI.h>

namespace CXI{
	Node * Node::createDataset(const char * s, hid_t dataType, hsize_t width, hsize_t height,
							   hsize_t length, hsize_t stackSize, int chunkSize, int heightChunkSize, const char * userAxis){
		hid_t loc = hid();

		if(dataType == H5T_NATIVE_CHAR){
			dataType = H5Tcopy(H5T_C_S1);
			if(H5Tset_size(dataType, width) < 0){
				ERROR("Cannot set type size.\n");
			}
			width = height;
			height = 0;
		}

		// right shift the dimensions
		hsize_t * dimsP[4] = {&width, &height, &length, &stackSize};
		for (int i=2; i>=0; i--){
			for (int j=i+1; j<4; j++){
				if(*dimsP[j]==0){
					*dimsP[j]=*dimsP[j-1];
					*dimsP[j-1]=0;
				}
			}
		}
		int ndims = 0;
		for (int i=0; i<4; i++){
			if (*dimsP[i]>0){
				ndims++;
			}
		}
		

		/*
		if(stackSize == 0){
			stackSize = length;
			length = 0;
		}


		if(height == 0){
			height = width;
			width = 0;
		}


		if(length == 0){
			length = height;
			height = 0;

		}

		if(height == 0){
			height = width;
			width = 0;
		}

		if(stackSize == 0){
			stackSize = length;
			length = 0;
		}

		

		int ndims = 1;		
		if(width > 0){
			ndims++;
		}
		if(height > 0){
			ndims++;
		}
		
		*/

		if(stackSize == H5S_UNLIMITED && chunkSize <= 0){
			chunkSize = CXI::chunkSize1D;
			if(ndims == 3){
				chunkSize = CXI::chunkSize2D;
			} else if(ndims == 4){
				chunkSize = CXI::chunkSize2D;
			}
		}

		if(length == 0){
			length = 1;
		}
		if(height == 0){
			height = 1;
		}
		if(width == 0){
			width = 1;
		}

		hsize_t dims[4] = {0, length, height, width};


		if(heightChunkSize == 0){
			dims[0] = lrintf(((float)chunkSize)/H5Tget_size(dataType)/height/length);
		}else{
			dims[0] = chunkSize/heightChunkSize;
			dims[1] = lrintf(((float)heightChunkSize)/H5Tget_size(dataType)/height);
		}

		if(!chunkSize){
			if(stackSize == 0){
				stackSize = 1;
			}
			dims[0] = stackSize;
		}else{
			if(dims[0] == 0){
				// Make sure the chunk is not 0
				dims[0] = 1;
			}
		}
		hsize_t maxdims[4] = {stackSize,length,height,width};
		hid_t dataspace = H5Screate_simple(ndims, dims, maxdims);
		if( dataspace<0 ) {ERROR("Cannot create dataspace.\n");}
		hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
		if(chunkSize){
			H5Pset_chunk(cparms, ndims, dims);
		}
		//  H5Pset_deflate (cparms, 2);
		hid_t dataset = H5Dcreate(loc, s, dataType, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
		if( dataset<0 ) {ERROR("Cannot create dataset.\n");}
		H5Sclose(dataspace);
		H5Pclose(cparms);		

		if((ndims == 3 || ndims == 4) && chunkSize){
			H5Pset_chunk_cache(H5Dget_access_plist(dataset),H5D_CHUNK_CACHE_NSLOTS_DEFAULT,1024*1024*16,1);
		}

		if(stackSize == H5S_UNLIMITED){
			addStackAttributes(dataset,ndims,userAxis);
		}
		return addNode(s, dataset, Dataset);    
	}

	template <class T> 
	void Node::write(T * data, int stackSlice, int sliceSize){  
		bool sliced = true;
		bool variableSlice = false;
		if(stackSlice == -1){
			stackSlice = 0;
			sliced = false;
		}
		if(sliceSize > 0){
			variableSlice = true;
		}

		hid_t hs,w;
		hsize_t count[4] = {1,1,1,1};
		hsize_t offset[4] = {stackSlice,0,0,0};
		/* stride is irrelevant in this case */
		hsize_t stride[4] = {1,1,1,1};
		hsize_t block[4];
		/* dummy */
		hsize_t mdims[4];
		hid_t dataset = hid();
		/* Use the existing dimensions as block size */
		hid_t dataspace = H5Dget_space(dataset);
		if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
		int ndims = H5Sget_simple_extent_ndims(dataspace);
		H5Sget_simple_extent_dims(dataspace, block, mdims);
		/* check if we need to extend the dataset */
		if(ndims > 0 && (int)block[0] <= stackSlice){
			while((int)block[0] <= stackSlice){
				block[0] *= 2;
			}
			H5Dset_extent (dataset, block);
			/* get enlarged dataspace */
			H5Sclose(dataspace);
			dataspace = H5Dget_space (dataset);
			if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
		}
		if(sliced){
			block[0] = 1;
		}

		/* check if we need to extend the dataset in the second dimension */
		if(variableSlice && (int)block[1] <= sliceSize){
			int tmp_block = block[0];
			H5Sget_simple_extent_dims(dataspace, block, mdims);
			while((int)block[1] <= sliceSize){
				block[1] *= 2;
			}
			H5Dset_extent (dataset, block);
			block[0] = tmp_block;
			/* get enlarged dataspace */
			H5Sclose(dataspace);
			dataspace = H5Dget_space (dataset);
			if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
		}
		if(variableSlice){
			block[1] = sliceSize;
		}

		hid_t memspace = H5Screate_simple (ndims, block, NULL);
		hid_t type = get_datatype(data);
		if(type == H5T_NATIVE_CHAR){
			type = H5Dget_type(dataset);
		}
		if (sliced){
			hs = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
			if( hs<0 ) {
				ERROR("Cannot select hyperslab.\n");
			}
		}
		w = H5Dwrite (dataset, type, memspace, dataspace, H5P_DEFAULT, data);
		if( w<0 ){
			ERROR("Cannot write to file.\n");
		}
		if(sliced){
			writeNumEvents(dataset,stackSlice);
		}
		H5Sclose(memspace);
		H5Sclose(dataspace);
	}

	Node * Node::addClass(const char * s){
		std::string key = nextKey(s);
		return createGroup(key.c_str());
	}

	Node * Node::createGroup(const char * s){
		hid_t gid = H5Gcreate(hid(),s, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if(gid < 0){
			return NULL;
		}
		return addNode(s,gid, Group);
	}

	Node * Node::createGroup(const char * prefix, int n){
		char buffer[1024];
		sprintf(buffer,"%s_%d",prefix,n);
		hid_t gid = H5Gcreate(hid(),buffer, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		if(gid < 0){
			return NULL;
		}
		return addNode(buffer,gid, Group);
	}

	Node * Node::addClassLink(const char * s, std::string target){
		std::string key = nextKey(s);
		return createLink(key.c_str(), target);
	}

	Node * Node::createLink(const char * s, std::string target){
		hid_t lid = H5Lcreate_soft(target.c_str(),hid(),s,H5P_DEFAULT,H5P_DEFAULT);
		if(lid < 0){
			return NULL;
		}
		return addNode(s, lid, Link);
	}


	void Node::closeAll(){
		// close all non-root open objects
		if(parent && hid() >= 0 && type != Link){
			H5Oclose(hid());
			id = -1;
		}
		for(Iter it = children.begin(); it != children.end(); it++) {
			it->second->closeAll();
		}
	}

	void Node::openAll(){
		if(parent && parent->hid() < 0){
			ERROR("Parent not open");
		}
		// open all non-root closed objects
		if(hid() < 0 && parent && type != Link){
			id = H5Oopen(parent->hid(),name.c_str(),H5P_DEFAULT);
		}
		for(Iter it = children.begin(); it != children.end(); it++) {
			it->second->openAll();
		}
	}

	std::string Node::path(){
		if(parent){
			return parent->path()+std::string("/")+name;
		}else{
			return name;
		}
	}

	Node & Node::child(std::string prefix, int n){
		char buffer[1024];
		sprintf(buffer,"%s_%d",prefix.c_str(),n);
		return (*this)[buffer];
	}

	void Node::trimAll(int stackSize){
		if(stackSize < 0){
			stackSize = stackCounter;
		}

		if(hid() >= 0 && type == Dataset){
			hsize_t block[3];
			hsize_t mdims[3];
			hid_t dataspace = H5Dget_space(hid());
			if( dataspace<0 ) {ERROR("Cannot get dataspace.\n");}
			H5Sget_simple_extent_dims(dataspace, block, mdims);
			if(mdims[0] == H5S_UNLIMITED){
				writeNumEvents(hid(), stackSize);
				block[0] = stackSize;
				H5Dset_extent(hid(), block);
			}
		}

		for(Iter it = children.begin(); it != children.end(); it++) {
			it->second->trimAll(stackSize);
		}
	}


	Node * Node::addNode(const char * s, hid_t oid, Type t){
		Node * n = new Node(s, oid, this, t);
		children[s] = n;
		return n;
	}

	std::string Node::nextKey(const char * s){			
		int i = 1;
		char buffer[1024];			
		for(;;i++){
			sprintf(buffer,"%s_%d",s,i);
			if(children.find(buffer) == children.end()){
				break;
			}
		}
		return std::string(buffer);
	}

	void Node::addStackAttributes(hid_t dataset, int ndims, const char * userAxis){
		const char * axis_1d = "experiment_identifier";
		const char * axis_2d = "experiment_identifier:coordinate";
		const char * axis_3d = "experiment_identifier:y:x";
		const char * axis_4d = "experiment_identifier:module_identifier:y:x";
		const char * axis;
		if(userAxis != NULL){
			axis = userAxis;
		}else if(ndims == 1){
			axis = axis_1d;
		}else if(ndims == 2){
			axis = axis_2d;
		}else if(ndims == 3){
			axis = axis_3d;
		}else if(ndims == 4){
			axis = axis_4d;
	    }
		
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
	}

	hid_t Node::writeNumEvents(hid_t dataset, int stackSlice){
		hid_t a = H5Aopen(dataset, CXI::ATTR_NAME_NUM_EVENTS, H5P_DEFAULT);
		hid_t w = -1;
		if(a>=0) {
			w = H5Awrite (a, H5T_NATIVE_INT32, &stackSlice);
			H5Aclose(a);
		}
		return w;
	}

	template <class T>
	hid_t Node::get_datatype(const T * foo){
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

	uint Node::getStackSlice(){
#ifdef __GNUC__
	return __sync_fetch_and_add(&stackCounter,1);
#else
	pthread_mutex_lock(&global->framefp_mutex);
	uint ret = stackCounter;
	cxi->stackCounter++;
	pthread_mutex_unlock(&global->framefp_mutex);
	return ret;
#endif
}

}


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

static CXI::Node * createCXISkeleton(const char * filename,cGlobal *global){
	/* Creates the initial skeleton for the CXI file.
	   We'll rely on HDF5 automatic error reporting. It's usually loud enough.
	*/

	using CXI::Node;

	puts("Creating Skeleton");
	CXI::Node * root = new Node(filename,global->cxiSWMR);

	root->createDataset("cxi_version",H5T_NATIVE_INT,1)->write(&CXI::version);

	Node * entry = root->addClass("entry");
	entry->createStack("experiment_identifier",H5T_NATIVE_CHAR,128);

	Node * instrument = entry->addClass("instrument");
	Node * source = instrument->addClass("source");

	source->createStack("energy",H5T_NATIVE_DOUBLE);
	source->createLink("experiment_identifier", "/entry_1/experiment_identifier");

	// If we have sample translation or electrojet voltage configured, write it out to file
	if(global->samplePosXPV[0] || global->samplePosYPV[0] || global->samplePosZPV[0] || global->sampleVoltage[0]){
		Node * sample = entry->addClass("sample");
		sample->addClass("geometry")->createDataset("translation",H5T_NATIVE_FLOAT,3,0,H5S_UNLIMITED);
		sample->addClass("injection")->createStack("voltage",H5T_NATIVE_FLOAT);
	}
	
	DETECTOR_LOOP{
		Node * detector = instrument->createGroup("detector",detID+1);
		
		entry->addClassLink("data",detector->path().c_str());
		detector->createStack("distance",H5T_NATIVE_DOUBLE);
		detector->createStack("description",H5T_NATIVE_CHAR,128);
		detector->createStack("x_pixel_size",H5T_NATIVE_DOUBLE);
		detector->createStack("y_pixel_size",H5T_NATIVE_DOUBLE);
    
		/* Raw images */
		if(global->saveRaw){
			// /entry_1/instrument_1/detector_i/
			int pix_nx = global->detector[detID].pix_nx;
			int pix_ny = global->detector[detID].pix_ny;

            if (global->saveRawInt16){
			    detector->createStack("data",H5T_STD_I16LE,pix_nx, pix_ny);
            }
            else {
                detector->createStack("data",H5T_NATIVE_FLOAT,pix_nx, pix_ny);
            }
			if(global->savePixelmask){
				detector->createStack("mask",H5T_NATIVE_UINT16,pix_nx, pix_ny);
			}

			detector->createDataset("mask_shared",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(global->detector[detID].pixelmask_shared);
			detector->createDataset("mask_shared_max",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(global->detector[detID].pixelmask_shared_max);
			detector->createDataset("mask_shared_min",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(global->detector[detID].pixelmask_shared_min);
			detector->createStack("thumbnail",H5T_STD_I16LE, pix_nx/CXI::thumbnailScale, pix_ny/CXI::thumbnailScale);

			detector->createLink("experiment_identifier", "/entry_1/experiment_identifier");
		}

		/* Raw images (modular detector) - see Example 3.6 (Figure 6) in CXI file format 1.40 */
		if (global->saveRawModules){
			
			int asic_nx = global->detector[detID].asic_nx;
			int asic_ny = global->detector[detID].asic_ny;
			int nasics_x = global->detector[detID].nasics_x;
			int nasics_y = global->detector[detID].nasics_y;
			int nasics = nasics_x * nasics_y;

			detector->createStack("data",H5T_STD_I16LE, asic_nx, asic_ny, nasics);
		
			//detector->createDataset("mask_shared",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(global->detector[detID].pixelmask_shared);
			//detector->createDataset("mask_shared_max",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(global->detector[detID].pixelmask_shared_max);
			//detector->createDataset("mask_shared_min",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(global->detector[detID].pixelmask_shared_min);
			//detector->createStack("thumbnail",H5T_STD_I16LE, pix_nx/CXI::thumbnailScale, pix_ny/CXI::thumbnailScale);
			detector->createLink("experiment_identifier", "/entry_1/experiment_identifier");

		}	



		/* Assembled images */
		if(global->saveAssembled){
			int image_nx = global->detector[detID].image_nx;
			int image_ny = global->detector[detID].image_ny;
			int i_image = detID+1;
			printf("N = %d\n",(i_image));
			Node * image = entry->createGroup("image",i_image);
			// /entry_1/image_i/
			image->createStack("data",H5T_NATIVE_FLOAT, image_nx, image_ny);
			if(global->savePixelmask){
				image->createStack("mask",H5T_NATIVE_UINT16, image_nx, image_ny);
			}
			uint16_t *image_pixelmask_shared = (uint16_t*) calloc(global->detector[detID].image_nn,sizeof(uint16_t));
			assemble2Dmask(image_pixelmask_shared, global->detector[detID].pixelmask_shared, 
						   global->detector[detID].pix_x, global->detector[detID].pix_y,
						   global->detector[detID].pix_nn, global->detector[detID].image_nx,
						   global->detector[detID].image_nn, global->assembleInterpolation);
			image->createDataset("mask_shared",H5T_NATIVE_UINT16,image_nx, image_ny)->write(image_pixelmask_shared);

			image->addClassLink("detector",detector->path());
			image->addClassLink("source",source->path());
			image->createStack("data_type",H5T_NATIVE_CHAR,128);
			image->createStack("data_space",H5T_NATIVE_CHAR,128);
			image->createStack("thumbnail",H5T_NATIVE_FLOAT, image_nx/CXI::thumbnailScale, image_ny/CXI::thumbnailScale);
			image->createLink("experiment_identifier", "/entry_1/experiment_identifier");

			if(global->detector[detID].downsampling > 1){
				int image_nx = global->detector[detID].imageXxX_nx;
				int image_ny = global->detector[detID].imageXxX_ny;
				int i_image = global->nDetectors+detID+1;
				printf("NXX = %d\n",i_image);
				image = entry->createGroup("image",i_image);
				// /entry_1/image_j/

				image->createStack("data",H5T_NATIVE_FLOAT, image_nx, image_ny);
				if(global->savePixelmask){
					image->createStack("mask",H5T_NATIVE_UINT16, image_nx, image_ny);
				}
				uint16_t *imageXxX_pixelmask_shared = (uint16_t*) calloc(global->detector[detID].imageXxX_nn,sizeof(uint16_t));
				if(global->detector[detID].downsamplingConservative==1){
					downsampleMaskConservative(image_pixelmask_shared,imageXxX_pixelmask_shared,global->detector[detID].image_nn,
											   global->detector[detID].image_nx,global->detector[detID].imageXxX_nn,
											   global->detector[detID].imageXxX_nx,global->detector[detID].downsampling);
				} else {
					downsampleMaskNonConservative(image_pixelmask_shared,imageXxX_pixelmask_shared,global->detector[detID].image_nn,
												  global->detector[detID].image_nx,global->detector[detID].imageXxX_nn,
												  global->detector[detID].imageXxX_nx,global->detector[detID].downsampling);
				}
				image->createDataset("mask_shared",H5T_NATIVE_UINT16,image_nx, image_ny)->write(imageXxX_pixelmask_shared);
				image->addClassLink("detector",detector->path());
				image->addClassLink("source",source->path());
				image->createStack("data_type",H5T_NATIVE_CHAR,128);
				image->createStack("data_space",H5T_NATIVE_CHAR,128);
				image->createStack("thumbnail",H5T_NATIVE_FLOAT, image_nx/CXI::thumbnailScale, image_ny/CXI::thumbnailScale);
				image->createLink("experiment_identifier", "/entry_1/experiment_identifier");
				free(imageXxX_pixelmask_shared);
			}
			free(image_pixelmask_shared);      
		}
	}

	if(global->TOFPresent){
		int tofDetectorIndex = 1;
		for(unsigned int i = 0;i<global->TOFChannelsPerCard.size();i++){
			if(global->TOFChannelsPerCard[i].size() > 0){
				Node * detector = instrument->createGroup("detector",tofDetectorIndex+global->nDetectors);
				detector->createStack("data",H5T_NATIVE_DOUBLE,global->AcqNumSamples,global->TOFChannelsPerCard[i].size());
				detector->createStack("tofTime", H5T_NATIVE_DOUBLE, global->AcqNumSamples,global->TOFChannelsPerCard[i].size());
				char buffer[1024];
				sprintf(buffer,"iTOF detector connected to Acqiris unit %d, channel(s) %d",i,global->TOFChannelsPerCard[i][0]%4);
				for(unsigned int j = 1;j<global->TOFChannelsPerCard[i].size();j++){
					sprintf(buffer+strlen(buffer),", %d",global->TOFChannelsPerCard[i][j]%4);
				}
				detector->createDataset("description",H5T_NATIVE_CHAR,MAX_FILENAME_LENGTH)->write(buffer);
				tofDetectorIndex++;
			}
		}
	}

	int resultIndex = 1;
	if(global->savePeakInfo && global->hitfinder){
		Node * result = entry->createGroup("result",resultIndex);

		result->createStack("nPeaks", H5T_NATIVE_INT);

		result->createStack("peakXPosAssembled", H5T_NATIVE_FLOAT, 0,H5S_UNLIMITED,H5S_UNLIMITED,0,
							CXI::peaksChunkSize[0],CXI::peaksChunkSize[1],"experiment_identifier:nPeaks");
		result->createStack("peakYPosAssembled",H5T_NATIVE_FLOAT, 0,H5S_UNLIMITED,H5S_UNLIMITED,0,
							CXI::peaksChunkSize[0],CXI::peaksChunkSize[1],"experiment_identifier:nPeaks");

		result->createStack("peakXPosRaw", H5T_NATIVE_FLOAT, 0,H5S_UNLIMITED,H5S_UNLIMITED,0,
							CXI::peaksChunkSize[0],CXI::peaksChunkSize[1],"experiment_identifier:nPeaks");
		result->createStack("peakYPosRaw", H5T_NATIVE_FLOAT, 0,H5S_UNLIMITED,H5S_UNLIMITED,0,
							CXI::peaksChunkSize[0],CXI::peaksChunkSize[1],"experiment_identifier:nPeaks");
		
		result->createStack("peakIntensity", H5T_NATIVE_FLOAT, 0,H5S_UNLIMITED,H5S_UNLIMITED,0,
							CXI::peaksChunkSize[0],CXI::peaksChunkSize[1],"experiment_identifier:nPeaks");
		result->createStack("peakNPixels", H5T_NATIVE_FLOAT, 0,H5S_UNLIMITED,H5S_UNLIMITED,0,
							CXI::peaksChunkSize[0],CXI::peaksChunkSize[1],"experiment_identifier:nPeaks");


		result->createLink("data", "peakIntensity");
		resultIndex++;
	}

	Node * lcls = root->createGroup("LCLS");	
	lcls->createStack("machineTime",H5T_NATIVE_INT32);
	lcls->createStack("fiducial",H5T_NATIVE_INT32);
	lcls->createStack("ebeamCharge",H5T_NATIVE_DOUBLE);
	lcls->createStack("ebeamL3Energy",H5T_NATIVE_DOUBLE);
	lcls->createStack("ebeamPkCurrBC2",H5T_NATIVE_DOUBLE);
	lcls->createStack("ebeamLTUPosX",H5T_NATIVE_DOUBLE);
	lcls->createStack("ebeamLTUPosY",H5T_NATIVE_DOUBLE);
	lcls->createStack("ebeamLTUAngX",H5T_NATIVE_DOUBLE);
	lcls->createStack("ebeamLTUAngY",H5T_NATIVE_DOUBLE);
	lcls->createStack("phaseCavityTime1",H5T_NATIVE_DOUBLE);
	lcls->createStack("phaseCavityTime2",H5T_NATIVE_DOUBLE);
	lcls->createStack("phaseCavityCharge1",H5T_NATIVE_DOUBLE);
	lcls->createStack("phaseCavityCharge2",H5T_NATIVE_DOUBLE);
	lcls->createStack("photon_energy_eV",H5T_NATIVE_DOUBLE);
	lcls->createStack("photon_wavelength_A",H5T_NATIVE_DOUBLE);
	lcls->createStack("f_11_ENRC",H5T_NATIVE_DOUBLE);
	lcls->createStack("f_12_ENRC",H5T_NATIVE_DOUBLE);
	lcls->createStack("f_21_ENRC",H5T_NATIVE_DOUBLE);
	lcls->createStack("f_22_ENRC",H5T_NATIVE_DOUBLE);
	lcls->createStack("evr41",H5T_NATIVE_DOUBLE);
	lcls->createStack("eventTimeString",H5T_NATIVE_CHAR,128);
	if(global->TOFPresent){
		lcls->createStack("tofTime",H5T_NATIVE_DOUBLE,1, global->AcqNumSamples);
		lcls->createStack("tofVoltage",H5T_NATIVE_DOUBLE,1, global->AcqNumSamples);
	}
	lcls->createLink("eventTime","eventTimeString");
	lcls->createLink("experiment_identifier","/entry_1/experiment_identifier");

	DETECTOR_LOOP{
		Node * detector = lcls->createGroup("detector",detID+1);
		detector->createStack("position",H5T_NATIVE_DOUBLE);
		detector->createStack("EncoderValue",H5T_NATIVE_DOUBLE);
	}

	// Save cheetah variables  
	Node * cheetah = root->createGroup("cheetah");
	Node * unshared = cheetah->createGroup("unshared");

	/* For some reason the swmr version of hdf5 can't cope with string stacks larger than 255 characters */
	unshared->createStack("eventName",H5T_NATIVE_CHAR,255);
	unshared->createStack("frameNumber",H5T_NATIVE_LONG);
	unshared->createStack("frameNumberIncludingSkipped",H5T_NATIVE_LONG);
	unshared->createStack("threadID",H5T_NATIVE_LONG);
	unshared->createStack("gmd1",H5T_NATIVE_DOUBLE);
	unshared->createStack("gmd2",H5T_NATIVE_DOUBLE);
	unshared->createStack("energySpectrumExist",H5T_NATIVE_INT);
	unshared->createStack("nPeaks",H5T_NATIVE_INT);
    unshared->createStack("nProtons",H5T_NATIVE_INT);
	unshared->createStack("peakNpix",H5T_NATIVE_FLOAT);
	unshared->createStack("peakTotal",H5T_NATIVE_FLOAT);
	unshared->createStack("peakResolution",H5T_NATIVE_FLOAT);
	unshared->createStack("peakResolutionA",H5T_NATIVE_FLOAT);
	unshared->createStack("peakDensity",H5T_NATIVE_FLOAT);
	unshared->createStack("pumpLaserCode",H5T_NATIVE_INT);
	unshared->createStack("pumpLaserDelay",H5T_NATIVE_DOUBLE);
	unshared->createStack("hit",H5T_NATIVE_INT);
	DETECTOR_LOOP{
		Node * detector = unshared->createGroup("detector",detID+1);
		detector->createStack("sum",H5T_NATIVE_FLOAT);
	}



	Node * shared = cheetah->createGroup("shared");
	shared->createStack("hit",H5T_NATIVE_INT);
	shared->createStack("nPeaks",H5T_NATIVE_INT);

	// First read configuration file to memory
	std::ifstream file(global->configFile, std::ios::binary);
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	
	std::vector<char> buffer(size);
	if(!file.read(buffer.data(), size)){
		ERROR("Could not read configuration file");
	}
	// Just write out input configuration file
	
	Node * configuration = cheetah->createGroup("configuration");
	configuration->createDataset("input",H5T_NATIVE_CHAR,size)->write(&(buffer[0]));

	DETECTOR_LOOP{
		Node * detector = shared->createGroup("detector",detID+1);
		detector->createStack("lastBgUpdate",H5T_NATIVE_LONG);
		detector->createStack("nHot",H5T_NATIVE_LONG);
		detector->createStack("lastHotPixUpdate",H5T_NATIVE_LONG);
		detector->createStack("hotPixCounter",H5T_NATIVE_LONG);
		detector->createStack("nHalo",H5T_NATIVE_LONG);
		detector->createStack("lastHaloPixUpdate",H5T_NATIVE_LONG);
		detector->createStack("haloPixCounter",H5T_NATIVE_LONG);

		POWDER_LOOP{
			Node * cl = detector->createGroup("class",powID+1);
			cPixelDetectorCommon * detector = &global->detector[detID];
			long pix_nx =  detector->pix_nx;
			long pix_ny =  detector->pix_ny;
			long radial_nn = detector->radial_nn;

			cl->createDataset("mean_raw",H5T_NATIVE_DOUBLE,pix_nx,pix_ny);
			cl->createDataset("mean_raw_radial",H5T_NATIVE_DOUBLE,radial_nn);
			cl->createDataset("sigma_raw",H5T_NATIVE_DOUBLE,pix_nx,pix_ny);
			cl->createDataset("mean_corrected",H5T_NATIVE_DOUBLE,pix_nx,pix_ny);
			cl->createDataset("mean_corrected_radial",H5T_NATIVE_DOUBLE,radial_nn);
			cl->createDataset("sigma_corrected",H5T_NATIVE_DOUBLE,pix_nx,pix_ny);
			if(global->assemblePowders && global->assemble2DImage) {
				long image_nx = detector->image_nx;
				long image_ny = detector->image_ny;
				cl->createDataset("mean_assembled",H5T_NATIVE_DOUBLE,image_nx,image_ny);
				cl->createDataset("sigma_assembled",H5T_NATIVE_DOUBLE,image_nx,image_ny);
			}
			if(global->assemblePowders && (global->detector[detID].downsampling > 1)){
				long imageXxX_nx = detector->imageXxX_nx;
				long imageXxX_ny = detector->imageXxX_ny;
				cl->createDataset("mean_downsampled",H5T_NATIVE_DOUBLE,imageXxX_nx,imageXxX_ny);
				cl->createDataset("sigma_downsampled",H5T_NATIVE_DOUBLE,imageXxX_nx,imageXxX_ny);
			}
		}
	}


#if defined H5F_ACC_SWMR_READ
	if(global->cxiSWMR){  
		root->closeAll();
		if(H5Fstart_swmr_write(root->hid()) < 0){
			ERROR("Cannot change to SWMR mode.\n");			
		}
		puts("Changed to SWMR mode.");
		root->openAll();
	}
#endif
	return root;
}

static std::vector<std::string> openFilenames = std::vector<std::string>();
static std::vector<CXI::Node* > openFiles = std::vector<CXI::Node *>();

static CXI::Node * getCXIFileByName(cGlobal *global){
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
	CXI::Node * cxi = createCXISkeleton(filename,global);
	openFiles.push_back(cxi);
	pthread_mutex_unlock(&global->framefp_mutex);
	return cxi;
}

void writeAccumulatedCXI(cGlobal * global){
	using CXI::Node;
#ifdef H5F_ACC_SWMR_WRITE  
	if(global->cxiSWMR){
		pthread_mutex_lock(&global->swmr_mutex);
	}
#endif
	CXI::Node * cxi = getCXIFileByName(global);

	cPixelDetectorCommon *detector;
	long	radial_nn;
	long	pix_nn,pix_nx,pix_ny;
	long	image_nn,image_nx,image_ny;
	long	imageXxX_nn,imageXxX_nx,imageXxX_ny;

	DETECTOR_LOOP{
		Node & det_node = (*cxi)["cheetah"]["shared"].child("detector",detID+1);
		POWDER_LOOP{
			Node & cl = det_node.child("class",powID+1);

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
				sigma_raw[i] =	sqrt( fabs(sum_rawSq[i] - sum_raw[i]*sum_raw[i]/(1.*detector->nPowderFrames[powID])) /
									  (1.*detector->nPowderFrames[powID]) );
			}
			double * mean_raw_radial = (double*) calloc(radial_nn, sizeof(double));
			double * mean_raw_angCnt = (double*) calloc(radial_nn, sizeof(double));
			calculateRadialAverage(mean_raw,mean_raw_radial,mean_raw_angCnt,global,detID);
			cl["mean_raw"].write(mean_raw);
			cl["mean_raw_radial"].write(mean_raw_radial);
			cl["sigma_raw"].write(sigma_raw);

			// corrected
			double * sum_corrected = detector->powderCorrected[powID];
			double * sum_correctedSq = detector->powderCorrectedSquared[powID];
			double * mean_corrected = (double*) calloc(pix_nn, sizeof(double));
			double * sigma_corrected = (double *) calloc(pix_nn,sizeof(double));
			for(long i = 0; i<pix_nn; i++){
				mean_corrected[i] = sum_corrected[i]/(1.*detector->nPowderFrames[powID]);
				sigma_corrected[i] = sqrt( fabs(sum_correctedSq[i] - sum_corrected[i]*sum_corrected[i]/(1.*detector->nPowderFrames[powID])) / 
										   (1.*detector->nPowderFrames[powID]) );
			}      
			double * mean_corrected_radial = (double*) calloc(radial_nn, sizeof(double));
			double * mean_corrected_angCnt = (double*) calloc(radial_nn, sizeof(double));
			calculateRadialAverage(mean_corrected,mean_corrected_radial,mean_corrected_angCnt,global,detID);
			cl["mean_corrected"].write(mean_corrected);
			cl["mean_corrected_radial"].write(mean_corrected);
			cl["sigma_corrected"].write(mean_corrected);

			// assembled
			double * sum_assembled = detector->powderAssembled[powID];
			double * sum_assembledSq = detector->powderAssembledSquared[powID];
			if(global->assemblePowders && global->assemble2DImage) {
				double * mean_assembled = (double*) calloc(image_nn, sizeof(double));
				double * sigma_assembled = (double*) calloc(image_nn, sizeof(double));

				for(long i = 0; i<image_nn; i++){
					mean_assembled[i] = sum_assembled[i]/(1.*detector->nPowderFrames[powID]);
					sigma_assembled[i] = sqrt( fabs(sum_assembledSq[i] - sum_assembled[i]*sum_assembled[i]/(1.*detector->nPowderFrames[powID])) / 
											   (1.*detector->nPowderFrames[powID]) );
				}
				cl["mean_assembled"].write(mean_assembled);
				cl["sigma_assembled"].write(sigma_assembled);
			}

			// downsampled
			double * sum_downsampled = detector->powderDownsampled[powID];
			double * sum_downsampledSq = detector->powderDownsampledSquared[powID];
			if(global->assemblePowders && (global->detector[detID].downsampling > 1)){
				double * mean_downsampled = (double*) calloc(imageXxX_nn, sizeof(double));
				double * sigma_downsampled = (double*) calloc(imageXxX_nn, sizeof(double));

				for(long i = 0; i<imageXxX_nn; i++){
					mean_downsampled[i] = sum_downsampled[i]/(1.*detector->nPowderFrames[powID]);
					sigma_downsampled[i] =
						sqrt( fabs(sum_downsampledSq[i] - sum_downsampled[i]*sum_downsampled[i]/(1.*detector->nPowderFrames[powID])) / (1.*detector->nPowderFrames[powID]) );
				}
				cl["mean_downsampled"].write(mean_downsampled);
				cl["sigma_downsampled"].write(sigma_downsampled);
			}

			free(mean_corrected_radial);
			free(mean_corrected_angCnt);
			free(mean_raw_radial);
			free(mean_raw_angCnt);
			free(mean_raw);
			free(mean_corrected);
			free(sigma_raw);
			free(sigma_corrected);
		}      
	}
#ifdef H5F_ACC_SWMR_WRITE  
	if(global->cxiSWMR){
		pthread_mutex_unlock(&global->swmr_mutex);
	}
#endif
}

static void  closeCXI(CXI::Node * cxi){
	cxi->trimAll();
	H5Fflush(cxi->hid(), H5F_SCOPE_GLOBAL);
	H5Fclose(cxi->hid());
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
	if(global->cxiSWMR){
		pthread_mutex_lock(&global->swmr_mutex);
	}
#endif
	/* Get the existing CXI file or open a new one */
	CXI::Node * cxi = getCXIFileByName(global);

	(*cxi)["cheetah"]["shared"]["hit"].write(&info->hit,global->nCXIEvents);
	(*cxi)["cheetah"]["shared"]["nPeaks"].write(&info->nPeaks,global->nCXIEvents);
	global->nCXIEvents += 1;
#ifdef H5F_ACC_SWMR_WRITE  
	if(global->cxiSWMR){
		pthread_mutex_unlock(&global->swmr_mutex);
	}
#endif
}


void writeCXI(cEventData *info, cGlobal *global ){
	using CXI::Node;
#ifdef H5F_ACC_SWMR_WRITE
	bool didDecreaseActive = false;
	if(global->cxiSWMR){
		pthread_mutex_lock(&global->nActiveThreads_mutex);
		if (global->nActiveThreads) {
			global->nActiveThreads--;
v			didDecreaseActive = true;
		}
		pthread_mutex_unlock(&global->nActiveThreads_mutex);
		pthread_mutex_lock(&global->swmr_mutex);
	}
#endif
	/* Get the existing CXI file or open a new one */
	CXI::Node * cxi = getCXIFileByName(global);
	Node & root = *cxi;

	uint stackSlice = cxi->getStackSlice();
	info->stackSlice = stackSlice;

	global->nCXIHits += 1;
	double en = info->photonEnergyeV * 1.60217646e-19;
	root["entry_1"]["instrument_1"]["source_1"]["energy"].write(&en,stackSlice);
	// remove the '.h5' from eventname
	info->eventname[strlen(info->eventname) - 3] = 0;
	root["entry_1"]["experiment_identifier"].write(info->eventname,stackSlice);
	// put it back
	info->eventname[strlen(info->eventname)] = '.';
  
	if(global->samplePosXPV[0] || global->samplePosYPV[0] || global->samplePosZPV[0] || global->sampleVoltage[0]){
		root["entry_1"]["sample_1"]["geometry_1"]["translation"].write(info->samplePos,stackSlice);
		root["entry_1"]["sample_1"]["injection_1"]["voltage"].write(info->sampleVoltage,stackSlice);
	}

	DETECTOR_LOOP {    
		/* Save assembled image under image groups */
		Node & detector = root["entry_1"]["instrument_1"].child("detector",detID+1);
		double tmp = global->detector[detID].detectorZ/1000.0;
		detector["distance"].write(&tmp,stackSlice);
		detector["x_pixel_size"].write(&global->detector[detID].pixelSize,stackSlice);
		detector["y_pixel_size"].write(&global->detector[detID].pixelSize,stackSlice);

		char buffer[1024];
		sprintf(buffer,"%s [%s]",global->detector[detID].detectorType,global->detector[detID].detectorName);
		detector["description"].write(buffer,stackSlice);
		if(global->saveAssembled){
			Node & image = root["entry_1"].child("image",detID+1);

			image["data"].write(info->detector[detID].image,stackSlice);
			if(global->savePixelmask){
				image["mask"].write(info->detector[detID].image_pixelmask,stackSlice);
			}
			float * thumbnail = generateThumbnail(info->detector[detID].image,global->detector[detID].image_nx,global->detector[detID].image_ny,CXI::thumbnailScale);
			image["thumbnail"].write(thumbnail, stackSlice);
			image["data_type"].write("intensities", stackSlice);
			image["data_space"].write("diffraction", stackSlice);
			delete [] thumbnail;      
			if (global->detector[detID].downsampling > 1){
				Node & image = root["entry_1"].child("image",global->nDetectors+detID+1);

				image["data"].write(info->detector[detID].imageXxX,stackSlice);
				if(global->savePixelmask){
					image["mask"].write(info->detector[detID].imageXxX_pixelmask,stackSlice);
				}
				float * thumbnail = generateThumbnail(info->detector[detID].imageXxX,global->detector[detID].imageXxX_nx,global->detector[detID].imageXxX_ny,CXI::thumbnailScale);
				image["thumbnail"].write(thumbnail, stackSlice);
				image["data_type"].write("intensities", stackSlice);
				image["data_space"].write("diffraction", stackSlice);
				delete [] thumbnail;
			}
		}
		if(global->saveRaw){
			int16_t* corrected_data_int16 = (int16_t*) calloc(global->detector[detID].pix_nn,sizeof(int16_t));
			for(long i=0;i<global->detector[detID].pix_nn;i++){
				corrected_data_int16[i] = (int16_t) lrint(info->detector[detID].corrected_data[i]);
			}
            if (global->saveRawInt16){
    			detector["data"].write(corrected_data_int16,stackSlice);
            }
            else { 
                detector["data"].write(info->detector[detID].corrected_data,stackSlice);
            }
			if(global->savePixelmask){
				detector["mask"].write(info->detector[detID].pixelmask,stackSlice);
			}
			int16_t * thumbnail = generateThumbnail(corrected_data_int16,global->detector[detID].pix_nx,global->detector[detID].pix_ny,CXI::thumbnailScale);
			detector["thumbnail"].write(thumbnail, stackSlice);
			delete [] thumbnail;
			free(corrected_data_int16);
		}

		if (global->saveRawModules){

			int asic_nx = global->detector[detID].asic_nx;
			int asic_ny = global->detector[detID].asic_ny;
			int asics_nn = global->detector[detID].asic_nn;
			int nasics_x = global->detector[detID].nasics_x;
			int nasics_y = global->detector[detID].nasics_y;
			
			uint16_t* raw_module = (uint16_t*) calloc(asics_nn*nasics_x*nasics_y, sizeof(uint16_t));
			stackModulesMask(info->detector[detID].raw_data, raw_module, asic_nx, asic_ny, nasics_x, nasics_y);
			detector["data"].write(raw_module, stackSlice);
		}

	}
	int resultIndex = 1;
	if(global->savePeakInfo && global->hitfinder) {
		long nPeaks = info->peaklist.nPeaks;
		
		Node & result = root["entry_1"].child("result",resultIndex);

		result["peakXPosAssembled"].write(info->peaklist.peak_com_x_assembled, stackSlice, nPeaks);
		result["peakYPosAssembled"].write(info->peaklist.peak_com_y_assembled, stackSlice, nPeaks);

		result["peakXPosRaw"].write(info->peaklist.peak_com_x, stackSlice, nPeaks);
		result["peakYPosRaw"].write(info->peaklist.peak_com_y, stackSlice, nPeaks);

		result["peakIntensity"].write(info->peaklist.peak_totalintensity, stackSlice, nPeaks);
		result["peakNPixels"].write(info->peaklist.peak_npix, stackSlice, nPeaks);
		result["nPeaks"].write(&nPeaks, stackSlice);
	}

	/*Write LCLS informations*/
	Node & lcls = root["LCLS"];
	DETECTOR_LOOP{
		lcls.child("detector",detID+1)["position"].write(&global->detector[detID].detectorZ,stackSlice);
		lcls.child("detector",detID+1)["EncoderValue"].write(&global->detector[detID].detectorEncoderValue,stackSlice);
	}
	lcls["machineTime"].write(&info->seconds,stackSlice);
	lcls["fiducial"].write(&info->fiducial,stackSlice);
	lcls["ebeamCharge"].write(&info->fEbeamCharge,stackSlice);
	lcls["ebeamL3Energy"].write(&info->fEbeamL3Energy,stackSlice);
	lcls["ebeamLTUAngX"].write(&info->fEbeamLTUAngX,stackSlice);
	lcls["ebeamLTUAngY"].write(&info->fEbeamLTUAngY,stackSlice);
	lcls["ebeamLTUPosX"].write(&info->fEbeamLTUPosX,stackSlice);
	lcls["ebeamLTUPosY"].write(&info->fEbeamLTUPosY,stackSlice);
	lcls["ebeamPkCurrBC2"].write(&info->fEbeamPkCurrBC2,stackSlice);
	lcls["phaseCavityTime1"].write(&info->phaseCavityTime1,stackSlice);
	lcls["phaseCavityTime2"].write(&info->phaseCavityTime2,stackSlice);
	lcls["phaseCavityCharge1"].write(&info->phaseCavityCharge1,stackSlice);
	lcls["phaseCavityCharge2"].write(&info->phaseCavityCharge2,stackSlice);
	lcls["photon_energy_eV"].write(&info->photonEnergyeV,stackSlice);
	lcls["photon_wavelength_A"].write(&info->wavelengthA,stackSlice);
	lcls["f_11_ENRC"].write(&info->gmd11,stackSlice);
	lcls["f_12_ENRC"].write(&info->gmd12,stackSlice);
	lcls["f_21_ENRC"].write(&info->gmd12,stackSlice);
	lcls["f_22_ENRC"].write(&info->gmd22,stackSlice);
	if(info->TOFPresent){
		lcls["tofVoltage"].write(info->TOFVoltage,stackSlice);
		lcls["tofTime"].write(info->TOFTime,stackSlice);
		
		int tofDetectorIndex = 0;
		for(unsigned int card = 0;card<global->TOFChannelsPerCard.size();card++){
			if(global->TOFChannelsPerCard[card].size() == 0){
				continue;
			}
			int detID = tofDetectorIndex+global->nDetectors;
			Node & detector = root["entry_1"]["instrument_1"].child("detector",detID+1);

			detector["data"].write(info->TOFAllVoltage[card],stackSlice);
			detector["tofTime"].write(info->TOFAllTime[card],stackSlice);
			tofDetectorIndex++;
		}
	}
	int LaserOnVal = (info->pumpLaserCode)?1:0;
	lcls["evr41"].write(&LaserOnVal,stackSlice);
	char timestr[26];
	time_t eventTime = info->seconds;
	ctime_r(&eventTime,timestr);
	lcls["eventTimeString"].write(timestr,stackSlice);

	Node & unshared = root["cheetah"]["unshared"];
	unshared["eventName"].write(info->eventname,stackSlice);
	unshared["frameNumber"].write(&info->frameNumber,stackSlice);
	unshared["frameNumberIncludingSkipped"].write(&info->frameNumberIncludingSkipped,stackSlice);
	unshared["threadID"].write(&info->threadNum,stackSlice);
	unshared["gmd1"].write(&info->gmd1,stackSlice);
	unshared["gmd2"].write(&info->gmd2,stackSlice);
	unshared["energySpectrumExist"].write(&info->energySpectrumExist,stackSlice);
	unshared["nPeaks"].write(&info->nPeaks,stackSlice);
    unshared["nProtons"].write(&info->nProtons,stackSlice);
	unshared["peakNpix"].write(&info->peakNpix,stackSlice);

	unshared["peakTotal"].write(&info->peakTotal,stackSlice);
	unshared["peakResolution"].write(&info->peakResolution,stackSlice);
	unshared["peakResolutionA"].write(&info->peakResolutionA,stackSlice);
	unshared["peakDensity"].write(&info->peakDensity,stackSlice);
	unshared["pumpLaserCode"].write(&info->pumpLaserCode,stackSlice);
	unshared["pumpLaserDelay"].write(&info->pumpLaserDelay,stackSlice);
	unshared["hit"].write(&info->hit,stackSlice);
  
	DETECTOR_LOOP{
		Node & detector = root["cheetah"]["shared"].child("detector",detID+1);
		detector["lastBgUpdate"].write(&global->detector[detID].bgLastUpdate,stackSlice);
		detector["nHot"].write(&global->detector[detID].nhot,stackSlice);
		detector["lastHotPixUpdate"].write(&global->detector[detID].hotpixLastUpdate,stackSlice);
		detector["hotPixCounter"].write(&global->detector[detID].hotpixCounter,stackSlice);
		detector["nHalo"].write(&global->detector[detID].nhalo,stackSlice);
		detector["lastHaloPixUpdate"].write(&global->detector[detID].halopixLastUpdate,stackSlice);
		detector["haloPixCounter"].write(&global->detector[detID].halopixCounter,stackSlice);

		Node & detector2 = root["cheetah"]["unshared"].child("detector",detID+1);
		detector2["sum"].write(&info->detector[detID].sum,stackSlice);
	}
#ifdef H5F_ACC_SWMR_WRITE  
	if(global->cxiSWMR){
		if(global->cxiFlushPeriod && (stackSlice % global->cxiFlushPeriod) == 0){
			H5Fflush(cxi->hid(),H5F_SCOPE_LOCAL);
		}
		
		if (didDecreaseActive) {
			pthread_mutex_lock(&global->nActiveThreads_mutex);
			global->nActiveThreads++;
			pthread_mutex_unlock(&global->nActiveThreads_mutex);
		}
		pthread_mutex_unlock(&global->swmr_mutex);
	}
#endif
}


