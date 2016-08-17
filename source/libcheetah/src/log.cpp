/*
 *  log.cpp
 *  cheetah
 */

#include <pthread.h>
#include <stdlib.h>

#include "cheetah.h"

void writeLog(cEventData *eventData, cGlobal * global) {
	// Write out information on each frame to a log file
	pthread_mutex_lock(&global->framefp_mutex);
    fprintf(global->framefp, "%s, ", eventData->eventname);
	fprintf(global->framefp, "%s, ", eventData->filename);
	fprintf(global->framefp, "%ld, ", eventData->stackSlice);
	fprintf(global->framefp, "%li, ", eventData->frameNumber);
	fprintf(global->framefp, "%i, ", eventData->hit);
//    fprintf(global->framefp, "%i, ", eventData->powderClass);
//	fprintf(global->framefp, "%g, ", eventData->hitScore);
	fprintf(global->framefp, "%g, ", eventData->photonEnergyeV);
	fprintf(global->framefp, "%g, ", eventData->wavelengthA);
//	fprintf(global->framefp, "%g, ", eventData->gmd1);
//	fprintf(global->framefp, "%g, ", eventData->gmd2);
	fprintf(global->framefp, "%g, ", eventData->detector[0].detectorZ);
//	fprintf(global->framefp, "%i, ", eventData->energySpectrumExist);
	fprintf(global->framefp, "%d, ", eventData->nPeaks);
	fprintf(global->framefp, "%g, ", eventData->peakNpix);
	fprintf(global->framefp, "%g, ", eventData->peakTotal);
	fprintf(global->framefp, "%g, ", eventData->peakResolution);
	fprintf(global->framefp, "%g, ", eventData->peakDensity);
//	fprintf(global->framefp, "%d, ", eventData->pumpLaserCode);
//	fprintf(global->framefp, "%g, ", eventData->pumpLaserDelay);
//   fprintf(global->framefp, "%d\n", eventData->pumpLaserOn);
	fprintf(global->framefp, "%lf\n ", eventData->exposureTime);
	pthread_mutex_unlock(&global->framefp_mutex);

	// Keep track of what has gone into each image class
	long powderClass = eventData->powderClass;
    if(global->powderlogfp[powderClass] != NULL) {
		pthread_mutex_lock(&global->powderfp_mutex);
        fprintf(global->powderlogfp[powderClass], "%s, ", eventData->eventname);
		fprintf(global->powderlogfp[powderClass], "%s, ", eventData->filename);
		fprintf(global->powderlogfp[powderClass], "%ld, ", eventData->stackSlice);
        fprintf(global->powderlogfp[powderClass], "%li, ", eventData->frameNumber);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->hitScore);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->photonEnergyeV);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->wavelengthA);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->detector[0].detectorZ);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->gmd1);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->gmd2);
        fprintf(global->powderlogfp[powderClass], "%i, ", eventData->energySpectrumExist);
        fprintf(global->powderlogfp[powderClass], "%d, ", eventData->nPeaks);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakNpix);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakTotal);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakResolution);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->peakDensity);
        fprintf(global->powderlogfp[powderClass], "%d, ", eventData->pumpLaserCode);
        fprintf(global->powderlogfp[powderClass], "%g, ", eventData->pumpLaserDelay);
        fprintf(global->powderlogfp[powderClass], "%d\n", eventData->pumpLaserOn);
		pthread_mutex_unlock(&global->powderfp_mutex);
	}

	// Frame lists for CrystFEL (may not be in sync with events, but should contain all events)
	if(global->framelist[powderClass] != NULL) {
		fprintf(global->framelist[powderClass], "%s //%li\n", eventData->filename, eventData->stackSlice);
	}
	
	
}
