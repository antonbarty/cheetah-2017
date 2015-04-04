//
//  spectrum.cpp
//  cheetah
//
//  Created by Richard Bean on 2/27/13.
//
//


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>

#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "data2d.h"



/*
 *	Wrapper for saving all time tool stacks
 */
void saveTimeToolStacks(cGlobal *global) {
	
    if(global->useTimeTool) {
		printf("Saving Time tool stacks\n");
		for(long powderType=0; powderType < global->nPowderClasses; powderType++) {
			saveTimeToolStack(global, powderType);
		}
	}
	
}




/*
 *	Time tool
 */

void addTimeToolToStack(cEventData *eventData, cGlobal *global, int powderClass){
	
    float   *stack = global->TimeToolStack[powderClass];
    float	*timetrace = eventData->TimeTool_hproj;
    long	length = global->TimeToolStackWidth;
    long    stackCounter = global->TimeToolStackCounter[powderClass];
    long    stackSize = global->TimeToolStackSize;

	// No FEE data means go home
	if(!eventData->TimeTool_present)
		return;
		
	
    // Lock
	pthread_mutex_lock(&global->TimeToolStack_mutex[powderClass]);
	
    // Data offsets
    long stackoffset = stackCounter % stackSize;
    long dataoffset = stackoffset*length;
	
    // Copy data and increment counter
    for(long i=0; i<length; i++) {
        stack[dataoffset+i] = (float) timetrace[i];
    }
	
	
	// Write filename to log file in sync with stack positions (** Important for being able to index the patterns!)
	fprintf(global->TimeToolLogfp[powderClass], "%li, %li, %li, %s/%s\n", stackCounter, eventData->frameNumber, eventData->stackSlice, eventData->eventSubdir, eventData->eventname);


    // Increment counter
    global->TimeToolStackCounter[powderClass] += 1;
	
	
    // Save data once stack is full
    if((global->TimeToolStackCounter[powderClass] % stackSize) == 0) {
        printf("Saving Time tool stack: %i\n", powderClass);
        saveTimeToolStack(global, powderClass);
        
        for(long j=0; j<length*stackSize; j++)
            global->TimeToolStack[powderClass][j] = 0;
    }
	
    pthread_mutex_unlock(&global->TimeToolStack_mutex[powderClass]);
}


/*
 *  Save time tool stack
 */
void saveTimeToolStack(cGlobal *global, int powderClass) {
	
    if(!global->useTimeTool)
        return;
	
	char	filename[1024];
    float   *stack = global->TimeToolStack[powderClass];
    long	length = global->TimeToolStackWidth;
    long    stackCounter = global->TimeToolStackCounter[powderClass];
    long    stackSize = global->TimeToolStackSize;
    pthread_mutex_t mutex = global->TimeToolStack_mutex[powderClass];
	
	if(global->TimeToolStackCounter[powderClass]==0)
		return;
	
    // Lock
	pthread_mutex_lock(&mutex);
	
	
	// We re-use stacks, what is this number?
	long	stackNum = stackCounter / stackSize;
 	if(stackNum == 0) stackNum = 1;
	
	// If stack is not full, how many rows are full?
    long    nRows = stackSize;
    if(stackCounter % stackSize != 0)
        nRows = (stackCounter % stackSize);
	
    sprintf(filename,"r%04u-TimeTool-class%i-stack%li.h5", global->runNumber, powderClass, stackNum);
    printf("Saving time tool stack: %s\n", filename);
    writeSimpleHDF5(filename, stack, length, nRows, H5T_NATIVE_FLOAT);
	
	
	// Flush stack index buffer
	if(global->TimeToolLogfp[powderClass] != NULL)
		fflush(global->TimeToolLogfp[powderClass]);
	
	pthread_mutex_unlock(&mutex);
	
}

