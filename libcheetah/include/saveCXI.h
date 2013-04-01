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
}LCLS;

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
  /*  This counter defines where in the file each image is stored.
   *  It is atomically incremented by each thread */
  int stackCounter;
}File;


 const int version = 130;
 const int initialStackSize = 16;
 const int thumbnailScale = 8;
}




