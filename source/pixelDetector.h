/*
 *  pixelDetector.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#define	MAX_POWDER_CLASSES 16
#define MAX_DETECTORS 2
#define	MAX_FILENAME_LENGTH 1024


/*
 *	Stuff from original LCLS code
 */
using namespace std;
static CspadCorrector*      corrector;
static Pds::CsPad::ConfigV1 configV1;
static Pds::CsPad::ConfigV2 configV2;
static Pds::CsPad::ConfigV3 configV3;
static const unsigned		CSPAD_ASIC_NX = 194;		// ASIC nx = extent of one ASIC in x
static const unsigned		CSPAD_ASIC_NY = 185;		// ASIC ny = extent of one ASIC in y
static const unsigned		CSPAD_nASICS_X = 8;			// 8 ASICs across in raw data stream
static const unsigned		CSPAD_nASICS_Y = 8;			// 8 ASICs down in raw data stresm
//static unsigned				configVsn;
//static unsigned				quadMask;
//static unsigned				asicMask;

static const unsigned int cbufsize = 1024;


#define DETECTOR_LOOP for(long detID=0; detID < global->nDetectors; detID++) 

class cGlobal;


/*
 *	Detector configuration common to all events
 */
class cPixelDetectorCommon {
	
public:
	
	// Detector info (cspad-specific)
	char					detectorName[MAX_FILENAME_LENGTH];
	char					detectorTypeName[MAX_FILENAME_LENGTH];
	Pds::DetInfo::Device	detectorType;
	Pds::DetInfo::Detector	detectorPdsDetInfo;

	unsigned         configVsn;
	unsigned         quadMask;
	unsigned         asicMask;
	
	
	// Detector configuration files
	char		geometryFile[MAX_FILENAME_LENGTH];		// File containing pixelmap (X,Y coordinate of each pixel in raw data stream)
	char		darkcalFile[MAX_FILENAME_LENGTH];		// File containing dark calibration
	char		gaincalFile[MAX_FILENAME_LENGTH];
	char		badpixelFile[MAX_FILENAME_LENGTH];
	char		wireMaskFile[MAX_FILENAME_LENGTH];		// File containing mask of area behind wires
	

		
	// Detector data block size
	long			pix_nx;
	long			pix_ny;
	long			pix_nn;
	
	// Real space pixel locations
	float			*pix_x;
	float			*pix_y;
	float			*pix_z;
	
	// Reciprocal space pixel coordinates (inverse A, no factor of 2*pi)
	float			*pix_kx; 
	float			*pix_ky;
	float			*pix_kz;
	float			pix_dx;
	float			pixelSize;

	//unsigned		module_rows;
	//unsigned		module_cols;
	long			image_nx;
	long			image_nn;

	// ASIC sizes
	long			asic_nx;
	long			asic_ny;
	long			asic_nn;
	long			nasics_x;
	long			nasics_y;

	// Radial averages 
	float			radial_max;
	long			radial_nn;
	float			*pix_r;
	float			*pix_kr;
	float			*pix_res;


	
	/*
	 *	Arrays for all sorts of stuff
	 */
	int32_t			*darkcal;
	int16_t			*peakmask;
	int16_t			*badpixelmask;
	int16_t			*bg_buffer;
	int16_t			*hotpix_buffer;
	int16_t			*hotpixelmask;
	int16_t			*wiremask;
	float			*selfdark;
	float			*gaincal;
	

	/*
	 *	Powder patterns/sums for this detector
	 */
	FILE			*powderlogfp[MAX_POWDER_CLASSES];
	long			nPowderClasses;
	long			nPowderFrames[MAX_POWDER_CLASSES];
	double			*powderRaw[MAX_POWDER_CLASSES];
	double			*powderRawSquared[MAX_POWDER_CLASSES];
	double			*powderAssembled[MAX_POWDER_CLASSES];
	pthread_mutex_t	powderRaw_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t	powderRawSquared_mutex[MAX_POWDER_CLASSES];
	pthread_mutex_t	powderAssembled_mutex[MAX_POWDER_CLASSES];
    

    /*
     *  Radial stacks for this detector
     */
	long			radialStackCounter[MAX_POWDER_CLASSES];
	float			*radialAverageStack[MAX_POWDER_CLASSES];
	pthread_mutex_t	radialStack_mutex[MAX_POWDER_CLASSES];
	
	
	
public:
	void readDetectorGeometry(char *);
    void updateKspace(cGlobal*, float);
	void readDarkcal(cGlobal*, char *);
	void readGaincal(cGlobal*, char *);
	void readPeakmask(cGlobal*, char *);
	void readBadpixelMask(cGlobal*, char *);
	void readWireMask(cGlobal*, char *);
	
	
};


class cPixelDetectorEvent {
	
public:

	int			cspad_fail;
	float		quad_temperature[4];
	uint16_t	*quad_data[4];
	uint16_t	*raw_data;
	float		*corrected_data;
	float		*detector_corrected_data;
	int16_t		*corrected_data_int16;
	int16_t		*image;
	float		*radialAverage;
	float		*radialAverageCounter;
	int16_t *   saturatedPixelMask;

};


