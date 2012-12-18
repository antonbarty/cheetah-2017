/*

 *  saveCxi.cpp
 *  cheetah
 *
 *  Created by Jing Liu on 05/11/12.
 *  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
 *
 */

#include "saveCxi.h"
#include "GetSnabshot.h"


#define PRO CxiFileHandlar::Instance()->para.pro
//#define PRO 1
void CXI::extend(hid_t & dataset, hsize_t* entendedSize ){
    herr_t status;
    status = H5Dextend (dataset, entendedSize);
    if (status < 0)
        printf("Extend Error in dataset %d \n", dataset);
}
void CXI:: extendAllDataset(int scale, cGlobal* global)
{
    hsize_t    size3[3];
    size3[0] = scale;
    size3[1] = CxiFileHandlar::Instance()->para.dims3[1];
    size3[2] = CxiFileHandlar::Instance()->para.dims3[2];
	
    hsize_t    size1[1] = {scale};
    CxiFileHandlar::Instance()->para.dims3[0] = scale;

    CxiFileHandlar::Instance()->para.dims1[0] = scale;
	//dataset under instrument group
    extend(CxiFileHandlar::Instance()->para.instrument.energy_did, size1);
	/*groups detector and Image*/
	DETECTOR_LOOP {
        int width = global->detector[detID].image_nx%PRO == 0?
                    global->detector[detID].image_nx/PRO :
                    global->detector[detID].image_nx/PRO +1;
        hsize_t    snapshotSize [3] = {scale, width, width};
        extend(CxiFileHandlar::Instance()->para.instrument.detectors[detID].distance_did, size1);
        extend(CxiFileHandlar::Instance()->para.instrument.detectors[detID].data_did, snapshotSize);
        extend(CxiFileHandlar::Instance()->para.instrument.detectors[detID].snapshot_did, snapshotSize);
        extend(CxiFileHandlar::Instance()->para.image[detID].data_did, snapshotSize);
        extend(CxiFileHandlar::Instance()->para.image[detID].snapshot_did, snapshotSize);
	}
	/*SUB groups detecor*/
	// end of group instrument
	
	//start of LCLS
    extend(CxiFileHandlar::Instance()->para.lcls.machineTime_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.fiducial_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamCharge_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamL3Energy_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamPkCurrBC2_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosX_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosY_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngX_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngY_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.phaseCavityTime1_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.phaseCavityTime2_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge1_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge2_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.photon_energy_eV_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.photon_wavelength_A_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.f_11_ENRC_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.f_12_ENRC_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.f_21_ENRC_did, size1);
    extend(CxiFileHandlar::Instance()->para.lcls.f_22_ENRC_did, size1);

    extend(CxiFileHandlar::Instance()->para.lcls.evr41_did, size1);

	DETECTOR_LOOP {
        extend(CxiFileHandlar::Instance()->para.lcls.detector_positions_dids[detID], size1);
        extend(CxiFileHandlar::Instance()->para.lcls.detector_EncoderValues_dids[detID], size1);
	}	
	
    extend(CxiFileHandlar::Instance()->para.lcls.eventTimeString_did, size1);
	//end of LCLS
	//end of all possible fields, end of extending

}

template <class T> void CXI::saveEleIntoDataset1(hid_t & dataset_id, int position, T data){
    hid_t type;
    herr_t      status;
    hsize_t     count[1];
    count[0] = 1;
    hsize_t     offset[1];
    offset[0] = position;
    hsize_t     stride[1];
    stride[0] = 1;
    hsize_t     block[1];
    block[0] = 1;

    hsize_t     mdims[1];
        mdims[0] = 1;


    hid_t memspace_id = H5Screate_simple (1, mdims, NULL);
    if(memspace_id < 0){
        printf("memspace_id  Error in line %d\n",__LINE__);
    }
    hid_t ndataspace_id = H5Dget_space (dataset_id);
    if(ndataspace_id < 0){
        printf("dataspace_id Error in line %d\n",__LINE__);
    }
    if(typeid(T) == typeid(int*))
        type = H5T_NATIVE_INT32;
    if(typeid(T) == typeid(double*))
        type = H5T_NATIVE_DOUBLE;
    if(typeid(T) == typeid(char*))
        type = H5Tcopy(H5T_C_S1);
    if(typeid(T) == typeid(unsigned int*))
        type = H5T_NATIVE_UINT32;

    status = H5Sselect_hyperslab (ndataspace_id, H5S_SELECT_SET, offset,
                                  stride, count, block);

    if(status < 0){
        printf("H5Sselect_hyperslab Error in line %d\n",__LINE__);
    }

    //printf("save h5dDwrite %d %d %d %d %f \n",dataset_id, type, memspace_id,
    //       ndataspace_id, data);
    status = H5Dwrite (dataset_id, type, memspace_id,
                       ndataspace_id, H5P_DEFAULT, data);
    if(status < 0){
        printf("Error in line %d\n",__LINE__);
    }
    H5Sclose(memspace_id);
    H5Sclose(ndataspace_id);
}


void CXI::saveArrayIntoDataset3( hid_t& dataset_id, int position, int16_t * data , hsize_t *   size){
    //printf("saveArrayIntoDataset3 start \n");
	herr_t      status;
    hsize_t     count[3];
    count[0] = 1;
    count[1]=1;
    count[2] = 1;
    /* size of subset in the file */
    hsize_t     offset[3];
    offset[0] = position;
    offset[1] = 0;
    offset[2] = 0;

    hsize_t     stride[3];
    stride[0] = 1;
    stride[1] = 1;
    stride[2] =1;
    hsize_t     block[3];
    block[0] = 1;
    block[1]=size[1];
    block[2] = size[2];

    hsize_t     mdims[3];
        mdims[0] = 1;
        mdims[1]=size[1];
        mdims[2] = size[2];

    hid_t memspace_id = H5Screate_simple (3, mdims, NULL);
	if(memspace_id < 0){
	    printf("memspace_id Error in line %d\n",__LINE__);
    }
  	hid_t ndataspace_id = H5Dget_space (dataset_id);
 	if(ndataspace_id < 0){
    	printf("dataspace_id Error in line %d\n",__LINE__);
  	}

    status = H5Sselect_hyperslab (ndataspace_id, H5S_SELECT_SET, offset,
                                  stride, count, block);
 	if(status < 0){
    	printf("H5Sselect_hyperslab Error in line %d\n",__LINE__);
  	}
    if (data == NULL)
        printf("data is null \n");
    else
        printf("what a data %d %d\n",data, data[0]);
    printf("all h5dwrite %d %d %d %d \n", dataset_id,  memspace_id,
           ndataspace_id, data);
    status = H5Dwrite (dataset_id, H5T_STD_I16LE, memspace_id,
                       ndataspace_id, H5P_DEFAULT, data);
  if(status < 0){
    printf("Error in line %d\n",__LINE__);
  }
  H5Sclose(memspace_id);
  H5Sclose(ndataspace_id);
  //printf("saveArrayIntoDataset3 end \n");
}


void CXI :: writeCxi(cEventData *info, cGlobal *global ){
	/*
	 *	Create filename based on date, time and fiducial for this image
	 */
    printf("Writting CXI file for frame number %ld \n", global->frameNumber);
	char outfile[1024];
    long frameNum = global->frameNumber;
	strcpy(outfile, info->eventname);
    /* extend datasets if necessary	*/
    if (CxiFileHandlar::Instance()->para.dims3[0] <= frameNum){
        //printf("Extending datasets %d %d \n\n",CxiFileHandlar::Instance()->para.dims3[0],frameNum);
        extendAllDataset(frameNum + 1, global);
    }
	pthread_mutex_lock(&global->framefp_mutex);
    global->frameNumber ++;
	fprintf(global->cleanedfp, "r%04u/%s, %i, %g, %g, %g, %g, %g\n",global->runNumber, info->eventname, info->nPeaks, info->peakNpix, info->peakTotal, info->peakResolution, info->peakResolutionA, info->peakDensity);
	pthread_mutex_unlock(&global->framefp_mutex);
	
    double en = info->photonEnergyeV * 1.60217646e-19;
    //printf("save engergydid %d %l %f\n", CxiFileHandlar::Instance()->para.instrument.energy_did,frameNum,en);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.instrument.energy_did,frameNum,&en);
	/*Save assembled image under image groups*/
    if (global->saveAssembled){
        //printf("save assembled! \n");

		DETECTOR_LOOP {
            //printf("para save %d %d %d %d %d %d\n",CxiFileHandlar::Instance()->para.image[detID].data_did,frameNum, info->detector[detID].image,CxiFileHandlar::Instance()->para.dims3,detID, info->threadID);
            int width = CxiFileHandlar::Instance()->para.snabdims3[2];
            hsize_t    snapshotSize [3] = {1, width, width};
            saveArrayIntoDataset3(CxiFileHandlar::Instance()->para.image[detID].data_did,frameNum,info->detector[detID].image, CxiFileHandlar::Instance()->para.dims3);
            int16_t * shot;
            shot = (int16_t *) malloc(sizeof(int16_t) * width * width);
            getSnapshot<int16_t *>(info->detector[detID].image, CxiFileHandlar::Instance()->para.dims3[2],PRO,width,shot);
            //printf("para save %d %d %d %d %d %d\n",CxiFileHandlar::Instance()->para.image[detID].data_did,frameNum,shot, snapshotSize[0],shot, shot[0]);
            saveArrayIntoDataset3(CxiFileHandlar::Instance()->para.image[detID].snapshot_did,frameNum,shot, snapshotSize);
            free(shot);

        }
	}
   // printf("global->saveRaw: %d\n", global->saveRaw);
    if(global->saveRaw) {
        //printf("global->saveRaw: %d\n", global->saveRaw);
        DETECTOR_LOOP{
            //printf("para save %d %d %d %d %d %d\n",CxiFileHandlar::Instance()->para.instrument.detectors[detID].data_did,
                  // frameNum, info->detector[detID].corrected_data_int16,CxiFileHandlar::Instance()->para.dims3,detID, info->threadID);
            int width = CxiFileHandlar::Instance()->para.snabdims3[2];
            hsize_t    snapshotSize [3] = {1, width, width};
            saveArrayIntoDataset3(CxiFileHandlar::Instance()->para.instrument.detectors[detID].data_did,frameNum, info->detector[detID].corrected_data_int16,CxiFileHandlar::Instance()->para.dims3);
            int16_t * shot;
            shot = (int16_t*) malloc(sizeof(int16_t) * width * width);
            getSnapshot<int16_t *>(info->detector[detID].corrected_data_int16, CxiFileHandlar::Instance()->para.dims3[2] ,PRO,width, shot);
            //printf("para save %d %d %d %d %d %d\n",CxiFileHandlar::Instance()->para.instrument.detectors[detID].snapshot_did,frameNum,shot, snapshotSize[0], shot[0],detID);
            saveArrayIntoDataset3(CxiFileHandlar::Instance()->para.instrument.detectors[detID].snapshot_did,frameNum,shot, snapshotSize);
            free(shot);

        }
    }

    /*Write LCLS informations*/
    DETECTOR_LOOP{
        saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.detector_positions_dids[detID], frameNum,&global->detector[detID].detectorZ);
        saveEleIntoDataset1<long*>(CxiFileHandlar::Instance()->para.lcls.detector_EncoderValues_dids[detID], frameNum,&detID);
    }
    saveEleIntoDataset1<int*>(CxiFileHandlar::Instance()->para.lcls.machineTime_did,frameNum,&info->seconds);
    saveEleIntoDataset1<unsigned int*>(CxiFileHandlar::Instance()->para.lcls.fiducial_did,frameNum,&info->fiducial);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamCharge_did,frameNum,&info->fEbeamCharge);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamL3Energy_did,frameNum,&info->fEbeamL3Energy);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngX_did,frameNum,&info->fEbeamLTUAngX);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngY_did,frameNum,&info->fEbeamLTUAngY);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosX_did,frameNum,&info->fEbeamLTUPosX);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosY_did,frameNum,&info->fEbeamLTUPosY);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.ebeamPkCurrBC2_did,frameNum,&info->fEbeamPkCurrBC2);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.phaseCavityTime1_did, frameNum,&info->phaseCavityTime1);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.phaseCavityTime2_did,frameNum,&info->phaseCavityTime2);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge1_did,frameNum,&info->phaseCavityCharge1);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge2_did,frameNum,&info->phaseCavityCharge2);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.photon_energy_eV_did,frameNum,&info->photonEnergyeV);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.photon_wavelength_A_did,frameNum,&info->wavelengthA);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.f_11_ENRC_did,frameNum,&info->gmd11);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.f_12_ENRC_did,frameNum, &info->gmd12);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.f_21_ENRC_did,frameNum,&info->gmd21);
    saveEleIntoDataset1<double*>(CxiFileHandlar::Instance()->para.lcls.f_22_ENRC_did,frameNum,&info->gmd22);
    int LaserOnVal = (info->laserEventCodeOn)?1:0;
    saveEleIntoDataset1<int*>(CxiFileHandlar::Instance()->para.lcls.evr41_did,frameNum,&LaserOnVal);
    char* timestr;
    time_t eventTime = info->seconds;
    timestr = ctime(&eventTime);
    saveEleIntoDataset1<char*>(CxiFileHandlar::Instance()->para.lcls.eventTimeString_did,frameNum,timestr);
}

