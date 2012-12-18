/*
 *	File: cxiPara.h
 *  cheetah
 *
 *  Created by Jing Liu on 05/11/12.
 *  Copyright 2012 Biophysics & TDB @ Uppsala University. All rights reserved.
 *
 */
#ifndef CXIPARA_H
#define CXIPARA_H


#include <hdf5.h>
#include "DetectorStruct.h"



typedef struct{
	hid_t instrument_gid;
	hid_t source_gid;
	hid_t energy_did;
    Detector *detectors;
}Instrument;


typedef struct{
	hid_t LCLS_gid;
	hid_t machineTime_did;
	hid_t fiducial_did;
	hid_t ebeamCharge_did;
	hid_t ebeamL3Energy_did;
	hid_t ebeamPkCurrBC2_did;
    hid_t ebeamLTUPosX_did;
	hid_t ebeamLTUPosY_did;
	hid_t ebeamLTUAngX_did;
	hid_t ebeamLTUAngY_did;
	hid_t phaseCavityTime1_did;
	hid_t phaseCavityTime2_did;
	hid_t phaseCavityCharge1_did;
	hid_t phaseCavityCharge2_did;
	hid_t photon_energy_eV_did;
	hid_t photon_wavelength_A_did;
	hid_t f_11_ENRC_did;
	hid_t f_12_ENRC_did;
	hid_t f_21_ENRC_did;
	hid_t f_22_ENRC_did;
	hid_t evr41_did;
	hid_t *detector_positions_dids;
	hid_t *detector_EncoderValues_dids;
	hid_t eventTimeString_did;
}LCLS;

typedef struct{
	hid_t *radialAverage;
	hid_t *radialAverageCounter;
}Radial;

typedef struct{
	hid_t fileId;
	hid_t entry_gid_1;
	hid_t data_gid_1;
    Instrument instrument;
    Image* image;
    LCLS lcls;
	int xci_version;
    int pro;
    hsize_t dims3 [3];
    hsize_t dims1 [1];
    hsize_t snabdims3[3];
}Cxi_para;

#endif
