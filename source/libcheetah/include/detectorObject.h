/*
 *  detectorObject.h
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
#ifndef DETECTOROBJECT_H
#define DETECTOROBJECT_H

#include <stdint.h>
#include "dataVersion.h"
#include "frameBuffer.h"

#include "streakfinder_wrapper.h"
#include "cheetahConversion.h"
#include "pythonWrapperConversions.h"
#include "mask.h"


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

//  CSPAD 2x2 //
static const unsigned  CSPAD2x2_nASICS_X = 2;   // 2 ASICs across in raw data stream
static const unsigned  CSPAD2x2_nASICS_Y = 2;   // 2 ASICs down in raw data stresm

//	PNCCD	//
static const unsigned  PNCCD_ASIC_NX = 512;	// ASIC nx = extent of one ASIC in x
static const unsigned  PNCCD_ASIC_NY = 512;	// ASIC ny = extent of one ASIC in y
static const unsigned  PNCCD_nASICS_X = 2;		// 2 ASICs across in raw data stream
static const unsigned  PNCCD_nASICS_Y = 2;		// 2 ASICs down in raw data stresm

//	SACLA mpCCD	//
static const unsigned  mpCCD_ASIC_NX = 512;     // ASIC nx = extent of one ASIC in x
static const unsigned  mpCCD_ASIC_NY = 1024;	// ASIC ny = extent of one ASIC in y
static const unsigned  mpCCD_nASICS_X = 1;		// 2 ASICs across in raw data stream
static const unsigned  mpCCD_nASICS_Y = 8;		// 2 ASICs down in raw data stresm


// Rayonix MX170HS //
static const unsigned  MX170HS_ASIC_NX = 3840;		// ASIC nx = extent of one ASIC in x
static const unsigned  MX170HS_ASIC_NY = 3840;		// ASIC ny = extent of one ASIC in y
static const unsigned  MX170HS_nASICS_X = 1;		// 2 ASICs across in raw data stream
static const unsigned  MX170HS_nASICS_Y = 1;		// 2 ASICs down in raw data stresm

static const unsigned int cbufsize = 1024;

/*
 * Pixelmasks
 */


/*
 * Bits for pixel masks
 * Oriented along conventions defined for CXI file format ( https://github.com/FilipeMaia/CXI/raw/master/cxi_file_format.pdf )
 * CONVENTIONS:
 * - All options are dominantly inherited during assembly and pixel integration (see assembleImage.cpp)
 * - The default value for all options is "false"
 */
static const uint16_t PIXEL_IS_PERFECT = 0;                 // Remember to change this value if necessary after adding a new option
static const uint16_t PIXEL_IS_INVALID = 1;                 // bit 0
static const uint16_t PIXEL_IS_SATURATED = 2;               // bit 1
static const uint16_t PIXEL_IS_HOT = 4;                     // bit 2
static const uint16_t PIXEL_IS_DEAD = 8;                    // bit 3
static const uint16_t PIXEL_IS_SHADOWED = 16;               // bit 4
static const uint16_t PIXEL_IS_IN_PEAKMASK = 32;            // bit 5
static const uint16_t PIXEL_IS_TO_BE_IGNORED = 64;          // bit 6
static const uint16_t PIXEL_IS_BAD = 128;                   // bit 7
static const uint16_t PIXEL_IS_OUT_OF_RESOLUTION_LIMITS = 256; // bit 8
static const uint16_t PIXEL_IS_MISSING = 512;                // bit 9
static const uint16_t PIXEL_IS_NOISY = 1024;                 // bit 10
static const uint16_t PIXEL_IS_ARTIFACT_CORRECTED = 2048;    // bit 11
static const uint16_t PIXEL_FAILED_ARTIFACT_CORRECTION = 4096;    // bit 12
static const uint16_t PIXEL_IS_PEAK_FOR_HITFINDER = 8192;    // bit 13
static const uint16_t PIXEL_IS_PHOTON_BACKGROUND_CORRECTED = 16384;    // bit 14
static const uint16_t PIXEL_IS_IN_JET = 32768;				// bit 15
static const uint16_t PIXEL_IS_ALL = PIXEL_IS_INVALID | PIXEL_IS_SATURATED | PIXEL_IS_HOT | PIXEL_IS_DEAD | PIXEL_IS_SHADOWED | PIXEL_IS_IN_PEAKMASK | PIXEL_IS_TO_BE_IGNORED | PIXEL_IS_BAD | PIXEL_IS_OUT_OF_RESOLUTION_LIMITS | PIXEL_IS_MISSING | PIXEL_IS_NOISY | PIXEL_IS_ARTIFACT_CORRECTED | PIXEL_FAILED_ARTIFACT_CORRECTION | PIXEL_IS_PEAK_FOR_HITFINDER | PIXEL_IS_PHOTON_BACKGROUND_CORRECTED | PIXEL_IS_IN_JET;   // all bits

// for combined options
inline bool isAnyOfBitOptionsSet(uint16_t value, uint16_t option) {return ((value & option)!=0);}
inline bool isNoneOfBitOptionsSet(uint16_t value, uint16_t option) {return ((value & option)==0);}
inline bool isAnyOfBitOptionsUnset(uint16_t value, uint16_t option) {return ((value & option)!=option);}
inline bool isNoneOfBitOptionsUnset(uint16_t value, uint16_t option) {return ((value & option)==option);}
// for single options
inline bool isBitOptionSet(uint16_t value, uint16_t option) {return isNoneOfBitOptionsUnset(value,option);}
inline bool isBitOptionUnset(uint16_t value, uint16_t option) {return isNoneOfBitOptionsSet(value,option);}

/*
 * Assemble modes (see assemble2Dimage.cpp)
 */
static const int ASSEMBLE_INTERPOLATION_LINEAR = 0; 
static const int ASSEMBLE_INTERPOLATION_NEAREST = 1; 
static const int ASSEMBLE_INTERPOLATION_DEFAULT = ASSEMBLE_INTERPOLATION_LINEAR;

#define DETECTOR_LOOP for(long detIndex=0; detIndex < global->nDetectors; detIndex++)
#define POWDER_LOOP for (long powderClass=0; powderClass < global->detector[detIndex].nPowderClasses; powderClass++) 

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
	// Only required if one wants to specify front/back detector (pnccds)
	long detectorID;

	/** @brief Detector configuration file */
	//char  detectorConfigFile[MAX_FILENAME_LENGTH];
	/** @brief File containing pixelmap (coordinates of pixels) */
	char  geometryFile[MAX_FILENAME_LENGTH];
	/** @brief File containing dark calibration */
	char  darkcalFile[MAX_FILENAME_LENGTH];
	/** @brief File containing gain calibration */
	char  gaincalFile[MAX_FILENAME_LENGTH];
	/** @brief File containing bad pixel mask */
	char  initialPixelmaskFile[MAX_FILENAME_LENGTH];
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
	long  image_ny;
	long  image_nn;

	// Assembled downsampled image size
	long  imageXxX_nx;
	long  imageXxX_ny;
	long  imageXxX_nn;
	/** @brief Downsampling factor (1: no downsampling) */
	long  downsampling;
	/** @brief Rescale intensities after downsamping but before saving to image (avoid clamping to maximum value of 16-bit int) (1.: no rescaling) */
	float downsamplingRescale;
	/** @brief If set to 1 (default) pixel values are summed up no matter what the mask value is set to */
	long  downsamplingConservative;

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
    
    // Solid angle
    double  solidAngleConst; // constant term of the solid angle for each pixel
    
	/*
	 *  Flags for detector processing options
	 */
	int    useInitialPixelmask;
	int    initialPixelmaskIsBitmask;	
	int    applyBadPixelMask;
	int    useBadDataMask;
	int    useDarkcalSubtraction;
	// Subtract common mode from each ASIC
	char	commonModeCorrection[MAX_FILENAME_LENGTH];
	int    cmModule;
	int    cspadSubtractUnbondedPixels;
	int    cspadSubtractBehindWires;
	float  cmFloor;         // CSPAD: use lowest x% of values to estimate DC offset
    int    cmStart;         // pnCCD: intensity (ADU) from which the peakfinding should start in the histogram
    int    cmStop;          // pnCCD: intensity (ADU) at which the peakfinding should stop in the histogram
    float  cmThreshold;     // pnCCD: noise threshold intensity (ADU) over which the peakfinding should consider as true peaks in the histogram
    float  cmRange;         // pnCCD: number of standard deviations from the mean of the insensitive pixels at which the peakfinding should accept the found zero-photon peak
	// Gain calibration
	int    useGaincal;
	int    invertGain;
	// Apply polarization correction
    int    usePolarizationCorrection;
    double horizontalFractionOfPolarization;
	// Apply solid angle correction
    int    useSolidAngleCorrection;
    int    solidAngleAlgorithm;
	// Saturated pixels
	int    maskSaturatedPixels;
	long   pixelSaturationADC;
	// Mask pnccd saturated pixels (thresholds hardcoded, for every quadrant different)
	int    maskPnccdSaturatedPixels;
	// Local background subtraction
	int    useLocalBackgroundSubtraction;
	int    useRadialBackgroundSubtraction;
	long   localBackgroundRadius;
	// Running background subtraction
	int    useSubtractPersistentBackground;
	int    subtractPersistentBackgroundMean;
	float  subtractPersistentBackgroundMinAbsBgOverStdRatio;
	//int    subtractBg;
	int    scaleBackground;
	int    useBackgroundBufferMutex;
	float  bgMedian;
	long   bgMemory;
	long   bgRecalc;
	long   bgCounter;
	pthread_mutex_t bg_update_mutex;
	int    bgCalibrated;
	long   bgLastUpdate;
	int    bgIncludeHits;
	int    bgNoBeamReset;
	int    bgFiducialGlitchReset;
	// Identify persistently hot pixels
	int    useAutoHotPixel;
	int    hotPixADC;
	int    hotPixMemory;
	int    hotPixRecalc;
	float  hotPixFreq;
	pthread_mutex_t hotPix_update_mutex;
	int    hotPixCalibrated;
	long   nHot;
	long   hotPixLastUpdate;
	int    useHotPixelBufferMutex;
	// Apply persistently hot pixels
	int    applyAutoHotPixel;
	// Identify persistently illuminated pixels (noisy)
	int    useAutoNoisyPixel;
	int    noisyPixIncludeHits;
	float  noisyPixMinDeviation;
	long   noisyPixRecalc;
	long   noisyPixMemory;
	pthread_mutex_t   noisyPix_update_mutex;
	int    noisyPixCalibrated;
	long   nNoisy;
	long   noisyPixLastUpdate;
	// Photon counting
	int		photonCount;
	float	photconv_adu;
	float	photconv_ev;
	// Start frames for calibration before output
	int    startFrames;
	// correction for PNCCD read out artifacts on back detector
	int    usePnccdOffsetCorrection;
	// correction for wiring error (can be fixed also with an adequate geometry)
	int    usePnccdFixWiringError;
	// correction for intensity drop in every 2nd line, interpolation of all affected lines
	int    usePnccdLineInterpolation;
	// declare pixel bad if they are located in bad lines
	int    usePnccdLineMasking;

	
	/*
	 *	Streak finding
	 */
	int		useStreakFinder;
	long	streak_filter_length;
	long	streak_min_filter_length;
	float	streak_filter_step;
	float	streak_sigma_factor;
	float	streak_elongation_min_steps_count;
	float	streak_elongation_radius_factor;
	float	streak_pixel_mask_radius;
	int		streak_num_lines_to_check;
	int		streak_background_region_preset;
	int		streak_background_region_dist_from_edge;
	char	streak_mask_filename[MAX_FILENAME_LENGTH];
	char	streak_mask_hdf5_path[MAX_FILENAME_LENGTH ];

	streakFinderConstantArguments_t *streakfinderConstants;
	
	// Ring frame buffers
	cFrameBuffer *frameBufferBlanks;
	cFrameBuffer *frameBufferHotPix;
	cFrameBuffer *frameBufferNoisyPix;

	int threadSafetyLevel;

	// Saving options
	// Data versions
	// Toggle saving raw data (no corrections applied)
	int   saveDetectorRaw;
	// Toggle saving detector corrected data (only corrections that clean up detector artefacts applied)
	int   saveDetectorCorrected;
	// Toggle saving detector and photon corrected data (all corrections applied)
	int   saveDetectorAndPhotonCorrected;
	// Data formats
	int   saveNonAssembled;
	int   saveAssembled;
	int   saveAssembledAndDownsampled;
	int   saveRadialAverage;
	// Bit options defining formats in which data shall be saved (non-assembled / assembled / assembled and downsampled / radial average)
	cDataVersion::dataFormat_t saveFormat;
	// Bit options defining versions of the data to be saved (raw / detector corrected / detector and photon corrected)	
	cDataVersion::dataVersion_t saveVersion;

	// Defining the data format that the main dataset link "data" shall point to
	cDataVersion::dataFormat_t dataFormatMain;
	// Defining the data version that the main dataset link "data" shall point to
	cDataVersion::dataVersion_t dataVersionMain;
	cDataVersion::dataVersion_t powderVersionMain;


	
	int savePixelmask;

	// Powder saving options
	// Data versions
	int   savePowderDetectorRaw;
	int   savePowderDetectorCorrected;
	int   savePowderDetectorAndPhotonCorrected;
	// Data formats
	int   savePowderNonAssembled;
	int   savePowderAssembled;
	int   savePowderAssembledAndDownsampled;
	int   savePowderRadialAverage;
	int	  savePowderMasked;
	int	  saveThumbnail;
	int	  saveDownsampled;
	
	// Bit options defining formats in which powder data shall be created and saved (non-assembled / assembled / assembled and downsampled / radial average)
	cDataVersion::dataFormat_t powderFormat;
	// Bit options defining versions of the data to be used for creating and saving powders (raw / detector corrected / detector and photon corrected)	
	cDataVersion::dataVersion_t powderVersion;

	/*
	 *  Shared static data
	 */
	float             *darkcal;
	float             *gaincal;

	/*
	 *  Shared dynamic data
	 */
	// Pixelmasks
	uint16_t          *pixelmask_shared;
	uint16_t          *pixelmask_shared_max;
	uint16_t          *pixelmask_shared_min;
	pthread_mutex_t   pixelmask_shared_mutex;
	pthread_mutex_t   pixelmask_shared_min_mutex;
	pthread_mutex_t   pixelmask_shared_max_mutex;
	// Powder data (accumulated sums and sums of squared values)
	long     nPowderClasses;
	long     nPowderFrames[MAX_POWDER_CLASSES];
	double   *powderData_raw[MAX_POWDER_CLASSES];
	double   *powderData_raw_squared[MAX_POWDER_CLASSES];
	long	 *powderData_raw_counter[MAX_POWDER_CLASSES];
	double   *powderData_detCorr[MAX_POWDER_CLASSES];
	double   *powderData_detCorr_squared[MAX_POWDER_CLASSES];
	long   *powderData_detCorr_counter[MAX_POWDER_CLASSES];
	double   *powderData_detPhotCorr[MAX_POWDER_CLASSES];
	double   *powderData_detPhotCorr_squared[MAX_POWDER_CLASSES];
	long   *powderData_detPhotCorr_counter[MAX_POWDER_CLASSES];
	double   *powderImage_raw[MAX_POWDER_CLASSES];
	double   *powderImage_raw_squared[MAX_POWDER_CLASSES];
	double   *powderImage_detCorr[MAX_POWDER_CLASSES];
	double   *powderImage_detCorr_squared[MAX_POWDER_CLASSES];
	double   *powderImage_detPhotCorr[MAX_POWDER_CLASSES];
	double   *powderImage_detPhotCorr_squared[MAX_POWDER_CLASSES];
	double   *powderImageXxX_raw[MAX_POWDER_CLASSES];
	double   *powderImageXxX_raw_squared[MAX_POWDER_CLASSES];
	double   *powderImageXxX_detCorr[MAX_POWDER_CLASSES];
	double   *powderImageXxX_detCorr_squared[MAX_POWDER_CLASSES];
	double   *powderImageXxX_detPhotCorr[MAX_POWDER_CLASSES];
	double   *powderImageXxX_detPhotCorr_squared[MAX_POWDER_CLASSES];
	double   *powderRadialAverage_raw[MAX_POWDER_CLASSES];
	double   *powderRadialAverage_raw_squared[MAX_POWDER_CLASSES];
	double   *powderRadialAverage_detCorr[MAX_POWDER_CLASSES];
	double   *powderRadialAverage_detCorr_squared[MAX_POWDER_CLASSES];
	double   *powderRadialAverage_detPhotCorr[MAX_POWDER_CLASSES];
	double   *powderRadialAverage_detPhotCorr_squared[MAX_POWDER_CLASSES];
	double   *powderPeaks[MAX_POWDER_CLASSES];
	pthread_mutex_t powderData_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t powderImage_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t powderImageXxX_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t powderRadialAverage_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t powderPeaks_mutex[MAX_POWDER_CLASSES];
	long            radialStackSize;
	long     radialStackCounter[MAX_POWDER_CLASSES];
	float    *radialAverageStack[MAX_POWDER_CLASSES];
	pthread_mutex_t radialStack_mutex[MAX_POWDER_CLASSES];
	// Histogram stack
	int		histogram;
	int     histogramDataVersion;
	int		histogramOnlyBlanks;
	long	histogramMin;
	long	histogramNbins;
	float   histogramBinSize;
	long	histogram_fs_min;
	long	histogram_fs_max;
	long	histogram_ss_min;
	long	histogram_ss_max;
	long	histogram_nfs;
	long	histogram_nss;
	long	histogram_nn;
	long	histogram_count;
	float	histogramMaxMemoryGb;
	uint64_t	histogram_nnn;
	uint16_t	*histogramData;
	float       *histogramScale;
	pthread_mutex_t histogram_mutex;
	//long	histogram_depth;

	/*
	 *  Radial stacks for this detector
	 */

	// This is just for checking for uninitialised mutexes
	pthread_mutex_t null_mutex;

	// Methods
	cPixelDetectorCommon();
	void configure(cGlobal * global);
	void parseConfigFile(char *);
	void allocateMemory();
	void freeMemory();
	void unlockMutexes();
	void readDetectorGeometry(char *);
	void updateKspace(cGlobal*, float);
	void readDarkcal(char *);
	void readGaincal(char *);
	void readPeakmask(cGlobal*, char *);
	void readInitialPixelmask(char *);
	void readBaddataMask(char *);
	void readWireMask(char *);


//private:

	int parseConfigTag(char*, char*);

};


class cPixelDetectorEvent {

public:
	/* FM: Warning. Constructor is not run when class is malloc'ed*/
	cPixelDetectorEvent();
	
	/* FLAGS */
	int       cspad_fail;
	int       pedSubtracted;
	
	/* DATA NON-ASSEMBLED */
	// Raw data as read from the XTC file
	uint16_t  *data_raw16;
	// Raw data as read from the XTC file but converted to float
	float     *data_raw;
	// Data after detector corrections applied (common-mode, detector artefacts...)
	float     *data_detCorr;
	// Data after both detector corrections and photon corrections (subtraction of persistent parasitic scattering, water ring removal,...)
	float     *data_detPhotCorr;
	// Holding place for data to go into persistent background buffer
	float	  *data_forPersistentBackgroundBuffer;
	// Pixelmask
	uint16_t  *pixelmask;
	/* DATA ASSEMBLED */
	
	// Raw data as read from the XTC file but converted to float
	float     *image_raw;
	// Data after detector corrections applied (common-mode, detector artefacts...)
	float     *image_detCorr;
	// Data after both detector corrections and photon corrections (subtraction of persistent parasitic scattering, water ring removal,...)
	float     *image_detPhotCorr;
	// Pixelmask
	uint16_t  *image_pixelmask;
	/* DATA ASSEMBLED AND DOWN-SAMPLED */
	// Raw data as read from the XTC file but converted to float
	float     *imageXxX_raw;
	// Data after detector corrections applied (common-mode, detector artefacts...)
	float     *imageXxX_detCorr;
	// Data after both detector corrections and photon corrections (subtraction of persistent parasitic scattering, water ring removal,...)
	float     *imageXxX_detPhotCorr;
	// Pixelmask
	uint16_t  *imageXxX_pixelmask;
	/* RADIAL AVERAGE */
	float     *radialAverage_raw;
	float     *radialAverage_detCorr;
	float     *radialAverage_detPhotCorr;
	uint16_t  *radialAverage_pixelmask;
	//float     *radialAverage;
	//float     *radialAverageCounter;
	double    detectorZ;
	float     sum;
};


#endif
