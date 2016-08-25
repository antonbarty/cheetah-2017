//
//  main.cpp
//  cheetah-cbf
//
//
//  Based a little bit on cheetah-sacla-hdf5.cpp and cheetah-rayonix (main-rayonix.cpp):
//  Created by Natasha Stander December, 2015.
//

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <cbf.h>
#include <cbf_simple.h>

#include "cheetah.h"

#define CHECK_RETURN(x,y) { if (x) { ERROR(y); } }

static void chomp(char *s)
{
	size_t i;

	if ( !s ) return;

	for ( i=0; i<strlen(s); i++ ) {
		if ( (s[i] == '\n') || (s[i] == '\r') ) {
			s[i] = '\0';
			return;
		}
	}
}

// Return 0 on success.
int parseCBFHeader(cbf_handle &, cEventData*);
int loadImage(cbf_handle &, cEventData*);

int main(int argc, const char * argv[])
{
    // Parse Arguments
    printf("CBF file parser\n");
    printf("Natasha Stander, December 2015\n");

    if (argc != 4) {
        printf("Usage: cheetah-cbf listfile inifile runnumber\n");
        return 0;
    }

    char filename[1024];
    strcpy(filename, argv[1]);

    // Initialize Cheetah
	printf("Setting up Cheetah...\n");
    static long frameNumber = 0;
    long runNumber = atoi(argv[3]); /* ?? */
	static cGlobal cheetahGlobal;
	static time_t startT = 0;
	time(&startT);
    strcpy(cheetahGlobal.configFile, argv[2]);
    strcpy(cheetahGlobal.experimentID, "APS2016");
	cheetahInit(&cheetahGlobal);
    cheetahGlobal.runNumber = runNumber;

    // Open List file
    FILE *fh = fopen(argv[1], "r");
    if (fh == NULL) {
        fprintf(stderr, "Couldn't open '%s'\n", argv[1]);
		return 1;
	}

    // Loop through each CBF file given
    char curline[MAX_FILENAME_LENGTH];
    char * curFile;
    while ( (curFile = fgets(curline, MAX_FILENAME_LENGTH, fh)) ) {
        chomp(curFile);
        printf("Processing %s\n", curFile);
        frameNumber ++;

        // Create CBF Object for file
        cbf_handle cbfh;
        CHECK_RETURN(cbf_make_handle(&cbfh), "creating cbf handle");

        // Open File
        FILE* cbfFH = fopen(curFile, "rb");
        if (cbfFH == NULL)
            ERROR("Couldn't open %s\n", curFile);

        // Read into cbf object
        CHECK_RETURN(cbf_read_widefile(cbfh, cbfFH, MSG_NODIGEST), "reading cbf file");

        // Build Event Data
        cEventData * eventData = cheetahNewEvent(&cheetahGlobal);

        eventData->frameNumber = frameNumber;
        strcpy(eventData->eventname,strrchr(curFile,'/')+1);
        eventData->runNumber = runNumber;
        eventData->nPeaks = 0;
        eventData->pumpLaserCode = 0;
        eventData->pumpLaserDelay = 0;
        eventData->photonEnergyeV = cheetahGlobal.defaultPhotonEnergyeV;
        eventData->wavelengthA = 0; // find in parseSLSHeader
        eventData->pGlobal = &cheetahGlobal;

        // Header will fill in photonEnergyeV and wavelengthA
        parseCBFHeader(cbfh, eventData);

        // Now, load image
        loadImage(cbfh, eventData);

        // done with that cbf file, cleanup
        CHECK_RETURN(cbf_free_handle(cbfh), "Cleaning up cbf file\n");

        // Process event
        cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);

    }

    // Cleanup
    fclose(fh);
    cheetahExit(&cheetahGlobal);
    
    printf("Clean Exit\n");

    return 0;
}

int parseCBFHeader(cbf_handle &cbfh, cEventData* eventData) {
    // First, try the built-in function for non-SLS headers
    if (cbf_get_wavelength(cbfh, &(eventData->wavelengthA)) == 0)
        return 0; // success

    // Built-in function didn't work, so try SLS header
    // Find Header
    const char * headerconst;
    const char * headertype;
    CHECK_RETURN(cbf_find_category(cbfh, "array_data"), "Finding array_data category");
    CHECK_RETURN(cbf_find_column(cbfh, "header_contents"), "Finding header_contents column");
    CHECK_RETURN(cbf_select_row(cbfh, 0), "Selecting row 0 of header_contents column");
    CHECK_RETURN(cbf_get_value(cbfh, &headerconst), "Getting header value");
    CHECK_RETURN(cbf_get_typeofvalue(cbfh, &headertype), "Getting header value");

    if (strcmp(headertype, "text") != 0)
        ERROR("Unable to find SLS header.\n");

    // In the SLS header, we're looking specifically for the line
    // # Wavelength 1.0000 A
    eventData->wavelengthA = 0;

    // copy header so we can modify the c string
    char * header = new char[strlen(headerconst) + 1];
    strcpy(header, headerconst);
    char * cur = header; // start of line
    char * end = header; // end of line
    bool atEnd = false; // at end of header
    char prefix[6];
    prefix[5] = 0;
    while (!atEnd) {
        // find the newline that signals end of line
        while (*end != '\r' && *end != '\n' && *end != 0) end++;
        if (*end == 0)
            atEnd = true; // done after this, found end of header str

        // make the newline appear as the end of the str
        *end = 0;

        // check the line for the values we're interested in
        
        sscanf(cur,"# Wavelength %lf", &(eventData->wavelengthA));

        sscanf(cur,"# Exposure_time %lf", &(eventData->exposureTime));
        sscanf(cur,"# Exposure_period %lf", &(eventData->exposurePeriod));
        sscanf(cur,"# Tau = %lf", &(eventData->tau));
        sscanf(cur,"# Count_cutoff %i", &(eventData->countCutoff));
        sscanf(cur,"# Threshold_setting %lf", &(eventData->threshold));
        sscanf(cur,"# N_excluded_pixels = %i", &(eventData->nExcludedPixels));
        sscanf(cur,"# Detector_distance %lf", &(eventData->detectorDistance));
        sscanf(cur,"# Beam_xy (%lf, %lf)", &(eventData->beamX), &(eventData->beamY));
        sscanf(cur,"# Start_angle %lf", &(eventData->startAngle));
        sscanf(cur,"# Angle_increment %lf", &(eventData->angleIncrement));
        sscanf(cur,"# Detector_2theta %lf", &(eventData->detector2Theta));
        sscanf(cur,"# Shutter_time %lf", &(eventData->shutterTime));


       // This is a hack that will break after the year 2019
       strncpy(prefix,cur + 2,5);
       if (strcmp(prefix,"# 201") == 0) {
           strcpy(eventData->timeString,cur);
       }
           
        

        // prepare current and end for next line
        end ++;
        cur = end;
    }
    // done with header, cleanup
    delete [] header;

    // Error if wavelength or photonEv not found
    if (eventData->wavelengthA == 0)       
        printf("Warning: Could not find wavelength in cbf file!\n");
    //printf("Debugging: found wavelength %f and photon ev %f\n", eventData->wavelengthA, eventData->photonEnergyeV);
    eventData->detector[0].detectorZ = eventData->detectorDistance;
    return 0;
}

int loadImage(cbf_handle & cbfh, cEventData* eventData) {
    const char * headertype;
    // First, find binary section in cbf
    CHECK_RETURN(cbf_find_category(cbfh, "array_data"), "Finding array_data category");
    CHECK_RETURN(cbf_find_column(cbfh, "data"), "Finding data column");
    CHECK_RETURN(cbf_select_row(cbfh, 0), "Selecting row 0 of data column");
    CHECK_RETURN(cbf_get_typeofvalue(cbfh, &headertype), "Finding binary data");
    if (strcmp(headertype, "bnry") != 0)
        ERROR("Unable to find binary image.\n");

    // a whole bunch of variables needed to get cbf_get_integerarrayparameters_wdims
    unsigned int compression;
    int binId, elSigned, elUnsigned, minEl, maxEl;
    size_t elSize, elements, fs, ss, sss, padding;
    const char * byteOrder;

    CHECK_RETURN(cbf_get_integerarrayparameters_wdims(cbfh, &compression, & binId, & elSize,
        &elSigned, &elUnsigned, &elements, &minEl, &maxEl, &byteOrder, & fs,
        & ss, &sss, &padding), "reading image meta data");

    // Sanity check that image matches what cheetah is expecting
    int detId = 0;
    if (fs != (size_t) eventData->pGlobal->detector[detId].pix_nx ||
        ss != (size_t) eventData->pGlobal->detector[detId].pix_ny)
        ERROR("Error: File image dimensions of %zu x %zu did not match detector dimensions of %li x %li\n",
               fs, ss, eventData->pGlobal->detector[detId].pix_nx, eventData->pGlobal->detector[detId].pix_ny);

    // At least with the files I'm testing with, the binary data is signed 32 bit integers,
    // which matches neither data_raw16 (uint16_t type nor data_raw (float)). So, like SACLA,
    // read into temporary array and then convert.
    size_t elements_read = 0;
    int* arr = (int*) calloc(eventData->pGlobal->detector[detId].pix_nn, sizeof(int));
    CHECK_RETURN(cbf_get_integerarray(cbfh, &binId, arr, sizeof(int), 1, elements, &elements_read), "Reading image");

//    printf("Debugging: Read %zu elements debug\n", elements_read);
//    printf("Debugging: First 10 elements debug are: ");
//    for (int i = 0; i < 10; i++) {
//        printf ("%i, ", arr[i]);
//    }

    // Prepare buffer for event data
    eventData->detector[detId].data_raw16 = (uint16_t*) calloc(eventData->pGlobal->detector[detId].pix_nn, sizeof(uint16_t));

    // copy, setting negative values to 0
    for (size_t i = 0; i < elements_read; i++) {
        eventData->detector[detId].data_raw16[i] = arr[i] < 0? 0 : (uint16_t) arr[i];
    }

//    printf("Debugging: First 10 elements are: ");
//    for (int i = 0; i < 10; i++) {
//        printf ("%u, ", eventData->detector[detId].data_raw16[i]);
//    }

    free(arr);
    arr=NULL;

    return 0;
}

