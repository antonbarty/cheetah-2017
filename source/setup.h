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
	
	// Track some statistics for the log file
	double summedPhotonEnergyeV;
	double summedPhotonEnergyeVSquared;
	double meanPhotonEnergyeV;
	double photonEnergyeVSigma; 

	// Start and stop frames
	long	startAtFrame;
	long	stopAtFrame;
	
	int			generateDarkcal;		// Flip this on to generate a darkcal (auto-turns-on appropriate other options)

	int			generateGaincal;		// Flip this on to generate a gaincal (auto-turns-on appropriate other options)
    
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
	int			powderSumHits;
	int			powderSumBlanks;
	int			powderthresh;
	int			saveInterval;
	int			savePeakInfo;
	int			savePeakList;

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
	
	
	
	
	/*
	 *	Common variables
	 */
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
	
	void writeInitialLog(void);
	void updateLogfile(void);
	void writeFinalLog(void);

	
private:
	void parseConfigTag(char*, char*);

	
};
