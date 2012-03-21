/*
 *  worker.h
 *  cheetah
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */






/*
 *	Structure used for passing information to worker threads
 */
typedef struct {
	
	// Reference to common global structure
	cGlobal		*pGlobal;
	int			busy;
	long		threadNum;
	
	// Detector data
	cPixelDetectorEvent		detector[MAX_DETECTORS];
	
	// Aqiris data
	int			TOFPresent;
	double		*TOFTime;
	double		*TOFVoltage;
	double		TOFtrigtime ;
	
	// 120Hz visible camera
	int				xppSb3Fail;
	int				xppSb3Width, xppSb3Height;
	unsigned short	*xppSb3Image;

	
	
	
	// Peak info
	int			nPeaks;
	int			nHot;
	long		*peak_com_index;		// closest pixel corresponding to peak center of mass
	float		*peak_com_x;			// peak center of mass x (in raw layout)
	float		*peak_com_y;			// peak center of mass y (in raw layout)
	float		*peak_com_x_assembled;	// peak center of mass x (in assembled layout)
	float		*peak_com_y_assembled;	// peak center of mass y (in assembled layout)
	float		*peak_com_r_assembled;	// peak center of mass r (in assembled layout)
	float		*peak_intensity;		// integrated peak intensities
	float		*peak_npix;				// Number of pixels in peak
	float       *peak_snr;           // Signal-to-noise of peak
	float		peakResolution;			// Radius of 80% of peaks
	float		peakDensity;			// Density of peaks within this 80% figure
	float		peakNpix;				// Number of pixels in peaks
	float		peakTotal;				// Total integrated intensity in peaks
	int			*good_peaks;           // Good peaks, after post peak-finding criteria	
	
	// Beamline data, etc
	int			seconds;
	int			nanoSeconds;
	unsigned	fiducial;
	char		timeString[1024];
	char		eventname[1024];
	bool		beamOn;
	unsigned	runNumber;

	double		photonEnergyeV;		// in eV
	double		wavelengthA;		// in Angstrom
	
	double      gmd1;
    double      gmd2;
	double		gmd11;
	double		gmd12;
	double		gmd21;
	double		gmd22;
    
	bool		laserEventCodeOn;
    double      laserDelay;
	
	double		fEbeamCharge;		// in nC
	double		fEbeamL3Energy;		// in MeV
	double		fEbeamLTUPosX;		// in mm
	double		fEbeamLTUPosY;		// in mm
	double		fEbeamLTUAngX;		// in mrad
	double		fEbeamLTUAngY;		// in mrad
	double		fEbeamPkCurrBC2;	// in Amps
	
	double		phaseCavityTime1;
	double		phaseCavityTime2;
	double		phaseCavityCharge1;
	double		phaseCavityCharge2;
	
	// Thread management
	int		threadID;
	
} tEventData;




static uint32_t nevents = 0;


#define ERROR(...) fprintf(stderr, __VA_ARGS__)
#define STATUS(...) fprintf(stderr, __VA_ARGS__)

#define DEBUGL1_ONLY if(global->debugLevel >= 1)
#define DEBUGL2_ONLY if(global->debugLevel >= 2)

/*
 *	Function prototypes
 */
void *worker(void *);
void assemble2Dimage(tEventData*, cGlobal*, int);
void checkSaturatedPixels(tEventData *eventData, cGlobal *global, int);

// detectorCorrection.cpp
//void subtractDarkcal(tEventData*, cGlobal*, int);
void subtractDarkcal(cPixelDetectorEvent, cPixelDetectorCommon);
void applyGainCorrection(cPixelDetectorEvent, cPixelDetectorCommon);
void applyBadPixelMask(cPixelDetectorEvent, cPixelDetectorCommon);
void cmModuleSubtract(tEventData*, cGlobal*, int);
void cmSubtractUnbondedPixels(tEventData*, cGlobal*, int);
void cmSubtractBehindWires(tEventData*, cGlobal*, int);
void calculateHotPixelMask(cGlobal*, int);
void killHotpixels(tEventData*, cGlobal*, int);


// backgroundCorrection.cpp
void updateBackgroundBuffer(tEventData*, cGlobal*, int);
void calculatePersistentBackground(cGlobal*, int);
void subtractPersistentBackground(tEventData*, cGlobal*, int);
void subtractLocalBackground(tEventData*, cGlobal*, int);

// saveFrame.cpp
void nameEvent(tEventData*, cGlobal*);
void writeHDF5(tEventData*, cGlobal*);
void writePeakFile(tEventData *eventData, cGlobal *global);
void writeSimpleHDF5(const char*, const void*, int, int, int);


// hitfinders.cpp
int  hitfinder(tEventData*, cGlobal*, int);

// powder.cpp
void addToPowder(tEventData*, cGlobal*, int, int);
void saveRunningSums(cGlobal*, int);
void saveDarkcal(cGlobal*, int);
void saveGaincal(cGlobal*, int);
void savePowderPattern(cGlobal*, int, int);
void writePowderData(char*, void*, int, int, void*, void*, long, long, int);


// RadialAverage.cpp
void addToRadialAverageStack(tEventData*, cGlobal*, int, int);
void saveRadialAverageStack(cGlobal*, int, int);
void saveRadialStacks(cGlobal*);

void calculateRadialAverage(float*, float*, float*, cGlobal*, int);
void calculateRadialAverage(double*, double*, double*, cGlobal*, int);

// median.cpp
int16_t kth_smallest(int16_t*, long, long);

// fudge...
void evr41fudge(tEventData *t, cGlobal *g);



