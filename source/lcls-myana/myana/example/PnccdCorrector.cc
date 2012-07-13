/* $Id: PnccdCorrector.cc,v 1.3 2011/05/11 01:06:45 caf Exp $ */
#include "PnccdCorrector.hh"
#include "PnccdGeometry.hh"

#include "../main.hh"
#include "../XtcRun.hh"

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"

#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"

#include "pdsdata/evr/DataV3.hh"


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <endian.h>

#define ITER_DEBUG_LEVEL  0

#define DARKCAL_HEIGHT  512
#define DARKCAL_WIDTH   2048
#define IMG_VERSION     0

// address PnccdFrame[] element by (row, column)
#define frame_index(r, c) (((r)*(PNCCD_COLS))+(c))

#if !defined(__BYTE_ORDER)
# error __BYTE_ORDER not defined
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN

static int writeImgHeader(unsigned short ver, unsigned short h, unsigned short w, FILE *f);
unsigned short readShort(FILE *f);
int writeShort(int val, FILE *p);

#else
# error __BYTE_ORDER != __LITTLE_ENDIAN
#endif


static const unsigned FramePixels = (PNCCD_ROWS) * (PNCCD_COLS);

PnccdCorrector::PnccdCorrector(Pds::DetInfo::Detector detId,
                               unsigned               devId,
                               unsigned               corrections) :
  _info       (0,detId,0,Pds::DetInfo::pnCCD,devId),
  _corrections(corrections)
{
  _offsets = new float[FramePixels];
  _goodpixels = new float[FramePixels];
  _laserback = new float[FramePixels];
  _result  = new float[FramePixels];
  _darkcalFile = NULL;
  _offsetFile = NULL;
  _goodpixelFile = NULL;
  _laserbackFile = NULL;
}

PnccdCorrector::~PnccdCorrector()
{
  delete[] _offsets;
  delete[] _goodpixels;
  delete[] _result ;
}

// #define frame_index(r, c) (((r)*(PNCCD_COLS))+(c))

float PnccdCorrector::median(PnccdFrame frame, int row, int firstCol, int lastCol)
{
  float rv = 0.0;
  int sortLen = (lastCol - firstCol + 1);
  if (sortLen > 1) {
    // general case: median = middle of sorted odd length list
    float sortme[sortLen];
    for (int ii = 0; firstCol <= lastCol; firstCol++, ii++) {
      sortme[ii] = frame[frame_index(row, firstCol)];
    }
    std::sort(sortme, sortme + sortLen - 1);
    rv = sortme[sortLen / 2];
  } else {
    // trivial case: median = sole element value
    rv = frame[frame_index(row, firstCol)];
  }
  return (rv);
}

//  Returns array of corrected pixel values (as floats)
PnccdFrame PnccdCorrector::apply(const unsigned char *image)
{
  float*          d = reinterpret_cast<float*>(_result);
  const uint16_t* s = reinterpret_cast<const uint16_t*>(image);
  const uint16_t* e = s + FramePixels;
  unsigned int uu;
  int row, col;

  if (_corrections & PixelGain) {
    printf("Error: PixelGain correction not supported\n");
  }
  if (_corrections & CTE) {
    printf("Error: CTE correction not supported\n");
  }

  if (_corrections & DarkFrameOffset) {
    const float* o = reinterpret_cast<const float*>(_offsets);
    d = reinterpret_cast<float*>(_result);
    s = reinterpret_cast<const uint16_t*>(image);
    e = s + FramePixels;
    {
      //
      //  Subtract the offset
      //
      while( s < e ) {
        if (*s > *o) {
          *d++ = float(*s++)-*o++;
        } else {
          *d++ = 0.0;
          s++;
          o++;
        }
      }
    }
  } else {
    //
    //  Copy the integer input into float output
    //
    while( s < e )
      *d++ = float(*s++);
  }

  if (_corrections & BadPixel) {
    const float* g = reinterpret_cast<const float*>(_goodpixels);
    d = reinterpret_cast<float*>(_result);
    for (uu = 0; uu < FramePixels; uu++, g++, d++) {
      //
      //  Multiply by the good pixel mask
      //
      if (*g < 0.5) {
        *d = 0.0;   // pixel cleared
      }
    }
  }

  if (_corrections & CommonModeNoise) {
    float ff;

    for (row = 0; row < PNCCD_ROWS; row++) {
      // left side common mode correction
      ff = median(_result, row, 1, 21);
      if (ff > 0.0) {
        // subtract median from each pixel in row
        for (col = 0; col < PNCCD_COLS / 2; col++) {
          _result[frame_index(row, col)] =
            std::max(0.0f, _result[frame_index(row, col)] - ff);
        }
      }
      // right side common mode correction
      ff = median(_result, row, PNCCD_COLS - 22, PNCCD_COLS - 2);
      if (ff > 0.0) {
        // subtract median from each pixel in row
        for (col = PNCCD_COLS / 2; col < PNCCD_COLS; col++) {
          _result[frame_index(row, col)] =
            std::max(0.0f, _result[frame_index(row, col)] - ff);
        }
      }
    }
  }

  // laser offset correction
  if (_corrections & LaserOffset) do {
    float *laser = _laserback;
    d = _result;
    float sum1Raw = 0.0, sum1Laser = 0.0, sum2Raw = 0.0, sum2Laser = 0.0;
    float scalingFactor;

    // calculate scaling factor
    if ((_laserRegion1[0] < 0) || (_laserRegion1[0] >= PNCCD_COLS) ||
        (_laserRegion1[1] < 0) || (_laserRegion1[1] >= PNCCD_ROWS) ||
        (_laserRegion1[2] < 0) || (_laserRegion1[2] >= PNCCD_COLS) ||
        (_laserRegion1[3] < 0) || (_laserRegion1[3] >= PNCCD_ROWS)) {
      fprintf(stderr, "Error: Laser Region 1 not within %dx%d\n", PNCCD_ROWS, PNCCD_COLS);
      break;
    }
    if ((_laserRegion2[0] < 0) || (_laserRegion2[0] >= PNCCD_COLS) ||
        (_laserRegion2[1] < 0) || (_laserRegion2[1] >= PNCCD_ROWS) ||
        (_laserRegion2[2] < 0) || (_laserRegion2[2] >= PNCCD_COLS) ||
        (_laserRegion2[3] < 0) || (_laserRegion2[3] >= PNCCD_ROWS)) {
      fprintf(stderr, "Error: Laser Region 2 not within %dx%d\n", PNCCD_ROWS, PNCCD_COLS);
      break;
    }

    for (row = _laserRegion1[0]; row <= _laserRegion1[2]; row++) {
      for (col = _laserRegion1[1]; col <= _laserRegion1[3]; col++) {
        sum1Raw += _result[frame_index(row,col)];
        sum1Laser += laser[frame_index(row,col)];
      }
    }

    for (row = _laserRegion2[0]; row <= _laserRegion2[2]; row++) {
      for (col = _laserRegion2[1]; col <= _laserRegion2[3]; col++) {
        sum2Raw += _result[frame_index(row,col)];
        sum2Laser += laser[frame_index(row,col)];
      }
    }

    if ((sum1Laser > 0.0) && (sum2Laser > 0.0)) {
      scalingFactor = ((sum1Raw / sum1Laser) + (sum2Raw / sum2Laser)) / 2.0;
    } else {
      // avoid dividing by zero
      fprintf(stderr, "Error: Laser Region 1 sum is %6.1f, Region 2 sum is %6.1f\n",
              sum1Laser, sum2Laser);
      scalingFactor = 0.0;
    }

    float newFactor = 1.0;
    char* factorString = getenv("MYANA_FACTOR");
    if (factorString && sscanf(factorString,"%f", &newFactor) == 1) {
      scalingFactor = newFactor;
    }

    for (uu = 0; uu < FramePixels; uu++, laser++, d++) {
      if ((_corrections & BadPixel) && (_goodpixels[uu] < 0.5)) {
        // skip bad pixel
        continue;
      } else {
        // apply laser offset correction
        *d = std::max(0.0f, *d - (scalingFactor * (*laser)));
      }
    }

  } while (0);

  // threshold correction is applied last
  if (_corrections & BlackThreshold) {
    d = _result;
    for (uu = 0; uu < FramePixels; uu++, d++) {
      if (*d < _black_threshold) {
        *d = 0;
      }
    }
  }

  return _result;
}

int PnccdCorrector::_openFiles()
{
  // files for input:
  char* darkcalFilename = getenv("MYANA_DARKCAL_FILE");
  char* laserbackFilename = getenv("MYANA_LASERBACK_FILE");
  // files for output:
  char* offsetFilename = getenv("MYANA_OFFSET_FILE");
  char* goodpixelFilename = getenv("MYANA_GOODPIXEL_FILE");
  int rv = 0;

  // darkcal file
  if ((_corrections & DarkFrameOffset) || (_corrections & BadPixel)) {
    if (!darkcalFilename) {
      fprintf(stderr, "MYANA_DARKCAL_FILE not defined.  Using default: %s\n",
              MYANA_DARKCAL_FILE_DEFAULT);
      darkcalFilename = MYANA_DARKCAL_FILE_DEFAULT;
    }
    _darkcalFile = fopen(darkcalFilename, "rb");
    if (!_darkcalFile) {
      perror("Open darkcal file for read");
      rv = -1;
    } else {
      if (offsetFilename) {
        _offsetFile = fopen(offsetFilename, "w");
        if (!_offsetFile) {
          perror("Open offset file for write");
          rv = -1;
        }
      }
      if (goodpixelFilename) {
        _goodpixelFile = fopen(goodpixelFilename, "w");
        if (!_goodpixelFile) {
          perror("Open goodpixel file for write");
          rv = -1;
        }
      }
    }
  }

  // laser background file
  if (_corrections & LaserOffset) {
    if (!laserbackFilename) {
      fprintf(stderr, "MYANA_LASERBACK_FILE not defined.  Using default: %s\n",
              MYANA_LASERBACK_FILE_DEFAULT);
      laserbackFilename = MYANA_LASERBACK_FILE_DEFAULT;
    }
    _laserbackFile = fopen(laserbackFilename, "rb");
    if (!_laserbackFile) {
      perror("Open laser background file for read");
      rv = -1;
    }
  }

  return (rv);
}

void PnccdCorrector::_closeFiles()
{
  if (_darkcalFile) {
    fclose(_darkcalFile);
    _darkcalFile = NULL;
  }
  if (_laserbackFile) {
    fclose(_laserbackFile);
    _laserbackFile = NULL;
  }
  if (_offsetFile) {
    fclose(_offsetFile);
    _offsetFile = NULL;
  }
  if (_goodpixelFile) {
    fclose(_goodpixelFile);
    _goodpixelFile = NULL;
  }
}

// static float darkcalIn[FramePixels];
static float darkcalIn[DARKCAL_HEIGHT][DARKCAL_WIDTH];

//  Load correction constants
void   PnccdCorrector::loadConstants(unsigned run)
{
  int row, col;
  float *pDest;
  int x1, y1, x2, y2;

  char *regionBuf = getenv("MYANA_LASER_REGION_1");

  if (regionBuf && (sscanf(regionBuf, "%d %d %d %d", &x1, &y1, &x2, &y2) == 4)) {
    _laserRegion1[0] = std::min(x1, x2);
    _laserRegion1[1] = std::min(y1, y2);
    _laserRegion1[2] = std::max(x1, x2);
    _laserRegion1[3] = std::max(y1, y2);
  } else {
    fprintf(stderr, "Error reading MYANA_LASER_REGION_1 as 4 integers\n");
  }
  regionBuf = getenv("MYANA_LASER_REGION_2");
  if (regionBuf && (sscanf(regionBuf, "%d %d %d %d", &x1, &y1, &x2, &y2) == 4)) {
    _laserRegion2[0] = std::min(x1, x2);
    _laserRegion2[1] = std::min(y1, y2);
    _laserRegion2[2] = std::max(x1, x2);
    _laserRegion2[3] = std::max(y1, y2);
  } else {
    fprintf(stderr, "Error reading MYANA_LASER_REGION_2 as 4 integers\n");
  }
  char *threshBuf = getenv("MYANA_BLACK_THRESHOLD");
  if (threshBuf && (sscanf(threshBuf, "%f", &_black_threshold) != 1)) {
    fprintf(stderr, "Error reading MYANA_BLACK_THRESHOLD as a float\n");
  }

  if (_openFiles()) {
    fprintf(stderr, "_openFiles() error\n");
  }

  if (_darkcalFile) do {
    pixel_t pixelBuf;

    // skip header of 1024 bytes */
    if (fseek(_darkcalFile, 1024, SEEK_SET) == -1) {
      perror("fseek darkcal file");
      break;
    }

    // read offset values into darkcalIn[] (512x2048)
    unsigned int darkCalErrCount = 0;
    for (row = 0; row < DARKCAL_HEIGHT; row++) {
      for (col = 0; col < DARKCAL_WIDTH; col++) {
        if (fread((void *)&pixelBuf, sizeof(pixelBuf), 1, _darkcalFile) != 1) {
          ++darkCalErrCount;
          break;
        } else {
          darkcalIn[row][col] = (float)pixelBuf.offset;
        }
      }
    }
    if (darkCalErrCount) {
      fprintf(stderr, "darkCalErrCount: %u\n", darkCalErrCount);
    }

    // reshape offset values to _offsets[] (1024x1024)
    pDest = _offsets;
    for (row = 0; row < PNCCD_ROWS / 2; row++) {
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // A
        *pDest++ = darkcalIn[row][col];
      }
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // B
        *pDest++ = darkcalIn[row][col + (3 * DARKCAL_WIDTH / 4)];
      }
    }
    for (row = 0; row < PNCCD_ROWS / 2; row++) {
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // C
        *pDest++ = darkcalIn[DARKCAL_HEIGHT - 1 - row][(DARKCAL_WIDTH / 2) - 1 - col];
      }
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // D
        *pDest++ = darkcalIn[DARKCAL_HEIGHT - 1 - row][(3 * DARKCAL_WIDTH / 4) - 1 - col];
      }
    }

    // write offset img (1024x1024)
    if (_offsetFile) {
      writeImgHeader(0, PNCCD_ROWS, PNCCD_COLS, _offsetFile);
      unsigned uu = fwrite((void *)_offsets, sizeof(float), FramePixels, _offsetFile);
      if (uu != FramePixels) {
        perror("fwrite");
      }
    }

    // read bad pixel map into darkcalIn[] (512x2048)
    char charBuf;
    unsigned int darkPixErrCount = 0;
    for (row = 0; row < DARKCAL_HEIGHT; row++) {
      for (col = 0; col < DARKCAL_WIDTH; col++) {
        if (fread((void *)&charBuf, sizeof(charBuf), 1, _darkcalFile) != 1) {
          ++darkPixErrCount;
          darkcalIn[row][col] = 0.0;
        } else {
          // inverting zero/nonzero to get good pixel map
          darkcalIn[row][col] = charBuf ? 0.0 : 1.0;
        }
      }
    }
    if (darkPixErrCount) {
      fprintf(stderr, "darkPixErrCount: %u\n", darkPixErrCount);
    }

    // reshape bad pixel values to _goodpixels[] (1024x1024)
    pDest = _goodpixels;
    for (row = 0; row < PNCCD_ROWS / 2; row++) {
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // A
        *pDest++ = darkcalIn[row][col];
      }
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // B
        *pDest++ = darkcalIn[row][col + (3 * DARKCAL_WIDTH / 4)];
      }
    }
    for (row = 0; row < PNCCD_ROWS / 2; row++) {
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // C
        *pDest++ = darkcalIn[DARKCAL_HEIGHT - 1 - row][(DARKCAL_WIDTH / 2) - 1 - col];
      }
      for (col = 0; col < PNCCD_COLS / 2; col++) {
        // D
        *pDest++ = darkcalIn[DARKCAL_HEIGHT - 1 - row][(3 * DARKCAL_WIDTH / 4) - 1 - col];
      }
    }

    // write good pixel img
    if (_goodpixelFile) {
      writeImgHeader(0, PNCCD_ROWS, PNCCD_COLS, _goodpixelFile);
      unsigned uu = fwrite((void *)_goodpixels, sizeof(float), FramePixels, _goodpixelFile);
      if (uu != FramePixels) {
        perror("fwrite");
      }
    }

  } while (0);

  if (_laserbackFile) {
    unsigned short height = 0;
    unsigned short width = 0;

    readShort(_laserbackFile);          // ignore version
    width = readShort(_laserbackFile);  // read width
    height = readShort(_laserbackFile); // read height

    if (height != PNCCD_ROWS || width != PNCCD_COLS) {
      fprintf(stderr, "Laser background H x W: %hu x %hu\n", height, width);
      fprintf(stderr, "Error: Laser background H x W is not %d x %d.\n", PNCCD_ROWS, PNCCD_COLS);
    } else if (fread((void *)_laserback, sizeof(float), FramePixels, _laserbackFile) != FramePixels) {
      perror("Read laser background");
    }
  }

  _closeFiles();
}

static int writeImgHeader(unsigned short ver, unsigned short h, unsigned short w, FILE *f)
{
  writeShort(ver, f);
  writeShort(w, f);
  writeShort(h, f);
  return (0);
}

unsigned short readShort(FILE *f)
{
  // little endian
  unsigned short s1 = getc(f);
  unsigned short s2 = getc(f);
  return (s2 * 256 + s1);
}

int writeShort(int val, FILE *p)
{
  int rv = 0;
  // little endian
  if (putc(val & 0xff, p) == EOF) {
    perror("putc");
    rv = -1;
  }
  if (putc(val >> 8, p) == EOF) {
    perror("putc");
    rv = -1;
  }
  return (rv);
}
