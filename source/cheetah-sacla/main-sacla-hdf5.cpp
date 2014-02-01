//
//  main.cpp
//  cheetah-sacla-hdf5
//
//  Created by Anton Barty on 20/1/14.
//  Copyright (c) 2014 Anton Barty. All rights reserved.
//

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cheetah.h"


/*
 *  Structure for SACLA HDF5 metadata
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
    

    // Device objects within a run
    long    ndetectors;
    char    **detector_name;
    
    
    // Detector event tags within a run
    long    nevents;
    char    **event_name;
    
} SACLA_h5_info_t;



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



int main(int argc, const char * argv[])
{
	

	printf("Simple SACLA HDF5 file parser\n");
	printf("Anton Barty, 21 January 2014\n");
	
	// Input data file and Cheetah configuration file
	char	filename[1024];
	char	cheetahini[1024];
	// Take configuration from command line arguments
	//strcpy(filename,argv[0]);
	//strcpy(cheetahini,argv[1]);

	// Hard code for testing
	strcpy(filename,"/data/scratch/sacla/141945_each.h5");
	strcpy(cheetahini,"lys.ini");

	
	
	/*
	 *	Initialise Cheetah
	 */
	printf("Setting up Cheetah...\n");
	static uint32_t ntriggers = 0;
	static long frameNumber = 0;
	static cGlobal cheetahGlobal;
	static time_t startT = 0;
	time(&startT);
	cheetahInit(&cheetahGlobal);
	
	

    /*
	 *	Open SACLA HDF5 file
	 *	Read file header and information
	 */
    SACLA_h5_info_t SACLA_header;
    SACLA_HDF5_ReadHeader(filename, &SACLA_header);

    

	
    
    // Create a buffer for holding the detector image data from all 8 panels
    // Hard code this for now since it's only test code at the moment
    long    fs_one = 512;
    long    ss_one = 1024;
    long    nn_one = fs_one*ss_one;
    long    fs = fs_one;
    long    ss = 8*ss_one;
    long    nn = fs*ss;
    float   *buffer = (float*) calloc(nn, sizeof(float));
    hsize_t dims[2];
    dims[0] = ss;
    dims[1] = fs;
    
    
    
    // Loop through runs in this HDF5 field
    
    for(long runID=0; runID<SACLA_header.nruns; runID++) {
        printf("Processing run: %s\n", SACLA_header.run_string[runID]);
        
        // Gather detector fields and event tags for this run
        SACLA_HDF5_Read2dDetectorFields(&SACLA_header, runID);
        SACLA_HDF5_ReadEventTags(&SACLA_header, runID);
        
        
        for(long eventID=0; eventID<SACLA_header.nevents; eventID++) {
            printf("Processing event: %s\n", SACLA_header.event_name[eventID]);
			frameNumber++;

			
			/*
			 *  Cheetah: Calculate time beteeen processing of data frames
			 */
			time_t	tnow;
			double	dtime, datarate;
			time(&tnow);
			
			dtime = difftime(tnow, cheetahGlobal.tlast);
			if(dtime > 1.) {
				datarate = (frameNumber - cheetahGlobal.lastTimingFrame)/dtime;
				cheetahGlobal.lastTimingFrame = frameNumber;
				time(&cheetahGlobal.tlast);
				cheetahGlobal.datarate = datarate;
			}

			
			/*
			 *	Cheetah: Create a new eventData structure in which to place all information
			 */
			cEventData	*eventData;
			eventData = cheetahNewEvent(&cheetahGlobal);
			ntriggers++;

			
			/*
			 *	SACLA: Read next image
			 */
            SACLA_HDF5_ReadImageRaw(&SACLA_header, runID, eventID, buffer, nn_one);

            
			/*
			 *	Cheetah: Process this event
			 */
			cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);

			
            // Dump buffer to a trivial HDF5 file
            // (to see whether this code works at all)
            char    outfile[1024];
            hid_t   outfile_id;
            sprintf(outfile,"/data/scratch/sacla/%s.h5", SACLA_header.event_name[eventID]);
            printf("Writing to temporary file: %s\n",outfile);
            outfile_id = H5Fcreate(outfile,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            H5LTmake_dataset_float(outfile_id, "data", 2, dims, buffer );
            H5Fclose(outfile_id);
            
        }
    }

    

		
	
	
	
	
	// Clean up stale IDs and exit
	std::cout << "Cleaning up HDF5 links\n";
	hid_t ids[256];
	long n_ids = H5Fget_obj_ids(SACLA_header.file_id, H5F_OBJ_ALL, 256, ids);
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
	
	H5Fclose(SACLA_header.file_id);

	
	/*
	 *	Cheetah: Cleanly exit by closing all files, releasing memory, etc.
	 */
	cheetahExit(&cheetahGlobal);
	
	time_t endT;
	time(&endT);
	double dif = difftime(endT,startT);
	std::cout << "time taken: " << dif << " seconds\n";

	
	// Clean exit
	std::cout << "Clean exit\n";
    return 0;
}

