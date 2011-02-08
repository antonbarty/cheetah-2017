/*
 *  setup.cpp
 *  cspad_cryst
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad-gjw/CspadTemp.hh"
#include "cspad-gjw/CspadCorrector.hh"
#include "cspad-gjw/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hdf5.h>

#include "worker.h"

//void setup_threads(tGlobal *global, tThreadInfo **threadInfo) {

void setup_threads(tGlobal *global) {

	global->nThreads = 2;
	global->nActiveThreads = 0;

	pthread_mutex_init(&global->nActiveThreads_mutex, NULL);
	global->threadID = (pthread_t*) calloc(global->nThreads, sizeof(pthread_t));
	for(int i=0; i<global->nThreads; i++) 
		global->threadID[i] = NULL;

	
}
