/*
 *  setup.cpp
 *  cheetah
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
	
	// Start and stop frames
	long	startAtFrame;
	long	stopAtFrame;
	
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
	float		cmFloor;				// Use lowest x% of values as the offset to subtract (typically lowest 2%)

	// Gain correction
	int			useGaincal;
	int			invertGain;
	char		gaincalFile[1024];
	int			generateGaincal;		// Flip this on to generate a gaincal (auto-turns-on appropriate other options)
	
	// Running background subtraction
	int			useSubtractPersistentBackground;
	int			subtractBg;
	int			scaleBackground;
	float		bgMedian;
	long		bgMemory;
	long		bgRecalc;
	long		bgCounter;
	long		last_bg_update;
	int			bgIncludeHits;
	int			bgNoBeamReset;
	int			bgFiducialGlitchReset;
	
	// Local background subtraction
	int			useLocalBackgroundSubtraction;
	long		localBackgroundRadius;
	
	
	// Kill persistently hot pixels
	int			useAutoHotpixel;
	int			hotpixADC;
	int			hotpixMemory;
	int			hotpixRecalc;
	float		hotpixFreq;
	long		hotpixCounter;
	long		nhot;
	long		last_hotpix_update;
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
	int			powderSumHits;
	int			powderSumBlanks;
	int			powderthresh;
	int			saveInterval;
	int			savePeakInfo;
	int			savePeakList;
	int			saveDetectorCorrectedOnly;
	int			saveDetectorRaw;

	
	// Saving options
	int			savehits;
	int			saveRaw;
	int			saveAssembled;
	int			hdf5dump;
	
	// Verbosity
	int			debugLevel;
	
	// Log files
	char		logfile[1024];
	char		framefile[1024];
	char		cleanedfile[1024];
	char		peaksfile[1024];
	
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
	pthread_mutex_t	powderHitsRaw_mutex;
	pthread_mutex_t	powderHitsAssembled_mutex;
	pthread_mutex_t	powderHitsRawSquared_mutex;
	pthread_mutex_t	powderBlanksRaw_mutex;
	pthread_mutex_t	powderBlanksAssembled_mutex;
	pthread_mutex_t	powderBlanksRawSquared_mutex;
	pthread_mutex_t	nhits_mutex;
	pthread_mutex_t	framefp_mutex;
	pthread_mutex_t	peaksfp_mutex;
	
	
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
	
	
	/*
	 *	Common variables
	 */
	int32_t			*darkcal;
	int16_t			*peakmask;
	int16_t			*badpixelmask;
	int16_t			*bg_buffer;
	int16_t			*hotpix_buffer;
	int16_t			*hotpixelmask;
	double			*powderHitsRaw;
	double			*powderHitsAssembled;
	double			*powderHitsRawSquared;
	double			*powderBlanksRaw;
	double			*powderBlanksAssembled;
	double			*powderBlanksRawSquared;
	float			*selfdark;
	float			*gaincal;
	float			avgGMD;
	long			npowderHits;
	long			npowderBlanks;
	long			nprocessedframes;
	long			nhits;
	double			detectorZ;
	
	time_t			tstart, tend;
	time_t			tlast, tnow;
	clock_t			lastclock;
	float			datarate;
	long			lastTimingFrame;

	
	
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

