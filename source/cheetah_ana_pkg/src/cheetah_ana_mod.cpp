
//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class cheetah_ana_mod...
//	This is a psana-based module for running Cheetah (Anton's hitfinder)
//
// Author List:
//      Chunhong Yoon	06/2012		chun.hong.yoon@cfel.de
//  	Copyright (c) 2012 CFEL. All rights reserved.
//
//------------------------------------------------------------------------

#include <stdlib.h>
//-----------------------
// This Class's Header --
//-----------------------
#include <cheetah_ana_mod.h>
#include <cheetah.h>
//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <string>
#include <time.h>
#include <signal.h>
//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
// Includes can be found in /reg/g/psdm/sw/releases/ana-current.psddl_psana/include/...
#include "PSEvt/EventId.h"
#include "psddl_psana/bld.ddl.h"
#include "psddl_psana/cspad.ddl.h"
#include "psddl_psana/cspad2x2.ddl.h"
#include "psddl_psana/evr.ddl.h"
#include "psddl_psana/acqiris.ddl.h"
#include "psddl_psana/camera.ddl.h"
#include "psddl_psana/pnccd.ddl.h"

// LCLS event codes
#define beamCode 140
#define verbose 0

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace cheetah_ana_pkg;
using namespace std;
PSANA_MODULE_FACTORY(cheetah_ana_mod)


//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------
namespace cheetah_ana_pkg {
	static long frameNumberIncludingSkipped = 0;
	static long frameNumber = 0;
	static cGlobal cheetahGlobal;
	static time_t startT = 0;

	class CspadDataWrapper {
    
	};


	void sig_handler(int signo)
	{
		if (signo == SIGINT){
			// Wait for threads to finish
			while(cheetahGlobal.nActiveThreads > 0) {
				printf("Waiting for %li worker threads to terminate\n", cheetahGlobal.nActiveThreads);
				usleep(100000);
			}
			printf("Attempting to close CXIs cleanly\n");
			writeAccumulatedCXI(&cheetahGlobal);
			closeCXIFiles(&cheetahGlobal);
			signal(SIGINT,SIG_DFL);
			kill(getpid(),SIGINT);
	    
		}
	}


	// Event code present?
	template <typename T>
	bool eventCodePresent(const ndarray<T, 1>& array, unsigned EvrCode){
		for (unsigned i = 0; i < array.size(); ++i) {
			if (array[i].eventCode() == EvrCode) {
				return true;
			}
		}
		return false;
	}

	
	//----------------
	// Constructors --
	//----------------
	cheetah_ana_mod::cheetah_ana_mod (const std::string& name)
		: Module(name)
	{
		cout << "*** Constructor ***" << endl;
        
        printf("Note: Support for the FEE spectrometer was added on 24 November 2013.\n");
        printf("This may require the following line to be added to your psana.conf file:\n");
        printf("    feeSpectrum = BldInfo(FEE-SPEC0)\n");
        printf("There have been reports of psana crashing with the error\n");
        printf("Standard exception caught in runApp(): PSEvt::Exception: Source string cannot be parsed: 'BldInfo(:FEE-SPEC0)' [in function parse at PSEvt/src/Source.cpp:409]\n");
        printf("if this line is not present, even though a default value has been specified\n");

        
		// Check if we're using psana of the same git commit
		if(!getenv("PSANA_GIT_SHA") || strcmp(getenv("PSANA_GIT_SHA"),GIT_SHA1)){
			fprintf(stderr,    "*******************************************************************************************\n");
			fprintf(stderr,"*** WARNING %s:%d ***\n",__FILE__,__LINE__);

			if(getenv("PSANA_GIT_SHA")){
				fprintf(stderr,"***        Using psana from git commit %s         ***\n",getenv("PSANA_GIT_SHA"));
				fprintf(stderr,"***        and cheetah_ana_mod from git commit %s ***\n",GIT_SHA1);
			}else{
				fprintf(stderr,"***         Using a psana version not compiled with cheetah!                            ***\n");
			}
			fprintf(stderr,    "*******************************************************************************************\n");
			sleep(10);
		}
		setenv("CHEETAH_ANA_MOD_GIT_SHA",GIT_SHA1,0);

		// get the values from configuration or use defaults
		m_key = configStr("inputKey", "");
		m_srcCspad0 = configStr("cspadSource0","DetInfo(:Cspad)");
		m_srcCspad1 = configStr("cspadSource1","DetInfo(:Cspad)");
		m_srcCspad2x2 = configStr("cspad2x2Source0","DetInfo(:Cspad2x2)");
		m_srcPnccd0 = configStr("pnccdSource0","DetInfo(:pnCCD)");
		m_srcPnccd1 = configStr("pnccdSource1","DetInfo(:pnCCD)");
		m_srcEvr = configStr("evrSource","DetInfo(:Evr)");
		m_srcBeam = configStr("beamSource","BldInfo(:EBeam)");
		m_srcFee = configStr("feeSource","BldInfo(:FEEGasDetEnergy)");
		m_srcFeeSpec = configStr("feeSpectrum","BldInfo(:FEE-SPEC0)");
		m_srcCav = configStr("cavitySource","BldInfo(:PhaseCavity)");
		m_srcAcq = configStr("acqirisSource","DetInfo(:Acqiris)");
		m_srcSpec = configStr("spectrumSource","DetInfo()");
		m_srcCam = configStr("cameraSource","DetInfo()");
	}

	//--------------
	// Destructor --
	//--------------
	cheetah_ana_mod::~cheetah_ana_mod ()
	{
	}

	
	/// Method which is called once at the beginning of the job
	void cheetah_ana_mod::beginJob(Event& evt, Env& env)
	{
		//cout << "*** beginJob ***" << endl;
		time(&startT);
		if (cheetahInit(&cheetahGlobal)){
			exit(0);
		}
		if(cheetahGlobal.saveCXI){
			signal(SIGINT, sig_handler);
		}

	}

	
	/// Method which is called at the beginning of the run
	///	Pass new run information to Cheetah
	void cheetah_ana_mod::beginRun(Event& evt, Env& env)
	{
		cout << "Experiment = " << env.experiment() << endl;
		//cout << "*** beginRun ***" << endl;
		int runNumber = 0;
		PSTime::Time evtTime;
		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
		
		if (eventId.get()) {
			runNumber = eventId->run();
		}

		cheetahGlobal.runNumber = runNumber;

		// Set CXI file name				
		sprintf(cheetahGlobal.cxiFilename,"%s-r%04d.cxi", env.experiment().c_str(),runNumber);

		cheetahNewRun(&cheetahGlobal);
		printf("User analysis beginrun() routine called.\n");
		printf("*** Processing r%04u ***\n",runNumber);
	}


	/// Method which is called at the beginning of the calibration cycle
	void 
	cheetah_ana_mod::beginCalibCycle(Event& evt, Env& env)
	{
		cout << "beginCalibCycle()" << endl;
		
		/*
		 *	pnCCD configuration
		 *	configuration objects (PNCCD::ConfigV2) do not appear in event but are stored in the environment. 
		 *	You should use the code like this to get configuration data and you should always check that 
		 *	you are returned non-zero pointer:
		 *	
		 *	config = env.configStore().get(m_srcPnccd);
		 *	if (not config) {
		 *		MsgLog(name(), error, "failed to retrieve configuration object , address: " << m_srcPnccd);
		 *		return;
		 *	}
		 */
		for(long detID=0; detID < cheetahGlobal.nDetectors; detID++)  {

			if (!strcmp(cheetahGlobal.detector[detID].detectorType, "pnccd")) {

				// Need to make this do 
				shared_ptr<Psana::PNCCD::ConfigV1> config1 = env.configStore().get(m_srcPnccd0);
				shared_ptr<Psana::PNCCD::ConfigV2> config2 = env.configStore().get(m_srcPnccd0);

				if (config1.get()) {
					cout << "PNCCD::ConfigV1:" << endl;
					cout << "\n  numLinks = " << config1->numLinks() << endl;
					cout << "\n  payloadSizePerLink = " << config1->payloadSizePerLink() << endl;
				}
				else if (config2.get()) {
					cout << "PNCCD::ConfigV2:" << endl;
					cout << "\n  numLinks = " << config2->numLinks() << endl;
					cout << "\n  payloadSizePerLink = " << config2->payloadSizePerLink() << endl;
					cout << "\n  numChannels = " << config2->numChannels() << endl;
					cout << "\n  numRows = " << config2->numRows() << endl;
					cout << "\n  numSubmoduleChannels = " << config2->numSubmoduleChannels() << endl;
					cout << "\n  numSubmoduleRows = " << config2->numSubmoduleRows() << endl;
					cout << "\n  numSubmodules = " << config2->numSubmodules() << endl;
					cout << "\n  camexMagic = " << config2->camexMagic() << endl;
					cout << "\n  info = " << config2->info() << endl;
					cout << "\n  timingFName = " << config2->timingFName() << endl;
				}
				else {
					cout << "Failed to retrieve pnCCD configuration object. " << detID << endl;
					cout << "cheetah_ana_mod::beginCalibCycle(Event& evt, Env& env)" << endl;
					cout << "Exiting..." << endl;
					exit(1);
				}
			}
			else {
				cout << "No configuration data" << endl;
			}
		}
	}

	///
	///	Event method
	/// This method is called with event data
	///	Copy across data into Cheetah structure and process
	///
	void cheetah_ana_mod::event(Event& evt, Env& env) {
		float random_float = (float)rand()/(float)RAND_MAX;
		frameNumberIncludingSkipped ++;
	  
		if (cheetahGlobal.skipFract > random_float && frameNumberIncludingSkipped > cheetahGlobal.nInitFrames && cheetahGlobal.calibrated) {
			printf("Skipping a frame (%ld)\n",frameNumberIncludingSkipped);
			skip();
			return;
		}
	  
	  

		static uint32_t nevents = 0;
	  
		frameNumber++;

		/*
		 *  Calculate time beteeen processing of data frames
		 */
		time_t	tnow;
		double	dtime, datarate;
		time(&tnow);
		
		dtime = difftime(tnow, cheetahGlobal.tlast);
		if(dtime > 1.) {
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
			printf("*** r%04u:%li (%3.1fHz): I/O Speed test #1 (psana event rate)\n", cheetahGlobal.runNumber, frameNumber, cheetahGlobal.datarate);		
			return;
		}

		
		
		//	Create a new eventData structure in which to place all information
		cEventData	*eventData;
		eventData = cheetahNewEvent(&cheetahGlobal);
		nevents++;
        
		
		
		/*
		 *  Get RunNumber & EventTime
		 *  PSEvt::EventId.get()
		 */
		int runNumber = 0;
		time_t sec = 0;
		time_t nsec = 0;
		PSTime::Time evtTime;
		boost::shared_ptr<PSEvt::EventId> eventId = evt.get();

		if (eventId.get()) {
			runNumber = eventId->run();
			evtTime = eventId->time();
			sec = evtTime.sec();
			nsec = evtTime.nsec();
			if (verbose) {
				cout << "*** runNumber: " << runNumber << endl; 
				cout << "*** eventTime: " << evtTime << endl;
			}
		}

        
		/* 
		 *  Get number event recorder data
         *  eg: fiducials & beam on state & EVR41 laser on
		 *  Psana::EvrData::DataV3.get()
		 */
		int numEvrData = 0;
		int fiducial = 0;
        bool    beamOn = 0;
        int     pumpLaserOn = 0;
        int     pumpLaserCode = 0;
        
		shared_ptr<Psana::EvrData::DataV3> data3 = evt.get(m_srcEvr);
		if (data3.get()) {
			numEvrData = data3->numFifoEvents();

			// Timestamps
			const ndarray<const Psana::EvrData::FIFOEvent, 1> array = data3->fifoEvents();
			fiducial = array[0].timestampHigh();
			if (verbose) { 
				cout << "*** fiducial: ";
				for (int i=0; i<numEvrData; i++) {
					fiducial = array[i].timestampHigh(); // array[0],array[1]
					cout << fiducial << " ";
				}
				cout << endl;
			}
            
			// Beam on
			beamOn = eventCodePresent(data3->fifoEvents(), beamCode);
			if (verbose) {
				cout << "***** beamOn: " << beamOn << endl;
			}

            
            /*
             *  Pump laser logic 
             *  (usually based on the EVR codes in some way)
             *  Search for 'Pump laser logic' to find all places in which code needs to be changed to implement a new schema
             */
            if(strcmp(cheetahGlobal.pumpLaserScheme, "evr41") == 0) {
                int evr41 = eventCodePresent(data3->fifoEvents(), 41);
                pumpLaserOn = evr41;
                pumpLaserCode = evr41;
            }
            // Schertler, June 2014, LD57
            else if(strcmp(cheetahGlobal.pumpLaserScheme, "LD57") == 0) {
                //  Step  DeltaBeam  EventCode  Device
                //  0     0          183        30 Hz Laser
                //  1     1          184        30 Hz Laser +1 shot
                //  2     1          186        30 Hz Laser +2 shots
                //  3     1          187        30 Hz Laser +3 shots
                int evr183 = eventCodePresent(data3->fifoEvents(), 183);
                int evr184 = eventCodePresent(data3->fifoEvents(), 184);
                int evr186 = eventCodePresent(data3->fifoEvents(), 186);
                int evr187 = eventCodePresent(data3->fifoEvents(), 187);
                
                pumpLaserOn = evr183;
                if(evr183) pumpLaserCode = 1;
                if(evr184) pumpLaserCode = 2;
                if(evr186) pumpLaserCode = 3;
                //if(evr187) pumpLaserCode = 4;
                if(!evr183 && !evr184 && !evr186) pumpLaserCode = 4;
            
                //FILE *fp;
                //fp = fopen("evrcodes.txt","a");
                //fprintf(fp, "%i, %i, %i, %i, %i\n", fiducial, evr183, evr184, evr186, evr187);
                //fclose(fp);
            }
            
            
		}
		else {
			printf("Event %li: Warning: Psana::EvrData::DataV3 failed\n", frameNumber);
			fiducial = frameNumber;
		}

        
        
		/*
		 *  Get Electron beam data
		 *  Psana::Bld::BldDataEBeamV3.get()
		 *
		 *  Note: We have to account for multiple versions of the ebeam configuration (!!!)
		 *  Try the newest versions first, assuming we are more likely to be looking at recent data
		 */
		float charge=0;
		float L3Energy=0;
		float LTUPosX=0; 
		float LTUPosY=0; 
		float LTUAngX=0; 
		float LTUAngY=0; 
		float PkCurrBC2=0;
		double peakCurrent = 0;
		double DL2energyGeV = 0;
		
		shared_ptr<Psana::Bld::BldDataEBeamV6> ebeam6 = evt.get(m_srcBeam);
		shared_ptr<Psana::Bld::BldDataEBeamV5> ebeam5 = evt.get(m_srcBeam);
		shared_ptr<Psana::Bld::BldDataEBeamV4> ebeam4 = evt.get(m_srcBeam);
		shared_ptr<Psana::Bld::BldDataEBeamV3> ebeam3 = evt.get(m_srcBeam);
		shared_ptr<Psana::Bld::BldDataEBeamV2> ebeam2 = evt.get(m_srcBeam);
		shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam1 = evt.get(m_srcBeam);
		shared_ptr<Psana::Bld::BldDataEBeamV0> ebeam0 = evt.get(m_srcBeam);

        // Ebeam v5
		if (ebeam6.get()) {
			charge = ebeam6->ebeamCharge();
			L3Energy = ebeam6->ebeamL3Energy();
			LTUPosX = ebeam6->ebeamLTUPosX();
			LTUPosY = ebeam6->ebeamLTUPosY();
			LTUAngX = ebeam6->ebeamLTUAngX();
			LTUAngY = ebeam6->ebeamLTUAngY();
			PkCurrBC2 = ebeam6->ebeamPkCurrBC2();
			
            peakCurrent = ebeam6->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam6->ebeamL3Energy();
			
			if (verbose) {
				cout << "* fEbeamCharge4=" << charge << "\n"
				<< "* fEbeamL3Energy4=" << L3Energy << "\n"
				<< "* fEbeamLTUPosX4=" << LTUPosX << "\n"
				<< "* fEbeamLTUPosY4=" << LTUPosY << "\n"
				<< "* fEbeamLTUAngX4=" << LTUAngX << "\n"
				<< "* fEbeamLTUAngY4=" << LTUAngY << "\n"
				<< "* fEbeamPkCurrBC24=" << PkCurrBC2 << endl;
			}
		}
        // Ebeam v5
		else if (ebeam5.get()) {
			charge = ebeam5->ebeamCharge();
			L3Energy = ebeam5->ebeamL3Energy();
			LTUPosX = ebeam5->ebeamLTUPosX();
			LTUPosY = ebeam5->ebeamLTUPosY();
			LTUAngX = ebeam5->ebeamLTUAngX();
			LTUAngY = ebeam5->ebeamLTUAngY();
			PkCurrBC2 = ebeam5->ebeamPkCurrBC2();
			
            peakCurrent = ebeam5->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam5->ebeamL3Energy();
			
			if (verbose) {
				cout << "* fEbeamCharge4=" << charge << "\n"
				<< "* fEbeamL3Energy4=" << L3Energy << "\n"
				<< "* fEbeamLTUPosX4=" << LTUPosX << "\n"
				<< "* fEbeamLTUPosY4=" << LTUPosY << "\n"
				<< "* fEbeamLTUAngX4=" << LTUAngX << "\n"
				<< "* fEbeamLTUAngY4=" << LTUAngY << "\n"
				<< "* fEbeamPkCurrBC24=" << PkCurrBC2 << endl;
			}
		}
        // Ebeam v4
		else if (ebeam4.get()) {
			charge = ebeam4->ebeamCharge();
			L3Energy = ebeam4->ebeamL3Energy();
			LTUPosX = ebeam4->ebeamLTUPosX();
			LTUPosY = ebeam4->ebeamLTUPosY();
			LTUAngX = ebeam4->ebeamLTUAngX();
			LTUAngY = ebeam4->ebeamLTUAngY();
			PkCurrBC2 = ebeam4->ebeamPkCurrBC2();
			
            peakCurrent = ebeam4->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam4->ebeamL3Energy();
			
			if (verbose) {
				cout << "* fEbeamCharge4=" << charge << "\n"
					 << "* fEbeamL3Energy4=" << L3Energy << "\n"
					 << "* fEbeamLTUPosX4=" << LTUPosX << "\n"
					 << "* fEbeamLTUPosY4=" << LTUPosY << "\n"
					 << "* fEbeamLTUAngX4=" << LTUAngX << "\n"
					 << "* fEbeamLTUAngY4=" << LTUAngY << "\n"
					 << "* fEbeamPkCurrBC24=" << PkCurrBC2 << endl;
			}
		}
		// Ebeam v3
		else if (ebeam3.get()) {
			charge = ebeam3->ebeamCharge();
			L3Energy = ebeam3->ebeamL3Energy();
			LTUPosX = ebeam3->ebeamLTUPosX();
			LTUPosY = ebeam3->ebeamLTUPosY();
			LTUAngX = ebeam3->ebeamLTUAngX();
			LTUAngY = ebeam3->ebeamLTUAngY();
			PkCurrBC2 = ebeam3->ebeamPkCurrBC2();

			peakCurrent = ebeam3->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam3->ebeamL3Energy();

			if (verbose) {
				cout << "* fEbeamCharge3=" << charge << "\n"
					 << "* fEbeamL3Energy3=" << L3Energy << "\n"
					 << "* fEbeamLTUPosX3=" << LTUPosX << "\n"
					 << "* fEbeamLTUPosY3=" << LTUPosY << "\n"
					 << "* fEbeamLTUAngX3=" << LTUAngX << "\n"
					 << "* fEbeamLTUAngY3=" << LTUAngY << "\n"
					 << "* fEbeamPkCurrBC23=" << PkCurrBC2 << endl;
			}
		}
		// Ebeam v2
		else if (ebeam2.get()) {
			charge = ebeam2->ebeamCharge();
			L3Energy = ebeam2->ebeamL3Energy();
			LTUPosX = ebeam2->ebeamLTUPosX();
			LTUPosY = ebeam2->ebeamLTUPosY();
			LTUAngX = ebeam2->ebeamLTUAngX();
			LTUAngY = ebeam2->ebeamLTUAngY();
			PkCurrBC2 = ebeam2->ebeamPkCurrBC2();

			peakCurrent = ebeam2->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam2->ebeamL3Energy();
            
			if (verbose) {
				cout << "* fEbeamCharge2=" << charge << "\n"
					 << "* fEbeamL3Energy2=" << L3Energy << "\n"
					 << "* fEbeamLTUPosX2=" << LTUPosX << "\n"
					 << "* fEbeamLTUPosY2=" << LTUPosY << "\n"
					 << "* fEbeamLTUAngX2=" << LTUAngX << "\n"
					 << "* fEbeamLTUAngY2=" << LTUAngY << "\n"
					 << "* fEbeamPkCurrBC22=" << PkCurrBC2 << endl;
			}
		}
		// Ebeam v1
		else if (ebeam1.get()) {
			charge = ebeam1->ebeamCharge();
			L3Energy = ebeam1->ebeamL3Energy();
			LTUPosX = ebeam1->ebeamLTUPosX();
			LTUPosY = ebeam1->ebeamLTUPosY();
			LTUAngX = ebeam1->ebeamLTUAngX();
			LTUAngY = ebeam1->ebeamLTUAngY();
			PkCurrBC2 = ebeam1->ebeamPkCurrBC2();
            
			peakCurrent = ebeam1->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam1->ebeamL3Energy();

			if (verbose) {
				cout << "* fEbeamCharge1=" << charge << "\n"
					 << "* fEbeamL3Energy1=" << L3Energy << "\n"
					 << "* fEbeamLTUPosX1=" << LTUPosX << "\n"
					 << "* fEbeamLTUPosY1=" << LTUPosY << "\n"
					 << "* fEbeamLTUAngX1=" << LTUAngX << "\n"
					 << "* fEbeamLTUAngY1=" << LTUAngY << "\n"
					 << "* fEbeamPkCurrBC21=" << PkCurrBC2 << endl;
			}
		}
		// Ebeam v0
		else if (ebeam0.get()) {
			charge = ebeam0->ebeamCharge();
			L3Energy = ebeam0->ebeamL3Energy();
			LTUPosX = ebeam0->ebeamLTUPosX();
			LTUPosY = ebeam0->ebeamLTUPosY();
			LTUAngX = ebeam0->ebeamLTUAngX();
			LTUAngY = ebeam0->ebeamLTUAngY();

			cout << "***** WARNING *****" << endl;
			cout << "EBeamV0 does not record peak current" << endl;
			cout << "Setting peak current to 0" << endl;
			peakCurrent = 0;
			DL2energyGeV = 0.001*ebeam0->ebeamL3Energy();

			if (verbose) {
				cout << "* fEbeamCharge0=" << charge << "\n"
					 << "* fEbeamL3Energy0=" << L3Energy << "\n"
					 << "* fEbeamLTUPosX0=" << LTUPosX << "\n"
					 << "* fEbeamLTUPosY0=" << LTUPosY << "\n"
					 << "* fEbeamLTUAngX0=" << LTUAngX << "\n"
					 << "* fEbeamLTUAngY0=" << LTUAngY << endl;
			}
		}
		else {
			printf("Event %li: Warning: Psana::Bld::BldDataEBeam failed\n", frameNumber);
		}

		

		/*
		 *  Calculate photon energy and wavelength
		 *  Calculate the resonant photon energy (ie: photon wavelength)
		 *  including wakeloss prior to undulators
		 */
		double photonEnergyeV=0;
		double wavelengthA=0;

		double LTUwakeLoss = 0.0016293*peakCurrent;
		// Spontaneous radiation loss per segment
		double SRlossPerSegment = 0.63*DL2energyGeV;
		// wakeloss in an undulator segment
		double wakeLossPerSegment = 0.0003*peakCurrent;
		// energy loss per segment
		double energyLossPerSegment = SRlossPerSegment + wakeLossPerSegment;
		// energy in first active undulator segment [GeV]
		double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss - 0.0005*energyLossPerSegment;
		// Calculate the resonant photon energy of the first active segment
		photonEnergyeV = 44.42*energyProfile*energyProfile;
		// Calculate wavelength in Angstrom
		wavelengthA = 12398.42/photonEnergyeV;
		if (verbose) {
			cout << "***** wavelengthA: " << wavelengthA << endl;
		}
	  
		
        
		/*
		 *  Gas detector values (in FEE)
		 *  Psana::Bld::BldDataFEEGasDetEnergy.get()
		 */
		double gmd1=0, gmd2=0;
		double gmd11=0, gmd12=0, gmd21=0, gmd22=0;

		shared_ptr<Psana::Bld::BldDataFEEGasDetEnergy> fee = evt.get(m_srcFee);
		shared_ptr<Psana::Bld::BldDataFEEGasDetEnergyV1> feeV1 = evt.get(m_srcFee);
		if (feeV1.get()) {
			gmd11 = feeV1->f_11_ENRC();
			gmd12 = feeV1->f_12_ENRC();
			gmd21 = feeV1->f_21_ENRC();
			gmd22 = feeV1->f_22_ENRC();
			gmd1 = (gmd11+gmd12)/2;
			gmd2 = (gmd21+gmd22)/2;
		}
		else if (fee.get()) {
			gmd11 = fee->f_11_ENRC();
			gmd12 = fee->f_12_ENRC();
			gmd21 = fee->f_21_ENRC();
			gmd22 = fee->f_22_ENRC();
			gmd1 = (gmd11+gmd12)/2;
			gmd2 = (gmd21+gmd22)/2;
		}
		else {
			printf("Event %li: Warning: Psana::Bld::BldDataFEEGasDetEnergy failed\n", frameNumber);
		}


		
		/*
		 *  Phase cavity data (timing)
		 *  Psana::Bld::BldDataPhaseCavity.get()
		 */
		float fitTime1=0;
		float fitTime2=0;
		float charge1=0;
		float charge2=0;

		shared_ptr<Psana::Bld::BldDataPhaseCavity> cav = evt.get(m_srcCav);
		if (cav.get()) {
			fitTime1 = cav->fitTime1();
			fitTime2 = cav->fitTime2();
			charge1 = cav->charge1();
			charge2 = cav->charge2();
			if (verbose) {
				cout << "* fitTime1=" << fitTime1 << "\n"
					 << "* fitTime2=" << fitTime2 << "\n"
					 << "* charge1=" << charge1 << "\n"
					 << "* charge2=" << charge2 << endl;
			}
		}
		//else {
		//	printf("Event %li: Warning: Psana::Bld::BldDataPhaseCavity failed\n", frameNumber);
		//}




	
		/*
		 *  Get EPICS data 
		 *  (slow data for stuff like motors)
		 */
		const EpicsStore& estore = env.epicsStore();
		std::vector<std::string> pvNames = estore.pvNames();

		// Detector position
		// Don't forget to initialize them
		std::vector<float> detectorPosition(MAX_DETECTORS,0);

		for(long detID=0; detID<=cheetahGlobal.nDetectors; detID++) {
			shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(cheetahGlobal.detector[detID].detectorZpvname);
			if (pv && pv->numElements() > 0) {
				const float& value = estore.value(cheetahGlobal.detector[detID].detectorZpvname,0);
				detectorPosition[detID] = value;
				if (verbose) {
					cout << "***** DetectorPosition[" << cheetahGlobal.detector[detID].detectorID << "]: " << value << endl;
				}
			}
		}

		// get pumpLaserDelay only for Neutze TiSa delay
		for(long detID=0; detID<=cheetahGlobal.nDetectors; detID++) {
			shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(cheetahGlobal.pumpLaserDelayPV);
			if (pv && pv->numElements() > 0) {
				const float& value = estore.value(cheetahGlobal.pumpLaserDelayPV,0);
				if (verbose) {
					cout << "pumpLaserDelay[" << detID << "]: " << value << endl;
				}  
			}
		}
        
		// Other EPICS PV float values
		for(long i=0; i < cheetahGlobal.nEpicsPvFloatValues; i++) {
			char * thisPv = & cheetahGlobal.epicsPvFloatAddresses[i][0];
			shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(thisPv);
			if (pv && pv->numElements() > 0) {
				const float& value = estore.value(thisPv,0);
				eventData->epicsPvFloatValues[i] = value;
				if (verbose) {
					cout << thisPv << " : " << value << endl;
				}  
			}
		}
				
		
        
		/*
		 *  Copy data into worker thread structure if we got this far.
		 *  SLAC libraries are NOT thread safe: any event info may get overwritten by the next event() call
		 *  Copy all image data into event structure for processing
		 */
		eventData->frameNumber = frameNumber;
		eventData->frameNumberIncludingSkipped = frameNumberIncludingSkipped;
		eventData->seconds = sec;
		eventData->nanoSeconds = nsec;
		eventData->fiducial = fiducial;
		eventData->runNumber = runNumber;
		eventData->beamOn = beamOn;
		eventData->nPeaks = 0;
		eventData->pumpLaserDelay = 0;
		eventData->gmd1 = gmd1;
		eventData->gmd2 = gmd2;
		eventData->gmd11 = gmd11;
		eventData->gmd12 = gmd12;
		eventData->gmd21 = gmd21;
		eventData->gmd22 = gmd22;
		eventData->fEbeamCharge = charge;		// in nC
		eventData->fEbeamL3Energy = L3Energy;		// in MeV
		eventData->fEbeamLTUPosX = LTUPosX;		// in mm
		eventData->fEbeamLTUPosY = LTUPosY;		// in mm
		eventData->fEbeamLTUAngX = LTUAngX;		// in mrad
		eventData->fEbeamLTUAngY = LTUAngY;		// in mrad
		eventData->fEbeamPkCurrBC2 = PkCurrBC2;		// in Amps
		eventData->photonEnergyeV = photonEnergyeV;	// in eV
		eventData->wavelengthA = wavelengthA;		// in Angstrom
		eventData->phaseCavityTime1 = fitTime1;
		eventData->phaseCavityTime2 = fitTime2;
		eventData->phaseCavityCharge1 = charge1;
		eventData->phaseCavityCharge1 = charge2;	
		eventData->pGlobal = &cheetahGlobal;
        
		/*
		 *	Record the visible pump laser on/off state 
         *  (Used for pump/probe and time resolved experiments)
		 */
		eventData->pumpLaserOn = pumpLaserOn;
		eventData->pumpLaserCode = pumpLaserCode;
	
        /*
         *  FEE photon inline spectrometer
         *  Psana::Bld::BldDataSpectrometerV0.get()
         */
		shared_ptr<Psana::Bld::BldDataSpectrometerV0> FEEspectrum0 = evt.get(m_srcFeeSpec);
		eventData->FEEspec_present=0;
		if(cheetahGlobal.useFEEspectrum) {
			if (FEEspectrum0.get()) {
				const ndarray<const uint32_t, 1>& hproj = FEEspectrum0->hproj();
				const ndarray<const uint32_t, 1>& vproj = FEEspectrum0->vproj();
				if (!hproj.empty() &&  !vproj.empty()) {
					long	hsize = hproj.shape()[0];
					long	vsize = hproj.shape()[0];
					//printf("FEEspectrum is %li x %li\n", hsize, vsize);
					
					eventData->FEEspec_hproj = (uint32_t*) calloc(hsize, sizeof(uint32_t));
					eventData->FEEspec_vproj = (uint32_t*) calloc(vsize, sizeof(uint32_t));
					memcpy(eventData->FEEspec_hproj, hproj.data(), hsize*sizeof(uint32_t));
					// The next line often throws a fatal memcpy() error.  No time to figure out why
					// For now, comment out as spectrum is contained in FEEspec_hproj so we don't actually need this right now
					//memcpy(eventData->FEEspec_vproj, vproj.data(), vsize*sizeof(uint32_t));
					eventData->FEEspec_present = 1;
				}
				else {
					printf("Event %li: Warning: Empty Psana::Bld::BldDataSpectrometerV0\n", frameNumber);
				}
			}
			else {
				printf("Event %li: Warning: Psana::Bld::BldDataSpectrometerV0.get() failed\n", frameNumber);
			}
		}
		

		
		
 				
		/*
         *  Copy primary area detector data into Cheetah event structure
         *  SLAC libraries are not thread safe: so we must copy the data and not simply pass a pointer
         */

		for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
      
			/*
			 *  cspad
			 */
			if(strcmp(cheetahGlobal.detector[detID].detectorType, "cspad") == 0 ) {
				
				// Pull out front or back detector depending on detID=0 or 1
				shared_ptr<Psana::CsPad::DataV1> data1;
				shared_ptr<Psana::CsPad::DataV2> data2;
				if (cheetahGlobal.detector[detID].detectorID == 0) {
					data1 = evt.get(m_srcCspad0, m_key);
					data2 = evt.get(m_srcCspad0, m_key);
				} 
				else if (cheetahGlobal.detector[detID].detectorID == 1) {
					data1 = evt.get(m_srcCspad1, m_key);
					data2 = evt.get(m_srcCspad1, m_key);
				}
				

				// V2 of the cspad structure
				if (data2.get()) {
					if (verbose) {
						cout << "CsPad::DataV2:";
						int nQuads = data2->quads_shape()[0];
						for (int q = 0; q < nQuads; ++ q) {
							const Psana::CsPad::ElementV2& el = data2->quads(q);
							cout << "\n  Element #" << q;
							cout << "\n    virtual_channel = " << el.virtual_channel();
							cout << "\n    lane = " << el.lane();
							cout << "\n    tid = " << el.tid();
							cout << "\n    acq_count = " << el.acq_count();
							cout << "\n    op_code = " << el.op_code();
							cout << "\n    quad = " << el.quad();
							cout << "\n    seq_count = " << el.seq_count();
							cout << "\n    ticks = " << el.ticks();
							cout << "\n    fiducials = " << el.fiducials();
							cout << "\n    frame_type = " << el.frame_type();
						}
						cout << endl;
					}

                    
					uint16_t *quad_data[4];
					long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
					long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
					long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
                        
                        
					// Allocate memory for detector data and set to zero
					int nQuads = data2->quads_shape()[0];
					for(int quadrant=0; quadrant<4; quadrant++)
						quad_data[quadrant] = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
					
					// loop over elements (quadrants)
					for (int q = 0; q < nQuads; ++ q) {
						const Psana::CsPad::ElementV2& el = data2->quads(q); 
						const ndarray<const int16_t, 3>& data = el.data();
						if(el.quad() < 4){
							// Which quadrant is this?
							int quadrant = el.quad();
							eventData->fiducial = el.fiducials();
							
							// Read 2x1 "sections" into data array in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
							for (unsigned s = 0; s != data.shape()[0]; ++s) {
								memcpy(&quad_data[quadrant][s*2*asic_nx*asic_ny],&data[s][0][0],2*asic_nx*asic_ny*sizeof(uint16_t));
							}
						}
					}        
					
					// Assemble data from all four quadrants into one large array (rawdata layout)
					// Memcpy is necessary for thread safety.
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
					for(int quadrant=0; quadrant<4; quadrant++) {
						free(quad_data[quadrant]);
					}
				}

				// V1 of the cspad data structure (less likely these days, but we came across it once already)
				else if (data1.get()) {
					if (verbose) {
						cout << "CsPad::DataV2:";
						int nQuads = data1->quads_shape()[0];
						for (int q = 0; q < nQuads; ++ q) {
							const Psana::CsPad::ElementV1& el = data1->quads(q);
							cout << "\n  Element #" << q;
							cout << "\n    virtual_channel = " << el.virtual_channel();
							cout << "\n    lane = " << el.lane();
							cout << "\n    tid = " << el.tid();
							cout << "\n    acq_count = " << el.acq_count();
							cout << "\n    op_code = " << el.op_code();
							cout << "\n    quad = " << el.quad();
							cout << "\n    seq_count = " << el.seq_count();
							cout << "\n    ticks = " << el.ticks();
							cout << "\n    fiducials = " << el.fiducials();
							cout << "\n    frame_type = " << el.frame_type();
						}
						cout << endl;
					}
					
					
					uint16_t *quad_data[4];
					long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
					long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
					long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
					
					
					// Allocate memory for detector data and set to zero
					int nQuads = data1->quads_shape()[0];
					for(int quadrant=0; quadrant<4; quadrant++) 
						quad_data[quadrant] = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
					
					// loop over elements (quadrants)
					for (int q = 0; q < nQuads; ++ q) {
						const Psana::CsPad::ElementV1& el = data1->quads(q);
						const ndarray<const int16_t, 3>& data = el.data();
						if(el.quad() < 4){
							// Which quadrant is this?
							int quadrant = el.quad();
							eventData->fiducial = el.fiducials();
							
							// Read 2x1 "sections" into data array in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
							for (unsigned s = 0; s != data.shape()[0]; ++s) {
								memcpy(&quad_data[quadrant][s*2*asic_nx*asic_ny],&data[s][0][0],2*asic_nx*asic_ny*sizeof(uint16_t));
							}
						}
					}
					
					// Assemble data from all four quadrants into one large array (rawdata layout)
					// Memcpy is necessary for thread safety.
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
					for(int quadrant=0; quadrant<4; quadrant++) {
						free(quad_data[quadrant]);
					}
				}

				// Neither V1 nor V2
				else {
					printf("%li: cspad frame data not available for detector ID %li\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
					printf("Event %li: Warning: CSPAD frame data not available for detector ID %li, skipping event.\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
					cheetahDestroyEvent(eventData);
					return;
				}
			}
			else if (strcmp(cheetahGlobal.detector[detID].detectorType, "cspad2x2") == 0) {
				long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
				long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
				long    asic_ny = cheetahGlobal.detector[detID].asic_ny;

				shared_ptr<Psana::CsPad2x2::ElementV1> singleQuad;
				singleQuad = evt.get(m_srcCspad2x2, m_key);
				if (singleQuad.get()) {
					eventData->detector[detID].raw_data = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
					const ndarray<const int16_t, 3>& data = singleQuad->data();
					int partsize = asic_nx * asic_ny * 2;
					for (unsigned s = 0; s < 2; s++) {
						for (int y = 0; y < asic_ny; y++) {
							for (int x = 0; x < asic_nx * 2; x++) {
								eventData->detector[detID].raw_data[s*partsize + y * asic_nx * 2 + x] = data[y][x][s];
							}
						}
					}
			   
				} else {
					printf("%li: cspad 2x2 frame data not available for detector ID %li\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
					/* NOTE: There might be a need to destroy event here as shown below */
					/*
					  printf("Event %li: Warning: CSPAD frame data not available for detector ID %li, skipping event.\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
					  cheetahDestroyEvent(eventData);
					*/
					return;
				}
			}

			
			/*
			 *	CsPad 2x2
			 */
			else if (strcmp(cheetahGlobal.detector[detID].detectorType, "cspad2x2") == 0) {
				long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
				long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
				long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
				
				shared_ptr<Psana::CsPad2x2::ElementV1> singleQuad;
				singleQuad = evt.get(m_srcCspad2x2, m_key);
				if (singleQuad.get()) {
					eventData->detector[detID].raw_data = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
					const ndarray<const int16_t, 3>& data = singleQuad->data();
					int partsize = asic_nx * asic_ny * 2;
					for (unsigned s = 0; s < 2; s++) {
						for (int y = 0; y < asic_ny; y++) {
							for (int x = 0; x < asic_nx * 2; x++) {
								eventData->detector[detID].raw_data[s*partsize + y * asic_nx * 2 + x] = data[y][x][s];
							}
						}
					}
				} else {
                    
                    printf("Event %li: Warning: CSPAD 2x2 frame data not available for detector ID %li, skipping event.\n", frameNumber,cheetahGlobal.detector[detID].detectorID);
                    cheetahDestroyEvent(eventData);
					return;
				}
			}
       
			/*
			 *
			 *	The data format used for pnccd data has changed in recent releases. 
			 *	To get the full image data for pnccd you need to use Psana::PNCCD::FullFrameV1 type now instead of Psana::PNCCD:: FrameV1 
			 *		https://pswww.slac.stanford.edu/swdoc/releases/ana-current/psddl_psana/type.Psana.PNCCD.FullFrameV1.html
			 *	For an example of use of this new type (which is very similar to the old one) check out the psana_examples/DumpPnccd module:
			 *		https://pswww.slac.stanford.edu/swdoc/releases/ana-current/psana-modules-doxy/html/DumpPnccd_8cpp-source.html
			 */
			else if(strcmp(cheetahGlobal.detector[detID].detectorType, "pnccd") == 0 ) {
				
				// Pull out front or back detector depending on detID=0 or 1
				// Make this selectable in the .ini file (easier: let which detector is whcih be set in psana.conf)
				shared_ptr<Psana::PNCCD::FrameV1> frame;
				shared_ptr<Psana::PNCCD::FullFrameV1> fullframe;
				
				if (cheetahGlobal.detector[detID].detectorID == 0) {
					//cout << "front" << endl;
					//frame = evt.get(m_srcPnccd0);	
					fullframe = evt.get(m_srcPnccd0);
				} 
				else if (cheetahGlobal.detector[detID].detectorID == 1) {
					//cout << "back" << endl;
					//frame = evt.get(m_srcPnccd1);
					fullframe = evt.get(m_srcPnccd1);
				}
				
				// copy data into event structure if successful
				if (fullframe) {
					const	ndarray<const uint16_t, 2> data = fullframe->data();
					long	nx = data.shape()[0];
					long	ny = data.shape()[1];
					long    pix_nn = nx*ny;
					//cout << nx << "x" << ny << " = " << pix_nn << endl;
					eventData->detector[detID].raw_data = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
					memcpy(&eventData->detector[detID].raw_data[0],&data[0][0],nx*ny*sizeof(uint16_t));
				}
				else {
                    printf("Event %li: Warning: pnCCD frame data not available (detectorID=%li), skipping event.\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
                    cheetahDestroyEvent(eventData);
					return;
				}
			}
			
			// Didn't find any recognised detectors??
			else {
				printf("Unknown detector type: %s, aborting/n", cheetahGlobal.detector[detID].detectorType);
				exit(1);
			}

		}	// end loop over detectors

		
		//	Copy TOF (aqiris) channel into Cheetah event for processing
		//  SLAC libraries are not thread safe: must copy data into event structure for processing
		//eventData->TOFPresent = 0; // DO NOT READ TOF
		if (cheetahGlobal.TOFPresent==1){
			int chan = cheetahGlobal.TOFchannel;
			Pds::Src src;
			shared_ptr<Psana::Acqiris::DataDescV1> acqData = evt.get(m_srcAcq);
			if (acqData) {
				shared_ptr<Psana::Acqiris::ConfigV1> acqConfig = env.configStore().get(m_srcAcq);
				const Psana::Acqiris::DataDescV1Elem& elem = acqData->data(chan);
				const Psana::Acqiris::VertV1& v = acqConfig->vert()[chan];
				double slope = v.slope();
				double offset = v.offset();
				const Psana::Acqiris::HorizV1& h = acqConfig->horiz();
				double sampInterval = h.sampInterval();
				const ndarray<const Psana::Acqiris::TimestampV1, 1>& timestamps = elem.timestamp();
				const ndarray<const int16_t, 2>& waveforms = elem.waveforms();
				int seg = 0;
				eventData->TOFtrigtime = timestamps[seg].pos();
				eventData->TOFTime = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
				eventData->TOFVoltage = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
				double * tempTime = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
				double * tempVoltage = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
				double timestamp = timestamps[seg].value();
				ndarray<const int16_t, 1> raw(waveforms[seg]);
				for (int i = 0; i < cheetahGlobal.AcqNumSamples; ++ i) {
					tempTime[i] = timestamp + i*sampInterval;
					tempVoltage[i] = raw[i]*slope + offset;
				}
				//Memcpy is necessary for thread safety.
				memcpy(eventData->TOFTime, &tempTime[0], cheetahGlobal.AcqNumSamples*sizeof(double));
				memcpy(eventData->TOFVoltage, &tempVoltage[0], cheetahGlobal.AcqNumSamples*sizeof(double));
				free(tempTime);
				free(tempVoltage);
				eventData->TOFPresent = 1;
			} else {
				eventData->TOFPresent = 0;
			}
		}
		

		
		/*
		 *  Copy Pulnix camera into Cheetah event for processing
		 *  Pulnix 120Hz CCD camera on CXI Questar micrscope
		 *  (where the actual camera is CxiSc1 not XppSb3PimCvd)
		 *  The choice of CxiEndstation is for a particular camera.
		 *  Here are some of the possible alternatives:
		 *      CxiDg1
		 *      CxiDg2
		 *      CxiDg4
		 *      CxiKb1
		 *      CxiSc1
		 *  SLAC libraries are not thread safe: must copy data into event structure for processing
		 */
		eventData->pulnixFail = 1;
		int usePulnix = 0;		// Ignore Pulnix camera
		if(usePulnix) {
			shared_ptr<Psana::Camera::FrameV1> frmData = evt.get(m_srcCam);
			if (frmData.get()) {
				if (verbose) {
					cout << "Camera::FrameV1: width=" << frmData->width()
						 << " height=" << frmData->height()
						 << " depth=" << frmData->depth()
						 << " offset=" << frmData->offset() ;
                    
					const ndarray<const uint8_t, 2>& data8 = frmData->data8();
					if (not data8.empty()) {
						cout << " data8=[" << int(data8[0][0])
							 << ", " << int(data8[0][1])
							 << ", " << int(data8[0][2]) << ", ...]";
					}
                    
					const ndarray<const uint16_t, 2>& data16 = frmData->data16();
					if (not data16.empty()) {
						cout << " data16=[" << int(data16[0][0])
							 << ", " << int(data16[0][1])
							 << ", " << int(data16[0][2]) << ", ...]";
					}  
					cout << endl;
				}
				eventData->pulnixFail = 0;
				eventData->pulnixWidth = frmData->width();
				eventData->pulnixHeight = frmData->height();

				const ndarray<const uint8_t, 2>& data8 = frmData->data8();
				if (not data8.empty()) {
					cout << "Pulnix(uint8_t) will not be passed to Cheetah. Complain to Chuck if you need this!" << endl;
					//eventData->pulnixImage = (uint8_t*) calloc(eventData->pulnixWidth*eventData->pulnixHeight, sizeof(uint8_t));
					//memcpy(eventData->pulnixImage, &data8[0][0], (long)eventData->pulnixWidth*(long)eventData->pulnixHeight*sizeof(uint8_t));
				}

				const ndarray<const uint16_t, 2>& data16 = frmData->data16();
				if (not data16.empty()) {
					eventData->pulnixImage = (uint16_t*) calloc(eventData->pulnixWidth*eventData->pulnixHeight, sizeof(uint16_t));
					memcpy(eventData->pulnixImage, &data16[0][0], (long)eventData->pulnixWidth*(long)eventData->pulnixHeight*sizeof(uint16_t));
				}  
			}
		}

        
		/*
		 *  Copy energy spectrum camera into Cheetah for processing
		 *  Currently an Opal2k camera (or Opal1k)
		 *  current spectrum camera is at CxiEndstation.0:Opal2000.1
		 *  SLAC libraries are not thread safe: must copy data into event structure
		 *  Only retrieve camera info if we want to look at the spectrum
		 */
		eventData->specFail = 1;
		if (cheetahGlobal.espectrum) {
			shared_ptr<Psana::Camera::FrameV1> specData = evt.get(m_srcSpec);

			if (specData.get()) {
				if (verbose) {
					cout << "Camera::FrameV1: width=" << specData->width()
						 << " height=" << specData->height()
						 << " depth=" << specData->depth()
						 << " offset=" << specData->offset() ;
                    
					const ndarray<const uint8_t, 2>& data8 = specData->data8();
					if (not data8.empty()) {
						cout << " data8=[" << int(data8[0][0])
							 << ", " << int(data8[0][1])
							 << ", " << int(data8[0][2]) << ", ...]";
					}
                    
					const ndarray<const uint16_t, 2>& data16 = specData->data16();
					if (not data16.empty()) {
						cout << " data16=[" << int(data16[0][0])
							 << ", " << int(data16[0][1])
							 << ", " << int(data16[0][2]) << ", ...]";
					}
					cout << endl;
				}

				eventData->specFail = 0;
				eventData->specWidth = specData->width();
				eventData->specHeight = specData->height();
            
				const ndarray<const uint8_t, 2>& data8 = specData->data8();
				if (not data8.empty()) {
					cout << "Opal2k(uint8_t) will not be passed to Cheetah. Complain if you need this!" << endl;
				}
            
				const ndarray<const uint16_t, 2>& data16 = specData->data16();
				if (not data16.empty()) {
					eventData->specImage = (uint16_t*) calloc(eventData->specWidth*eventData->specHeight, sizeof(uint16_t));
					memcpy(eventData->specImage, &data16[0][0], (long)eventData->specWidth*(long)eventData->specHeight*sizeof(uint16_t));
				}  
			}
		}
		
		// Update detector positions
		for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {        
			eventData->detector[detID].detectorZ = detectorPosition[detID];
		}

        
		// Call cheetah in multi-threaded mode (ensures that cheetah cleans up event data when done)
		cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);
	}
	// End of psana event method
	 
		
		
	/// Method which is called at the end of the calibration cycle
	void 
	cheetah_ana_mod::endCalibCycle(Event& evt, Env& env)
	{
	}

	/// Method which is called at the end of the run
	void cheetah_ana_mod::endRun(Event& evt, Env& env)
	{
		
		/*
		 *	Wait for all worker threads to finish
		 *	Sometimes the program hangs here, so wait no more than 10 minutes before exiting anyway
		 */
		time_t	tstart, tnow;
		time(&tstart);
		double	dtime;
		float	maxwait = 10 * 60.;
		int p=0, pp=0;
		
		while(cheetahGlobal.nActiveThreads > 0) {
			p = cheetahGlobal.nActiveThreads;
			if ( pp != p){
				pp = p;
				printf("Ending run. Waiting for %li worker threads to finish.\n", cheetahGlobal.nActiveThreads);
			}
			time(&tnow);
			dtime = difftime(tnow, tstart);
			if(dtime > maxwait) {
				printf("\t%li threads still active after waiting %f seconds\n", cheetahGlobal.nActiveThreads, dtime);
				printf("\tGiving up and exiting anyway\n");
				cheetahGlobal.nActiveThreads = 0;
				break;
			}
			usleep(100000);
		}
		
		if(cheetahGlobal.saveCXI) {
			printf("Writing accumulated CXIDB file\n");
			writeAccumulatedCXI(&cheetahGlobal);
			closeCXIFiles(&cheetahGlobal);
		}
	}


	/// Method which is called once at the end of the job
	///	Clean up all variables associated with libCheetah
	void cheetah_ana_mod::endJob(Event& evt, Env& env)
	{
		cheetahExit(&cheetahGlobal);
	  
		time_t endT;
		time(&endT);
		double dif = difftime(endT,startT);
		cout << "time taken: " << dif << " seconds" << endl;
		// We shouldn't exit, and specially not with a value of 1
		//	  exit(1);
		// Just retuning allows the proper destructors to be called
		return;
	}

} // namespace cheetah_ana_pkg
