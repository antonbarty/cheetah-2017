//
//  cheetah.h
//  cheetah
//
//  Created by Anton Barty on 11/04/12.
//  Copyright (c) 2012 CFEL. All rights reserved.
//


#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"


static uint32_t nevents = 0;

void cheetahInit(cGlobal *);
void cheetahNewRun(cGlobal *);
void cheetahProcessEvent(cGlobal *, cEventData *);
void cheetahDeleteEvent(cEventData *);
void cheetahExit(cGlobal *);
cEventData* cheetahNewEvent();

