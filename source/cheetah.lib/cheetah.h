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
void cheetahUpdateGlobal(cGlobal *, cEventData *);
void cheetahProcessEvent(cGlobal *, cEventData *);
void cheetahProcessEventMultithreaded(cGlobal *, cEventData *);
void cheetahDestroyEvent(cEventData *);
void cheetahExit(cGlobal *);
cEventData* cheetahNewEvent();
void cheetahDestroyEvent(cEventData *);
