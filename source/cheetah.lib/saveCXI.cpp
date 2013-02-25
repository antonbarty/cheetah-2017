/*

 *  saveCxi.cpp
 *  cheetah
 *
 *  Created by Jing Liu on 05/11/12.
 *  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
 *
 */

#include <string>
#include <vector>

#include "saveCXI.h"

template <class T>
static T * generateThumbnail(const T * src,const int srcWidth, const int srcHeight, const int scale)
{
  int dstWidth = srcWidth/scale;
  int dstHeight = srcHeight/scale;
  T * dst = new T[srcWidth*srcHeight];
  for(int x = 0; x <dstWidth; x++){
    for(int y = 0; y<dstHeight; y++){
      double res=0;
      for (int xx = x*scale; xx <x*scale+scale; xx++){
	for(int yy = y*scale; yy <y*scale+scale; yy++){
	  res += src[yy*srcWidth+xx];
	} 
      }
      dst[y*dstWidth+x] = res/(scale*scale);
    }
  }
  return dst;
}

static int getStackSlice(CXI::File * cxi){
#ifdef __GNUC__
  return __sync_fetch_and_add(&(cxi->stackCounter),1);
#else
  pthread_mutex_lock(&global->framefp_mutex);
  int ret = cxi->stackCounter;
  cxi->stackCounter++;
  pthread_mutex_unlock(&global->framefp_mutex);
  return ret;
#endif
}

static hid_t createScalarStack(const char * name, hid_t loc, hid_t dataType){
  hsize_t dims[1] = {CXI::initialStackSize};
  hsize_t maxdims[1] = {H5S_UNLIMITED};
  hid_t cparms = H5Pcreate(H5P_DATASET_CREATE);
  hid_t dataspace = H5Screate_simple(1, dims, maxdims);
  /* Modify dataset creation properties, i.e. enable chunking  */
  H5Pset_chunk(cparms, 1, dims);
  //  H5Pset_deflate (cparms, 2);

  hid_t dataset = H5Dcreate(loc, name, dataType, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);

  const char * axis = "experiment_identifier";
  hsize_t one = 1;
  hid_t datatype = H5Tcopy(H5T_C_S1);
  H5Tset_size(datatype, strlen(axis));
  hid_t memspace = H5Screate_simple(1,&one,NULL);
  hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attr,datatype,axis);
  H5Aclose(attr);
  return dataset;    
}

template <class T> 
static void writeScalarToStack(hid_t dataset, int stackSlice, T value){
  hsize_t count[1] = {1};
  hsize_t offset[1] = {stackSlice};
  hsize_t stride[1] = {1};
  hsize_t block[1] = {1};
  /* dummy */
  hsize_t mdims[1];

  
  hid_t dataspace = H5Dget_space (dataset);
  H5Sget_simple_extent_dims(dataspace, block, mdims);
  /* check if we need to extend the dataset */
  if(block[0] <= stackSlice){
    while(block[0] <= stackSlice){
      block[0] *= 2;
    }
    H5Dset_extent(dataset, block);
    /* get enlarged dataspace */
    dataspace = H5Dget_space (dataset);
  }
  block[0] = 1;
  hid_t memspace = H5Screate_simple (1, block, NULL);

  hid_t type;
  if(typeid(T) == typeid(int)){
    type = H5T_NATIVE_INT32;
  }else if(typeid(T) == typeid(double)){
    type = H5T_NATIVE_DOUBLE;
  }else if(typeid(T) == typeid(float)){
    type = H5T_NATIVE_FLOAT;
  }else if(typeid(T) == typeid(unsigned int)){
    type = H5T_NATIVE_UINT32;
  }else if(typeid(T) == typeid(long)){
    type = H5T_NATIVE_LONG;
  }else{
    fprintf(stderr,"Cannot find right H5 type!\n");
    abort();
  }

  H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
  if(H5Dwrite(dataset, type, memspace, dataspace, H5P_DEFAULT, &value) < 0){
    abort();
  }
  H5Sclose(memspace);
  H5Sclose(dataspace);
}

/* Create a 2D stack. The fastest changing dimension is along the width */
static hid_t create2DStack(const char *name, hid_t loc, int width, int height, hid_t dataType){
  hsize_t dims[3] = {CXI::initialStackSize,height,width};
  hsize_t maxdims[3] = {H5S_UNLIMITED,height,width};
  hid_t dataspace = H5Screate_simple(3, dims, maxdims);
  hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
  H5Pset_chunk(cparms, 3, dims);
  //  H5Pset_deflate (cparms, 2);
  hid_t dataset = H5Dcreate(loc, name, dataType, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);
  H5Pset_chunk_cache(H5Dget_access_plist(dataset),H5D_CHUNK_CACHE_NSLOTS_DEFAULT,1024*1024*16,1);

  const char * axis = "experiment_identifier:y:x";
  hsize_t one = 1;
  hid_t datatype = H5Tcopy(H5T_C_S1);
  H5Tset_size(datatype, strlen(axis));
  hid_t memspace = H5Screate_simple(1,&one,NULL);
  hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attr,datatype,axis);
  H5Aclose(attr);
  return dataset;    
}

template <class T> 
static void write2DToStack(hid_t dataset, int stackSlice, T * data){  
  hsize_t count[3] = {1,1,1};
  hsize_t offset[3] = {stackSlice,0,0};
  /* stride is irrelevant in this case */
  hsize_t stride[3] = {1,1,1};
  hsize_t block[3];
  /* dummy */
  hsize_t mdims[3];
  /* Use the existing dimensions as block size */
  hid_t dataspace = H5Dget_space (dataset); 
  H5Sget_simple_extent_dims(dataspace, block, mdims);
  /* check if we need to extend the dataset */
  if(block[0] <= stackSlice){
    while(block[0] <= stackSlice){
      block[0] *= 2;
    }
    H5Dextend (dataset, block);
    /* get enlarged dataspace */
    dataspace = H5Dget_space (dataset);
  }
  block[0] = 1;
  hid_t memspace = H5Screate_simple (3, block, NULL);

  hid_t type;
  if(typeid(T) == typeid(int)){
    type = H5T_NATIVE_INT32;
  }else if(typeid(T) == typeid(double)){
    type = H5T_NATIVE_DOUBLE;
  }else if(typeid(T) == typeid(unsigned int)){
    type = H5T_NATIVE_UINT32;
  }else if(typeid(T) == typeid(unsigned short)){
    type = H5T_NATIVE_UINT16;
  }else if(typeid(T) == typeid(short)){
    type = H5T_NATIVE_INT16;
  }

  H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
  if(H5Dwrite (dataset, type, memspace, dataspace, H5P_DEFAULT, data) < 0){
    abort();
  }
  H5Sclose(memspace);
  H5Sclose(dataspace);
}

static hid_t createStringStack(const char * name, hid_t loc, int maxSize = 128){
  /* FM: This is probably wrong */
  hid_t datatype = H5Tcopy(H5T_C_S1);
  H5Tset_size(datatype, maxSize);
  hsize_t dims[1] = {CXI::initialStackSize};
  hsize_t maxdims[1] = {H5S_UNLIMITED};
  hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
  hid_t dataspace = H5Screate_simple(1, dims, maxdims);
  H5Pset_chunk (cparms, 1, dims);
  //  H5Pset_deflate(cparms, 2);
  hid_t dataset = H5Dcreate(loc, name, datatype, dataspace, H5P_DEFAULT, cparms, H5P_DEFAULT);

  const char * axis = "experiment_identifier";
  hsize_t one = 1;
  datatype = H5Tcopy(H5T_C_S1);
  H5Tset_size(datatype, strlen(axis));
  hid_t memspace = H5Screate_simple(1,&one,NULL);
  hid_t attr = H5Acreate(dataset,"axes",datatype,memspace,H5P_DEFAULT,H5P_DEFAULT);
  H5Awrite(attr,datatype,axis);
  H5Aclose(attr);
  return dataset;    
}

static void writeStringToStack(hid_t dataset, int stackSlice, const char * value){
  hsize_t count[1] = {1};
  hsize_t offset[1] = {stackSlice};
  hsize_t stride[1] = {1};
  hsize_t block[1] = {1};
  /* dummy */
  hsize_t mdims[1];

  
  hid_t dataspace = H5Dget_space (dataset);
  H5Sget_simple_extent_dims(dataspace, block, mdims);
  /* check if we need to extend the dataset */
  if(block[0] <= stackSlice){
    while(block[0] <= stackSlice){
      block[0] *= 2;
    }
    H5Dset_extent(dataset, block);
    /* get enlarged dataspace */
    dataspace = H5Dget_space (dataset);
  }
  block[0] = 1;
  hid_t memspace = H5Screate_simple (1, block, NULL);

  hid_t type = H5Tcopy(H5T_C_S1);
  H5Tset_size(type, strlen(value));

  H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, offset,stride, count, block);
  if(H5Dwrite(dataset, type, memspace, dataspace, H5P_DEFAULT, value) < 0){
    abort();
  }
  H5Sclose(memspace);
  H5Sclose(dataspace);
}

static CXI::File * createCXISkeleton(const char * filename,cGlobal *global){
  /* Creates the initial skeleton for the CXI file.
     We'll rely on HDF5 automatic error reporting. It's usually loud enough.
  */
  CXI::File * cxi = new CXI::File;
  hid_t fid = H5Fcreate(filename,H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);
  cxi->self = fid;
  cxi->stackCounter = 0;
  hsize_t dims[3];

  /* Create /cxi_version */
  dims[0] = 1;
  hid_t dataspace = H5Screate_simple(1, dims, dims);
  hid_t dataset = H5Dcreate(cxi->self, "cxi_version", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite (dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &CXI::version);

  /* /entry_1 */
  cxi->entry.self = H5Gcreate(cxi->self, "/entry_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Gcreate(cxi->entry.self, "data_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Lcreate_soft("/entry_1/instrument_1/detector_1/data",cxi->entry.self,"data_1/data",H5P_DEFAULT,H5P_DEFAULT);
  
  H5Gcreate(cxi->entry.self, "data_2", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Lcreate_soft("/entry_1/instrument_1/detector_2/data",cxi->entry.self,"data_2/data",H5P_DEFAULT,H5P_DEFAULT);

  cxi->entry.instrument.self = H5Gcreate(cxi->self, "/entry_1/instrument_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  cxi->entry.instrument.source.self = H5Gcreate(cxi->self, "/entry_1/instrument_1/source_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  H5Lcreate_soft("/entry_1/experiment_identifier",cxi->entry.instrument.source.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);
  cxi->entry.instrument.source.energy = createScalarStack("energy",cxi->entry.instrument.source.self,H5T_NATIVE_DOUBLE);

  cxi->entry.experimentIdentifier = createStringStack("experiment_identifier",cxi->entry.self);
  DETECTOR_LOOP{
    char detectorName[1024];
    char imageName[1024];
    /* Raw images */
    sprintf(detectorName,"detector_%ld",detID+1);
    CXI::Detector d;
    d.self = H5Gcreate(cxi->entry.instrument.self, detectorName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    d.distance = createScalarStack("distance", d.self,H5T_NATIVE_DOUBLE);
    d.description = createStringStack("description",d.self);
    d.xPixelSize = createScalarStack("x_pixel_size",d.self,H5T_NATIVE_DOUBLE);
    d.yPixelSize = createScalarStack("y_pixel_size",d.self,H5T_NATIVE_DOUBLE);
    
    if(global->saveRaw){

      d.data = create2DStack("data", d.self, global->detector[detID].pix_nx, global->detector[detID].pix_ny, H5T_STD_I16LE);
      d.thumbnail = create2DStack("thumbnail", d.self, global->detector[detID].pix_nx/CXI::thumbnailScale, global->detector[detID].pix_ny/CXI::thumbnailScale, H5T_STD_I16LE);
      H5Lcreate_soft("/entry_1/experiment_identifier",d.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);
    }
    cxi->entry.instrument.detectors.push_back(d);

    /* Assembled images */
    sprintf(imageName,"image_%ld",detID+1);
    CXI::Image img;
    img.self = H5Gcreate(cxi->entry.self, imageName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if(global->saveAssembled){
      img.data = create2DStack("data", img.self, global->detector[detID].image_nx, global->detector[detID].image_nx, H5T_STD_I16LE);
      H5Lcreate_soft((std::string("/entry_1/instrument_1/") + detectorName).c_str(),img.self,"detector_1",H5P_DEFAULT,H5P_DEFAULT);
      H5Lcreate_soft("/entry_1/instrument_1/source_1",img.self,"source_1",H5P_DEFAULT,H5P_DEFAULT);
      img.dataType = createStringStack("data_type",img.self);
      img.dataSpace = createStringStack("data_space",img.self);
      img.thumbnail = create2DStack("thumbnail", img.self, global->detector[detID].image_nx/CXI::thumbnailScale, global->detector[detID].image_nx/CXI::thumbnailScale, H5T_STD_I16LE);
      H5Lcreate_soft("/entry_1/experiment_identifier",img.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);
    }
    cxi->entry.images.push_back(img);
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
  H5Lcreate_soft("/LCLS/eventTimeString", cxi->self, "/LCLS/eventTime",H5P_DEFAULT,H5P_DEFAULT);
  H5Lcreate_soft("/entry_1/experiment_identifier",cxi->lcls.self,"experiment_identifier",H5P_DEFAULT,H5P_DEFAULT);

  DETECTOR_LOOP{
    char buffer[1024];
    sprintf(buffer,"detector%li-position", detID +1);
    cxi->lcls.detector_positions.push_back(createScalarStack(buffer, cxi->lcls.self,H5T_NATIVE_DOUBLE));
    sprintf(buffer,"detector%li-EncoderValue", detID +1);
    cxi->lcls.detector_EncoderValues.push_back(createScalarStack(buffer, cxi->lcls.self,H5T_NATIVE_DOUBLE));
  }
  return cxi;
}

static std::vector<std::string> openFilenames = std::vector<std::string>();
static std::vector<CXI::File* > openFiles = std::vector<CXI::File *>();

static CXI::File * getCXIFileByName(const char * filename, cGlobal *global){
  pthread_mutex_lock(&global->framefp_mutex);
  /* search again to be sure */
  for(int i = 0;i<openFilenames.size();i++){
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


static void  closeCXI(CXI::File * cxi){
  hid_t ids[256];
  int n_ids = H5Fget_obj_ids(cxi->self, H5F_OBJ_DATASET, 256, ids);
  for (int i=0; i<n_ids; i++){
    H5I_type_t type;
    hsize_t block[3];
    hsize_t mdims[3];
    hid_t dataspace = H5Dget_space (ids[i]);
    H5Sget_simple_extent_dims(dataspace, block, mdims);
    if(mdims[0] == H5S_UNLIMITED){
      block[0] = cxi->stackCounter;
      H5Dset_extent(ids[i], block);
    }
  }
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
  for(int i = 0;i<openFilenames.size();i++){
    closeCXI(openFiles[i]);    
  }
  openFiles.clear();
  openFilenames.clear();
  pthread_mutex_unlock(&global->framefp_mutex);
#endif
  H5close();
}

void writeCXI(cEventData *info, cGlobal *global ){
  /* Get the existing CXI file or open a new one */
  CXI::File * cxi = getCXIFileByName(info->cxiFilename, global);
  int stackSlice = getStackSlice(cxi);
  printf("Writting CXI file for stack slice number %ld \n", stackSlice);
  
  double en = info->photonEnergyeV * 1.60217646e-19;
  writeScalarToStack(cxi->entry.instrument.source.energy,stackSlice,en);
  // remove the '.h5' from eventname
  info->eventname[strlen(info->eventname) - 3] = 0;
  writeStringToStack(cxi->entry.experimentIdentifier,stackSlice,info->eventname);
  // put it back
  info->eventname[strlen(info->eventname)] = '.';

  DETECTOR_LOOP {    
    /* Save assembled image under image groups */
    writeScalarToStack(cxi->entry.instrument.detectors[detID].distance,stackSlice,global->detector[detID].detectorZ/1000.0);
    writeScalarToStack(cxi->entry.instrument.detectors[detID].xPixelSize,stackSlice,global->detector[detID].pixelSize);
    writeScalarToStack(cxi->entry.instrument.detectors[detID].yPixelSize,stackSlice,global->detector[detID].pixelSize);

    char buffer[1024];
    sprintf(buffer,"%s [%s]",global->detector[detID].detectorType,global->detector[detID].detectorName);
    writeStringToStack(cxi->entry.instrument.detectors[detID].description,stackSlice,buffer);
    if(global->saveAssembled){
      write2DToStack(cxi->entry.images[detID].data,stackSlice,info->detector[detID].image);
      int16_t * thumbnail = generateThumbnail(info->detector[detID].image,global->detector[detID].image_nx,global->detector[detID].image_nx,CXI::thumbnailScale);
      write2DToStack(cxi->entry.images[detID].thumbnail,stackSlice,thumbnail);
      writeStringToStack(cxi->entry.images[detID].dataType,stackSlice,"intensities");
      writeStringToStack(cxi->entry.images[detID].dataSpace,stackSlice,"diffraction");
      delete thumbnail;      
    }
    if(global->saveRaw){
      write2DToStack(cxi->entry.instrument.detectors[detID].data,stackSlice,info->detector[detID].corrected_data_int16);
      int16_t * thumbnail = generateThumbnail(info->detector[detID].corrected_data_int16,global->detector[detID].pix_nx,global->detector[detID].pix_ny,CXI::thumbnailScale);
      write2DToStack(cxi->entry.instrument.detectors[detID].thumbnail,stackSlice,thumbnail);
      delete thumbnail;
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
  int LaserOnVal = (info->laserEventCodeOn)?1:0;
  writeScalarToStack(cxi->lcls.evr41,stackSlice,LaserOnVal);
  char timestr[26];
  time_t eventTime = info->seconds;
  ctime_r(&eventTime,timestr);
  writeStringToStack(cxi->lcls.eventTimeString,stackSlice,timestr);
  printf("r%04u:%li (%2.1f Hz): Writing %s to %s slice %d\n",global->runNumber, info->threadNum,global->datarate, info->eventname, info->cxiFilename, stackSlice);
}

