#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "tofDetector.h"

/*
 *	Process tags for both configuration file and command line options
 */
int cTOFDetectorCommon::parseConfigTag(char *tag, char *value) {

	int fail = 0;

	/*
	 *	Convert to lowercase
	 */
	for(int i=0; i<((int) strlen(tag)); i++) 
		tag[i] = tolower(tag[i]);

	if (!strcmp(tag, "detectorname")) {
		strcpy(detectorName, value);
		configFromName();
	}else if (!strcmp(tag, "detectortype")) {
		if(strcasecmp(value,"tof") != 0){
            fprintf(stderr,"Error: detectortype is not tof in a cTOFDetectorCommon\n");            
			fprintf(stderr,"Quitting...\n");
			exit(1);
		}
		strcpy(detectorType, value);
	}else if (!strcmp(tag, "channel")) {
		channel = atoi(value);
	}else if (!strcmp(tag, "numsamples")) {
		numSamples = atoi(value);
	}else if (!strcmp(tag, "hitfinderminsample")) {
		hitfinderMinSample = atoi(value);
	}else if (!strcmp(tag, "hitfindermaxsample")) {
	    hitfinderMaxSample = atoi(value);
	}else if (!strcmp(tag, "hitfinderthreshold")) {
	    hitfinderThreshold = atof(value);
	}else if (!strcmp(tag, "hitfindermeanbackground")) {
	    hitfinderMeanBackground = atof(value);
	}else if (!strcmp(tag, "numsamples")) {
		numSamples = atoi(value);
	}else if (!strcmp(tag, "description")) {
		strcpy(description, value);
	}else{
		// Unknown tags
		fail = 1;
	}
	return fail;
}

void cTOFDetectorCommon::configFromName(){
	if(strcasecmp(detectorName,"acqiris0") == 0){
		numSamples = 12288;
		strcpy(sourceIdentifier,"DetInfo(:Acqiris.0)");
	}else if(strcasecmp(detectorName,"acqiris1") == 0){
		numSamples = 12288;
		strcpy(sourceIdentifier,"DetInfo(:Acqiris.1)");
	}
}
