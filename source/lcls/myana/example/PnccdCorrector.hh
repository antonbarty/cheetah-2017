/* $Id: PnccdCorrector.hh,v 1.3 2011/05/11 01:06:45 caf Exp $ */
#ifndef PnccdCorrector_hh
#define PnccdCorrector_hh

#include <stdio.h>

#include "pdsdata/xtc/DetInfo.hh"

typedef float * PnccdFrame;

// default input file names
#define MYANA_LASERBACK_FILE_DEFAULT  "laserback.img"
#define MYANA_DARKCAL_FILE_DEFAULT    "pnccd.darkcal"

// pixel structure in darkcal file
typedef struct pixel {
  double  d1;
  double  offset;
  double  sigma;
  double  d4;
  int     i1;
  short   u1;
  short   u2;
} pixel_t;

class PnccdPixelStatus {
public:
  PnccdPixelStatus();
public:
  bool ok(unsigned col, unsigned row) const;
public:
  uint32_t* _map;
};

class PnccdCorrector {
public:
  enum { DarkFrameOffset=1,
         CommonModeNoise=2,
         PixelGain      =4,
         CTE            =8,
         BadPixel       =16,
         LaserOffset    =32,
         BlackThreshold =64 };
  //  Constructor with correction list; e.g. 'DarkFrameOffset | CommonModeNoise'
  PnccdCorrector(Pds::DetInfo::Detector detId,
                  unsigned               devId,
                  unsigned               corrections);
  ~PnccdCorrector();
public:
  //  Returns array of corrected pixel values
  PnccdFrame apply(const unsigned char *image);
  //  Returns status of pixels
  PnccdPixelStatus& status();
  //  Returns median of array subset
  float median(PnccdFrame array, int col, int firstRow, int lastRow);

public:
  //  Load correction constants
  void   loadConstants(unsigned run);
  bool   event();
private:
  int   _openFiles();
  void  _closeFiles();
private:
  Pds::DetInfo  _info;
  unsigned      _corrections;
  float         _black_threshold;
  PnccdFrame    _offsets;  // current valid dark frame offsets
  PnccdFrame    _goodpixels; // inverse of bad pixel map
  PnccdFrame    _laserback;  // laser background
  PnccdFrame    _result ;  // we keep ownership of result array
  FILE *_darkcalFile;      // darkcal file descriptor
  FILE *_laserbackFile;    // laser background file descriptor
  FILE *_offsetFile;       // offset file descriptor
  FILE *_goodpixelFile;    // goodpixel file descriptor
  int _laserRegion1[4];
  int _laserRegion2[4];
};

#endif
