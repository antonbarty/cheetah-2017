#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <hdf5.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <vector>


#include "detectorObject.h"
#include "cheetahGlobal.h"
#include "cheetahEvent.h"
#include "cheetahmodules.h"
#include "median.h"


namespace CXI{
  const char* ATTR_NAME_NUM_EVENTS = "numEvents";

typedef struct{
    hid_t self;
    hid_t distance;
    hid_t data;
    hid_t mask;
    hid_t mask_shared;
    hid_t thumbnail;
    hid_t description;
    hid_t xPixelSize;
    hid_t yPixelSize;
}Detector;


typedef struct{
  /* ID of the group itself */
  hid_t self;
  hid_t data;
  hid_t mask;
  hid_t mask_shared;
  hid_t mask_shared_min;
  hid_t mask_shared_max;
  hid_t thumbnail;
  hid_t dataType;
  hid_t dataSpace;
}Image;

typedef struct{
  hid_t self;

  hid_t energy;
}Source;

typedef struct{
  hid_t self;

  Source source;
  std::vector<Detector> detectors;
}Instrument;

typedef struct{
  hid_t self;

  hid_t machineTime;
  hid_t fiducial;
  hid_t ebeamCharge;
  hid_t ebeamL3Energy;
  hid_t ebeamPkCurrBC2;
  hid_t ebeamLTUPosX;
  hid_t ebeamLTUPosY;
  hid_t ebeamLTUAngX;
  hid_t ebeamLTUAngY;
  hid_t phaseCavityTime1;
  hid_t phaseCavityTime2;
  hid_t phaseCavityCharge1;
  hid_t phaseCavityCharge2;
  hid_t photon_energy_eV;
  hid_t photon_wavelength_A;
  hid_t f_11_ENRC;
  hid_t f_12_ENRC;
  hid_t f_21_ENRC;
  hid_t f_22_ENRC;
  hid_t evr41;
  std::vector<hid_t> detector_positions;
  std::vector<hid_t> detector_EncoderValues;
  hid_t eventTimeString;
  hid_t tofTime;
  hid_t tofVoltage;
}LCLS;

typedef struct{
  hid_t self;
}ConfValues;

typedef struct{
  hid_t self;
  hid_t eventName;
  hid_t frameNumber;
  hid_t frameNumberIncludingSkipped;
  hid_t threadID;
  hid_t gmd1;
  hid_t gmd2;
  hid_t energySpectrumExist;
  hid_t nPeaks;
  hid_t peakNpix;
  hid_t peakTotal;
  hid_t peakResolution;
  hid_t peakResolutionA;
  hid_t peakDensity;
  hid_t laserEventCodeOn;
  hid_t laserDelay;
  hid_t hit;
}UnsharedValues;

typedef struct{
  hid_t self;
  hid_t lastBgUpdate[MAX_DETECTORS];
  hid_t nHot[MAX_DETECTORS];
  hid_t lastHotPixUpdate[MAX_DETECTORS];
  hid_t hotPixCounter[MAX_DETECTORS];
  hid_t nHalo[MAX_DETECTORS];
  hid_t lastHaloPixUpdate[MAX_DETECTORS];
  hid_t haloPixCounter[MAX_DETECTORS];
  hid_t hit;
  hid_t nPeaks;
}SharedValues;

typedef struct{
  hid_t self;
  //needs to be implemented:
  //hid_t cheetah_version;
  ConfValues confVal;
  UnsharedValues unsharedVal;
  SharedValues sharedVal;
}CheetahValues;

typedef struct{
  std::vector<hid_t> radialAverage;
  std::vector<hid_t> radialAverageCounter;
}Radial;

typedef struct{
  hid_t self;
}Data;

typedef struct{
  hid_t self;

  Data data;
  Instrument instrument;
  hid_t experimentIdentifier;
  std::vector<Image> images;
}Entry;

typedef struct{
  hid_t self;
  Entry entry;
  LCLS lcls;
  CheetahValues cheetahVal;
  
  /*  This counter defines where in the file each image is stored.
   *  It is atomically incremented by each thread */
  uint stackCounter;
}File;


 const int version = 130;
 const int initialStackSize = 16;
 const int thumbnailScale = 8;
}




