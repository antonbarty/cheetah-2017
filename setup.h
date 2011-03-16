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
	// Switches for processing options
	int			startFrames;
	int			cmModule;
	int			cmSubModule;
	int			generateDarkcal;
	int			subtractBg;
	int			subtractDarkcal;
	int			selfDarkcal;
	int			hitfinder;
	int			savehits;
	int			powdersum;
	int			saveRaw;
	int			debugLevel;
	int			hdf5dump;
	int			autohotpixel;
	
	
	// Power user settings
	float		cmFloor;
	int			saveInterval;
	int			powderthresh;
	int			hitfinderADC;
	int			hitfinderNAT;
	int			hitfinderNpeaks;
	int			hitfinderNpeaksMax;
	int			hitfinderAlgorithm;
	int			hitfinderMinPixCount;
	int			hitfinderMaxPixCount;
	int			hitfinderCluster;
	int			hitfinderUsePeakmask;
	int			scaleDarkcal;
	int			hotpixADC;
	int			hotpixMemory;
	float		hotpixFreq;
	float		selfDarkMemory;
	
	
	// Configuration files
	char		configFile[1024];
	char		geometryFile[1024];
	char		darkcalFile[1024];
	char		gaincalFile[1024];
	char		peaksearchFile[1024];
	char		logfile[1024];
	char		framefile[1024];
	char		cleanedfile[1024];
	FILE		*framefp;
	FILE		*cleanedfp;
	
	// Run information
	unsigned	runNumber;
	
	
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
	
	void writeInitialLog(void);
	void updateLogfile(void);
	void writeFinalLog(void);

	
private:
	void parseConfigTag(char*, char*);

	
};

