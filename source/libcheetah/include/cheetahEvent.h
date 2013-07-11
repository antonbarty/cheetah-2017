/*
 *  cheetahEvent.h
 *  cheetah
 *
 *  Created by Anton Barty on 6/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *
 */


#include <stdarg.h>


#ifndef CHEETAHEVENT_H
#define CHEETAHEVENT_H

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
	uint stackSlice;
	bool writeFlag;
	
	// Detector data
	cPixelDetectorEvent		detector[MAX_DETECTORS];

	// Misc. EPICS data
	float       epicsPvFloatValues[MAX_EPICS_PVS];
	
	// Aqiris data
	int			TOFPresent;
	double		*TOFTime;
	double		*TOFVoltage;
	double		TOFtrigtime ;
	
	// Pulnix 120Hz visible camera
	int				pulnixFail;
	int				pulnixWidth, pulnixHeight;
	unsigned short	*pulnixImage;
	
	// Opal2k energy spectrum camera
	int             specFail;
	int             specWidth, specHeight;
	unsigned short  *specImage;
	
	// energy spectrum data
	int             energySpectrumExist;
	double          *energySpectrum1D;


	// Hit finding
	int			hit;
	
	
	// Peak info
	int	        nPeaks;
	int	        nHot;
	long		*peak_com_index;		// closest pixel corresponding to peak center of mass
	float		*peak_com_x;			// peak center of mass x (in raw layout)
	float		*peak_com_y;			// peak center of mass y (in raw layout)
	float		*peak_com_x_assembled;	// peak center of mass x (in assembled layout)
	float		*peak_com_y_assembled;	// peak center of mass y (in assembled layout)
	float		*peak_com_r_assembled;	// peak center of mass r (in assembled layout)
	float		*peak_intensity;		// integrated peak intensities
	float		*peak_npix;				// Number of pixels in peak
	float           *peak_snr;           // Signal-to-noise of peak
	float		peakResolution;			// Radius of 80% of peaks
	float		peakResolutionA;			// Radius of 80% of peaks
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
	char		eventStamp[1024];
	char		eventSubdir[1024];

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
    
	bool        laserEventCodeOn;
	double      laserDelay;
	int         samplePumped;
	
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
	
} ;


#define ERROR(...) cheetahError(__FILE__, __LINE__, __VA_ARGS__)

void static cheetahError(const char *filename, int line, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	fprintf(stderr,"CHEETAH-ERROR in %s:%d: ",filename,line);
	vfprintf(stderr,format,ap);
	va_end(ap);
	puts("");
	abort();
}


#define STATUS(...) fprintf(stderr, __VA_ARGS__)

#define DEBUGL1_ONLY if(global->debugLevel >= 1)
#define DEBUGL2_ONLY if(global->debugLevel >= 2)


#endif

