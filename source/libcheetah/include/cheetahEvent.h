/*
 *  cheetahEvent.h
 *  cheetah
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */


#include <stdarg.h>
#include <vector>
#include "peakfinders.h"


#ifndef CHEETAHEVENT_H
#define CHEETAHEVENT_H

// Need to announce this is included from elsewhere
//typedef tPeakList;


/*
 *	Structure used for passing information to worker threads
 */
class cEventData {
	
public:
	// Reference to common global structure
	cGlobal		*pGlobal;
	int			busy;
	long		threadNum;
	long        frameNumber;
	long        frameNumberIncludingSkipped;
	long        frameNum;
	long		stackSlice;
	bool		writeFlag;
	
	char		eventname[1024];
	char		filename[1024];
	char		eventStamp[1024];
	char		eventSubdir[1024];

	
	// Detector data
	cPixelDetectorEvent		detector[MAX_DETECTORS];

	// Misc. EPICS data
	float       epicsPvFloatValues[MAX_EPICS_PVS];
	
	// TOF Detector data
	cTOFDetectorEvent		tofDetector[MAX_TOF_DETECTORS];
	int			TOFPresent;
	
	// Pulnix 120Hz visible camera
	bool			Pulnix_present;
	int				pulnixWidth, pulnixHeight;
	unsigned short	*pulnixImage;
	
	// Opal2k energy spectrum camera
	bool            CXIspec_present;
	int             specWidth, specHeight;
	unsigned short  *CXIspec_image;
	
	// energy spectrum data (2D camera downstream)
	int             energySpectrumExist;
	double          *energySpectrum1D;

	// FEE energy spectrum data
	int				FEEspec_present;
	uint32_t		*FEEspec_hproj;
	uint32_t		*FEEspec_vproj;
	long			FEEspec_hproj_size;
	long			FEEspec_vproj_size;
	
	// Time tool data
	int				TimeTool_present;
	float			*TimeTool_hproj;
	float			*TimeTool_vproj;
	long			TimeTool_width;
	long			TimeTool_height;
	

	// Hit finding
	int			hit;
	float		hitScore;
	int			powderClass;
	
	
    // Tof hitfinding
    int            nProtons;
    
	// Peak list
	tPeakList	peaklist;
	
	
	// Peak info
	int	        nPeaks;
	int	        nHot;
	float		peakResolution;			// Radius of 80% of peaks
	float		peakResolutionA;			// Radius of 80% of peaks
	float		peakDensity;			// Density of peaks within this 80% figure
	float		peakNpix;				// Number of pixels in peaks
	float		peakTotal;				// Total integrated intensity in peaks
	//int			*good_peaks;           // Good peaks, after post peak-finding criteria
	
	
	// Beamline data, etc
	int			seconds;
	int			nanoSeconds;
	unsigned	fiducial;
	char		timeString[1024];

	bool		beamOn;
	unsigned	runNumber;

	double		photonEnergyeV;		// in eV
	double		wavelengthA;		// in Angstrom
	
	double      gmd;
	double      gmd1;
	double      gmd2;
	double		gmd11;
	double		gmd12;
	double		gmd21;
	double		gmd22;
    
	int         pumpLaserOn;
	int         pumpLaserCode;
	double      pumpLaserDelay;
	
	// Position of the sample stage
	double      samplePos[3]; // in um

	// Electrojet voltage
	double      sampleVoltage[1]; // in Volt

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
	int	threadID;
	int     useThreads;

    // APS
    double exposureTime;
    double exposurePeriod;
    double tau;
    int countCutoff;
    int nExcludedPixels;
    double detectorDistance;
    double beamX;
    double beamY;
    double startAngle;
    double angleIncrement;
    double detector2Theta;
    double shutterTime;
	
} ;

#define ERROR(...) cheetahError(__FILE__, __LINE__, __VA_ARGS__)

#define DEBUGL1_ONLY if(global->debugLevel >= 1)
#define DEBUGL2_ONLY if(global->debugLevel >= 2)

#define DEBUG1(...) if(global->debugLevel >= 1) cheetahDebug(__FILE__, __LINE__, __VA_ARGS__)
#define DEBUG2(...) if(global->debugLevel >= 2) cheetahDebug(__FILE__, __LINE__, __VA_ARGS__)
#define DEBUG3(...) if(global->debugLevel >= 3) cheetahDebug(__FILE__, __LINE__, __VA_ARGS__)

#define DEBUG(...) cheetahDebug(__FILE__, __LINE__, __VA_ARGS__)

void cheetahError(const char *filename, int line, const char *format, ...);
void cheetahDebug(const char *filename, int line, const char *format, ...);

#define STATUS(...) fprintf(stderr, __VA_ARGS__)
#define INFO(...) fprintf(stdout, __VA_ARGS__)

#endif

