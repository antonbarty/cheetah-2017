//
//  sacla-hdf5-reader.h
//  cheetah-ab
//
//  Created by Anton Barty on 7/02/2014.
//  Copyright (c) 2014 Anton Barty. All rights reserved.
//

#ifndef __cheetah_ab__sacla_hdf5_reader__
#define __cheetah_ab__sacla_hdf5_reader__

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>


/*
 *  Structure for holding SACLA HDF5 metadata
 */
typedef struct {
    // File info
    char    filename[1024];
    hid_t   file_id;
    
    // Run information
    int     nruns;
    char    **run_string;
    int64_t *run_number;
    int64_t *start_tag_number;
    int64_t *end_tag_number;
    float   *photon_energy_in_eV;
    float   *photon_wavelength_in_nm;
    
    
    // Device objects within a run
    long    ndetectors;
    char    **detector_name;
    
    
    // Detector event tags within a run
    long    nevents;
    char    **event_name;
    
} SACLA_h5_info_t;


/*
 *  Prototypes for functions written to read SACLA HDF5 data
 */
int SACLA_HDF5_ReadHeader(const char*, SACLA_h5_info_t*);
int SACLA_HDF5_Read2dDetectorFields(SACLA_h5_info_t*, long);
int SACLA_HDF5_ReadEventTags(SACLA_h5_info_t*, long);
int SACLA_HDF5_ReadImageRaw(SACLA_h5_info_t*, long, long, float*, long);
int SACLA_HDF5_cleanup(SACLA_h5_info_t*);

    

#endif /* defined(__cheetah_ab__sacla_hdf5_reader__) */
