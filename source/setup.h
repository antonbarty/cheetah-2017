/*
 *  setup.cpp
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
 *	Global variables
 */
class cGlobal {
	
public:
	
	/*
	 *	Various switches and processing options
	 */
	// ini file to read
	char		configFile[MAX_FILENAME_LENGTH];
	
	// Default experiment info (in case beamline data is missing)
	float	defaultPhotonEnergyeV;
	
	// Pixel detector readout
    int                     nDetectors;
	cPixelDetectorCommon	detector[MAX_DETECTORS];

	//char					detectorName[MAX_FILENAME_LENGTH];
	//char					detectorTypeName[MAX_FILENAME_LENGTH];
	//Pds::DetInfo::Device	detectorType;
	//Pds::DetInfo::Detector	detectorPdsDetInfo;
   
	
	// Detector position
    //char					detectorZpvname[MAX_FILENAME_LENGTH];
	
	// Track some statistics for the log file
	double summedPhotonEnergyeV;
	double summedPhotonEnergyeVSquared;
	double meanPhotonEnergyeV;
	double photonEnergyeVSigma; 

	// Start and stop frames
	long	startAtFrame;
	long	stopAtFrame;
	
	// Real-space geometry
	//char		geometryFile[MAX_FILENAME_LENGTH];		// File containing pixelmap (X,Y coordinate of each pixel in raw data stream)
	//float		pixelSize;
	//float		defaultCameraLengthMm;
    //float       cameraLengthOffset;
    //float       cameraLengthScale;
	
	// Bad pixel masks
	//int			useBadPixelMask;
	//char		badpixelFile[MAX_FILENAME_LENGTH];
	
	// Static dark calibration (static offsets on each pixel to be subtracted)
	//char		darkcalFile[MAX_FILENAME_LENGTH];		// File containing dark calibration
	//int			useDarkcalSubtraction;	// Subtract the darkcal (or not)?
	int			generateDarkcal;		// Flip this on to generate a darkcal (auto-turns-on appropriate other options)
	
	// Common mode and pedastal subtraction
	//char		wireMaskFile[MAX_FILENAME_LENGTH];		// File containing mask of area behind wires
	//int			cmModule;				// Subtract common mode from each ASIC
	//int			cmSubtractUnbondedPixels;
	//int			cmSubtractBehindWires;
	//float		cmFloor;				// Use lowest x% of values as the offset to subtract (typically lowest 2%)

	// Gain correction
	//int			useGaincal;
	//int			invertGain;
	//char		gaincalFile[MAX_FILENAME_LENGTH];
	int			generateGaincal;		// Flip this on to generate a gaincal (auto-turns-on appropriate other options)
	
	// Running background subtraction
	//int			useSubtractPersistentBackground;
	//int			subtractBg;
	//int			scaleBackground;
    //int         useBackgroundBufferMutex;
	//float		bgMedian;
	//long		bgMemory;
	//long		bgRecalc;
	//long		bgCounter;
	//long		last_bg_update;
	//int			bgIncludeHits;
	//int			bgNoBeamReset;
	//int			bgFiducialGlitchReset;
	
	// Local background subtraction
	//int			useLocalBackgroundSubtraction;
	//long		localBackgroundRadius;

	// Saturated pixels
	//int         maskSaturatedPixels;
	//long        pixelSaturationADC;	
	
	// Kill persistently hot pixels
	//int			useAutoHotpixel;
	//int			hotpixADC;
	//int			hotpixMemory;
	//int			hotpixRecalc;
	//float		hotpixFreq;
	//long		hotpixCounter;
	//long		nhot;
	//long		last_hotpix_update;
	//int			startFrames;
	
    
	// Hitfinding
	int         hitfinder;
	int         hitfinderAlgorithm;
	int         hitfinderADC;
	long        hitfinderNAT;
	float       hitfinderTAT;
	int         hitfinderNpeaks;
	int         hitfinderNpeaksMax;
	int         hitfinderMinPixCount;
	int         hitfinderMaxPixCount;
	int         hitfinderCheckGradient;
	float       hitfinderMinGradient;
	int         hitfinderCluster;
	int         hitfinderUsePeakmask;
	char        peaksearchFile[MAX_FILENAME_LENGTH];
	int         hitfinderUseTOF;
	int         hitfinderTOFMinSample;
	int         hitfinderTOFMaxSample;
	double      hitfinderTOFThresh;
	int         hitfinderCheckPeakSeparation;
	float       hitfinderMaxPeakSeparation;
	int			hitfinderSubtractLocalBG;
	int			hitfinderLocalBGRadius;
	int         hitfinderLocalBGThickness;
	int         hitfinderLimitRes;
	float       hitfinderMinRes;
	float       hitfinderMaxRes;
	int        *hitfinderResMask;
	float       hitfinderMinSNR;
	
	//	TOF
	Pds::DetInfo::Device	tofType;
	Pds::DetInfo::Detector	tofPdsDetInfo;
	char		tofName[MAX_FILENAME_LENGTH];
	int			TOFchannel;
	int			TOFPresent;
	int			AcqNumChannels;
	int			AcqNumSamples;
	double		AcqSampleInterval;

	
	// Powder pattern generation
	//int			powdersum;
	int			powderSumHits;
	int			powderSumBlanks;
	int			powderthresh;
	int			saveInterval;
	int			savePeakInfo;
	int			savePeakList;
	//int			saveDetectorCorrectedOnly;
	//int			saveDetectorRaw;

	// Radial stacks
    int         saveRadialStacks;
    long        radialStackSize;
    
    // Pv values
    char        laserDelayPV[MAX_FILENAME_LENGTH];
    float       laserDelay;
    
    
	// Saving options
	int			savehits;
	int			saveRaw;
	int			saveAssembled;
	int			hdf5dump;
	
	// Verbosity
	int			debugLevel;
	
	// Log files
	char		logfile[MAX_FILENAME_LENGTH];
	char		framefile[MAX_FILENAME_LENGTH];
	char		cleanedfile[MAX_FILENAME_LENGTH];
	char		peaksfile[MAX_FILENAME_LENGTH];
	
	// I/O speed test
	int			ioSpeedTest;
	
	/*
	 *	Stuff used for managing the program execution
	 */
	// Run information
	unsigned	runNumber;

	// Log file pointers
	FILE		*framefp;
	FILE		*cleanedfp;
	FILE		*peaksfp;
	
	// Thread management
	int				useHelperThreads;
	long			nThreads;
	long			nActiveThreads;
	long			threadCounter;
	long			threadPurge;
	pthread_t		*threadID;
	pthread_mutex_t	nActiveThreads_mutex;
	pthread_mutex_t	hotpixel_mutex;
	pthread_mutex_t	selfdark_mutex;
	pthread_mutex_t	bgbuffer_mutex;
	pthread_mutex_t	nhits_mutex;
	pthread_mutex_t	framefp_mutex;
	pthread_mutex_t	powderfp_mutex;
	pthread_mutex_t	peaksfp_mutex;
	
	
	// Detector geometry
	//long			pix_nx;
	//long			pix_ny;
	//long			pix_nn;
	//float			*pix_x;
	//float			*pix_y;
	//float			*pix_z;
	//float			*pix_r;
	//float			*pix_kx; // this is reciprocal space (inverse A, no factor of 2*pi)
	//float			*pix_ky;
	//float			*pix_kz;
	//float			*pix_kr;
	//float			*pix_res;
	//float			pix_dx;
	//unsigned		module_rows;
	//unsigned		module_cols;
	//long			image_nx;
	//long			image_nn;
	//long			asic_nx;
	//long			asic_ny;
	//long			asic_nn;
	//long			nasics_x;
	//long			nasics_y;
    //float			radial_max;
	//long			radial_nn;

	//float			detectorZprevious;	
	//float			detposprev;	
	
	
	/*
	 *	Common variables
	 */
	//int32_t			*darkcal;
	//int16_t			*peakmask;
	//int16_t			*badpixelmask;
	//int16_t			*bg_buffer;
	//int16_t			*hotpix_buffer;
	//int16_t			*hotpixelmask;
	//int16_t			*wiremask;
	//float			*selfdark;
	//float			*gaincal;
	float			avgGMD;
	
	/*
	 *	Powder patterns/sums
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
     *  Radial stacks
     */
	long			radialStackCounter[MAX_POWDER_CLASSES];
	float			*radialAverageStack[MAX_POWDER_CLASSES];
	pthread_mutex_t	radialStack_mutex[MAX_POWDER_CLASSES];
	


	long			npowderHits;
	long			npowderBlanks;

	long			nprocessedframes;
	long			nhits;
	long			nrecentprocessedframes;
	long			nrecenthits;
	//double			detectorZ;
	//double			detectorEncoderValue;	
	
	time_t			tstart, tend;
	time_t			tlast, tnow;
	clock_t			lastclock;
	float			datarate;
	long			lastTimingFrame;

	// Attempt to fix missing EVR41 signal based on Acqiris signal?
	int			fudgeevr41;		
	
public:
	void defaultConfiguration(void);
	void parseConfigFile(char *);
	void parseCommandLineArguments(int, char**);
	void setup(void);
	//void readDetectorGeometry(char *);
    //void updateKspace(float);
	//void readDarkcal(char *);
	//void readGaincal(char *);
	//void readPeakmask(char *);
	//void readBadpixelMask(char *);
	//void readWireMask(char *);
	
	void writeInitialLog(void);
	void updateLogfile(void);
	void writeFinalLog(void);

	
private:
	void parseConfigTag(char*, char*);

	
};
