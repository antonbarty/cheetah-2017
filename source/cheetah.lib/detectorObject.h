/*
 *  pixelDetector.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

//#include "release/pdsdata/cspad/ConfigV1.hh"
//#include "release/pdsdata/cspad/ConfigV2.hh"
//#include "release/pdsdata/cspad/ConfigV3.hh"
//#include "cspad/CspadTemp.hh"
//#include "cspad/CspadCorrector.hh"
//#include "cspad/CspadGeometry.hh"


#define MAX_POWDER_CLASSES 16
#define MAX_DETECTORS 2
#define MAX_FILENAME_LENGTH 1024


/*
 * Detector geometries
 */
//	CSPAD	//
static const unsigned  CSPAD_ASIC_NX = 194;  // ASIC nx = extent of one ASIC in x
static const unsigned  CSPAD_ASIC_NY = 185;  // ASIC ny = extent of one ASIC in y
static const unsigned  CSPAD_nASICS_X = 8;   // 8 ASICs across in raw data stream
static const unsigned  CSPAD_nASICS_Y = 8;   // 8 ASICs down in raw data stresm

//	PNCCD	//
static const unsigned  PNCCD_ASIC_NX = 512;	// ASIC nx = extent of one ASIC in x
static const unsigned  PNCCD_ASIC_NY = 512;	// ASIC ny = extent of one ASIC in y
static const unsigned  PNCCD_nASICS_X = 2;		// 2 ASICs across in raw data stream
static const unsigned  PNCCD_nASICS_Y = 2;		// 2 ASICs down in raw data stresm


static const unsigned int cbufsize = 1024;


#define DETECTOR_LOOP for(long detID=0; detID < global->nDetectors; detID++)

class cGlobal;


/** @brief Detector configuration common to all events */
class cPixelDetectorCommon {

public:

	/** @brief ID of grouped configuration keywords */
	char configGroup[MAX_FILENAME_LENGTH];
	/** @brief Name of the detector */
	char     detectorName[MAX_FILENAME_LENGTH];
	/** @brief Type of detector */
	char     detectorType[MAX_FILENAME_LENGTH];
	//Pds::DetInfo::Device detectorType;
    	//Pds::DetInfo::Detector detectorPdsDetInfo;

	//unsigned         configVsn;
	//unsigned         quadMask;
	//unsigned         asicMask;


	/** @brief Detector configuration file */
	char  detectorConfigFile[MAX_FILENAME_LENGTH];
	/** @brief File containing pixelmap (coordinates of pixels) */
	char  geometryFile[MAX_FILENAME_LENGTH];
	/** @brief File containing dark calibration */
	char  darkcalFile[MAX_FILENAME_LENGTH];
	/** @brief File containing gain calibration */
	char  gaincalFile[MAX_FILENAME_LENGTH];
	/** @brief File containing bad pixel mask */
	char  badpixelFile[MAX_FILENAME_LENGTH];
	/** @brief What is this? */
	char  baddataFile[MAX_FILENAME_LENGTH];
	/** @brief File containing mask of area behind wires */
	char  wireMaskFile[MAX_FILENAME_LENGTH];  // File containing mask of area behind wires


	// Detector data block size
	long  pix_nx;
	long  pix_ny;
	long  pix_nn;

	// Real space pixel locations
	float  *pix_x;
	float  *pix_y;
	float  *pix_z;

	// Reciprocal space pixel coordinates (inverse A, no factor of 2*pi)
	float  *pix_kx;
	float  *pix_ky;
	float  *pix_kz;
	float  pixelSize;

	// Assembled image size
	long  image_nx;
	long  image_nn;

	// ASIC module size
	long  asic_nx;
	long  asic_ny;
	long  asic_nn;
	long  nasics_x;
	long  nasics_y;

	// Radial averages
	float   radial_max;
	long    radial_nn;
	float   *pix_r;
	float   *pix_kr;
	float   *pix_res;


	// Detector position
	char    detectorZpvname[MAX_FILENAME_LENGTH];
	float   defaultCameraLengthMm;
	float   fixedCameraLengthMm;
	float   cameraLengthOffset;
	float   cameraLengthScale;
	float   detectorZprevious;
	float   detposprev;
	double  detectorZ;
	double  detectorEncoderValue;

	// Beam center
	double  beamCenterPixX;
	double  beamCenterPixY;

	/*
	 *  Flags for detector processing options
	 */
	int    useBadPixelMask;
	int    useBadDataMask;
	int    useDarkcalSubtraction;
	// Subtract common mode from each ASIC
	int    cmModule;
	int    cmSubtractUnbondedPixels;
	int    cmSubtractBehindWires;
	float  cmFloor;    // Use lowest x% of values to estimate DC offset
	// Gain calibration
	int    useGaincal;
	int    invertGain;
	// Saturated pixels
	int    maskSaturatedPixels;
	long   pixelSaturationADC;
	// Local background subtraction
	int    useLocalBackgroundSubtraction;
	long   localBackgroundRadius;
	// Running background subtraction
	int    useSubtractPersistentBackground;
	int    subtractBg;
	int    scaleBackground;
	int    useBackgroundBufferMutex;
	float  bgMedian;
	long   bgMemory;
	long   bgRecalc;
	long   bgCounter;
	long   last_bg_update;
	int    bgIncludeHits;
	int    bgNoBeamReset;
	int    bgFiducialGlitchReset;
	// Kill persistently hot pixels
	int    useAutoHotpixel;
	int    hotpixADC;
	int    hotpixMemory;
	int    hotpixRecalc;
	float  hotpixFreq;
	long   hotpixCounter;
	long   nhot;
	long   last_hotpix_update;
	int    startFrames;
	// correction for PNCCD read out artifacts on back detector
	int    usePnccdOffsetCorrection;
	

	// Saving options
	int   saveDetectorCorrectedOnly;
	int   saveDetectorRaw;


	/*
	 * Arrays for all sorts of stuff
	 */
	int16_t   *peakmask;
	int16_t   *badpixelmask;
	int16_t   *baddatamask;
	int16_t   *bg_buffer;
	int16_t   *hotpix_buffer;
	int16_t   *hotpixelmask;
	int16_t   *wiremask;
	float     *darkcal;
	float     *selfdark;
	float     *gaincal;


	/*
	 * Powder patterns/sums for this detector
	 */
	FILE     *powderlogfp[MAX_POWDER_CLASSES];
	long     nPowderClasses;
	long     nPowderFrames[MAX_POWDER_CLASSES];
	double   *powderRaw[MAX_POWDER_CLASSES];
	double   *powderRawSquared[MAX_POWDER_CLASSES];
	double   *powderAssembled[MAX_POWDER_CLASSES];
	pthread_mutex_t powderRaw_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t powderRawSquared_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t powderAssembled_mutex[MAX_POWDER_CLASSES];


	/*
	 *  Radial stacks for this detector
	 */
	long            radialStackSize;
	long   radialStackCounter[MAX_POWDER_CLASSES];
	float   *radialAverageStack[MAX_POWDER_CLASSES];
	pthread_mutex_t radialStack_mutex[MAX_POWDER_CLASSES];



public:

	cPixelDetectorCommon();
	void configure(void);
	void parseConfigFile(char *);
	void allocatePowderMemory(cGlobal*);
	void readDetectorGeometry(char *);
	void updateKspace(cGlobal*, float);
	void readDarkcal(char *);
	void readGaincal(char *);
	void readPeakmask(cGlobal*, char *);
	void readBadpixelMask(char *);
	void readBaddataMask(char *);
	void readWireMask(char *);

//private:

	int parseConfigTag(char*, char*);

};


class cPixelDetectorEvent {

public:

	cPixelDetectorEvent();
	
	int       cspad_fail;
	//float     quad_temperature[4];
	//uint16_t  *quad_data[4];
	uint16_t  *raw_data;
	float     *corrected_data;
	float     *detector_corrected_data;
	int16_t   *corrected_data_int16;
	int16_t   *image;
	float     *radialAverage;
	float     *radialAverageCounter;
	int16_t   *saturatedPixelMask;
	double    detectorZ;

};


