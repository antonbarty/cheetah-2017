/*
 *  setup.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */

#define MAX_POWDER_CLASSES 16
#define MAX_DETECTORS 2
#define MAX_FILENAME_LENGTH 1024




/********************************************************************//**
 * \brief Global variables.
 *
 * TODO: A detailed explanation of what this class is intended for.  
 * Configuration parameters, and things that don't change often.
************************************************************************/
class cGlobal {

public:

	/********************************************************************//**
	 * \brief What's this for?
	************************************************************************/
	cGlobal     *self;

	/********************************************************************//**
	 * \brief Path to the global configuration file.
	 *
	 * Cheetah can parse this configuration file and populate the members of
	 * cGlobal with user-specified values. TODO: link to the parser function.
	************************************************************************/
	char     configFile[MAX_FILENAME_LENGTH];

	/********************************************************************//**
	 * \brief Default photon wavelength.
	 *
	 * To be used in the event that wavelength information is not provided
	 * (or recorded) in the acquisition data stream.
	 ***********************************************************************/
	float    defaultPhotonEnergyeV;

	/********************************************************************//**
	 * \brief Number of pixel-array detectors present.
	 ***********************************************************************/
	int      nDetectors;

	/********************************************************************//**
	 * \brief Array of pixel-array detector settings?
	 ***********************************************************************/
	cPixelDetectorCommon detector[MAX_DETECTORS];

	double   summedPhotonEnergyeV; /**< For photon wavelength statistics */
	double   summedPhotonEnergyeVSquared; /**< For photon wavelength
	                                           statistics */
	double   meanPhotonEnergyeV; /**< For photon wavelength statistics */
	double   photonEnergyeVSigma; /**< For photon wavelength statistics */

	/********************************************************************//**
	 * \brief Skip all frames prior to this one (no processing will be done).
	 ***********************************************************************/
	long     startAtFrame;
	/********************************************************************//**
	 * \brief Skip all frames after this one (no processing willl be done).
	 ***********************************************************************/
	long     stopAtFrame;

	/********************************************************************//**
	 * \brief Toggle the creation of a darkcal image.
	 *
	 * This run is assumed to contain data without the X-ray beam on.
	 ***********************************************************************/
	int      generateDarkcal;
	/********************************************************************//**
	 * \brief Toggle the creation of a gaincal image.
	 *
	 ***********************************************************************/
	int      generateGaincal;





	/********************************************************************//**
	 * \brief Toggle the usage of a hitfinder
	 *
	 * TODO: link to the hitfinding documentation.
	 ***********************************************************************/
	int      hitfinder;
	/********************************************************************//**
	 * \brief Specify the hitfinder algorithm
	 *
	 * Presently, the acceptable values are 1-6.  TODO: provide documentation
	 * of all the hitfinder algorithms.
	 ***********************************************************************/
	int      hitfinderAlgorithm;
	/********************************************************************//**
	 * \brief Intensity threshold for hitfinder algorithm.
	 *
	 * The influence of this value depends on the hitfinding algorithm.
	 * TODO: provide reference to hitfinder algorithms.
	 ***********************************************************************/
	int      hitfinderADC;
	/********************************************************************//**
	 * \brief Required number of connected pixels that constitute a Bragg
	 * peak.
	 *
	 * TODO: check that this is still in use.
	 ***********************************************************************/
	long     hitfinderNAT;
	/********************************************************************//**
	 * \brief Unknown
	 *
	 * TODO: figure out what this does.
	 ***********************************************************************/
	float    hitfinderTAT;
	/********************************************************************//**
	 * \brief The minimum number of Bragg peaks that constitute a hit.
	 ***********************************************************************/
	int      hitfinderNpeaks;
	/********************************************************************//**
	 * \brief  The maximum number of Bragg peaks that constitute a hit.
	 ***********************************************************************/
	int      hitfinderNpeaksMax;
	/********************************************************************//**
	 * \brief The minimum number of connected pixels in a Bragg peak.
	 ***********************************************************************/
	int      hitfinderMinPixCount;
	/********************************************************************//**
	 * \brief The maximum number of connected pixels in a Bragg peak.
	 ***********************************************************************/
	int      hitfinderMaxPixCount;
	/********************************************************************//**
	 * \brief Toggle the useage of gradient testing during peakfinding.
	 ***********************************************************************/
	int      hitfinderCheckGradient;
	/********************************************************************//**
	 * \brief Minimum acceptable gradient allowable for a Bragg peak.
	 ***********************************************************************/
	float    hitfinderMinGradient;
	/********************************************************************//**
	 * \brief Unknown
	 *
	 * TODO: figure out what this does.
	 ***********************************************************************/
	int      hitfinderCluster;
	/********************************************************************//**
	 * \brief Toggle the useage of a peak mask.
	 ***********************************************************************/
	int      hitfinderUsePeakmask;
	/********************************************************************//**
	 * \brief Path to the peak mask file.
	 ***********************************************************************/
	char     peaksearchFile[MAX_FILENAME_LENGTH];
	/********************************************************************//**
	 * \brief Toggle the useage of the TOF-based hitfinder.
	 *
	 * TODO: Is this really needed? isn't it specified by hitfinderAlgorithm?
	 ***********************************************************************/
	int      hitfinderUseTOF;
	/********************************************************************//**
	 * \brief First sample in the TOF scan to consider.
	 ***********************************************************************/
	int      hitfinderTOFMinSample;
	/********************************************************************//**
	 * \brief Last sample in the TOF scan to consider.
	 ***********************************************************************/
	int      hitfinderTOFMaxSample;
	/********************************************************************//**
	 * \brief Intensity threshold of TOF for hitfinding.
	 ***********************************************************************/
	double   hitfinderTOFThresh;
	/********************************************************************//**
	 * \brief Toggle the checking of Bragg peak separations.
	 *
	 * Closely space Bragg peaks will be eliminated.  TODO: link to hitfinder
	 * algorithms for more details.
	 ***********************************************************************/
	int      hitfinderCheckPeakSeparation;
	/********************************************************************//**
	 * \brief The maximum allowable separation between Bragg peaks.
	 ***********************************************************************/
	float    hitfinderMaxPeakSeparation;
	/********************************************************************//**
	 * \brief Toggle the subtraction of local background.
	 ***********************************************************************/
	int      hitfinderSubtractLocalBG;
	/********************************************************************//**
	 * \brief Inner radius of the local background annulus.
	 *
	 * TODO: link to an explanation of the hitfinder algorithms.
	 ***********************************************************************/
	int      hitfinderLocalBGRadius;
	/********************************************************************//**
	 * \brief Thickness of the local background annulus.
	 *
	 * TODO: link to an explanation of the hitfinder algorithms.
	 ***********************************************************************/
	int      hitfinderLocalBGThickness;
	/********************************************************************//**
	 * \brief Toggle the useage of a resolution-based annulus mask.
	 ***********************************************************************/
	int      hitfinderLimitRes;
	/********************************************************************//**
	 * \brief The minimum resolution to be considered in hitfinding.
	 *
	 * The units are angstroms.  Minimum means the *numerical* minimum.
	 * TODO: link to the hitfinder documenation.
	 ***********************************************************************/
	float    hitfinderMinRes;
	/********************************************************************//**
	 * \brief The maximum resolution to be considered in hitfinding.
	 *
	 * The units are angstroms.  Maximum means the *numerical* maximum.
	 * TODO: link to the hitfinder documenation.
	 ***********************************************************************/
	float    hitfinderMaxRes;
	/********************************************************************//**
	 * \brief Binary map of pixels excluded based on resolution.
	 *
	 * 0 means exclude, 1 means accept.
	 ***********************************************************************/
	int     *hitfinderResMask;
	/********************************************************************//**
	 * \brief The minimum SNR for peakfinding purposes.
	 *
	 * TODO: link to the hitfinder documentation.
	 ***********************************************************************/
	float    hitfinderMinSNR;





	/********************************************************************//**
	 * \brief TODO: name of the time-of-flight instrument.
	 ***********************************************************************/
	char     tofName[MAX_FILENAME_LENGTH];
	/********************************************************************//**
	 * \brief Indicate the presence of TOF data.
	 ***********************************************************************/
	int      TOFPresent;
	/********************************************************************//**
	 * \brief TODO: This doesn't belong here (it's LCLS specific)
	 ***********************************************************************/
	int      TOFchannel;
	/********************************************************************//**
	 * \brief TODO: This doesn't belong here (it's LCLS specific)
	 ***********************************************************************/
	int      AcqNumChannels;
	/********************************************************************//**
	 * \brief TODO: This doesn't belong here (it's LCLS specific)
	 ***********************************************************************/
	int      AcqNumSamples;
	/********************************************************************//**
	 * \brief TODO: This doesn't belong here (it's LCLS specific)
	 ***********************************************************************/
	double   AcqSampleInterval;





	/********************************************************************//**
	 * \brief Toggle the creation of a virtual powder pattern from hits.
	 ***********************************************************************/
	int      powderSumHits;
	/********************************************************************//**
	 * \brief Tiggle the creation of a virtual powder pattern from non-hits.
	 ***********************************************************************/
	int      powderSumBlanks;
	/********************************************************************//**
	 * \brief The intensity threshold for powder pattern formation.
	 *
	 * TODO: link to the powder pattern function.
	 ***********************************************************************/
	int      powderthresh;




	/********************************************************************//**
	 * \brief Interval between saving powder patterns and radial profiles.
	 ***********************************************************************/
	int      saveInterval;
	/********************************************************************//**
	 * \brief Toggle the writing of Bragg peak information in hdf5 files.
	 ***********************************************************************/
	int      savePeakInfo;
	/********************************************************************//**
	 * \brief Toggle the writing of Bragg peak information into a text file.
	 ***********************************************************************/
	int      savePeakList;




	/********************************************************************//**
	 * \brief Toggle the writing of radial intensity profile data.
	 ***********************************************************************/
	int      saveRadialStacks;
	/********************************************************************//**
	 * \brief The number of radial profiles per data file.
	 ***********************************************************************/
	long     radialStackSize;




	/********************************************************************//**
	 * \brief The Epics process variable for the pump laser delay.
	 ***********************************************************************/
	char     laserDelayPV[MAX_FILENAME_LENGTH];
	/********************************************************************//**
	 * \brief The pump laser delay.
	 ***********************************************************************/
	float    laserDelay;




	/********************************************************************//**
	 * \brief Toggle the writing of hdf5 files for frames containing hits.
	 ***********************************************************************/
	int      savehits;
	/********************************************************************//**
	 * \brief Toggle the writing of raw images in hdf5 files.
	 ***********************************************************************/
	int      saveRaw;
	/********************************************************************//**
	 * \brief Toggle the writing of assembled (i.e. interpolatee) images.
	 ***********************************************************************/
	int      saveAssembled;
	/********************************************************************//**
	 * \brief Force the writing of every nth data frame (ignore hit status).
	 ***********************************************************************/
	int      hdf5dump;

	// Verbosity
	int      debugLevel;

	// Log files
	char     logfile[MAX_FILENAME_LENGTH];
	char     framefile[MAX_FILENAME_LENGTH];
	char     cleanedfile[MAX_FILENAME_LENGTH];
	char     peaksfile[MAX_FILENAME_LENGTH];

	// I/O speed test
	int      ioSpeedTest;

	/*
	 *	Stuff used for managing the program execution
	 */
	// Run information
	unsigned runNumber;

	// Log file pointers
	FILE    *framefp;
	FILE    *cleanedfp;
	FILE    *peaksfp;

	// Thread management
	int      useHelperThreads;
	long     nThreads;
	long     nActiveThreads;
	long     threadCounter;
	long     threadPurge;
	pthread_t  *threadID;
	pthread_mutex_t  nActiveThreads_mutex;
	pthread_mutex_t  hotpixel_mutex;
	pthread_mutex_t  selfdark_mutex;
	pthread_mutex_t  bgbuffer_mutex;
	pthread_mutex_t  nhits_mutex;
	pthread_mutex_t  framefp_mutex;
	pthread_mutex_t  powderfp_mutex;
	pthread_mutex_t  peaksfp_mutex;




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

	//double			*powderRaw[MAX_POWDER_CLASSES];
	//double			*powderRawSquared[MAX_POWDER_CLASSES];
	//double			*powderAssembled[MAX_POWDER_CLASSES];
	//pthread_mutex_t	powderRaw_mutex[MAX_POWDER_CLASSES];
	//pthread_mutex_t	powderRawSquared_mutex[MAX_POWDER_CLASSES];
	//pthread_mutex_t	powderAssembled_mutex[MAX_POWDER_CLASSES];

    /*
     *  Radial stacks
     */
	//long			radialStackCounter[MAX_POWDER_CLASSES];
	//float			*radialAverageStack[MAX_POWDER_CLASSES];
	//pthread_mutex_t	radialStack_mutex[MAX_POWDER_CLASSES];



	long     npowderHits;
	long     npowderBlanks;

	long     nprocessedframes;
	long     nhits;
	long     nrecentprocessedframes;
	long     nrecenthits;

	time_t   tstart, tend;
	time_t   tlast, tnow;
	clock_t  lastclock;
	float    datarate;
	long     lastTimingFrame;

	// Attempt to fix missing EVR41 signal based on Acqiris signal?
	int      fudgeevr41;

public:
	/********************************************************************//**
	 * \brief Set the default configuration.
	************************************************************************/
	void defaultConfiguration(void);
	/********************************************************************//**
	 * \brief Parse a global configuration file, update things.
	 *
	 * \usage Should be called only at the beginning of an analysis job.
	 *
	 * \param configFilePath The full path to the configuration file.
	************************************************************************/
	void parseConfigFile(char * configFilePath);
	/********************************************************************//**
	 * \brief TODO: does this work now?
	************************************************************************/
	void parseCommandLineArguments(int, char**);
	/********************************************************************//**
	 * \brief What's this for?
	************************************************************************/
	void setup(void);

	void writeInitialLog(void);
	void updateLogfile(void);
	void writeFinalLog(void);


private:
	void parseConfigTag(char*, char*);


};
