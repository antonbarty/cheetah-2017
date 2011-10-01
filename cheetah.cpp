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


#include "myana/myana.hh"
#include "myana/main.hh"
#include "myana/XtcRun.hh"
#include "release/pdsdata/cspad/ConfigV1.hh"
#include "release/pdsdata/cspad/ConfigV2.hh"
#include "release/pdsdata/cspad/ConfigV3.hh"
#include "release/pdsdata/cspad/ElementHeader.hh"
#include "release/pdsdata/cspad/ElementIterator.hh"
#include "cspad/CspadTemp.hh"
#include "cspad/CspadCorrector.hh"
#include "cspad/CspadGeometry.hh"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <hdf5.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits>
#include <stdint.h>

#include "setup.h"
#include "worker.h"


static cGlobal		global;
static long			frameNumber;

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
	 * Get time information
	 */
	int seconds, nanoSeconds;

	getTime( seconds, nanoSeconds );	
	// fail = getLocalTime( time );

	
	/*
	 *	New csPad corrector
	 */
	corrector = new CspadCorrector(global.detectorPdsDetInfo,0,CspadCorrector::DarkFrameOffset);

				 
	/*
	 *	Stuff for worker thread management
	 */
	global.defaultConfiguration();
	//global.parseConfigFile("cspad-cryst.ini");
	global.parseConfigFile(global.configFile);
	global.readDetectorGeometry(global.geometryFile);
	global.setup();
	global.readDarkcal(global.darkcalFile);
	global.readGaincal(global.gaincalFile);
	global.readPeakmask(global.peaksearchFile);
	global.readBadpixelMask(global.badpixelFile);
	global.readWireMask(global.wireMaskFile);
	global.writeInitialLog();
}



void fetchConfig()
{
	int fail = 0;
	// cspad config
	if (getCspadConfig( global.detectorPdsDetInfo, configV1 )==0) {
		configVsn= 1;
		quadMask = configV1.quadMask();
		asicMask = configV1.asicMask();
		printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask,asicMask,configV1.runDelay());
		printf("\tintTime %d/%d/%d/%d\n", configV1.quads()[0].intTime(), configV1.quads()[1].intTime(), configV1.quads()[2].intTime(), configV1.quads()[3].intTime());
	}
	else if (getCspadConfig( global.detectorPdsDetInfo, configV3 )==0) {
		configVsn= 2;
		quadMask = configV2.quadMask();
		asicMask = configV2.asicMask();
		printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask,asicMask,configV2.runDelay());
		printf("\tintTime %d/%d/%d/%d\n", configV2.quads()[0].intTime(), configV2.quads()[1].intTime(), configV2.quads()[2].intTime(), configV2.quads()[3].intTime());
	}
	else if (getCspadConfig( global.detectorPdsDetInfo, configV3 )==0) {
		configVsn= 3;
		quadMask = configV3.quadMask();
		asicMask = configV3.asicMask();
		printf("CSPAD configuration: quadMask %x  asicMask %x  runDelay %d\n", quadMask,asicMask,configV3.runDelay());
		printf("\tintTime %d/%d/%d/%d\n", configV3.quads()[0].intTime(), configV3.quads()[1].intTime(), configV3.quads()[2].intTime(), configV3.quads()[3].intTime());
	}
	else {
		configVsn= 0;
		printf("Failed to get CspadConfig\n");
	}
	
	// Acqiris config
	if((fail=getAcqConfig(Pds::DetInfo(0,global.tofPdsDetInfo,0,Pds::DetInfo::Acqiris,0) , global.AcqNumChannels, global.AcqNumSamples, global.AcqSampleInterval))==0){
		global.TOFPresent = 1;
		printf("Acqiris configuration: %d channels, %d samples, %lf sample interval\n", global.AcqNumChannels, global.AcqNumSamples, global.AcqSampleInterval);
		if (global.hitfinderTOFMaxSample > global.AcqNumSamples){
			printf("hitfinderTOFMaxSample greater than number of TOF samples. hitfinderUseTOF turned off\n");
			global.hitfinderUseTOF = 0;
		}
	}
	else {
		global.TOFPresent = 0;
		printf("Failed to get AcqirisConfig with fail code %d.\n", fail);
		global.hitfinderUseTOF = 0 ;
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
	corrector->loadConstants(getRunNumber());
	global.runNumber = getRunNumber();
	frameNumber = 0;

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
	 *	Get run number
	 */
	unsigned runNumber;
	runNumber = getRunNumber();
	
	
	/*
	 *	Get event information
	 */	 
	int 			numEvrData;
	unsigned		fiducial;
	unsigned int 	eventCode;
	unsigned int 	timeStamp;	
	numEvrData = getEvrDataNumber();
	for (long i=0; i<numEvrData; i++) {
		fail = getEvrData( i, eventCode, fiducial, timeStamp );
	}
	
	
	
	/*
	 *	How quickly are we processing the data? (average over last 10 events)
	 */	
	time_t	tnow;
	double	dt, datarate1;
	double	dtime, datarate2;
	//int		hrs, mins, secs; 

	time(&tnow);
	dtime = difftime(tnow, global.tlast);
	dt = clock() - global.lastclock;
	
	if(dtime > 0) {
		datarate1 = ((float)CLOCKS_PER_SEC)/dt;
		datarate2 = (frameNumber - global.lastTimingFrame)/dtime;
		global.lastclock = clock();
		global.lastTimingFrame = frameNumber;
		time(&global.tlast);

		global.datarate = datarate2;
	}
	
	/*
	 *	Skip frames if we only want a part of the data set
	 */
	if(global.startAtFrame != 0 && frameNumber < global.startAtFrame) {
		printf("r%04u:%li (%3.1fHz): Skipping to start frame %li\n", global.runNumber, frameNumber, global.datarate, global.startAtFrame);		
		return;
	}
	if(global.stopAtFrame != 0 && frameNumber > global.stopAtFrame) {
		printf("r%04u:%li (%3.1fHz): Skipping from end frame %li\n", global.runNumber, frameNumber, global.datarate, global.stopAtFrame);		
		return;
	}

	

	/*
	 * Get event time information
	 */
	int seconds, nanoSeconds;
	getTime( seconds, nanoSeconds );

	/*
	 *	Get event fiducials
	 */
	fail = getFiducials(fiducial);

	
	/* 
	 *	Is the beam on?
	 */
	bool beam = beamOn();
	//printf("Beam %s : fiducial %x\n", beam ? "On":"Off", fiducial);
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
		// then use that, else 
		if ( global.defaultPhotonEnergyeV != 0 ) {
			photonEnergyeV = global.defaultPhotonEnergyeV;
			wavelengthA = 12398.42/photonEnergyeV;
		} else {
			wavelengthA = std::numeric_limits<double>::quiet_NaN();
			photonEnergyeV = std::numeric_limits<double>::quiet_NaN();
		}

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
	 *	(we probably won't need this info)
	 */     
	double 	phaseCavityTime1;
	double	phaseCavityTime2;
	double	phaseCavityCharge1;
	double	phaseCavityCharge2;
	fail = getPhaseCavity(phaseCavityTime1, phaseCavityTime2, phaseCavityCharge1, phaseCavityCharge2);
	
	
	/*
	 *	CXI detector position (Z)
	 */
	float detposnew;
	int update_camera_length;
	if ( getPvFloat("CXI:DS1:MMS:06", detposnew) == 0 ) {
		/* When encoder reads -500mm, detector is at its closest possible
		 * position to the specimen, and is 79mm from the centre of the 
		 * 8" flange where the injector is mounted.  The injector itself is
		 * about 4mm further away from the detector than this. */
		// printf("New detector pos %e\n", detposnew);
		global.detectorZ = 500.0 + detposnew + 79.0;
		update_camera_length = 1;
		/* FYI: the function getPvFloat seems to misbehave.  Firstly, if you
		 * skip the first few XTC datagrams, you will likely get error messages
		 * telling you that the EPICS PV is invalid.  Seems that this PV is
		 * updated at only about 1 Hz.  More worrysome is the fact that it
		 * occasionally gives a bogus value of detposnew=0, without a fail
		 * message.  Hardware problem? 
		 */
	} 

	if ( global.detectorZ == 0 ) {
		/* What to do if there is no camera length information?  Keep skipping
		 * frames until this info is found?  In some cases, our analysis doesn't
		 * need to know about this, so OK to skip in that case.  For now, the
		 * solution is for the user to set a (non-zero) default camera length.
		 */
		if ( global.defaultCameraLengthMm == 0 ) {
			printf("======================================================\n");
			printf("WARNING: Camera length is zero!\n");
			printf("I'm skipping this frame.  If the problem persists, try\n");
			printf("setting the keyword defaultCameraLengthMm in your ini\n"); 
			printf("file.\n");
			printf("======================================================\n");
			return;
		} else {
			printf("MESSAGE: Setting default camera length (%gmm).\n",global.defaultCameraLengthMm);
			global.detectorZ = global.defaultCameraLengthMm;	
			update_camera_length = 1;
		}
	}
	
	/*
	 * If the camera length has changed, recalculate reciprocal space geometry.
	 * Also, skip this frame if it isn't the first one.  Let's not bother with 
	 * frames collected while the camera is moving...
	 */
	
	if ( update_camera_length && ( global.detectorZprevious != global.detectorZ ) ) {
		
		printf("MESSAGE: Camera length changed from %gmm to %gmm.\n",
		                                   global.detectorZprevious,global.detectorZ);
	
		if ( isnan(wavelengthA ) ) {
			printf("MESSAGE: Bad wavelength data (NaN). Consider using defaultPhotonEnergyeV keyword.\n");
		}	
		long i;
		float  x, y, z, r;
		float kx,ky,kz,kr;
		float res;
		
		// don't tinker with global geometry while there are active threads...
		while (global.nActiveThreads > 0) usleep(10000);
		
		global.detectorZprevious = global.detectorZ;

		for ( i=0; i<global.pix_nn; i++ ) {
			x = global.pix_x[i]*global.pixelSize;
			y = global.pix_y[i]*global.pixelSize;
			z = global.pix_z[i]*global.pixelSize + global.detectorZ*0.001;
			r = sqrt(x*x + y*y + z*z);
			kx = x/r/wavelengthA;
			ky = y/r/wavelengthA;
			kz = (z/r - 1)/wavelengthA; // assuming incident beam is along +z direction
			kr = sqrt(kx*kx + ky*ky + kz*kz);
			res = 1/kr;
			global.pix_kx[i] = kx;
			global.pix_ky[i] = ky;
			global.pix_kz[i] = kz;
			global.pix_kr[i] = kr;
			global.pix_res[i] = res;
			if ( global.hitfinderLimitRes == 1 ) {
				if ( ( res < global.hitfinderMinRes ) && (res > global.hitfinderMaxRes) ) {
					global.hitfinderResMask[i] = 1;
				} else {
					global.hitfinderResMask[i] = 0;
				}
			}
		}
		
		// if its the first thread then continue, else skip this event
		if ( frameNumber != 1 ) return;
	
	}	

	/*
	 *	Create a new threadInfo structure in which to place all information
	 */
	tThreadInfo	*threadInfo;
	threadInfo = (tThreadInfo*) malloc(sizeof(tThreadInfo));
		
	
	/*
	 *	Copy all interesting information into worker thread structure if we got this far.
	 *	(ie: presume that myana itself is NOT thread safe and any event info may get overwritten)
	 */
	threadInfo->seconds = seconds;
	threadInfo->nanoSeconds = nanoSeconds;
	threadInfo->fiducial = fiducial;
	threadInfo->runNumber = getRunNumber();
	threadInfo->beamOn = beam;
	threadInfo->nPeaks = 0;
	
	threadInfo->laserEventCodeOn = laserOn();
	
	threadInfo->gmd11 = gasdet[0];
	threadInfo->gmd12 = gasdet[1];
	threadInfo->gmd21 = gasdet[2];
	threadInfo->gmd22 = gasdet[3];
	
	threadInfo->fEbeamCharge = fEbeamCharge;		// in nC
	threadInfo->fEbeamL3Energy = fEbeamL3Energy;	// in MeV
	threadInfo->fEbeamLTUPosX = fEbeamLTUPosX;		// in mm
	threadInfo->fEbeamLTUPosY = fEbeamLTUPosY;		// in mm
	threadInfo->fEbeamLTUAngX = fEbeamLTUAngX;		// in mrad
	threadInfo->fEbeamLTUAngY = fEbeamLTUAngY;		// in mrad
	threadInfo->fEbeamPkCurrBC2 = fEbeamPkCurrBC2;	// in Amps
	threadInfo->photonEnergyeV = photonEnergyeV;	// in eV
	threadInfo->wavelengthA = wavelengthA;			// in Angstrom
	
	threadInfo->phaseCavityTime1 = phaseCavityTime1;
	threadInfo->phaseCavityTime2 = phaseCavityTime2;
	threadInfo->phaseCavityCharge1 = phaseCavityCharge1;
	threadInfo->phaseCavityCharge1 = phaseCavityCharge2;
	
	threadInfo->pGlobal = &global;
	
	for(int quadrant=0; quadrant<4; quadrant++) {
		threadInfo->quad_data[quadrant] = (uint16_t*) calloc(CSPAD_ASIC_NX*CSPAD_ASIC_NY*16, sizeof(uint16_t));
		memset(threadInfo->quad_data[quadrant], 0, CSPAD_ASIC_NX*CSPAD_ASIC_NY*16*sizeof(uint16_t));
	}
	


	/*
	 *	Copy raw cspad image data into worker thread structure for processing
	 */
	Pds::CsPad::ElementIterator iter;
	
	fail=getCspadData(global.detectorPdsDetInfo, iter);

	if (fail) {
		printf("getCspadData fail %d (%x)\n",fail,fiducial);
		threadInfo->cspad_fail = fail;
		return;
	}
	else {
		nevents++;
		const Pds::CsPad::ElementHeader* element;

		// loop over elements (quadrants)
		while(( element=iter.next() )) {  
			if(element->quad() < 4) {
				// Which quadrant is this?
				int quadrant = element->quad();
				
				// Have we accidentally jumped to a new fiducial (ie: a different event??)
				
				// Get temperature on strong back 
				float	temperature = CspadTemp::instance().getTemp(element->sb_temp((element->quad()%2==0)?3:0));
				threadInfo->quad_temperature[quadrant] = temperature;
				
				
				// Read 2x1 "sections" into data array in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
				const Pds::CsPad::Section* s;
				unsigned section_id;
				while(( s=iter.next(section_id) )) {  
					memcpy(&threadInfo->quad_data[quadrant][section_id*2*CSPAD_ASIC_NX*CSPAD_ASIC_NY],s->pixel[0],2*CSPAD_ASIC_NX*CSPAD_ASIC_NY*sizeof(uint16_t));
				}
			}
		}
	}
	
	
	/*
	 *	Copy TOF (aqiris) channel into worker thread for processing
	 */
	threadInfo->TOFPresent = global.TOFPresent ;	
	if (global.TOFPresent==1){
		double *tempTOFTime;
		double *tempTOFVoltage;
		double tempTrigTime;
		threadInfo->TOFTime = (double*) malloc(global.AcqNumSamples*sizeof(double));
		threadInfo->TOFVoltage = (double*) malloc(global.AcqNumSamples*sizeof(double));
		fail = getAcqValue(Pds::DetInfo(0,global.tofPdsDetInfo,0,Pds::DetInfo::Acqiris,0), global.TOFchannel, tempTOFTime,tempTOFVoltage, tempTrigTime);
		threadInfo->TOFtrigtime = tempTrigTime;
		//Memcopy is necessary for thread safety.
		memcpy(threadInfo->TOFTime, tempTOFTime, global.AcqNumSamples*sizeof(double));
		memcpy(threadInfo->TOFVoltage, tempTOFVoltage, global.AcqNumSamples*sizeof(double));
		if (fail){
			printf("getAcqValue fail %d (%x)\n",fail,fiducial);
			return ;
		}
	}

	
	/*
	 *	I/O speed test?
	 */
	if(global.ioSpeedTest) {
		printf("r%04u:%li (%3.1fHz): I/O Speed test\n", global.runNumber, frameNumber, global.datarate);		
		for(int quadrant=0; quadrant<4; quadrant++) {
			free(threadInfo->quad_data[quadrant]);
		}
		free(threadInfo);
		return;
	}
	
	
	/*
	 *	Spawn worker thread to process this frame
	 *	Threads are created detached so we don't have to wait for anything to happen before returning
	 *		(each thread is responsible for cleaning up its own threadInfo structure when done)
	 */
	pthread_t		thread;
	pthread_attr_t	threadAttribute;
	int				returnStatus;

	
	
	// Periodically pause and let all threads finish
	// On cfelsgi we seem to get mutex-lockup on some threads if we don't do this
	if( global.threadPurge && (global.nprocessedframes%global.threadPurge)==0 ){
		while(global.nActiveThreads > 0) {
			printf("Pausing to let remaining %li worker threads to terminate\n", global.nActiveThreads);
			usleep(10000);
		}
	}

	// Wait until we have a spare thread in the thread pool
	while(global.nActiveThreads >= global.nThreads) {
		usleep(1000);
	}


	// Increment threadpool counter
	pthread_mutex_lock(&global.nActiveThreads_mutex);
	global.nActiveThreads += 1;
	threadInfo->threadNum = ++global.threadCounter;
	pthread_mutex_unlock(&global.nActiveThreads_mutex);
	
	// Set detached state
	pthread_attr_init(&threadAttribute);
	pthread_attr_setdetachstate(&threadAttribute, PTHREAD_CREATE_DETACHED);
	
	// Create a new worker thread for this data frame
	returnStatus = pthread_create(&thread, &threadAttribute, worker, (void *)threadInfo); 
	pthread_attr_destroy(&threadAttribute);
	global.nprocessedframes += 1;
	global.nrecentprocessedframes += 1;
	

	
	/*
	 *	Save periodic powder patterns
	 */
	if(global.saveInterval!=0 && (global.nprocessedframes%global.saveInterval)==0 && (global.nprocessedframes > global.startFrames+50) ){
		saveRunningSums(&global);
		global.updateLogfile();
	}
	
	
	
	
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


	// Wait for threads to finish
	while(global.nActiveThreads > 0) {
		printf("Waiting for %li worker threads to terminate\n", global.nActiveThreads);
		usleep(100000);
	}
	
	
	// Save powder patterns
	saveRunningSums(&global);
	global.writeFinalLog();
	
	// Hitrate?
	printf("%li files processed, %li hits (%2.2f%%)\n",global.nprocessedframes, global.nhits, 100.*( global.nhits / (float) global.nprocessedframes));

	
	// Cleanup
	free(global.darkcal);
	free(global.powderHitsAssembled);
	free(global.powderHitsRaw);
	free(global.powderHitsRawSquared);
	free(global.powderBlanksAssembled);
	free(global.powderBlanksRaw);
	free(global.powderBlanksRawSquared);
	free(global.hotpixelmask);
	free(global.wiremask);
	free(global.selfdark);
	free(global.gaincal);
	free(global.peakmask);
	free(global.bg_buffer);
	free(global.hotpix_buffer);
	pthread_mutex_destroy(&global.nActiveThreads_mutex);
	pthread_mutex_destroy(&global.powderHitsRaw_mutex);
	pthread_mutex_destroy(&global.powderHitsRawSquared_mutex);
	pthread_mutex_destroy(&global.powderHitsAssembled_mutex);
	pthread_mutex_destroy(&global.powderBlanksRaw_mutex);
	pthread_mutex_destroy(&global.powderBlanksRawSquared_mutex);
	pthread_mutex_destroy(&global.powderBlanksAssembled_mutex);
	pthread_mutex_destroy(&global.selfdark_mutex);
	pthread_mutex_destroy(&global.hotpixel_mutex);
	pthread_mutex_destroy(&global.bgbuffer_mutex);
	pthread_mutex_destroy(&global.framefp_mutex);
	pthread_mutex_destroy(&global.peaksfp_mutex);
	

	
	
	printf("Done!\n");
}
