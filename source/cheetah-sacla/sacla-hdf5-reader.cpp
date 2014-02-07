//
//  sacla-hdf5-reader.cpp
//  cheetah-ab
//
//  Created by Anton Barty on 7/02/2014.
//  Copyright (c) 2014 Anton Barty. All rights reserved.
//

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "sacla-hdf5-reader.h"


/*
 *  Function for parsing SACLA HDF5 metadata
 */
int SACLA_HDF5_ReadHeader(const char *filename, SACLA_h5_info_t *result) {
    
    char    h5field[1024];
	hid_t   dataset_id;
	hid_t   dataspace_id;
	hid_t   datatype_id;
	H5T_class_t dataclass;
    
    
    // Does the file exist?
    FILE *fp = fopen(filename, "r");
    if(fp) {
        fclose(fp);
    }
    else {
		printf("ERROR: File does not exist %s\n",filename);
		exit(1);
    }
    
    
    // Open the HDF5 file
    hid_t   file_id;
	printf("Filename: %s\n", filename);
	file_id = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
	if(file_id < 0){
		printf("ERROR: Could not open HDF5 file %s\n",filename);
		exit(1);
	}
    result->file_id = file_id;
    strcpy(result->filename, filename);
    
    
	// Determine how many runs are contained in this HDF5 file
    // (So far I have only seen one run per file, but there appears to be scope for more than one run per HDF5 file..??)
	int     ndims;
    int     nruns;
	hsize_t dims[3];
	dataset_id = H5Dopen(file_id, "file_info/run_number_list", NULL);
	dataspace_id = H5Dget_space(dataset_id);
	ndims = H5Sget_simple_extent_ndims(dataspace_id);
	H5Sget_simple_extent_dims(dataspace_id,dims,NULL);
	nruns = (int) dims[0];
    result->nruns = nruns;
	H5Sclose(dataspace_id);
	H5Dclose(dataset_id);
	printf("Number of runs in SACLA HDF5 file: %i\n", result->nruns);
    
    
    // Allocate storage space for information on each run
	result->run_number = (int64_t*) calloc(nruns, sizeof(int64_t));
	result->start_tag_number = (int64_t*) calloc(nruns, sizeof(int64_t));
	result->end_tag_number = (int64_t*) calloc(nruns, sizeof(int64_t));
    result->run_string = (char**) calloc(nruns, sizeof(char*));
    result->photon_energy_in_eV = (float*) calloc(nruns, sizeof(float));
    result->photon_wavelength_in_nm = (float*) calloc(nruns, sizeof(float));
	
    
    // Read list of run identifiers in this HDF5 file
	H5LTread_dataset(file_id, "file_info/run_number_list", H5T_NATIVE_INT64, result->run_number);
	for(long i=0; i<result->nruns; i++) {
        result->run_string[i] = (char*) calloc(1024, sizeof(char));
		sprintf(result->run_string[i], "run_%lli",result->run_number[i]);
	}
    
    
    // Collect start and end tag numbers for each run
	for(long i=0; i<nruns; i++) {
		sprintf(h5field, "%s/run_info/start_tag_number",result->run_string[i]);
		H5LTread_dataset(file_id, h5field, H5T_NATIVE_INT64, &result->start_tag_number[i]);
        
		sprintf(h5field, "%s/run_info/end_tag_number",result->run_string[i]);
		H5LTread_dataset(file_id, h5field, H5T_NATIVE_INT64, &result->end_tag_number[i]);
    }

	
    // Collect photon energy for each run
	for(long i=0; i<nruns; i++) {
        float   photon_wavelength_nm;
        sprintf(h5field, "%s/run_info/sacla_config/photon_energy_in_eV",result->run_string[i]);
		H5LTread_dataset(file_id, h5field, H5T_NATIVE_FLOAT, &result->photon_energy_in_eV[i]);
        photon_wavelength_nm = 1239.842 / result->photon_energy_in_eV[i];
        result->photon_wavelength_in_nm[i] = photon_wavelength_nm;
    }
    
	// Print out what we have found out about the contents of this HDF5 file
	for(long i=0; i<nruns; i++) {
        printf("Run number: %lli\n", result->run_number[i]);
		printf("Run field: %s\n", result->run_string[i]);
		printf("Start tag number: %lli\n", result->start_tag_number[i]);
		printf("End tag number: %lli\n", result->end_tag_number[i]);
    }
    
    return 1;
}

/*
 *  Function for collecting names of the 2D detectors saved in this run
 */
int SACLA_HDF5_Read2dDetectorFields(SACLA_h5_info_t *result, long run_index) {
    
    char    h5field[1024];
    char    tempstr[1024];
    long    counter = 0;
    
    
	hid_t   group;
	hsize_t	nfields;
	sprintf(h5field, "%s",result->run_string[run_index]);
	group = H5Gopen( result->file_id, h5field, NULL );
    
    
	H5Gget_num_objs(group, &nfields);
	printf("Number of fields in %s: %llu\n", h5field, nfields);
    result->detector_name = (char**) calloc(nfields, sizeof(char*));
    
    
    printf("Finding 2D detectors\n");
	for(long i=0; i< nfields; i++) {
		ssize_t r;
		r = H5Gget_objname_by_idx( group, i, tempstr, 1024);
		printf("%li : %s", i, tempstr);
        
        if(strncmp(tempstr,"detector_2d", 10)) {
            printf("\t(not a 2D detector)\n");
            continue;
        }
        printf("\t(2D detector)\n");
        result->detector_name[counter] = (char*) calloc(1024, sizeof(char));
        strcpy(result->detector_name[counter], tempstr);
        counter++;
	}
	H5Gclose(group);
	
    result->ndetectors = counter;
    printf("Number of unique 2D detectors: %li\n", result->ndetectors);
    
    return 1;
}


/*
 *  Function for reading event names in SACLA HDF5 file
 */
int SACLA_HDF5_ReadEventTags(SACLA_h5_info_t *result, long run_index) {
    
    char    h5field[1024];
    char    tempstr[1024];
    long    counter = 0;
    
    
	hid_t   group;
	hsize_t	nfields;
	sprintf(h5field, "%s/detector_2d_1",result->run_string[run_index]);
	group = H5Gopen( result->file_id, h5field, NULL );
    
    
	H5Gget_num_objs(group, &nfields);
	printf("Number of fields in %s: %llu\n", h5field, nfields);
    result->event_name = (char**) calloc(nfields, sizeof(char*));
    
    
    printf("Finding event names\n");
	for(long i=0; i< nfields; i++) {
		ssize_t r;
 		r = H5Gget_objname_by_idx( group, i, tempstr, 1024);
		printf("%li : %s", i, tempstr);
        
        if(strncmp(tempstr,"tag", 3)) {
            printf("\t(not a new event)\n");
            continue;
        }
        printf("\t(new event)\n");
        result->event_name[counter] = (char*) calloc(1024, sizeof(char));
        strcpy(result->event_name[counter], tempstr);
        counter++;
        
		herr_t result;
		result = H5LTfind_dataset ( group, tempstr );
		if(result==1) printf("(H5LTfind_dataset=true)\n"); else printf("(H5LTfind_dataset=false)\n");
        
        //int	nd;
        //strcat(tempstr, "/detector_data");
        //result = H5LTget_dataset_ndims( group, tempstr, &nd);
        //printf("ndims=%i\n", nd);
        
        //hsize_t         dims[3];
        //H5T_class_t     h5_class;
        //size_t          size;
        //result = H5LTget_dataset_info( group, tempstr, dims, &h5_class, &size);
        //for(long k=0; k<=nd; k++)
        //    printf("%lli\t",dims[k]);
        //printf("\n");
        
	}
	H5Gclose(group);
	
    result->nevents = counter;
    printf("Number of unique events: %li\n", result->nevents);
    
    
    return 1;
}

/*
 *  Function for reading all <n> 2D detectors into one massive 2D array (for passing to Cheetah or CrystFEL)
 */
int SACLA_HDF5_ReadImageRaw(SACLA_h5_info_t *header, long runID, long eventID, float *buffer, long offset) {
    
    char    h5field[1024];
    char    h5group[1024];
	hid_t   group;
    
    printf("Reading event: %s\n",header->event_name[eventID]);
    
    
    for(long moduleID=0; moduleID < header->ndetectors; moduleID++) {
        // Open the run group
        sprintf(h5group, "%s/%s/%s",header->run_string[runID],header->detector_name[moduleID], header->event_name[eventID]);
        group = H5Gopen( header->file_id, h5group, NULL );
        
        
        // Name for this part of the data
        sprintf(h5field, "detector_data");
        
        // Error check: does this group/field even exist (before we try to read it)?
        herr_t herr;
		herr = H5LTfind_dataset ( group, h5field );
        if(herr!=1) {
            printf("%s/%s : H5LTfind_dataset=false\n",h5group, h5field);
            H5Gclose(group);
            continue;
        }
        
        
        // Check dimensions of this data set
        //int	nd;
        //hsize_t         dims[3];
        //H5T_class_t     h5_class;
        //size_t          size;
        //strcat(tempstr, "/detector_data");
        //result = H5LTget_dataset_ndims( group, tempstr, &nd);
        //result = H5LTget_dataset_info( group, tempstr, dims, &h5_class, &size);
        //printf("ndims=%i\n", nd);
        //for(long k=0; k<=nd; k++)
        //    printf("%lli\t",dims[k]);
        //printf("\n");
        
        // Read the data set
        H5LTread_dataset_float( group, h5field, buffer + offset*moduleID);
        H5Gclose(group);
    }
    
    return 1;
}

/*
 *  Cleanup stale HDF5 references and close the file
 */
int SACLA_HDF5_cleanup(SACLA_h5_info_t *header) {
    
    std::cout << "Cleaning up HDF5 links\n";
	hid_t ids[256];
	long n_ids = H5Fget_obj_ids(header->file_id, H5F_OBJ_ALL, 256, ids);
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
		if ( type == H5I_DATATYPE )
			H5Dclose(id);
	}
	
	H5Fclose(header->file_id);

    return 1;
}

