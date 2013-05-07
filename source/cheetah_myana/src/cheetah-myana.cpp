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


#include <main.hh>
#include <pdsdata/cspad/ConfigV1.hh>
#include <pdsdata/cspad/ConfigV2.hh>
#include <pdsdata/cspad/ConfigV3.hh>
#include <pdsdata/cspad/ConfigV4.hh>
#include <pdsdata/cspad/ElementHeader.hh>
#include <pdsdata/cspad/ElementIterator.hh>

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
#include <iostream>
#include <signal.h>


#include <cheetah.h>


static cGlobal		cheetahGlobal;
static long			frameNumber;



using namespace std;
static Pds::CsPad::ConfigV1 configV1;
static Pds::CsPad::ConfigV2 configV2;
static Pds::CsPad::ConfigV3 configV3;
static Pds::CsPad::ConfigV4 configV4;

// Cheetah specific
Pds::DetInfo::Device        detectorType[MAX_DETECTORS];
Pds::DetInfo::Detector      detectorPdsDetInfo[MAX_DETECTORS];
unsigned                    configVsn[MAX_DETECTORS];
unsigned                    quadMask[MAX_DETECTORS];
unsigned                    asicMask[MAX_DETECTORS];
Pds::DetInfo::Device		tofType;
Pds::DetInfo::Detector		tofPdsDetInfo;

static std::string getCXIfromXTC(std::string filename);

using namespace Pds;

void sig_handler(int signo)
{
  if (signo == SIGINT){
    // Wait for threads to finish
    while(cheetahGlobal.nActiveThreads > 0) {
      printf("Waiting for %li worker threads to terminate\n", cheetahGlobal.nActiveThreads);
      usleep(100000);
    }
    printf("Attempting to close CXIs cleanly\n");
    closeCXIFiles(&cheetahGlobal);
    H5close();
    signal(SIGINT,SIG_DFL);
    kill(getpid(),SIGINT);
    
  }
}

/*
 *	Return true or false if a given event code is present
 */
bool eventCodePresent(uint EvrCode)
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
static bool beamOn(){
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

  /* catch SIGINT and handle appropriately */
  if(cheetahGlobal.saveCXI){
    signal(SIGINT, sig_handler);
  }

    
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
    else if (!strcmp(cheetahGlobal.detector[detID].detectorName, "pnCCD")) {
      detectorType[detID] = Pds::DetInfo::pnCCD;
      detectorPdsDetInfo[detID] = Pds::DetInfo::Camp;
    }
    else {
      printf("Error: unknown detector %s\n", cheetahGlobal.detector[detID].detectorName);
      printf("cheetah-myana.cpp, beginjob()\n");
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
    else if (getCspadConfig( detectorPdsDetInfo[detID], configV4 )==0) {
      configVsn[detID]= 4;
      quadMask[detID] = configV4.quadMask();
      asicMask[detID] = configV4.asicMask();
      printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask[detID], asicMask[detID], configV4.runDelay());
      printf("\tintTime %d/%d/%d/%d\n", configV4.quads()[0].intTime(), configV4.quads()[1].intTime(), configV4.quads()[2].intTime(), configV4.quads()[3].intTime());
    }
    else if (!strcmp(cheetahGlobal.detector[detID].detectorName, "pnCCD")) {
      // pnCCD needs nothing ??
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
  printf("+++ Processing r%04u +++\n",getRunNumber());
  fetchConfig();
  frameNumber = 0;


  /*
   *	Pass new run information to Cheetah
   */
  cheetahGlobal.runNumber = getRunNumber();
  strcpy(cheetahGlobal.cxiFilename,getCXIfromXTC(getXTCFilename()).c_str());
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
	
  static uint32_t nevents = 0;        

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
  if(dtime > 1) {
    datarate = (frameNumber - cheetahGlobal.lastTimingFrame)/(float)dtime;
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
   *	Skip frames if we only want a part of the data set
   */
  if(cheetahGlobal.startAtFrame != 0 && frameNumber < cheetahGlobal.startAtFrame) {
    printf("r%04u:%li (%3.1fHz): Skipping to start frame %li\n", cheetahGlobal.runNumber, frameNumber, cheetahGlobal.datarate, cheetahGlobal.startAtFrame);		
    return;
  }
  if(cheetahGlobal.stopAtFrame != 0 && frameNumber > cheetahGlobal.stopAtFrame) {
    printf("r%04u:%li (%3.1fHz): Skipping from end frame %li\n", cheetahGlobal.runNumber, frameNumber, cheetahGlobal.datarate, cheetahGlobal.stopAtFrame);		
    return;
  }
    


	
  /*
   *	Get run number
   */
  unsigned runNumber;
  runNumber = getRunNumber();

  // This section is moved to endrun()
  /*if(cheetahGlobal.saveCXI){
    std::string cxiFilename = getCXIfromXTC(getXTCFilename());


    if(cxiFilename.compare(cheetahGlobal.cxiFilename)!=0){
      while(cheetahGlobal.nActiveThreads > 0){
	printf("Waiting for %li worker threads to terminate for new ones\n", cheetahGlobal.nActiveThreads);
	usleep(100000);
      }
      if(strcmp(cheetahGlobal.cxiFilename,"") != 0){
	closeCXIFiles(&cheetahGlobal);
      }
      strcpy(cheetahGlobal.cxiFilename,cxiFilename.c_str());
    }
    }*/
	
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
	
  fail = getEBeam(fEbeamCharge, fEbeamL3Energy, fEbeamLTUPosX, fEbeamLTUPosY,
		  fEbeamLTUAngX, fEbeamLTUAngY, fEbeamPkCurrBC2);
  if ( fail ) {
		
    // If no beamline data, but default wavelength specified in ini file
    // then use that
    printf("getEBeam error: unable to calculate wavelength for this event\n");
    if ( cheetahGlobal.defaultPhotonEnergyeV != 0 ) {
      photonEnergyeV = cheetahGlobal.defaultPhotonEnergyeV;
      wavelengthA = 12398.42/photonEnergyeV;
    } 
    else {
      wavelengthA = std::numeric_limits<double>::quiet_NaN();
      photonEnergyeV = std::numeric_limits<double>::quiet_NaN();
    }
    // printf("wavelengthA set to %g\n", wavelengthA);

  } 
  else {
		
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
  fail = getFeeGasDet(gasdet);
  if ( fail ) {
    gmd1 = std::numeric_limits<double>::quiet_NaN();
    gmd2 = std::numeric_limits<double>::quiet_NaN();
  } 
  else {
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
  fail = getPvFloat(cheetahGlobal.laserDelayPV, laserDelay);
  if( fail == 0 ) {
    printf("New laser delay: %f\n",laserDelay);
  }

    
    
  /*
   *	Detector position (Z) for each detector
   *	This PV is updated at only about 1 Hz.  
   *	The function getPvFloat seems to misbehave.  
   *	Firstly, if you skip the first few XTC datagrams, you will likely
   *	get error messages telling you that the EPICS PV is invalid.  
   *	More worrysome is the fact that it occasionally gives a bogus value 
   *	of detposnew=0, without a fail message.  Hardware problem? 
   *  Fixes for this flakey behaviour are found in cheetahUpdateGlobal()
   */
  float detposnew;
  float detectorPosition[MAX_DETECTORS];

  // Loop through all detectors
  for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
		
    fail = getPvFloat(cheetahGlobal.detector[detID].detectorZpvname, detposnew);
    if (  fail == 0 ) {
      detectorPosition[detID] = detposnew;
      //printf("DetectorPosition[%i] = %g\n", detID, detposnew);
    }
    else {
      detectorPosition[detID] = std::numeric_limits<float>::quiet_NaN();
      /*
       *	Do not print error message
       *	Detector position is in slow data stream, updated at 1 Hz
       *	Printing error each event (120Hz) will create perception of errors 
       *	when there are in fact none
       */
    }
  }    
    
  
  /*
   *	Create a new eventData structure in which to place all information
   */
  cEventData	*eventData;
  eventData = cheetahNewEvent(&cheetahGlobal);
		
  sprintf(cheetahGlobal.cxiFilename,"LCLS-r%04d.cxi",runNumber); 
		
  /* This laser event code */
  int evr41 = laserOn();

  /* Check that previous frame is present (not damaged) */
  double thistime = seconds + nanoSeconds*1e-9;
  double deltime = thistime - cheetahGlobal.lasttime;
  double deltimefel = 1.0/120;
  double deltimejitter = 0.10*deltimefel;
  int havePreviousPulse = 0;
  if ( fabs(deltime - deltimefel) < deltimejitter/2.0 ) havePreviousPulse = 1;

  /* Has the sample been pumped with the laser? */
  switch ( cheetahGlobal.laserPumpScheme) {
		
    /* Scheme 1: simply trust the evr41 signal (default action) */
  case 1:
    if ( evr41 == 1 ) {
      eventData->samplePumped = 1;
    } else {
      eventData->samplePumped = 0;
    }
    break;
	
    /* Scheme 2: the previous evr41 corresponds to this frame */
  case 2:
    if ( havePreviousPulse != 1){
      eventData->samplePumped = -1;
    } else {
      eventData->samplePumped = cheetahGlobal.evr41previous;
    }
    break;

    /* Scheme 3: the previous evr41 corresponds to this frame, but pulse is possibly too long to know if it is dark with certainty */
  case 3:
    if ( havePreviousPulse != 1){
      eventData->samplePumped = -1;
    } else {
      if ( cheetahGlobal.evr41previous == 1){
	eventData->samplePumped = 1;
      } else {
	eventData->samplePumped = -1;
      }
    }
  } 

  //	printf("=======================================\n");
  //	printf("evr41        = %d\n",evr41);
  //	printf("evr41previous= %d\n",cheetahGlobal.evr41previous);
  //	printf("delta time   = %g\n",deltime);
  //	printf("previous     ? %d\n",havePreviousPulse);
  //	printf("pumped       ? %d\n",eventData->samplePumped);
  //	printf("=======================================\n");	

  cheetahGlobal.evr41previous = evr41;
  cheetahGlobal.lasttime = thistime;

   
	
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
    
  eventData->laserEventCodeOn = evr41;
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
	

  /*
   *	Copy raw cspad image data into Cheetah event structure for processing
   *  SLAC libraries are not thread safe: must copy data into event structure for processing
   */
  for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
		
    // cspad data
    if ( strcmp(cheetahGlobal.detector[detID].detectorType, "cspad") == 0 ) {

      //uint16_t *quad_data[4];        
      Pds::CsPad::ElementIterator iter;
			
      //std::cout << "detectorPdsDetInfo[" << detID << "]: " << detectorPdsDetInfo[detID] << std::endl;
      fail=getCspadData(detectorPdsDetInfo[detID], iter);
      if (fail) {
	printf("getCspadData fail for detector %li (%d, %x)\n",detID, fail,fiducial);
	eventData->detector[detID].cspad_fail = fail;
	return;
      }
      else {
	nevents++;

	long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
	long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
	long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
	//long    nasics_x = cheetahGlobal.detector[detID].nasics_x;
	//long    nasics_y = cheetahGlobal.detector[detID].nasics_y;
	uint16_t    *quad_data[4];

	//printf("nasics_x/y asic_nx/ny: %d/%d %d/%d\n",nasics_x,nasics_y,asic_nx,asic_ny);          
	// Allocate memory for detector data and set to zero
	for(int quadrant=0; quadrant<4; quadrant++) {
	  quad_data[quadrant] = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
	  memset(quad_data[quadrant], 0, pix_nn*sizeof(uint16_t));
	}
				

	// loop over quadrants and read out the data
	const Pds::CsPad::ElementHeader* element;
	while(( element=iter.next() )) {  
	  if(element->quad() < 4) {
	    // Which quadrant is this?
	    int quadrant = element->quad();
						
	    // Read 2x1 "sections" into data array in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
	    const Pds::CsPad::Section* s;
	    unsigned section_id;
	    while(( s=iter.next(section_id) )) {  
	      //printf("quadrant/section_id: %d/%d\n",quadrant,section_id);
	      //printf("pixel: %d\n",s->pixel[0]);
	      memcpy(&quad_data[quadrant][section_id*2*asic_nx*asic_ny],s->pixel[0],2*asic_nx*asic_ny*sizeof(uint16_t));
	    }

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
	    //printf("quadrant/k/quad_data: %d/%d/%d\n",quadrant,k,quad_data[quadrant][0]);					
	    eventData->detector[detID].raw_data[ii] = quad_data[quadrant][k];
	  }
	}

	// quadrant data no longer needed
	for(int quadrant=0; quadrant<4; quadrant++) 
	  free(quad_data[quadrant]);
      }
    }
    // pnccd
    else if ( strcmp(cheetahGlobal.detector[detID].detectorType, "pnccd") == 0 ) {
      fail = 1;
      int             pnCcdImageWidth, pnCcdImageHeight;
      unsigned char*  pnCcdImage;
			
      if (cheetahGlobal.detector[detID].detectorID == 0) 
	fail = getPnCcdValue( 0, pnCcdImage, pnCcdImageWidth, pnCcdImageHeight);			
      else if (cheetahGlobal.detector[detID].detectorID == 1) 
	fail = getPnCcdValue( 1, pnCcdImage, pnCcdImageWidth, pnCcdImageHeight);

      if ( fail ) {
	printf("%li: pnCCD frame data not available (detID=%li)\n", frameNumber, detID);
	eventData->detector[detID].cspad_fail = fail;
	return;
      }
      else {
	//printf( "Get PnCCD Image from device %d, width = %d height = %d\n", 0, pnCcdImageWidth, pnCcdImageHeight );
	long pix_nn = (long)pnCcdImageWidth * (long)pnCcdImageHeight;
				
	eventData->detector[detID].raw_data = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
	memcpy(&eventData->detector[detID].raw_data[0], pnCcdImage, pix_nn*sizeof(uint16_t));
      }

    }
    else {
      printf("Unsupported detector type: %s\n", cheetahGlobal.detector[detID].detectorType);
      printf("Detector name: %s\n", cheetahGlobal.detector[detID].detectorName);
      exit(1);
    }
		
    // Update detector positions
    eventData->detector[detID].detectorZ = detectorPosition[detID];
		
  }	
	
    
  /*
   *	Copy TOF (aqiris) channel into Cheetah event for processing
   *  SLAC libraries are not thread safe: must copy data into event structure for processing
   *  Assumes that myana creates a copy of the Acqiris data arrays and will never need these data arrays later. 
   */
  cheetahGlobal.TOFPresent=0;
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
        
    free(tempTOFTime);
    free(tempTOFVoltage);
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
   *  Assumes that myana creates a copy of the pulnix data array and will never need these memory locations again. 
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
        
      //free(pulnixImage);
    }

  eventData->specFail = 1;
    
    
  /*
   *	Call Cheetah to process this event
   */
  // Multiple processing threads
  cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);

  // Single processing thread    
  //cheetahProcessEvent(&cheetahGlobal, eventData);
  //cheetahDestroyEvent(eventData);
	
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
  if(cheetahGlobal.saveCXI){
    // Wait for all workers to finish
    while(cheetahGlobal.nActiveThreads > 0) {
      printf("Waiting for %li worker threads to terminate before processing a new run\n", cheetahGlobal.nActiveThreads);
      usleep(100000);
    }
    writeAccumulatedCXI(&cheetahGlobal);
  }
}

  // This section is moved to endrun()
  /*if(cheetahGlobal.saveCXI){
    std::string cxiFilename = getCXIfromXTC(getXTCFilename());


    if(cxiFilename.compare(cheetahGlobal.cxiFilename)!=0){
      while(cheetahGlobal.nActiveThreads > 0){
	printf("Waiting for %li worker threads to terminate for new ones\n", cheetahGlobal.nActiveThreads);
	usleep(100000);
      }
      if(strcmp(cheetahGlobal.cxiFilename,"") != 0){
	closeCXIFiles(&cheetahGlobal);
      }
      strcpy(cheetahGlobal.cxiFilename,cxiFilename.c_str());
    }
    }*/


void endjob()
{
  printf("User analysis endjob() routine called.\n");
	
  /*
   *	Clean up all variables associated with libCheetah
   */
  cheetahExit(&cheetahGlobal);
  exit(1);
}




/* Change file extension from .xtc to .cxi */
static std::string getCXIfromXTC(std::string filename){
  size_t end,start;
  start = filename.find_last_of("/");
  if(start == std::string::npos){
    start = 0;
  }else{
    start++;
  }
  end = filename.find_last_of("-s");
  std::string sub =filename.substr(start,end-start+1-5)  + ".cxi";
  return sub;
}
