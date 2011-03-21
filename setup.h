/*
 *  setup.cpp
 *  cspad_cryst
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */



/*
 *	Global variables
 */
class cGlobal {
	
public:
	
	/*
	 *	Various switches and processing options
	 */
	// ini file to read
	char		configFile[1024];
	
	// Real-space geometry
	char		geometryFile[1024];		// File containing pixelmap (X,Y coordinate of each pixel in raw data stream)
	float		pixelSize;
	
	// Bad pixel masks
	int			useBadPixelMask;
	char		badpixelFile[1024];
	
	// Static dark calibration (static offsets on each pixel to be subtracted)
	char		darkcalFile[1024];		// File containing dark calibration
	int			useDarkcalSubtraction;	// Subtract the darkcal (or not)?
	int			generateDarkcal;		// Flip this on to generate a darkcal (auto-turns-on appropriate other options)
	
	// Common mode and pedastal subtraction
	int			cmModule;				// Subtract common mode from each ASIC
	int			cmSubModule;			// Subtract common mode from subsets of each ASIC (currently 16 sub-portions)
	float		cmFloor;				// Use lowest x% of values as the offset to subtract (typically lowest 2%)

	// Gain correction
	int			useGaincal;
	int			invertGain;
	char		gaincalFile[1024];
	
	// Running background subtraction
	int			useSubtractPersistentBackground;
	int			subtractBg;
	int			scaleDarkcal;
	float		selfDarkMemory;
	
	// Kill persistently hot pixels
	int			useAutoHotpixel;
	int			hotpixADC;
	int			hotpixMemory;
	float		hotpixFreq;
	int			startFrames;
	
	// Hitfinding
	int			hitfinder;
	int			hitfinderAlgorithm;
	int			hitfinderADC;
	int			hitfinderNAT;
	int			hitfinderNpeaks;
	int			hitfinderNpeaksMax;
	int			hitfinderMinPixCount;
	int			hitfinderMaxPixCount;
	int			hitfinderCluster;
	int			hitfinderUsePeakmask;
	char		peaksearchFile[1024];
	
	// Powder pattern generation
	int			powdersum;
	int			powderthresh;
	int			saveInterval;
	
	// Saving options
	int			savehits;
	int			saveRaw;
	int			hdf5dump;
	
	// Verbosity
	int			debugLevel;
	
	// Log files
	char		logfile[1024];
	char		framefile[1024];
	char		cleanedfile[1024];
	
	
	/*
	 *	Stuff used for managing the program execution
	 */
	// Run information
	unsigned	runNumber;

	// Log file pointers
	FILE		*framefp;
	FILE		*cleanedfp;
	
	// Thread management
	long			nThreads;
	long			nActiveThreads;
	long			threadCounter;
	pthread_t		*threadID;
	pthread_mutex_t	nActiveThreads_mutex;
	pthread_mutex_t	hotpixel_mutex;
	pthread_mutex_t	selfdark_mutex;
	pthread_mutex_t	powdersum1_mutex;
	pthread_mutex_t	powdersum2_mutex;
	pthread_mutex_t	nhits_mutex;
	pthread_mutex_t	framefp_mutex;
	
	
	// Detector geometry
	long			pix_nx;
	long			pix_ny;
	long			pix_nn;
	float			*pix_x;
	float			*pix_y;
	float			*pix_z;
	float			pix_dx;
	unsigned		module_rows;
	unsigned		module_cols;
	long			image_nx;
	long			image_nn;
	
	
	// Common variables
	int32_t			*darkcal;
	int64_t			*powderRaw;
	int64_t			*powderAssembled;
	int16_t			*peakmask;
	int16_t			*badpixelmask;
	float			*hotpixelmask;
	float			*selfdark;
	float			*gaincal;
	float			avgGMD;
	long			npowder;
	long			nprocessedframes;
	long			nhits;
	double			detectorZ;
	
	clock_t			lastclock;
	timeval			lasttime;	
	float			datarate;
	time_t			tstart, tend;

	
	
public:
	void defaultConfiguration(void);
	void parseConfigFile(char *);
	void parseCommandLineArguments(int, char**);
	void setup(void);
	void readDetectorGeometry(char *);
	void readDarkcal(char *);
	void readGaincal(char *);
	void readPeakmask(char *);
	void readBadpixelMask(char *);
	
	void writeInitialLog(void);
	void updateLogfile(void);
	void writeFinalLog(void);

	
private:
	void parseConfigTag(char*, char*);

	
};

