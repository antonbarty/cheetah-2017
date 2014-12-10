/*
 *  gmd.cpp
 *  cheetah
 */

#include <pthread.h>
#include <stdlib.h>

#include "cheetah.h"


void calculateGmd(cEventData *eventData){
	eventData->gmd = (eventData->gmd21+eventData->gmd22)/2;
}

bool gmdBelowThreshold(cEventData *eventData, cGlobal *global){
	/*
	 *	Check whether or not we shall skip this frame because GMD value too low (FEL off)
	 */
	return (global->skipEventsBelowGmdThreshold && eventData->gmd < global->gmdThreshold);
}

void updateAvgGmd(cEventData *eventData, cGlobal *global){
	/*
	 *	Remember GMD values  (why is this here?)
	 */
	pthread_mutex_lock(&global->gmd_mutex);
	global->avgGmd = ( eventData->gmd + (global->detector[0].bgMemory-1)*global->avgGmd) / global->detector[0].bgMemory;
	pthread_mutex_unlock(&global->gmd_mutex);
}



