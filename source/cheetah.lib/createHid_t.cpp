#include "createHid_t.h"
#include "staticPara.h"
#define RANK3 3
#define RANK1 1
#define INIT 10
#define POR CxiFileHandlar::Instance()->para.pro
#define SCALE 8

hid_t create_dataset3(char *name, hid_t loc, hsize_t dims3[3], hsize_t maxdims3[3], hid_t dataType){
    //printf("create dataset3 %s, %d, %d, %d, %d,%d, %d \n\n", name,loc,dims3[0], dims3[1],maxdims3[0],maxdims3[1],maxdims3[2]);
    hid_t dataspace_id = H5Screate_simple(RANK3, dims3, maxdims3);
    //printf("%d \n",__LINE__);
    if (dataspace_id < 0)
        printf("error dataspace_id %d\n", __LINE__);
    /* Modify dataset creation properties, i.e. enable chunking  */
    hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);
          printf("%d \n",__LINE__);
    if (cparms < 0)
        printf("error cparms %d\n", __LINE__);
    herr_t err = H5Pset_chunk (cparms, RANK3, dims3);
      printf("%d \n",__LINE__);
    if (err < 0)
        printf("set chunk printf %d\n", __LINE__);

   // hid_t dataset_id = H5Dcreate(loc, name, H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);
    hid_t dataset_id = H5Dcreate(loc, name, dataType, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);
    //     printf("%d \n",__LINE__);
    if (dataset_id <0)
        printf("Creating printf in dataset %s, %s", name, loc);
    return dataset_id;
}


hid_t create_group(char* name, hid_t loc, hid_t file){
    hid_t temp =  H5Gcreate(loc,name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (temp < 0 ){
        printf("Creating: Couldn't create group instrument_1\n");
        H5Fclose(file);
        return -1;
    }
    return temp;
}

hid_t create_string_dataset(char * name, hid_t loc){

    hid_t    datatype = H5Tcopy(H5T_C_S1);

    hsize_t dims[1];
    dims[0] = INIT;
    hsize_t maxdims[1];
    maxdims[0] = H5S_UNLIMITED;
    herr_t err;
    hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);

    hid_t dataspace_1 = H5Screate_simple(RANK1, dims, maxdims);

    /* Modify dataset creation properties, i.e. enable chunking  */

    err = H5Pset_chunk (cparms, 1, dims);

    if (err < 0)
        printf("set chunk printf %d\n", __LINE__);
    hid_t temp =  H5Dcreate(loc, name, datatype, dataspace_1, H5P_DEFAULT, cparms, H5P_DEFAULT);
    if (temp < 0 ){
        printf("create dataset printf %s %s\n", name, loc);
    }
    return temp;

}

hid_t create_dataset1(char* name, hid_t loc, hid_t dataType){
    hsize_t dims[1];
    dims[0] = INIT;
    hsize_t maxdims[1];
    maxdims[0] = H5S_UNLIMITED;
    herr_t err;
    hid_t cparms = H5Pcreate (H5P_DATASET_CREATE);

    hid_t dataspace_1 = H5Screate_simple(RANK1, dims, maxdims);

    /* Modify dataset creation properties, i.e. enable chunking  */

    err = H5Pset_chunk (cparms, 1, dims);

    if (err < 0)
        printf("set chunk printf %d\n", __LINE__);
    hid_t temp =  H5Dcreate(loc, name, dataType, dataspace_1, H5P_DEFAULT, cparms, H5P_DEFAULT);
    if (temp < 0 ){
        printf("create dataset printf %s %s\n", name, loc);
    }
    return temp;
}

void write_version(const hid_t root, const int version){
	hsize_t dims[1];
	dims[0] = 1;
	hid_t dataspace_1 = H5Screate_simple(1, dims, dims);
	hid_t temp =  H5Dcreate(root, "version", H5T_NATIVE_INT32, dataspace_1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	herr_t status = H5Dwrite (temp, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, &version);
	if (status < 0){
		printf("Write version error ! \n");		
	}
	printf("Write version 130 \n");
}

void close(){
    printf("*************************************\n Close Event!!!!\n\n\n\n\n\n");

	int n_ids;
	hid_t ids[256];
    int i = 0;
    n_ids = H5Fget_obj_ids(CxiFileHandlar::Instance()->para.fileId, H5F_OBJ_ALL, 256, ids);
    for (; i<n_ids; i++ ) {
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
	H5Fclose(CxiFileHandlar::Instance()->para.fileId);
    printf("************************************\n\n\n\n");
    usleep(100000);
}
void create (cGlobal *global){

	hid_t dataspace_1;
	hid_t datasapce_3;
    CxiFileHandlar::Instance()->para.dims1[0] =INIT;
	hsize_t maxdims3[3];
    CxiFileHandlar::Instance()->para.pro = SCALE;
    printf("Currnent CXI filename in create at cxi_para %s in line %d %d\n", global->currentCXIFileName , __LINE__, CxiFileHandlar::Instance()->para.fileId);

    CxiFileHandlar::Instance()->para.fileId = H5Fcreate(global->currentCXIFileName,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    if (CxiFileHandlar::Instance()->para.fileId < 0)
    {
        printf("printf: cannot create file with filename %s \n", global->currentCXIFileName);
    }
	write_version(CxiFileHandlar::Instance()->para.fileId ,130);
    CxiFileHandlar::Instance()->para.entry_gid_1 = H5Gcreate(CxiFileHandlar::Instance()->para.fileId, "entry_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (CxiFileHandlar::Instance()->para.entry_gid_1 < 0 ){
        printf("Creating: Couldn't create group entry_1\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return ;
	}

    CxiFileHandlar::Instance()->para.data_gid_1 = H5Gcreate(CxiFileHandlar::Instance()->para.entry_gid_1, "data_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (CxiFileHandlar::Instance()->para.data_gid_1 < 0 ){
        printf("Creating: Couldn't create group entry_1\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
//printf("%d \n",__LINE__);
	/*start of create instrument group*/
    CxiFileHandlar::Instance()->para.instrument.instrument_gid =  H5Gcreate(CxiFileHandlar::Instance()->para.entry_gid_1, "instrument_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (CxiFileHandlar::Instance()->para.instrument.instrument_gid  < 0 ){
        printf("Creating: Couldn't create group instrument_1\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    //printf("%d \n",__LINE__);
    CxiFileHandlar::Instance()->para.instrument.source_gid = H5Gcreate(CxiFileHandlar::Instance()->para.instrument.instrument_gid, "source_1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (CxiFileHandlar::Instance()->para.instrument.source_gid < 0 ){
        printf("Creating: Couldn't create group source_1\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
   // printf("%d \n",__LINE__);
    CxiFileHandlar::Instance()->para.instrument.energy_did = create_dataset1("energy", CxiFileHandlar::Instance()->para.instrument.source_gid, H5T_NATIVE_DOUBLE);
    if (CxiFileHandlar::Instance()->para.instrument.energy_did < 0 ){
        printf("Creating: Couldn't create dataset energy\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
	
   // printf("%d \n",__LINE__);
    CxiFileHandlar::Instance()->para.instrument.detectors = (Detector *) malloc (sizeof(Detector) * global->nDetectors);
    if (CxiFileHandlar::Instance()->para.instrument.detectors < 0){
        printf("Malloc printf for detectors");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
//printf("%d \n",__LINE__);
	char detectorName[1024];
	/*SUB groups detecor*/
	DETECTOR_LOOP {
		sprintf(detectorName,"detector_%ld",detID+1);
        CxiFileHandlar::Instance()->para.instrument.detectors[detID].detector_gid =  H5Gcreate(CxiFileHandlar::Instance()->para.instrument.instrument_gid, detectorName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        //printf("detector group id %d \n\n", CxiFileHandlar::Instance()->para.instrument.detectors[detID].detector_gid);
        if (CxiFileHandlar::Instance()->para.instrument.detectors[detID].detector_gid < 0 ){
            printf("Creating: Couldn't create group source_1\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
            //return;
        }
        //printf("%d \n",__LINE__);
        CxiFileHandlar::Instance()->para.instrument.detectors[detID].distance_did = create_dataset1("distance", CxiFileHandlar::Instance()->para.instrument.detectors[detID].detector_gid,H5T_NATIVE_DOUBLE);
        if (CxiFileHandlar::Instance()->para.instrument.detectors[detID].distance_did <0){
            printf("Creating printf in dataset distance of detetor %ld \n", detID);
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
//printf("%d \n",__LINE__);
        //printf("detector image nx %d init %d por %d\n", global->detector[detID].image_nx, INIT, POR);
        CxiFileHandlar::Instance()->para.dims3 [0] = INIT;
        CxiFileHandlar::Instance()->para.dims3 [1] = global->detector[detID].image_nx;
        CxiFileHandlar::Instance()->para.dims3 [2] = global->detector[detID].image_nx;
        //printf("%d \n",__LINE__);
        maxdims3[0] = H5S_UNLIMITED;
        maxdims3[1] = global->detector[detID].image_nx;
        maxdims3[2] = global->detector[detID].image_nx;
        //printf("%d \n",__LINE__);
        char rn [200];
        sprintf(rn,"data_%ld", detID + 1);
        printf("%s %d maxdims0 %d\n",rn,__LINE__, maxdims3[0]);
        CxiFileHandlar::Instance()->para.instrument.detectors[detID].data_did = create_dataset3(rn, CxiFileHandlar::Instance()->para.instrument.detectors[detID].detector_gid, CxiFileHandlar::Instance()->para.dims3, maxdims3,H5T_STD_I16LE);
       /// printf("%d \n",__LINE__);
        if (CxiFileHandlar::Instance()->para.instrument.detectors[detID].data_did <0){
            printf("Creating: Couldn't create dataset data under detector\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
       // printf("%d \n",__LINE__);
        CxiFileHandlar::Instance()->para.snabdims3 [0] = INIT;
        CxiFileHandlar::Instance()->para.snabdims3 [1] = global->detector[detID].image_nx/POR;
        CxiFileHandlar::Instance()->para.snabdims3 [2] = global->detector[detID].image_nx/POR;
        maxdims3[0] = H5S_UNLIMITED;
        maxdims3[1] = global->detector[detID].image_nx/POR;
        maxdims3[2]= global->detector[detID].image_nx/POR;
         // printf("%d \n",__LINE__);
        char sn [200];
        sprintf(sn,"thumbnail_%ld", detID + 1);
         printf("%d \n",__LINE__);
        CxiFileHandlar::Instance()->para.instrument.detectors[detID].snapshot_did = create_dataset3(sn, CxiFileHandlar::Instance()->para.instrument.detectors[detID].detector_gid, CxiFileHandlar::Instance()->para.snabdims3, maxdims3,H5T_STD_I16LE);
        if (CxiFileHandlar::Instance()->para.instrument.detectors[detID].snapshot_did <0){
            printf("Creating: Couldn't create dataset snapshot under detector\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
    }
    //printf("%d \n",__LINE__);

    //herr_t stu = H5Lcreate_soft("/entry_1/instrument_1/detector_1/data_1", CxiFileHandlar::Instance()->para.fileId, "/data_1/data",0,0);
    //if (stu < 0){
     //   printf("softlink create error %d\n", __LINE__);
    //}
	/*end of create detector groups*/
	/*end of create instrument group*/


	/*start of image groups*/
    CxiFileHandlar::Instance()->para.image = (Image *) malloc (sizeof(Image) * global->nDetectors);
    if (CxiFileHandlar::Instance()->para.image < 0){
        printf("Malloc printf for detectors\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
    //	return;
	}

	char imageName[1023];
	DETECTOR_LOOP {
		sprintf(imageName,"image_%ld",detID+1);
	printf("Imge group image %s %d\n",imageName,__LINE__);
        CxiFileHandlar::Instance()->para.image[detID].image_gid =  H5Gcreate(CxiFileHandlar::Instance()->para.entry_gid_1, imageName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (CxiFileHandlar::Instance()->para.image[detID].image_gid < 0 ){
            printf("Creating: Couldn't create group image\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
        CxiFileHandlar::Instance()->para.dims3 [0] = INIT;
        CxiFileHandlar::Instance()->para.dims3 [1] = global->detector[detID].image_nx;
        CxiFileHandlar::Instance()->para.dims3 [2] = global->detector[detID].image_nx;
        maxdims3[0] = H5S_UNLIMITED;
        maxdims3[1] = global->detector[detID].image_nx;
        maxdims3[2] = global->detector[detID].image_nx;
        char rn [200];
        sprintf(rn,"data_%ld", detID + 1);
printf("Imge group image %s %d\n",rn,__LINE__);
        CxiFileHandlar::Instance()->para.image[detID].data_did = create_dataset3(rn,CxiFileHandlar::Instance()->para.image[detID].image_gid, CxiFileHandlar::Instance()->para.dims3, maxdims3, H5T_STD_I16LE);
        if (CxiFileHandlar::Instance()->para.image[detID].data_did < 0 ){
            printf("Creating : Couldn't create dataset under image\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
        }

        CxiFileHandlar::Instance()->para.snabdims3 [0] = INIT;
        CxiFileHandlar::Instance()->para.snabdims3 [1] = global->detector[detID].image_nx/POR;
        CxiFileHandlar::Instance()->para.snabdims3 [2] = global->detector[detID].image_nx/POR;

        maxdims3[0] = H5S_UNLIMITED;
        maxdims3[1] = global->detector[detID].image_nx/POR;
        maxdims3[2] = global->detector[detID].image_nx/POR;
        char sn [200];
        sprintf(sn,"thumbnail_%ld", detID + 1);
printf("Imge group image %s %d\n",sn,__LINE__);
        CxiFileHandlar::Instance()->para.image[detID].snapshot_did = create_dataset3(sn, CxiFileHandlar::Instance()->para.image[detID].image_gid, CxiFileHandlar::Instance()->para.snabdims3, maxdims3,H5T_STD_I16LE);
        if (CxiFileHandlar::Instance()->para.image[detID].snapshot_did < 0){
            printf("Creating: Couldn't create dataset snapshot under detector \n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
    }
    //printf("%d \n",__LINE__);
    //herr_t hdf_printf = H5Lcreate_soft( "/entry_1/image_1/source_1", CxiFileHandlar::Instance()->para.fileId, "/entry_1/instrument_1/source_1",0,0);
    //if (hdf_printf < 0){
    //    printf("softlink create error %d\n", __LINE__);
    //}
	/*end of image groups*/

	/*start of LCLS group*/
    CxiFileHandlar::Instance()->para.lcls.LCLS_gid = H5Gcreate(CxiFileHandlar::Instance()->para.fileId, "LCLS", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (CxiFileHandlar::Instance()->para.lcls.LCLS_gid < 0 ){
            printf("Creating: Couldn't create group LCLS\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
    CxiFileHandlar::Instance()->para.lcls.machineTime_did = create_dataset1("machineTime", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_INT32 );
    if (CxiFileHandlar::Instance()->para.lcls.machineTime_did < 0 ){
        printf("Creating: Couldn't create dataset machineTime\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
    //	return;
	}

    CxiFileHandlar::Instance()->para.lcls.fiducial_did = create_dataset1("fiducial", CxiFileHandlar::Instance()->para.lcls.LCLS_gid ,H5T_NATIVE_INT32);
    if (CxiFileHandlar::Instance()->para.lcls.fiducial_did < 0 ){
        printf("Creating: Couldn't create dataset fiducial\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
    //	return;
	}


    CxiFileHandlar::Instance()->para.lcls.ebeamCharge_did = create_dataset1("ebeamCharge", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamCharge_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamCharge\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}


    CxiFileHandlar::Instance()->para.lcls.ebeamL3Energy_did = create_dataset1("ebeamL3Energy", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamL3Energy_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamL3Energy\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.ebeamPkCurrBC2_did = create_dataset1("ebeamPkCurrBC2", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamPkCurrBC2_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamL3Energy\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosX_did = create_dataset1("ebeamLTUPosX", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosX_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamLTUPosX\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
    }
    CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosY_did = create_dataset1("ebeamLTUPosY", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamLTUPosY_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamLTUPosY\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngX_did = create_dataset1("ebeamLTUAngX", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngX_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamLTUAngX\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngY_did = create_dataset1("ebeamLTUAngY", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.ebeamLTUAngY_did < 0 ){
        printf("Creating: Couldn't create dataset ebeamLTUAngY \n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.phaseCavityTime1_did = create_dataset1("phaseCavityTime1", CxiFileHandlar::Instance()->para.lcls.LCLS_gid ,H5T_NATIVE_DOUBLE);
    if (CxiFileHandlar::Instance()->para.lcls.phaseCavityTime1_did < 0 ){
        printf("Creating: Couldn't create dataset phaseCavityTime1\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
//printf("%d \n",__LINE__);

    CxiFileHandlar::Instance()->para.lcls.phaseCavityTime2_did = create_dataset1("phaseCavityTime2", CxiFileHandlar::Instance()->para.lcls.LCLS_gid , H5T_NATIVE_DOUBLE);
    if (CxiFileHandlar::Instance()->para.lcls.phaseCavityTime1_did < 0 ){
        printf("Creating: Couldn't create dataset phaseCavityTime2\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge1_did = create_dataset1("phaseCavityCharge1", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge1_did < 0 ){
        printf("Creating: Couldn't create dataset phaseCavityCharge1\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge2_did = create_dataset1("phaseCavityCharge2", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.phaseCavityCharge2_did < 0 ){
        printf("Creating: Couldn't create dataset phaseCavityCharge2\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
//printf("%d \n",__LINE__);

    CxiFileHandlar::Instance()->para.lcls.photon_energy_eV_did = create_dataset1("photon_energy_eV", CxiFileHandlar::Instance()->para.lcls.LCLS_gid ,H5T_NATIVE_DOUBLE);
    if (CxiFileHandlar::Instance()->para.lcls.photon_energy_eV_did < 0 ){
        printf("Creating: Couldn't create dataset photon_energy_eV\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.photon_wavelength_A_did = create_dataset1("photon_wavelength_A", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.photon_wavelength_A_did < 0 ){
        printf("Creating: Couldn't create dataset  photon_wavelength_A\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}

    CxiFileHandlar::Instance()->para.lcls.f_11_ENRC_did = create_dataset1("f_11_ENRC", CxiFileHandlar::Instance()->para.lcls.LCLS_gid ,H5T_NATIVE_DOUBLE);
    if (CxiFileHandlar::Instance()->para.lcls.f_11_ENRC_did < 0 ){
        printf("Creating: Couldn't create dataset f_11_ENRC\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.f_12_ENRC_did = create_dataset1("f_12_ENRC", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.f_12_ENRC_did < 0 ){
        printf("Creating: Couldn't create dataset f_12_ENRC\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
//printf("%d \n",__LINE__);
    CxiFileHandlar::Instance()->para.lcls.f_21_ENRC_did = create_dataset1("f_21_ENRC", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.f_21_ENRC_did < 0 ){
        printf("Creating: Couldn't create dataset f_21_ENRC\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.f_22_ENRC_did = create_dataset1("f_22_ENRC", CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE );
    if (CxiFileHandlar::Instance()->para.lcls.f_22_ENRC_did < 0 ){
        printf("Creating: Couldn't create dataset f_22_ENRC\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.evr41_did = create_dataset1("evr41", CxiFileHandlar::Instance()->para.lcls.LCLS_gid , H5T_NATIVE_INT);
    if (CxiFileHandlar::Instance()->para.lcls.evr41_did < 0 ){
        printf("Creating: Couldn't create dataset evr41\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.eventTimeString_did = create_string_dataset("eventTimeString", CxiFileHandlar::Instance()->para.lcls.LCLS_gid );
    if (CxiFileHandlar::Instance()->para.lcls.eventTimeString_did < 0 ){
        printf("Creating: Couldn't create dataset eventTimeString\n");
        H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //return;
	}
    CxiFileHandlar::Instance()->para.lcls.detector_positions_dids = (hid_t *) malloc(sizeof (hid_t) * global->nDetectors);
    CxiFileHandlar::Instance()->para.lcls.detector_EncoderValues_dids = (hid_t *) malloc(sizeof (hid_t) * global->nDetectors);
	char detName[1023];
	DETECTOR_LOOP {
		sprintf(detName,"detector%li-position", detID +1);
        CxiFileHandlar::Instance()->para.lcls.detector_positions_dids[detID] = create_dataset1(detName, CxiFileHandlar::Instance()->para.lcls.LCLS_gid ,H5T_NATIVE_DOUBLE);
        if (CxiFileHandlar::Instance()->para.lcls.detector_positions_dids[detID] < 0 ){
            printf("Creating: Couldn't create dataset detector_positions\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
		sprintf(detName,"detector%li-EncoderValue", detID +1);
        CxiFileHandlar::Instance()->para.lcls.detector_EncoderValues_dids[detID] = create_dataset1(detName, CxiFileHandlar::Instance()->para.lcls.LCLS_gid,H5T_NATIVE_DOUBLE);
        if (CxiFileHandlar::Instance()->para.lcls.detector_EncoderValues_dids[detID] < 0 ){
            printf("Creating: Couldn't create dataset detector_EncoderValues\n");
            H5Fclose(CxiFileHandlar::Instance()->para.fileId);
        //	return;
		}
	}

	/*end of LCLS group*/

	/*close all groups, dataspace, datatype, attributes.*/
	int n_ids;
	hid_t ids[256];
    int i = 0;
    n_ids = H5Fget_obj_ids(CxiFileHandlar::Instance()->para.fileId, H5F_OBJ_ALL, 256, ids);
    for (; i<n_ids; i++ ) {
		hid_t id;
		H5I_type_t type;
		id = ids[i];
		type = H5Iget_type(id);
		if ( type == H5I_GROUP ) H5Gclose(id);
		//if ( type == H5I_DATASET ) H5Dclose(id);
		if ( type == H5I_DATATYPE ) H5Tclose(id);
		if ( type == H5I_DATASPACE ) H5Sclose(id);
		if ( type == H5I_ATTR ) H5Aclose(id);
	}
	
    //return para;
}


