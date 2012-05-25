/*
 *  cheetah.cpp
 *  cheetah
 *
 *  Created by Anton Barty on 7/2/11.
 *  Copyright 2011 CFEL. All rights reserved.
 *	
 *	This file based on myana_cspad.cc,v 1.6 2010/10/21 22:09:43 by Matt Weaver, SLAC 
 *
 */


#include "lcls/myana/main.hh"
#include "lcls/myana/release/pdsdata/cspad/ConfigV1.hh"
#include "lcls/myana/release/pdsdata/cspad/ConfigV2.hh"
#include "lcls/myana/release/pdsdata/cspad/ConfigV3.hh"
#include "lcls/myana/release/pdsdata/cspad/ElementHeader.hh"
#include "lcls/myana/release/pdsdata/cspad/ElementIterator.hh"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <hdf5.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits>
#include <stdint.h>
#include <unistd.h>


#include "cheetah.h"


static cGlobal		cheetahGlobal;
static long			frameNumber;



using namespace std;
static Pds::CsPad::ConfigV1 configV1;
static Pds::CsPad::ConfigV2 configV2;
static Pds::CsPad::ConfigV3 configV3;

// Cheetah specific
Pds::DetInfo::Device        detectorType[MAX_DETECTORS];
Pds::DetInfo::Detector      detectorPdsDetInfo[MAX_DETECTORS];
unsigned                    configVsn[MAX_DETECTORS];
unsigned                    quadMask[MAX_DETECTORS];
unsigned                    asicMask[MAX_DETECTORS];
Pds::DetInfo::Device		tofType;
Pds::DetInfo::Detector		tofPdsDetInfo;



using namespace Pds;
/*
 *	Return true or false if a given event code is present
 */
bool eventCodePresent(int EvrCode)
{
	int nfifo = getEvrDataNumber();
	for(int i=0; i<nfifo; i++) {
		unsigned eventCode, fiducial, timestamp;
		if (getEvrData(i,eventCode,fiducial,timestamp)) 
			printf("Failed to fetch evr fifo data\n");
		else if (eventCode==EvrCode)
			return true;
	}
	return false;;
}

/*
 *	Beam on or off??
 */
static bool beamOn()
{
	return eventCodePresent(140);
}

/*
 *	Pump laser on or off?? (eventCode 41)
 */
static bool laserOn()
{
	return eventCodePresent(41);
}


// This function is called once at the beginning of the analysis job,
// You can ask for detector "configuration" information here.
void beginjob() {
	printf("User analysis beginjob() routine called.\n");
	
	/*
	 *	Initialise libCheetah
	 */
    cheetahInit(&cheetahGlobal);

    
	/*
	 *	Determine detector type and address
	 *	A list of addresses can be found in:
	 *		release/pdsdata/xtc/Detinfo.hh
	 *		release/pdsdata/xtc/src/Detinfo.cc
	 */
	for(long detID=0; detID < cheetahGlobal.nDetectors; detID++)  {
		if(!strcmp(cheetahGlobal.detector[detID].detectorName, "CxiDs1")) {
			detectorType[detID] = Pds::DetInfo::Cspad;
			detectorPdsDetInfo[detID] = Pds::DetInfo::CxiDs1;
		}
		else if (!strcmp(cheetahGlobal.detector[detID].detectorName, "CxiDs2")) {
			detectorType[detID] = Pds::DetInfo::Cspad;
			detectorPdsDetInfo[detID] = Pds::DetInfo::CxiDs2;
		}
		else if (!strcmp(cheetahGlobal.detector[detID].detectorName, "CxiDsd")) {
			detectorType[detID] = Pds::DetInfo::Cspad;
			detectorPdsDetInfo[detID] = Pds::DetInfo::CxiDsd;
		}
		else if (!strcmp(cheetahGlobal.detector[detID].detectorName, "XppGon")) {
			detectorType[detID] = Pds::DetInfo::Cspad;
			detectorPdsDetInfo[detID] = Pds::DetInfo::XppGon;
		}
		else {
			printf("Error: unknown detector %s\n", cheetahGlobal.detector[detID].detectorName);
			printf("Quitting\n");
			exit(1);
		}
	}

    
    /*
	 *	Determine TOF (Acqiris) address
	 *	A list of addresses can be found in:
	 *		release/pdsdata/xtc/Detinfo.hh
	 *		release/pdsdata/xtc/src/Detinfo.cc
	 */
	if(!strcmp(cheetahGlobal.tofName, "CxiSc1")) {
		tofType = Pds::DetInfo::Acqiris;
		tofPdsDetInfo = Pds::DetInfo::CxiSc1;
	}

    
    
	/*
	 *	New Cspad corrector
	 */
	//corrector = new CspadCorrector(detectorPdsDetInfo[0],0,CspadCorrector::DarkFrameOffset);

}



void fetchConfig()
{
	int fail = 0;
	// cspad config
	for(long detID=0; detID < cheetahGlobal.nDetectors; detID++)  {
		if (getCspadConfig( detectorPdsDetInfo[detID], configV1 )==0) {
			configVsn[detID]= 1;
			quadMask[detID] = configV1.quadMask();
			asicMask[detID] = configV1.asicMask();
			printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask[detID],asicMask[detID],configV1.runDelay());
			printf("\tintTime %d/%d/%d/%d\n", configV1.quads()[0].intTime(), configV1.quads()[1].intTime(), configV1.quads()[2].intTime(), configV1.quads()[3].intTime());
		}
		else if (getCspadConfig( detectorPdsDetInfo[detID], configV2 )==0) {
			configVsn[detID] = 2;
			quadMask[detID] = configV2.quadMask();
			asicMask[detID] = configV2.asicMask();
			printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask[detID],asicMask[detID], configV2.runDelay());
			printf("\tintTime %d/%d/%d/%d\n", configV2.quads()[0].intTime(), configV2.quads()[1].intTime(), configV2.quads()[2].intTime(), configV2.quads()[3].intTime());
		}
		else if (getCspadConfig( detectorPdsDetInfo[detID], configV3 )==0) {
			configVsn[detID]= 3;
			quadMask[detID] = configV3.quadMask();
			asicMask[detID] = configV3.asicMask();
			printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask[detID], asicMask[detID], configV3.runDelay());
			printf("\tintTime %d/%d/%d/%d\n", configV3.quads()[0].intTime(), configV3.quads()[1].intTime(), configV3.quads()[2].intTime(), configV3.quads()[3].intTime());
		}
		else {
			configVsn[detID] = 0;
			printf("Failed to get CspadConfig\n");
            exit(1);
		}
	}
	
    
    /*
     *  Get Acqiris config
     */
	if((fail=getAcqConfig(Pds::DetInfo(0,tofPdsDetInfo,0,Pds::DetInfo::Acqiris,0) , cheetahGlobal.AcqNumChannels, cheetahGlobal.AcqNumSamples, cheetahGlobal.AcqSampleInterval))==0){
		cheetahGlobal.TOFPresent = 1;
		printf("Acqiris configuration: %d channels, %d samples, %lf sample interval\n", cheetahGlobal.AcqNumChannels, cheetahGlobal.AcqNumSamples, cheetahGlobal.AcqSampleInterval);
		if (cheetahGlobal.hitfinderTOFMaxSample > cheetahGlobal.AcqNumSamples){
			printf("hitfinderTOFMaxSample greater than number of TOF samples. hitfinderUseTOF turned off\n");
            if(cheetahGlobal.hitfinderUseTOF){
                printf("HitfinderUseTOF=%i needs Acqiris data to work\n",cheetahGlobal.hitfinderUseTOF);
                printf("*** Cheetah quitting ***\n\n");
                exit(1);
            }
			cheetahGlobal.hitfinderUseTOF = 0;
		}
	}
	else {
		cheetahGlobal.TOFPresent = 0;
		printf("Failed to get AcqirisConfig with fail code %d.\n", fail);
        if(cheetahGlobal.hitfinderUseTOF){
            printf("HitfinderUseTOF=%i needs Acqiris data to work\n",cheetahGlobal.hitfinderUseTOF);
            printf("*** Cheetah quitting ***\n\n");
            exit(1);
        }
		cheetahGlobal.hitfinderUseTOF = 0 ;
	}

}


/*
 * 	This function is called once for each run.  You should check to see
 *  	if detector configuration information has changed.
 */
void beginrun() 
{
	printf("User analysis beginrun() routine called.\n");    
	printf("Processing r%04u\n",getRunNumber());
	fetchConfig();
	frameNumber = 0;


    /*
	 *	Pass new run information to Cheetah
	 */
	cheetahGlobal.runNumber = getRunNumber();
    cheetahNewRun(&cheetahGlobal);
}


/*
 *	Calibration
 */
void begincalib()
{
	printf("User analysis begincalib() routine called.\n");
	fetchConfig();
}



/*
 *	This function is called once per shot
 */
void event() {
	
        
	// Variables
	frameNumber++;
	int fail = 0;
        
    /*
     *  Calculate time beteeen processing of data frames
     */
    time_t	tnow;
    double	dtime, datarate;    
    time(&tnow);
    
    dtime = difftime(tnow, cheetahGlobal.tlast);
    if(dtime > 0) {
        datarate = (frameNumber - cheetahGlobal.lastTimingFrame)/dtime;
        cheetahGlobal.lastTimingFrame = frameNumber;
        time(&cheetahGlobal.tlast);
        
        cheetahGlobal.datarate = datarate;
    }

    /*
     *  Raw I/O speed test
     *  How fast is event() being called by myana?
     *  This is the fastest we can ever hope to run.
     */
    if(cheetahGlobal.ioSpeedTest==1) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test #1\n", cheetahGlobal.runNumber, frameNumber, cheetahGlobal.datarate);		
		return;
	}

    

	
	/*
	 *	Get run number
	 */
	unsigned runNumber;
	runNumber = getRunNumber();
	
	
	/*
	 * Get event time information
	 */
	int seconds, nanoSeconds;
	getTime( seconds, nanoSeconds );
    
	/*
	 *	Get event fiducials
	 */
	unsigned		fiducial;
	fail = getFiducials(fiducial);
    
	

	/*
	 *	Get event information
	 */	 
	int 			numEvrData;
	unsigned int 	eventCode;
	unsigned int 	timeStamp;	
	numEvrData = getEvrDataNumber();
	for (long i=0; i<numEvrData; i++) {
		fail = getEvrData( i, eventCode, fiducial, timeStamp );
	}
	
    
	/* 
	 *	Is the beam on?
	 */
	bool beam = beamOn();
	if(!beam)
		return;
  
	
	/*
	 * Get electron beam parameters from beamline data and calculate resonant photon energy
	 *
	 * To first order, photon energy is given by the standard undulator equation. 
	 *	This is the energy at which SASE lasing initiates.  Calculation requires such stuff 
	 *	as undulator K-factors (known from the undulator design and calibrations).  
	 *	A summary of undulator radiation formulae can be found here (in section B.2 - undulator radiation)
	 *	http://xdb.lbl.gov/Section2/Sec_2-1.html
	 * The other terms are corrections to the electron energy due to wakefield losses and the like and 
	 *	make for relatively small corrections (<1%) to the undulator energy. 
	 *
	 * Here's an e-mail description:
	 * Marc,
	 * The formula calculates the resonant photon energy based on an estimate of the electron beam 
	 *	energy at the first in-line undulator segment, and the K of that segment.
	 * The energy estimate for the first in-line segment starts with the DL2 bend energy, 
	 *	and on a shot by shot basis, adds a correction based on the DL2 bpms for the incoming energy, 
	 *	a correction for the wakefield energy loss which depends on the measured peak bunch current 
	 *	(averaged over 10 shots), and a correction for the spontaneous energy loss due to emission 
	 *	from the undulator segments.
	 * We still have some problems coming up with good values for wakeloss, especially if beam 
	 *	conditions are unusual. Also there may be a slight shift between the resonant photon energy
	 *	and the FEL peak of the spectrum. Another factor that is uncertain is what exact value to use
	 *	for the K in the resonance formula. Any suggestions would be appreciated.
	 * The matlab code is below (* not included here *). 
	 *	-- Jim
	 *
	 */     
	double fEbeamCharge;    // in nC
	double fEbeamL3Energy;  // in MeV
	double fEbeamLTUPosX;   // in mm
	double fEbeamLTUPosY;   // in mm
	double fEbeamLTUAngX;   // in mrad
	double fEbeamLTUAngY;   // in mrad
	double fEbeamPkCurrBC2; // in Amps
	double photonEnergyeV;
	double wavelengthA;
	
	if ( getEBeam(fEbeamCharge, fEbeamL3Energy, fEbeamLTUPosX, fEbeamLTUPosY,
	              fEbeamLTUAngX, fEbeamLTUAngY, fEbeamPkCurrBC2) ) {
		
		// If no beamline data, but default wavelength specified in ini file
		// then use that
		printf("getEBeam error: unable to calculate wavelength etc.\n");
		if ( cheetahGlobal.defaultPhotonEnergyeV != 0 ) {
			photonEnergyeV = cheetahGlobal.defaultPhotonEnergyeV;
			wavelengthA = 12398.42/photonEnergyeV;
		} else {
			wavelengthA = std::numeric_limits<double>::quiet_NaN();
			photonEnergyeV = std::numeric_limits<double>::quiet_NaN();
		}
		printf("wavelengthA = %g\n", wavelengthA);

	} else {
		
		/* Calculate the resonant photon energy (ie: photon wavelength) */
		// Get the present peak current in Amps
		double peakCurrent = fEbeamPkCurrBC2;
		// Get present beam energy [GeV]
		double DL2energyGeV = 0.001*fEbeamL3Energy;
		// wakeloss prior to undulators
		double LTUwakeLoss = 0.0016293*peakCurrent;
		// Spontaneous radiation loss per segment
		double SRlossPerSegment = 0.63*DL2energyGeV;
		// wakeloss in an undulator segment
		double wakeLossPerSegment = 0.0003*peakCurrent;
		// energy loss per segment
		double energyLossPerSegment = SRlossPerSegment + wakeLossPerSegment;
		// energy in first active undulator segment [GeV]
		double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss
		- 0.0005*energyLossPerSegment;
		// Calculate the resonant photon energy of the first active segment
		photonEnergyeV = 44.42*energyProfile*energyProfile;
		// Calculate wavelength in Angstrom
		wavelengthA = 12398.42/photonEnergyeV;
	}
	
	/*
	 * 	FEE gas detectors (pulse energy in mJ)
	 */     
	double gasdet[4];
	double gmd1;
	double gmd2;
	if ( getFeeGasDet(gasdet) ) {
		gmd1 = std::numeric_limits<double>::quiet_NaN();
		gmd2 = std::numeric_limits<double>::quiet_NaN();
	} else {
		gmd1 = (gasdet[0]+gasdet[1])/2;
		gmd2 = (gasdet[2]+gasdet[3])/2;
	}
	
	
	/*
	 * Phase cavity data
	 */     
	double 	phaseCavityTime1;
	double	phaseCavityTime2;
	double	phaseCavityCharge1;
	double	phaseCavityCharge2;
	fail = getPhaseCavity(phaseCavityTime1, phaseCavityTime2, phaseCavityCharge1, phaseCavityCharge2);
	
    
	/*
     *  Laser delay setting
     */
    float laserDelay = 0;
    if( getPvFloat(cheetahGlobal.laserDelayPV, laserDelay) == 0 ) {
        printf("New laser delay: %f\n",laserDelay);
    }

    
    
	/*
	 *	Detector position (Z) for each detector
     *  This encoder can be flakey and is fixed in the event processing loop
	 */
    float detposnew;
    float detectorPosition[MAX_DETECTORS];
    for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
		fail = getPvFloat(cheetahGlobal.detector[detID].detectorZpvname, detposnew);
        if (  fail == 0 ) {
            detectorPosition[detID] = detposnew;
        }
        else {
			printf("DetectorPosition[%i]: getPvFloat failed (%i)\n", detID, fail);
            detectorPosition[detID] = std::numeric_limits<float>::quiet_NaN();
        }
		printf("DetectorPosition[%i] = %g\n", detID, detectorPosition[detID]);
    }    
    
    
	
    
    
	/*
	 *	Create a new eventData structure in which to place all information
	 */
	cEventData	*eventData;
	eventData = cheetahNewEvent();
		
	
	/*
	 *	Copy all interesting information into worker thread structure if we got this far.
     *  SLAC libraries are NOT thread safe: any event info may get overwritten by the next event() call
     *  Copy all image data into event structure for processing
	 */
    eventData->frameNumber = frameNumber;
	eventData->seconds = seconds;
	eventData->nanoSeconds = nanoSeconds;
	eventData->fiducial = fiducial;
	eventData->runNumber = getRunNumber();
	eventData->beamOn = beam;
	eventData->nPeaks = 0;
    
	eventData->laserEventCodeOn = laserOn();
    eventData->laserDelay = cheetahGlobal.laserDelay;
	
    eventData->gmd1 = gmd1;
    eventData->gmd2 = gmd2;
	eventData->gmd11 = gasdet[0];
	eventData->gmd12 = gasdet[1];
	eventData->gmd21 = gasdet[2];
	eventData->gmd22 = gasdet[3];
	
	eventData->fEbeamCharge = fEbeamCharge;		// in nC
	eventData->fEbeamL3Energy = fEbeamL3Energy;	// in MeV
	eventData->fEbeamLTUPosX = fEbeamLTUPosX;		// in mm
	eventData->fEbeamLTUPosY = fEbeamLTUPosY;		// in mm
	eventData->fEbeamLTUAngX = fEbeamLTUAngX;		// in mrad
	eventData->fEbeamLTUAngY = fEbeamLTUAngY;		// in mrad
	eventData->fEbeamPkCurrBC2 = fEbeamPkCurrBC2;	// in Amps
	eventData->photonEnergyeV = photonEnergyeV;	// in eV
	eventData->wavelengthA = wavelengthA;			// in Angstrom
	
	eventData->phaseCavityTime1 = phaseCavityTime1;
	eventData->phaseCavityTime2 = phaseCavityTime2;
	eventData->phaseCavityCharge1 = phaseCavityCharge1;
	eventData->phaseCavityCharge1 = phaseCavityCharge2;
	
	eventData->pGlobal = &cheetahGlobal;
	
	for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
        eventData->detector[detID].detectorZ = detectorPosition[detID];
    }


	/*
	 *	Copy raw cspad image data into Cheetah event structure for processing
     *  SLAC libraries are not thread safe: must copy data into event structure for processing
	 */
	for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {

        uint16_t *quad_data[4];        
        Pds::CsPad::ElementIterator iter;
        
		fail=getCspadData(detectorPdsDetInfo[detID], iter);
		if (fail) {
			printf("getCspadData fail %d (%x)\n",fail,fiducial);
			eventData->detector[detID].cspad_fail = fail;
			return;
		}
		else {
            nevents++;

            long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
			long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
			long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
            long    nasics_x = cheetahGlobal.detector[detID].nasics_x;
            long    nasics_y = cheetahGlobal.detector[detID].nasics_y;
            uint16_t    *quad_data[4];

            
            // Allocate memory for detector data and set to zero
            for(int quadrant=0; quadrant<4; quadrant++) {
                quad_data[quadrant] = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
                memset(quad_data[quadrant], 0, pix_nn*sizeof(uint16_t));
            }
            

			// loop over elements (quadrants)
			const Pds::CsPad::ElementHeader* element;
			while(( element=iter.next() )) {  
				if(element->quad() < 4) {
					// Which quadrant is this?
					int quadrant = element->quad();
					
					// Read 2x1 "sections" into data array in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
					const Pds::CsPad::Section* s;
					unsigned section_id;
					while(( s=iter.next(section_id) )) {  
						memcpy(&quad_data[quadrant][section_id*2*asic_nx*asic_ny],s->pixel[0],2*asic_nx*asic_ny*sizeof(uint16_t));
					}

                    // Get temperature on strong back, just in case we want it for anything 
					//float	temperature = std::numeric_limits<float>::quiet_NaN();;
					//eventData->detector[detID].quad_temperature[quadrant] = temperature;
				}
			}
        
        
			/*
			 *	Assemble data from all four quadrants into one large array (rawdata layout)
             *      Memcpy is necessary for thread safety.
			 */
			eventData->detector[detID].raw_data = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
			for(int quadrant=0; quadrant<4; quadrant++) {
				long	i,j,ii;
				for(long k=0; k<2*asic_nx*8*asic_ny; k++) {
					i = k % (2*asic_nx) + quadrant*(2*asic_nx);
					j = k / (2*asic_nx);
					ii  = i+(cheetahGlobal.detector[detID].nasics_x*asic_nx)*j;
					
					eventData->detector[detID].raw_data[ii] = quad_data[quadrant][k];
				}
			}

            // quadrant data no longer needed
			for(int quadrant=0; quadrant<4; quadrant++) 
				free(quad_data[quadrant]);
		}
    }	
	
    
	/*
	 *	Copy TOF (aqiris) channel into Cheetah event for processing
     *  SLAC libraries are not thread safe: must copy data into event structure for processing
	 */
	eventData->TOFPresent = cheetahGlobal.TOFPresent ;	
	if (cheetahGlobal.TOFPresent==1){
		double *tempTOFTime;
		double *tempTOFVoltage;
		double tempTrigTime;
		fail = getAcqValue(Pds::DetInfo(0,tofPdsDetInfo,0,Pds::DetInfo::Acqiris,0), cheetahGlobal.TOFchannel, tempTOFTime,tempTOFVoltage, tempTrigTime);
		if (fail){
			printf("getAcqValue fail %d (%x)\n",fail,fiducial);
			return ;
		}
		//Memcpy is necessary for thread safety.
		eventData->TOFtrigtime = tempTrigTime;
		eventData->TOFTime = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
		eventData->TOFVoltage = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
		memcpy(eventData->TOFTime, tempTOFTime, cheetahGlobal.AcqNumSamples*sizeof(double));
		memcpy(eventData->TOFVoltage, tempTOFVoltage, cheetahGlobal.AcqNumSamples*sizeof(double));
	}
	
    
    
	/*
	 *	Copy Pulnix camera into Cheetah event for processing
     *
	 *	Pulnix 120Hz CCD camera on CXI Questar micrscope
	 *	(where the actual camera is CxiSc1 not XppSb3PimCvd)
	 *	The choice of CxiEndstation is for a particular camera.  
	 *	Here are some of the possible alternatives:
	 *		CxiDg1
	 *		CxiDg2
	 *		CxiDg4
	 *		CxiKb1
	 *		CxiSc1
     *  SLAC libraries are not thread safe: must copy data into event structure for processing
	 */
	int				pulnixWidth, pulnixHeight;
	unsigned short	*pulnixImage;
	DetInfo pulnixInfo(0,DetInfo::CxiSc1, 0, DetInfo::TM6740, 0);
	eventData->pulnixFail = getTm6740Value(pulnixInfo, pulnixWidth, pulnixHeight, pulnixImage);
	if ( eventData->pulnixFail == 0 )
	{
		//Memcpy is necessary for thread safety.
        eventData->pulnixWidth = pulnixWidth;
        eventData->pulnixHeight = pulnixHeight;
        eventData->pulnixImage = (unsigned short*) calloc((long)pulnixWidth*(long)pulnixHeight, sizeof(unsigned short));
        memcpy(eventData->pulnixImage, pulnixImage, (long)pulnixWidth*(long)pulnixHeight*sizeof(unsigned short));
	}

    
    
    
    
    /*
	 *	Call Cheetah to process this event
	 */
    cheetahProcessEvent(&cheetahGlobal, eventData);
	
}
// End of event data processing block




/*
 *	Stuff that happens at the end
 */
void endcalib() {
	printf("User analysis endcalib() routine called.\n");
}

void endrun() 
{
	printf("User analysis endrun() routine called.\n");
}

void endjob()
{
	printf("User analysis endjob() routine called.\n");
	
    /*
	 *	Clean up all variables associated with libCheetah
	 */
    cheetahExit(&cheetahGlobal);
}
