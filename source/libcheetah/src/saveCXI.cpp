
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

	// Do not call this method during parallel writing!
	herr_t Node::groupExists(const char * s,int n){
		char buffer[1024];
		sprintf(buffer,"%s_%d",prefix,n);
	    return groupExists(buffer);
	}

	// Do not call this method during parallel writing!
	herr_t Node::groupExists(const char * s){
		herr_t status; 
        /* Save old error handler */
		herr_t (*old_func)(void*);
		void *old_client_data;
		H5Eget_auto(&old_func, &old_client_data);
        /* Turn off error handling temporarily */
		H5Eset_auto(NULL, NULL);
		status = H5Gget_objinfo(hid(),s,0,NULL);
        /* Restore previous error handler */
		H5Eset_auto(old_func, old_client_data);
		if (status == 0){
			return 1;
		} else {
			return 0;
		}
	}

	Node * Node::addClassLink(const char * s, std::string target){
		std::string key = nextKey(s);
		return createLink(key.c_str(), target);
	}

	Node * Node::addDatasetLink(const char * s, std::string target){
		char buffer[1024];
		sprintf(buffer,"%s/%s",target,s);
		hid_t lid = H5Lcreate_soft(buffer.c_str(),hid(),s,H5P_DEFAULT,H5P_DEFAULT);
		if(lid < 0){
			return NULL;
		}
		return addNode(s, lid, Link);
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
  |- data_1 ---------\
  |                  |
  |- instrument_1    | symlink
  |   |              |
  |   |-detector_1 <---------------------------------------\
  |   |   |                                                |
  |   |   |- data [non-assembled data, 3D array]           |
  |   |   |- (mask) [non-asslembled masks, 3D array]       |
  |   |   |- mask_shared [non-assembled mask, 2D array]    | 
  |   |   |- ...                                           | symlink
  |   .   .                                                |
  |                                                        |
  |- (image_1)                                             |
  |    |                                                   |
  |    |- detector_1 --------------------------------------/
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
	entry->createStack("experiment_identifier",H5T_NATIVE_CHAR,CXI::stringSize);

	Node * instrument = entry->addClass("instrument");
	Node * source = instrument->addClass("source");
	char buffer[1024];

	source->createStack("energy",H5T_NATIVE_DOUBLE);
	source->createLink("experiment_identifier", "/entry_1/experiment_identifier");

	// If we have sample translation or electrojet voltage configured, write it out to file
	if(global->samplePosXPV[0] || global->samplePosYPV[0] || global->samplePosZPV[0] || global->sampleVoltage[0]){
		Node * sample = entry->addClass("sample");
		sample->addClass("geometry")->createDataset("translation",H5T_NATIVE_FLOAT,3,0,H5S_UNLIMITED);
		sample->addClass("injection")->createStack("voltage",H5T_NATIVE_FLOAT);
	}
	
	DETECTOR_LOOP{
		// /entry_1/instrument_1/detector_[i]/
		Node * detector = instrument->createGroup("detector",detIndex+1);

		// For convenience define some detector specific variables
		int asic_nx = dataV.detector.asic_nx;
		int asic_ny = dataV.detector.asic_ny;
		int asic_nn = asic_nx * asic_ny;
		int nasics_x = dataV.detector.nasics_x;
		int nasics_y = dataV.detector.nasics_y;
		int nasics = nasics_x * nasics_y;
		long pix_nn = global->detector[detIndex].pix_nn;
		long pix_nx = global->detector[detIndex].pix_nx;
		long pix_ny = global->detector[detIndex].pix_ny;
		long image_nn = global->detector[detIndex].image_nn;
		long image_nx = global->detector[detIndex].image_nx;
		long image_ny = global->detector[detIndex].image_ny;
		long imageXxX_nn = global->detector[detIndex].imageXxX_nn;
		long imageXxX_nx = global->detector[detIndex].imageXxX_nx;
		long imageXxX_ny = global->detector[detIndex].imageXxX_ny;
		long radial_nn = global->detector[detIndex].radial_nn;
		uint16_t pixelmask_shared = global->detector[detIndex].pixelmask_shared;
		uint16_t pixelmask_shared_min = global->detector[detIndex].pixelmask_shared_min;
		uint16_t pixelmask_shared_max = global->detector[detIndex].pixelmask_shared_max;
		int downsampling = global->detector[detIndex].downsampling;

		// What does this do?
		entry->addClassLink("data",detector->path().c_str());
		// ?
		detector->createStack("distance",H5T_NATIVE_DOUBLE);
		detector->createStack("description",H5T_NATIVE_CHAR,CXI::stringSize);
		detector->createStack("x_pixel_size",H5T_NATIVE_DOUBLE);
		detector->createStack("y_pixel_size",H5T_NATIVE_DOUBLE);
		detector->createLink("experiment_identifier", "/entry_1/experiment_identifier");


		// DATA_FORMAT_NON_ASSEMBLED
		if (global->saveNonAssembled) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_NON_ASSEMBLED);
			while (dataV.next()) {
				if (global->saveModular) {
					// Non-assembled images, modular (4D: N_frames x N_modules x Ny_module x Nx_module)

					// Create group /entry_1/instrument_1/detector_[i]/data_modular_[datver]/
					sprintf(buffer,"modular_data_%s",dataV.name);
					Node * data_node = detector->createGroup(buffer);
					data_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
					data_node->createStack("data",H5T_NATIVE_FLOAT, asic_nx, asic_ny, nasics);
					data_node->createStack("corner_positions",H5T_NATIVE_FLOAT, 3, nasics, H5S_UNLIMITED, 0, 0, 0, "experiment_identifier:module_identifier:coordinate");
					data_node->createStack("basis_vectors", H5T_NATIVE_FLOAT, 3, 2, nasics, H5S_UNLIMITED, 0, 0, "experiment_identifier:module_identifier:dimension:coordinate");
					data_node->createStack("module_identifier", H5T_NATIVE_CHAR, CXI::stringSize, nasics, 0, H5S_UNLIMITED, 0,0,"experiment_identifier:module_identifier");
					if(global->savePixelmask){
						data_node->createStack("mask",H5T_NATIVE_UINT16,asic_nx, asic_ny, nasics);
					}				
					uint16_t* mask = (uint16_t *) calloc(asic_nn*nasics_x*nasics_y, sizeof(uint16_t));
					stackModulesMask(pixelmask_shared, mask, asic_nx, asic_ny, nasics_x, nasics_y);
					data_node->createDataset("mask_shared",H5T_NATIVE_UINT16,asic_nx, asic_ny, nasics)->write(mask);
					stackModulesMask(pixelmask_shared_max, mask, asic_nx, asic_ny, nasics_x, nasics_y);
					data_node->createDataset("mask_shared_max",H5T_NATIVE_UINT16,asic_nx, asic_ny, nasics)->write(mask);
					stackModulesMask(pixelmask_shared_min, mask, asic_nx, asic_ny, nasics_x, nasics_y);
					data_node->createDataset("mask_shared_min",H5T_NATIVE_UINT16,asic_nx, asic_ny, nasics)->write(mask);					
					free(mask);

					// If this is the main dataset we create symbolic links
					if (dataV.isMainDataset == 1) {
						detector->addDatasetLink("data",data_node->path().c_str());
						detector->addDatasetLink("corner_positions",data_node->path().c_str());
						detector->addDatasetLink("basis_vectors",data_node->path().c_str());
						detector->addDatasetLink("module_identifier",data_node->path().c_str());
						if(global->savePixelmask){
							detector->addDatasetLink("mask",data_node->path().c_str());
						}
						detector->addDatasetLink("mask_shared",data_node->path().c_str());
						detector->addDatasetLink("mask_shared_max",data_node->path().c_str());
						detector->addDatasetLink("mask_shared_min",data_node->path().c_str());
					}
				} else {
					// Non-assembled images (3D: N_frames x Ny_frame x Nx_frame)
					int pix_nx = global->detector[detIndex].pix_nx;
					int pix_ny = global->detector[detIndex].pix_ny;
				
					// Create group /entry_1/instrument_1/detector_[i]/data_[datver]/
					sprintf(buffer,"data_%s",dataV.name);
					Node * data_node = detector->createGroup(buffer);
					data_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
					data_node->createStack("data",H5T_NATIVE_FLOAT,pix_nx, pix_ny);
					if(global->savePixelmask){
						data_node->createStack("mask",H5T_NATIVE_UINT16,pix_nx, pix_ny);
					}
					data_node->createDataset("mask_shared",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(pixelmask_shared);
					data_node->createDataset("mask_shared_max",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(pixelmask_shared_max);
					data_node->createDataset("mask_shared_min",H5T_NATIVE_UINT16,pix_nx, pix_ny)->write(pixelmask_shared_min);
					data_node->createStack("thumbnail",H5T_STD_I16LE, pix_nx/CXI::thumbnailScale, pix_ny/CXI::thumbnailScale);

					// If this is the main dataset we create links to all datasets
					if (dataV.isMainDataset) {
						detector->addDatasetLink("data",data_node->path().c_str());
						if(global->savePixelmask){
							detector->addDatasetLink("mask",data_node->path().c_str());
						}
						detector->addDatasetLink("mask_shared",data_node->path().c_str());
						detector->addDatasetLink("mask_shared_max",data_node->path().c_str());
						detector->addDatasetLink("mask_shared_min",data_node->path().c_str());
					}
				}
			}
		}
		
		// DATA_FORMAT_ASSEMBLED
		if (global->saveAssembled) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_ASSEMBLED);
			while (dataV.next()) {
				// Assembled images (3D: N_frames x Ny_image x Nx_image)
				int i_image = detIndex+1;			
				// Create group /entry_1/image_i
				if (entry->groupExists("image",i_image) == 0) {
					Node * image_node = entry->createGroup("image",i_image);
					image_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
					image_node->addClassLink("detector",detector->path());
					image_node->addClassLink("source",source->path());			
				} else {
					Node & image_node = root["entry_1"].child("image",i_image);
				}
				// Create group /entry_1/image_i/data_[datver]/
				sprintf(buffer,"data_%s",dataV.name);
				Node * data_node = image_node->createGroup(dataV.name);		
				data_node->createStack("data",H5T_NATIVE_FLOAT, image_nx, image_ny);
				if(global->savePixelmask){
					data_node->createStack("mask",H5T_NATIVE_UINT16, image_nx, image_ny);
				}
				uint16_t *image_pixelmask_shared = (uint16_t*) calloc(image_nn,sizeof(uint16_t));
				assemble2Dmask(image_pixelmask_shared, pixelmask_shared, 
							   pix_nx, pix_ny, pix_nn, image_nx, image_nn, global->assembleInterpolation);
				data_node->createDataset("mask_shared",H5T_NATIVE_UINT16,dataV.pix_nx, dataV.pix_ny)->write(image_pixelmask_shared);
				free(image_pixelmask_shared);      
				data_node->createStack("data_type",H5T_NATIVE_CHAR,CXI::stringSize);
				data_node->createStack("data_space",H5T_NATIVE_CHAR,CXI::stringSize);
				data_node->createStack("thumbnail",H5T_NATIVE_FLOAT, dataV.pix_nx/CXI::thumbnailScale, dataV.pix_ny/CXI::thumbnailScale);
				data_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
				// If this is the main dataset we create links to all datasets
				if (dataV.isMainDataset == 1) {
					image_node->addDatasetLink("data",data_node->path().c_str());
					if(global->savePixelmask){
						image_node->addDatasetLink("mask",data_node->path().c_str())
							}
					image_node->addDatasetLink("mask_shared",data_node->path().c_str());
					image_node->addDatasetLink("data_type",data_node->path().c_str());
					image_node->addDatasetLink("data_space",data_node->path().c_str());
					image_node->addDatasetLink("thumbnail",data_node->path().c_str());
				}
			}
		}	

		// DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED
		if (downsampling > 1) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED);
			while (dataV.next()) {
				// Assembled images (3D: N_frames x Ny_imageXxX x Nx_imageXxX)
				int i_image = global->nDetectors+detIndex+1;
				// Create group /entry_1/image_[i]
				if (entry->groupExists("image",i_image) == 0) {
					Node * image_node = entry->createGroup("image",i_image);
					image_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
					image_node->addClassLink("detector",detector->path());
					image_node->addClassLink("source",source->path());			
				} else {
					Node & image_node = root["entry_1"].child("image",i_image);
				}
				// Create group /entry_1/image_i/data_[datver]/
				sprintf(buffer,"data_%s",dataV.name);
				Node * data_node = image_node->createGroup(dataV.name);			
				data_node->createStack("data",H5T_NATIVE_FLOAT, dataV.pix_nx, dataV.pix_ny);
				if(global->savePixelmask){
					image_dataver->createStack("mask",H5T_NATIVE_UINT16, dataV.pix_nx, dataV.pix_ny);
				}
				uint16_t *image_pixelmask_shared = (uint16_t*) calloc( global->detector[detIndex].image_nn,sizeof(uint16_t));
				assemble2Dmask(image_pixelmask_shared, pixelmask_shared, pix_nx, pix_ny, pix_nn, image_nx, image_nn, global->assembleInterpolation);
				uint16_t *imageXxX_pixelmask_shared = (uint16_t*) calloc(dataV.pix_nn,sizeof(uint16_t));
				if(global->detector[detIndex].downsamplingConservative==1){
					downsampleMaskConservative(image_pixelmask_shared,imageXxX_pixelmask_shared, image_nn, image_nx, imageXxX_nx, imageXxX_nn, downsampling);
				} else {
					downsampleMaskNonConservative(image_pixelmask_shared,imageXxX_pixelmask_shared, image_nn, image_nx, imageXxX_nx, imageXxX_nn, downsampling);
				}
				data_node->createDataset("mask_shared",H5T_NATIVE_UINT16,image_nx, image_ny)->write(imageXxX_pixelmask_shared);
				free(imageXxX_pixelmask_shared);
				free(image_pixelmask_shared);
				data_node->createStack("data_type",H5T_NATIVE_CHAR,CXI::stringSize);
				data_node->createStack("data_space",H5T_NATIVE_CHAR,CXI::stringSize);
				data_node->createStack("thumbnail",H5T_NATIVE_FLOAT, image_nx/CXI::thumbnailScale, image_ny/CXI::thumbnailScale);
				data_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
			}
		}

		// DATA_FORMAT_RADIAL_AVERAGE
		if (global->detector[detIndex].saveRadialAverage) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_RADIAL_AVERAGE);
			while (dataV.next()) {
				// Radial average (2D: N_frames x N_radial)
				int i_image = global->nDetectors*2+detIndex+1;
				// Create group /entry_1/image_[i]
				if (entry->groupExists("image",i_image) == 0) {
					Node * image_node = entry->createGroup("image",i_image);
					image_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
					image_node->addClassLink("detector",detector->path());
					image_node->addClassLink("source",source->path());			
				} else {
					Node & image_node = root["entry_1"].child("image",i_image);
				}
				// Create group /entry_1/image_i/data_[datver]/
				sprintf(buffer,"data_%s",dataV.name);
				Node * data_node = image_node->createGroup(dataV.name);		
				// Create group /entry_1/image_i/[datver]/
				Node * data_node = image->createGroup(datver);		
				data_node->createStack("data",H5T_NATIVE_FLOAT, radial_nn);
				if(global->savePixelmask){
					image_dataver->createStack("mask",H5T_NATIVE_UINT16, radial_nn);
				}
				uint16_t *radial_pixelmask_shared = (uint16_t*) calloc(radial_nn,sizeof(uint16_t));
				float *foo1 = calloc(global->detector[detIndex].pix_nn,sizeof(float));
				float *foo2 = calloc(global->detector[detIndex].radial_nn,sizeof(float));
				calculateRadialAverage(foo1, global->detector[detIndex].pixelmask_shared, foo2, radial_pixelmask_shared, global->detector[detIndex].pix_r, global->detector[detIndex].radial_nn, global->detector[detIndex].pix_nn);
				data_node->createDataset("mask_shared",H5T_NATIVE_UINT16,image_nx, image_ny)->write(radial_pixelmask_shared);
				free(radial_pixelmask_shared);
				free(foo1);
				free(foo2);
				data_node->createStack("data_type",H5T_NATIVE_CHAR,CXI::stringSize);
				data_node->createStack("data_space",H5T_NATIVE_CHAR,CXI::stringSize);
				data_node->createLink("experiment_identifier", "/entry_1/experiment_identifier");
			}
		}
	}

	if(global->TOFPresent){
		for(int i = 0;i<global->nTOFDetectors;i++){
			Node * detector = instrument->createGroup("detector",1+i+global->nDetectors);
			detector->createStack("data",H5T_NATIVE_DOUBLE,global->tofDetector[i].numSamples);
			detector->createStack("tofTime",H5T_NATIVE_DOUBLE,global->tofDetector[i].numSamples);
			char buffer[1024];
			sprintf(buffer,"TOF detector with source %s and channel %d\n%s",global->tofDetector[i].sourceIdentifier,
					global->tofDetector[i].channel, global->tofDetector[i].description);
			detector->createDataset("description",H5T_NATIVE_CHAR,MAX_FILENAME_LENGTH)->write(buffer);
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
	lcls->createStack("eventTimeString",H5T_NATIVE_CHAR,CXI::stringSize);
	lcls->createLink("eventTime","eventTimeString");
	lcls->createLink("experiment_identifier","/entry_1/experiment_identifier");

	DETECTOR_LOOP{
		Node * detector = lcls->createGroup("detector",detIndex+1);
		detector->createStack("position",H5T_NATIVE_DOUBLE);
		detector->createStack("EncoderValue",H5T_NATIVE_DOUBLE);
		detector->createStack("SolidAngleConst",H5T_NATIVE_DOUBLE);
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
		Node * detector = unshared->createGroup("detector",detIndex+1);
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
		Node * det_node = shared->createGroup("detector",detIndex+1);
		det_node->createStack("lastBgUpdate",H5T_NATIVE_LONG);
		det_node->createStack("nHot",H5T_NATIVE_LONG);
		det_node->createStack("lastHotPixUpdate",H5T_NATIVE_LONG);
		det_node->createStack("hotPixCounter",H5T_NATIVE_LONG);
		det_node->createStack("nHalo",H5T_NATIVE_LONG);
		det_node->createStack("lastHaloPixUpdate",H5T_NATIVE_LONG);
		det_node->createStack("haloPixCounter",H5T_NATIVE_LONG);

		POWDER_LOOP{
			Node * cl = det_node->createGroup("class",powID+1);
			foreach (uint16_t i_f in DATA_FORMATS) {
				cDataVersion dataV(eventData->detector[detIndex],DATA_LOOP_MODE_POWDER,i_f);
				while (dataV.next()) {
					if (dataV.dataFormat != DATA_FORMAT_RADIAL_AVERAGE) {
						sprintf(buffer,"mean_%s",dataV.name);
						cl->createDataset(buffer,H5T_NATIVE_DOUBLE,dataV.pix_nx,dataV.pix_ny);
						sprintf(buffer,"sigma_%s",dataV.name);
						cl->createDataset(buffer,H5T_NATIVE_DOUBLE,dataV.pix_nx,dataV.pix_ny);				
					} else {
						sprintf(buffer,"mean_%s",dataV.name);
						cl->createDataset(buffer,H5T_NATIVE_DOUBLE,dataV.pix_nn);
						sprintf(buffer,"sigma_%s",dataV.name);
						cl->createDataset(buffer,H5T_NATIVE_DOUBLE,dataV.pix_nn);									
					}
				}
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
	char    buffer[1024];

	DETECTOR_LOOP{
		Node & det_node = (*cxi)["cheetah"]["shared"].child("detector",detIndex+1);
		POWDER_LOOP{
			Node & cl = det_node.child("class",powID+1);
			foreach (uint16_t i_f in DATA_FORMATS) {
				cDataVersion dataV(eventData->detector[detIndex],DATA_LOOP_MODE_POWDER,i_f);
				while (dataV.next()) {
					
					detector = &dataV.detector;
					pix_nn =  dataV.pix_nn;

					// raw
					double * sum = detector->powder[powID];
					double * sum_sq = detector->powderSquared[powID];     
					double * mean = (double*) calloc(pix_nn, sizeof(double));
					double * sigma = (double *) calloc(pix_nn,sizeof(double));
					for(long i = 0; i<pix_nn; i++){
						mean[i] = sum_raw[i]/(1.*detector->nPowderFrames[powID]);
						sigma[i] =	sqrt( fabs(sum_sq[i] - sum[i]*sum[i]/(1.*detector->nPowderFrames[powID])) /
											  (1.*detector->nPowderFrames[powID]) );
					}
					sprintf(buffer,"mean_%s",dataV.name); 
					cl[buffer].write(mean);
					sprintf(buffer,"sigma_%s",dataV.name); 
					cl[buffer].write(sigma);

					free(mean);
					free(sigma);

				}      
			}
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
			didDecreaseActive = true;
		}
		pthread_mutex_unlock(&global->nActiveThreads_mutex);
		pthread_mutex_lock(&global->swmr_mutex);
	}
#endif
	/* Get the existing CXI file or open a new one */
	CXI::Node * cxi = getCXIFileByName(global);
	Node & root = *cxi;
	char buffer[1024];
	float * data_datver,datver_node,imageXxX_datver;

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
		Node & detector = root["entry_1"]["instrument_1"].child("detector",detIndex+1);
		double tmp = global->detector[detIndex].detectorZ/1000.0;

		// For convenience define some detector specific variables
		int asic_nx = dataV.detector.asic_nx;
		int asic_ny = dataV.detector.asic_ny;
		int asic_nn = asic_nx * asic_ny;
		int nasics_x = dataV.detector.nasics_x;
		int nasics_y = dataV.detector.nasics_y;
		int nasics = nasics_x * nasics_y;
		long pix_nn = global->detector[detIndex].pix_nn;
		long pix_nx = global->detector[detIndex].pix_nx;
		long pix_ny = global->detector[detIndex].pix_ny;
		long image_nn = global->detector[detIndex].image_nn;
		long image_nx = global->detector[detIndex].image_nx;
		long image_ny = global->detector[detIndex].image_ny;
		long imageXxX_nn = global->detector[detIndex].imageXxX_nn;
		long imageXxX_nx = global->detector[detIndex].imageXxX_nx;
		long imageXxX_ny = global->detector[detIndex].imageXxX_ny;
		long radial_nn = global->detector[detIndex].radial_nn;
		int downsampling = global->detector[detIndex].downsampling;
		int saveRadialAverage = global->detector[detIndex].saveRadialAverage;

		detector["distance"].write(&tmp,stackSlice);
		detector["x_pixel_size"].write(&global->detector[detIndex].pixelSize,stackSlice);
		detector["y_pixel_size"].write(&global->detector[detIndex].pixelSize,stackSlice);

		sprintf(buffer,"%s [%s]",global->detector[detIndex].detectorType,global->detector[detIndex].detectorName);
		detector["description"].write(buffer,stackSlice);

		// DATA_FORMAT_NON_ASSEMBLED
		if (global->saveNonAssembled) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_NON_ASSEMBLED);
			while (dataV.next()) {
				if (global->saveModular){
					// Non-assembled images, modular (4D: N_frames x N_modules x Ny_module x Nx_module)
					sprintf(buffer,"modular_data_%s",dataV.name);
					Node & data_node = detector[buffer];

					float * dataModular = (float *) calloc(asics_nn*nasics, sizeof(float));
					stackModulesData(data_datver, dataModular, asic_nx, asic_ny, nasics_x, nasics_y);
					data_node["data"].write(dataModular, stackSlice);
					free(dataModular);

					float * cornerPos = (float *) calloc(nasics*3, sizeof(float));
					cornerPositions(cornerPos, global->detector[detIndex].pix_x, global->detector[detIndex].pix_y, global->detector[detIndex].pix_z, global->detector[detIndex].pixelSize, asic_nx, asic_ny, nasics_x, nasics);
					detector["corner_positions"].write(cornerPos, stackSlice);
					free(cornerPos);

					float * basisVec = (float *) calloc(nasics*2*3, sizeof(float));
					basisVectors(basisVec, global->detector[detIndex].pix_x, global->detector[detIndex].pix_y, global->detector[detIndex].pix_z, asic_nx, asic_ny, nasics_x, nasics);
					data_node["basis_vectors"].write(basisVec, stackSlice);
					free(basisVec);
					
					char * moduleId = (char *) calloc(nasics*CXI::stringSize, sizeof(char));							   
					moduleIdentifier(moduleId, nasics_x*nasics_y, CXI::stringSize);
					data_node["module_identifier"].write(moduleId, stackSlice);
					free(moduleId);

					if(global->savePixelmask){
						uint16_t* maskModular = (uint16_t *) calloc(asics_nn*nasics_x*nasics_y, sizeof(uint16_t));
						stackModulesMask(info->detector[detIndex].pixelmask, maskModular, asic_nx, asic_ny, nasics_x, nasics_y);
						data_node["mask"].write(maskModular,stackSlice);
						free(maskModular);
					}
				} else {
					// Non-assembled images (3D: N_frames x Ny_frame x Nx_frame)
					sprintf(buffer,"data_%s",dataV.name);
					Node & data_node = detector[buffer];
					data_node["data"].write(dataV,stackSlice);	
					if(global->savePixelmask){
						data_node["mask"].write(dataV.pixelmask,stackSlice);
					}
					float * thumbnail = generateThumbnail(dataV.data,pix_nx,pix_ny,CXI::thumbnailScale);
					data_node["thumbnail"].write(thumbnail, stackSlice);
					delete [] thumbnail;
				}			
			}
		}

        // DATA_FORMAT_ASSEMBLED
		if (global->saveAssembled) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_ASSEMBLED);
			while (dataV.next()) {
				// Assembled images (3D: N_frames x Ny_image x Nx_image)
				if (global->detector[detIndex].assemble)
					int i_image = detIndex+1;
				sprintf(buffer,"data_%s",dataV.name);
				Node & data_node = root["entry_1"].child("image",detIndex+1)[buffer];
				data_node["data"].write(dataV.data,stackSlice);
				if(global->savePixelmask){
					data_node["mask"].write(dataV.pixelmask,stackSlice);
				}
				float * thumbnail = generateThumbnail(dataV.data,image_nx,image_ny,CXI::thumbnailScale);
				data_node["thumbnail"].write(thumbnail, stackSlice);
				data_node["data_type"].write("intensities", stackSlice);
				data_node["data_space"].write("diffraction", stackSlice);
				delete [] thumbnail;
			}
		}

		// DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED
		if (downsampling > 1) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED);
			while (dataV.next()) {
				// Assembled images (3D: N_frames x Ny_imageXxX x Nx_imageXxX)
				sprintf(buffer,"data_%s",dataV.name);
				Node & data_node = root["entry_1"].child("image",global->nDetectors+detIndex+1)[buffer];
				data_node["data"].write(dataV.data,stackSlice);
				if(global->savePixelmask){
					data_node["mask"].write(dataV.pixelmask,stackSlice);
				}
				float * thumbnail = generateThumbnail(imageXxX_datver,imageXxX_nx,imageXxX_ny,CXI::thumbnailScale);
				data_node["thumbnail"].write(thumbnail, stackSlice);
				data_node["data_type"].write("intensities", stackSlice);
				data_node["data_space"].write("diffraction", stackSlice);
				delete [] thumbnail;
			}
		}

		// DATA_FORMAT_ASSEMBLED_AND_DOWNSAMPLED
		if (saveRadialAverage) {
			cDataVersion dataV(eventData->detector[detIndex], DATA_LOOP_MODE_SAVE, DATA_FORMAT_RADIAL_AVERAGE);
			while (dataV.next()) {
				// Radial average (2D: N_frames x N_radial)
				sprintf(buffer,"data_%s",dataV.name);
				Node & data_node = root["entry_1"].child("image",global->nDetectors*2+detIndex+1)[buffer];
				data_node["data"].write(dataV.data,stackSlice);
				if(global->savePixelmask){
					data_node["mask"].write(dataV.pixelmask,stackSlice);
				}
				data_node["data_type"].write("intensities", stackSlice);
				data_node["data_space"].write("diffraction", stackSlice);
			}
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
		lcls.child("detector",detIndex+1)["position"].write(&global->detector[detIndex].detectorZ,stackSlice);
		lcls.child("detector",detIndex+1)["EncoderValue"].write(&global->detector[detIndex].detectorEncoderValue,stackSlice);
		lcls.child("detector",detIndex+1)["SolidAngleConst"].write(&global->detector[detIndex].solidAngleConst,stackSlice);
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
		for(int i = 0; i<global->nTOFDetectors;i++){
			int tofDetIndex = i+global->nDetectors;
			Node & detector = root["entry_1"]["instrument_1"].child("detector",tofDetIndex+1);
			detector["data"].write(&(info->tofDetector[i].voltage[0]),stackSlice);
			detector["tofTime"].write(&(info->tofDetector[i].time[0]),stackSlice);
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
		Node & detector = root["cheetah"]["shared"].child("detector",detIndex+1);
		detector["lastBgUpdate"].write(&global->detector[detIndex].bgLastUpdate,stackSlice);
		detector["nHot"].write(&global->detector[detIndex].nhot,stackSlice);
		detector["lastHotPixUpdate"].write(&global->detector[detIndex].hotpixLastUpdate,stackSlice);
		detector["hotPixCounter"].write(&global->detector[detIndex].hotpixCounter,stackSlice);
		detector["nHalo"].write(&global->detector[detIndex].nhalo,stackSlice);
		detector["lastHaloPixUpdate"].write(&global->detector[detIndex].halopixLastUpdate,stackSlice);
		detector["haloPixCounter"].write(&global->detector[detIndex].halopixCounter,stackSlice);

		Node & detector2 = root["cheetah"]["unshared"].child("detector",detIndex+1);
		detector2["sum"].write(&.sum,stackSlice);
		
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


