/*
 *  setup.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#ifndef CHEETAHGLOBAL_H
#define CHEETAHGLOBAL_H
#include "detectorObject.h"
#define MAX_POWDER_CLASSES 16
#define MAX_DETECTORS 2
#define MAX_FILENAME_LENGTH 1024
#define MAX_EPICS_PVS 10
#define MAX_EPICS_PV_NAME_LENGTH 512

#define POWDER_LOOP for(long powID=0; powID < global->nPowderClasses; powID++)


/** @brief Global variables.
 *
 * Configuration parameters, and things that don't change often.
 */
class cGlobal {

public:

	/** @brief Default constructor (set default values) */
	cGlobal();

	/** @brief What's this? */
	cGlobal     *self;

	/** @brief Path to the global configuration file */
	char     configFile[MAX_FILENAME_LENGTH];
	char     configOutFile[MAX_FILENAME_LENGTH];
	char     cxiFilename[MAX_FILENAME_LENGTH];
	/** @brief Default photon energy. */
	float    defaultPhotonEnergyeV;

	char     epicsPvFloatAddresses[MAX_EPICS_PVS][MAX_EPICS_PV_NAME_LENGTH];
	int      nEpicsPvFloatValues;

	/** @brief Number of pixel-array detectors present. */
	int      nDetectors;

	long frameNumber;

	/** @brief Detector settings that don't change from shot to shot. */
	cPixelDetectorCommon detector[MAX_DETECTORS];

	/** @brief Summed photon energy. */
	double   summedPhotonEnergyeV;
	/** @brief Summed squared photon energy. */
	double   summedPhotonEnergyeVSquared;
	/** @brief Mean photon energy. */
	double   meanPhotonEnergyeV;
	/** @brief Standard deviation in photon energy. */
	double   photonEnergyeVSigma;

	/** @brief Skip all frames prior to this one. */
	long     startAtFrame;
	/** @brief Skip all frames after this one. */
	long     stopAtFrame;
	/** @brief Skip a random fraction of the given size (value between 0. and 1.). */
	float    skipFract;

	/** @brief Toggle the creation of a darkcal image. */
	int      generateDarkcal;
	/** @brief Toggle the creation of a gaincal image. */
	int      generateGaincal;


	/** @brief Toggle the usage of a hitfinder. */
	int      hitfinder;
	/** @brief Which detector to use for hitfinding (only one is currently used). */
	int      hitfinderDetector;
	/** @brief Specify the hitfinder algorithm. */
	int      hitfinderAlgorithm;
	/** @brief Intensity threshold for hitfinder algorithm. */
	int      hitfinderADC;
	/** @brief What's this? */
	float    hitfinderTAT;
	/** @brief Minimum number of Bragg peaks that constitute a hit. */
	int      hitfinderNpeaks;
	/** @brief Maximum number of Bragg peaks that constitute a hit. */
	int      hitfinderNpeaksMax;
	int      hitfinderPeakBufferSize;
	/** @brief Minimum number of connected pixels in a Bragg peak. */
	int      hitfinderMinPixCount;
	/** @brief Maximum number of connected pixels in a Bragg peak. */
	int      hitfinderMaxPixCount;
	/** @brief Toggle gradient checks during peakfinding. */
	int      hitfinderCheckGradient;
	/** @brief Minimum acceptable gradient for a Bragg peak. */
	float    hitfinderMinGradient;
	/** @brief What's this? */
	int      hitfinderCluster;
	/** @brief Toggle the useage of a peak mask. */
	int      hitfinderUsePeakmask;
	/** @brief Path to the peak mask file. */
	char     peaksearchFile[MAX_FILENAME_LENGTH];
	/** @brief Toggle the useage of the TOF-based hitfinder.
	 *
	 * Isn't this specified by hitfinderAlgorithm? */
	int      hitfinderUseTOF;
	/** @brief First sample in the TOF scan to consider. */
	int      hitfinderTOFMinSample;
	/** @brief Last sample in the TOF scan to consider. */
	int      hitfinderTOFMaxSample;
	/** @brief Intensity threshold of TOF for hitfinding. */
	double   hitfinderTOFThresh;
	/** @brief Window used for moving average in some TOF hitfinding. */
	double   hitfinderTOFWindow;
	/** @brief Peak count constraint used in some TOF hitfinding. */
	double   hitfinderTOFMinCount;
	/** @brief Toggle the checking of peak separations. */
	int      hitfinderCheckPeakSeparation;
	/** @brief The maximum allowable separation between Bragg peaks. */
	float    hitfinderMinPeakSeparation;
	/** @brief Toggle the subtraction of local background. */
	int      hitfinderSubtractLocalBG;
	/** @brief Inner radius of the local background annulus. */
	int      hitfinderLocalBGRadius;
	/** @brief Thickness of the local background annulus. 
	 *
	 * The outer radius of the annulus is thus hitfinderLocalBGradius +
	 * hitfinderLocalBGThickness.*/
	int      hitfinderLocalBGThickness;
	/** @brief Minimum resolution to be considered in hitfinding.
	 * If the unit is Angstrom (hitfinderResolutionUnitPixel==0) this means the minimum (smallest) resolution element.
	 * If the unit is pixel (hitfinderResolutionUnitPixel==1) this means the minimum distance from the center.
	 */
	float    hitfinderMinRes;
	/** @brief The maximum resolution to be considered in hitfinding.
	 * If the unit is Angstrom (hitfinderResolutionUnitPixel==0) this means the maximum (largest) resolution element. 
	 * If the unit is pixel (hitfinderResolutionUnitPixel==1) this means the maximum distance from the center.
	 */
	float    hitfinderMaxRes;
	/** @brief If set to "1" hitfinderMinRes und hitfinderMaxRes will be interpreted in unit detector pixel
	 * and not angstrom. See hitfinderMinRes and hitfinderMaxRes for more details. 
	 */
	int      hitfinderResolutionUnitPixel;
	/** @brief Binary map of pixels excluded based on resolution. */
	int     *hitfinderResMask;
	/** @brief The minimum signal/noise ratio for peakfinding purposes. */
	float    hitfinderMinSNR;
	/** @brief Toggle ignoring halo pixels during hitfinding. */
	int      hitfinderIgnoreHaloPixels;
	/** @brief Downsampling factor that will be applied to pattern during hitfinding (decoupled from output). */
	long      hitfinderDownsampling;
	/** @brief Data for hitfinding only based on detector corrected data (photon correction ignored for hitfinding). Only hitfinder 1. */
	long      hitfinderOnDetectorCorrectedData;

	int		hitfinderFastScan;

	/** @brief Name of the time-of-flight instrument? */
	char     tofName[MAX_FILENAME_LENGTH];
	/** @brief Indicate the presence of TOF data. */
	int      TOFPresent;
	/** @brief Channel of the TOF instrument. */
	int      TOFchannel;
	/** @brief What's this? */
	int      AcqNumChannels;
	/** @brief What's this? */
	int      AcqNumSamples;
	/** @brief What's this? */
	double   AcqSampleInterval;
    
	/** @brief Toggle energy spectrum creation. */
    int      espectrum;
	int      espectrum1D;
	/** @brief Angle of spectra from horizontal (clockwise). */
	double   espectrumTiltAng;
	/** @brief Length in pixels of energy spectrum (same as spectrum camera height). */
	int      espectrumLength;
	/** @brief Width in pixels of energy spectrum (same as spectrum camera width). */
	int      espectrumWidth;
	/** @brief Spread in eV of energy spectrum detector (40eV for the current setup). */
	int      espectrumSpreadeV;
	/** @brief Toggle energy spectrum dark correction. */
	int      espectrumDarkSubtract;
	/** @brief File name of energy spectrum dark calibration file. */
	char     espectrumDarkFile[MAX_FILENAME_LENGTH];
	/** @brief File name of energy spectrum scale calibration file. */
	char     espectrumScaleFile[MAX_FILENAME_LENGTH];


	/** @brief Toggle the creation of a virtual powder pattern from hits. */
	int      powderSumHits;
	/** @brief Toggle the creation of virtual powder patterns from non-hits. */
	int      powderSumBlanks;
    int      powderSumWithBackgroundSubtraction;
	/** @brief Lower intensity threshold for forming powder patterns. */
	float   powderthresh;
	/** @brief Toggle intensity threshold for forming powder patterns. */
	int		usePowderThresh;


	/** @brief Interval between saving of powder patterns, etc. */
	int      saveInterval;
	/** @brief Toggle the writing of Bragg peak information in hdf5 files. */
	int      savePeakInfo;
	/** @brief Toggle the writing of Bragg peak information into a text file. */
	int      savePeakList;


	/** @brief Toggle the writing of radial intensity profile data. */
	int      saveRadialStacks;
	/** @brief The number of radial profiles per data file. */
	long     radialStackSize;

	/** @brief The number of initial calibration frames */
	long       nInitFrames;
	/** @brief flag encoding status of calibration  */
	int       calibrated;

	/** @brief The Epics process variable for the pump laser delay. */
	char     laserDelayPV[MAX_FILENAME_LENGTH];
	/** @brief The pump laser delay. */
	float    laserDelay;


	/** @brief Toggle the writing of hdf5 files for frames containing hits. */
	int      savehits;
	/** @brief Toggle the writing of raw images in hdf5 files. */
	int      saveRaw;
	/** @brief Toggle the writing of assembled (i.e. interpolated) images. */
	int      saveAssembled;
	int      assemble2DMask;
	int      assemble2DImage;
	/** @brief Toggle assemble interpolation mode (0: linear, weight nearest 4 pixels, 1: nearest, pick value of nearest pixel).*/
	int      assembleInterpolation;
	/** @brief Toggle the writing of individual pixelmask. */
	int      savePixelmask;
	/**
	 * @brief Force the writing of hdf5 files (ignoring hit status).
	 *
	 * The value of this parameter sets the automatic saving of hdf5 files.
	 * When set to a value of n, every nth frame will be saved, regardless
	 * of hit status.
	 */
	int      hdf5dump;
	/** @brief Python script to be hosted for shared memory visualization */
	char     pythonFile[MAX_FILENAME_LENGTH];
	int		 h5compress;
    
	/** @brief Output 1 HDF5 per image by default */
	bool saveCXI;

	/** @brief  Only one thread during calibration */
	int useSingleThreadCalibration;


	/** @brief Toggle the verbosity of Cheetah. */
	int      debugLevel;

	/** @brief TODO: Explain what goes here. */
	char     logfile[MAX_FILENAME_LENGTH];
	/** @brief TODO: Explain what goes here. */
	char     framefile[MAX_FILENAME_LENGTH];
	/** @brief TODO: Explain what goes here. */
	char     cleanedfile[MAX_FILENAME_LENGTH];
	/** @brief TODO: Explain what goes here. */
	char     peaksfile[MAX_FILENAME_LENGTH];

	/** @brief Check the file input/output speed, without data processing. */
	int      ioSpeedTest;

	/*
	 *	Stuff used for managing the program execution
	 */
	/** @brief TODO: Where is this used? */
	unsigned runNumber;

	/*
	 *	File management
	 */
	FILE    *framefp;
	FILE    *cleanedfp;
	FILE    *peaksfp;
	
	/*
	 *	Subdir management
	 */
	long	subdirFileCount;
	long	subdirNumber;
	char    subdirName[MAX_FILENAME_LENGTH];

	
	// Thread management
	int      useHelperThreads;
	long     nThreads;
	long     nActiveThreads;
	long     threadCounter;
	long     threadPurge;
	pthread_t  *threadID;
	pthread_mutex_t  nActiveThreads_mutex;
	pthread_mutex_t  hotpixel_mutex;
	pthread_mutex_t  halopixel_mutex;
	pthread_mutex_t  selfdark_mutex;
	pthread_mutex_t  bgbuffer_mutex;
	pthread_mutex_t  nhits_mutex;
	pthread_mutex_t  framefp_mutex;
	pthread_mutex_t  powderfp_mutex;
	pthread_mutex_t  peaksfp_mutex;
	pthread_mutex_t  subdir_mutex;
	pthread_mutex_t  nespechits_mutex;
	pthread_mutex_t  espectrumRun_mutex;
	pthread_mutex_t  espectrumBuffer_mutex;
	pthread_mutex_t  datarateWorker_mutex;
	pthread_mutex_t  saveCXI_mutex;
	pthread_mutex_t  pixelmask_shared_mutex;
	//pthread_mutex_t  hitVector_mutex;
	pthread_mutex_t  gmd_mutex;

	/*
	 *	Common variables
	 */
	float    avgGMD;

	/*
	 *	Powder patterns/sums
	 */
	long     nPowderClasses;
	long     nPowderFrames[MAX_POWDER_CLASSES];
	FILE    *powderlogfp[MAX_POWDER_CLASSES];
	int nPeaksMin[MAX_POWDER_CLASSES];
	int nPeaksMax[MAX_POWDER_CLASSES];


    // counters updated with event data
	long     npowderHits;
	long     npowderBlanks;
	long     nprocessedframes;
	long     nhits;
	long     nrecentprocessedframes;
	long     nrecenthits;
    long     nespechits;
    long nCXIEvents;
    long nCXIHits;
    
	// variable to hold the updating run integrated spectrum
	double  *espectrumRun;
	double  *espectrumBuffer;
	double  *espectrumDarkcal;
	double  *espectrumScale;
	long	espectrumStackSize;
	long	espectrumStackCounter[MAX_POWDER_CLASSES];
	float   *espectrumStack[MAX_POWDER_CLASSES];
	pthread_mutex_t espectrumStack_mutex[MAX_POWDER_CLASSES];

	
	// time keeping
	time_t   tstart, tend;
	time_t   tlast, tnow;
	clock_t  lastclock;
	double    datarate;
	long      lastTimingFrame;
	timeval   datarateWorkerTimevalLast;
	double    datarateWorker;
	double    datarateWorkerMemory;
	long      datarateWorkerSkipCounter;


	// Attempt to fix missing EVR41 signal based on Acqiris signal?
	int      fudgeevr41;
	int      evr41previous;
	double   lasttime;
	int      laserPumpScheme;

public:
	/**
	 * @brief Set the default configuration.
	**/
	void defaultConfiguration(void);
	/**
	 * @brief Parse a global configuration file, update things.
	 *
	 * \usage Should be called only at the beginning of an analysis job.
	 *
	 * \param configFilePath The full path to the configuration file.
	**/
	void parseConfigFile(char * configFilePath);
	/**
	 * @brief TODO: does this work now?
	**/
	void parseCommandLineArguments(int, char**);
	/**
	 * @brief What's this for?
	**/
	void setup(void);
	void updateCalibrated(void);

	void writeInitialLog(void);
	void updateLogfile(void);
    void writeStatus(const char *);
	void writeFinalLog(void);
	void writeConfigurationLog(void);
	void freeMutexes(void);



private:
	int parseConfigTag(char*, char*);

};
#endif
