
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
#define laserCode 41
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
	static int laserSwitch = 0;
	static int prevLaser = 0;
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
		/* If SIT_DATA is undefined set it to the builtin value */
		setenv("SIT_DATA",CHEETAH_SIT_DATA,0);

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
		cout << "*** beginJob ***" << endl;
		time(&startT);
		cheetahInit(&cheetahGlobal);
		if(cheetahGlobal.saveCXI){
		  signal(SIGINT, sig_handler);
		}

	}

	
	/// Method which is called at the beginning of the run
	///	Pass new run information to Cheetah
	void cheetah_ana_mod::beginRun(Event& evt, Env& env)
	{
	cout << "Experiment = " << env.experiment() << endl;
	cout << "*** beginRun ***" << endl;
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
	  
	  if (cheetahGlobal.skipFract > random_float && frameNumberIncludingSkipped > cheetahGlobal.nInitFrames) {
	      printf("Skipping a frame (%d)\n",frameNumberIncludingSkipped);
	      skip();
	      return;
	  }
	  
	  

	  static uint32_t nevents = 0;
	  
		frameNumber++;

		//	Create a new eventData structure in which to place all information
		cEventData	*eventData;
		eventData = cheetahNewEvent(&cheetahGlobal);
		nevents++;

		// Calculate time beteeen processing of data frames
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
		  printf("*** r%04u:%li (%3.1fHz): I/O Speed test #1\n", cheetahGlobal.runNumber, frameNumber, cheetahGlobal.datarate);		
		  return;
		}

		
		
		// get RunNumber & EventTime
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

		// get number of EvrData & fiducials
		int numEvrData = 0;
		int fiducial = 0;
		shared_ptr<Psana::EvrData::DataV3> data3 = evt.get(m_srcEvr);

		if (data3.get()) {
			numEvrData = data3->numFifoEvents();

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
		}

		// get EBeam
		// EBeamV0 ~ EBeamV3
		// EBeamV2 not implemented yet
		float charge=0;
		float L3Energy=0;
		float LTUPosX=0; 
		float LTUPosY=0; 
		float LTUAngX=0; 
		float LTUAngY=0; 
		float PkCurrBC2=0;
		
		// Ebeam v0
		shared_ptr<Psana::Bld::BldDataEBeamV0> ebeam0 = evt.get(m_srcBeam);
		if (ebeam0.get()) {
			charge = ebeam0->ebeamCharge();
			L3Energy = ebeam0->ebeamL3Energy();
			LTUPosX = ebeam0->ebeamLTUPosX();
			LTUPosY = ebeam0->ebeamLTUPosY();
			LTUAngX = ebeam0->ebeamLTUAngX();
			LTUAngY = ebeam0->ebeamLTUAngY();
			if (verbose) {
			cout << "* fEbeamCharge0=" << charge << "\n"
			     << "* fEbeamL3Energy0=" << L3Energy << "\n"
			     << "* fEbeamLTUPosX0=" << LTUPosX << "\n"
			     << "* fEbeamLTUPosY0=" << LTUPosY << "\n"
			     << "* fEbeamLTUAngX0=" << LTUAngX << "\n"
			     << "* fEbeamLTUAngY0=" << LTUAngY << endl;
			}
		}

		// Ebeam v1
		shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam1 = evt.get(m_srcBeam);
		if (ebeam1.get()) {
			charge = ebeam1->ebeamCharge();
			L3Energy = ebeam1->ebeamL3Energy();
			LTUPosX = ebeam1->ebeamLTUPosX();
			LTUPosY = ebeam1->ebeamLTUPosY();
			LTUAngX = ebeam1->ebeamLTUAngX();
			LTUAngY = ebeam1->ebeamLTUAngY();
			PkCurrBC2 = ebeam1->ebeamPkCurrBC2();
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

		// Ebeam v2
		shared_ptr<Psana::Bld::BldDataEBeamV2> ebeam2 = evt.get(m_srcBeam);
		if (ebeam2.get()) {
		charge = ebeam2->ebeamCharge();
		L3Energy = ebeam2->ebeamL3Energy();
		LTUPosX = ebeam2->ebeamLTUPosX();
		LTUPosY = ebeam2->ebeamLTUPosY();
		LTUAngX = ebeam2->ebeamLTUAngX();
		LTUAngY = ebeam2->ebeamLTUAngY();
		PkCurrBC2 = ebeam2->ebeamPkCurrBC2();
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

		// Ebeam v3
		shared_ptr<Psana::Bld::BldDataEBeamV3> ebeam3 = evt.get(m_srcBeam);
		if (ebeam3.get()) {
		charge = ebeam3->ebeamCharge();
		L3Energy = ebeam3->ebeamL3Energy();
		LTUPosX = ebeam3->ebeamLTUPosX();
		LTUPosY = ebeam3->ebeamLTUPosY();
		LTUAngX = ebeam3->ebeamLTUAngX();
		LTUAngY = ebeam3->ebeamLTUAngY();
		PkCurrBC2 = ebeam3->ebeamPkCurrBC2();
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

		// Calculate photon energy and wavelength
		double photonEnergyeV=0;
		double wavelengthA=0;
		double peakCurrent = 0;
		double DL2energyGeV = 0;
		if (ebeam0.get()) {
			cout << "***** WARNING *****" << endl;
			cout << "EBeamV0 does not record peak current" << endl;
			cout << "Setting peak current to 0" << endl;
			peakCurrent = 0;
			DL2energyGeV = 0.001*ebeam0->ebeamL3Energy();
		} else if (ebeam1.get()) {
			// Get the present peak current in Amps
			// Get present beam energy [GeV]
			peakCurrent = ebeam1->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam1->ebeamL3Energy();
		} else if (ebeam2.get()) {
			peakCurrent = ebeam2->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam2->ebeamL3Energy();
		} else if (ebeam3.get()) {
			peakCurrent = ebeam3->ebeamPkCurrBC2();
			DL2energyGeV = 0.001*ebeam3->ebeamL3Energy();
		}
		
		// get wavelengthA
		// Calculate the resonant photon energy (ie: photon wavelength) 
		// wakeloss prior to undulators
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
	  
		
		// Gas detector values	
		// get gasdet[4]
		double gmd1=0, gmd2=0;
		double gmd11=0, gmd12=0, gmd21=0, gmd22=0;

		shared_ptr<Psana::Bld::BldDataFEEGasDetEnergy> fee = evt.get(m_srcFee);
		if (fee.get()) {
			gmd11 = fee->f_11_ENRC();
			gmd12 = fee->f_12_ENRC();
			gmd21 = fee->f_21_ENRC();
			gmd22 = fee->f_22_ENRC();
			gmd1 = (gmd11+gmd12)/2;
			gmd2 = (gmd21+gmd22)/2;
			if (verbose) {
				cout << "*** gmd1 , gmd2: " << gmd1 << " , " << gmd2 << endl;  
			}
		} 

		
		// get PhaseCavity data (timing)
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

		// get laserDelay only for Neutze TiSa delay
		// access EPICS store
		const EpicsStore& estore = env.epicsStore(); 
		// get the names of EPICS PVs
		std::vector<std::string> pvNames = estore.pvNames();   
		for(long detID=0; detID<=cheetahGlobal.nDetectors; detID++) {
			shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(cheetahGlobal.laserDelayPV);
			if (pv && pv->numElements() > 0) {
					const float& value = estore.value(cheetahGlobal.laserDelayPV,0);
					if (verbose) {
						cout << "laserDelay[" << detID << "]: " << value << endl;
			}
			}
		}


		// Misc. EPICS PV float values
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

	

		// Detector position
		float detectorPosition[MAX_DETECTORS];
		//!! get detector position (Z)
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

	//! get beamOn
	bool beamOn;
	if (data3.get()) {
		beamOn = eventCodePresent(data3->fifoEvents(), beamCode);
		if (verbose) {
			cout << "***** beamOn: " << beamOn << endl;
		}

		
		//! get laserOn
		bool laserOn = eventCodePresent(data3->fifoEvents(), laserCode);
		if (frameNumber == 1) {
			// initialize
			prevLaser = laserOn;
			laserSwitch = 1;
		} 
		else {
			if (prevLaser != laserOn) {
				laserSwitch++;
				prevLaser = laserOn;
			}
		}
		if (verbose) {
		cout << "*** laserOn: " << laserOn << "\n"
			 << "laserSwitch/frameNumber: " << laserSwitch << "/" << frameNumber << endl;
		}
		// laserSwitch should be as large as count (50% on and off)
	}

		//!! get CsPadData
		for (long detID=0; detID<cheetahGlobal.nDetectors; detID++){

			shared_ptr<Psana::CsPad::DataV2> data2;
			if (cheetahGlobal.detector[detID].detectorID == 0) {
				data2 = evt.get(m_srcCspad0, m_key);
			} 
			else if (cheetahGlobal.detector[detID].detectorID == 1) {
				data2 = evt.get(m_srcCspad1, m_key);
			}
			else if (cheetahGlobal.detector[detID].detectorID == 2) {
				data2 = evt.get(m_srcCspad2x2, m_key);
			}
			
			if (data2.get()) {
				if (verbose) {
					cout << "CsPad::DataV2:";
					int nQuads = data2->quads_shape()[0];
					for (int q = 0; q < nQuads; ++ q) {
						const Psana::CsPad::ElementV2& el = data2->quads(q);
						cout << "\n  Element #" << q 
						<< "\n    virtual_channel = " << el.virtual_channel()         
						<< "\n    lane = " << el.lane() 
						<< "\n    tid = " << el.tid() 
						<< "\n    acq_count = " << el.acq_count() 
						<< "\n    op_code = " << el.op_code() 
						<< "\n    quad = " << el.quad() 
						<< "\n    seq_count = " << el.seq_count() 
						<< "\n    ticks = " << el.ticks() 
						<< "\n    fiducials = " << el.fiducials() 
						<< "\n    frame_type = " << el.frame_type();
					}
					cout << endl;
				}
			}
		}

		
		
		//? get Acqiris
		shared_ptr<Psana::Acqiris::DataDescV1> acq = evt.get(m_srcAcq);
		if (acq.get()) {
		  if (verbose) {	
		    // find matching config object
				shared_ptr<Psana::Acqiris::ConfigV1> acqConfig = env.configStore().get(m_srcAcq);
				// loop over channels
				int nchan = acq->data_shape()[0];
				cout << "nchan: " << nchan << endl;
				for (int chan = 0; chan < nchan; ++ chan) {
					const Psana::Acqiris::DataDescV1Elem& elem = acq->data(chan);
					const Psana::Acqiris::VertV1& v = acqConfig->vert()[chan];
					double slope = v.slope();
					double offset = v.offset(); 
					cout << "slope, offset: " << v.slope() << " , " << v.offset() << endl;
					const Psana::Acqiris::HorizV1& h = acqConfig->horiz();
					double sampInterval = h.sampInterval();
					cout << "sampInterval: " << sampInterval << endl;
				cout << "Acqiris::DataDescV1: channel=" << chan
						 << "\n  nbrSegments=" << elem.nbrSegments()
						 << "\n  nbrSamplesInSeg=" << elem.nbrSamplesInSeg()
						 << "\n  indexFirstPoint=" << elem.indexFirstPoint();
					const ndarray<const Psana::Acqiris::TimestampV1, 1>& timestamps = elem.timestamp();
					const ndarray<const int16_t, 2>& waveforms = elem.waveforms();
					// loop over segments
					for (unsigned seg = 0; seg<elem.nbrSegments(); ++seg) {
						unsigned size = std::min(elem.nbrSamplesInSeg(), 32U);
					cout << "\n  size: " << size << endl;
					cout << "\n  Segment #" << seg
							<< "\n    timestamp=" << timestamps[seg].pos()
							<< "\n    data=[";
							for (unsigned i = 0; i < size; ++ i) {
						// -offset in myanao getAcqValue
								cout << (waveforms[seg][i]*slope + offset) << ", ";
						}
						cout << "...]";
					}
				cout << endl;
				}
			}
		}

		// get Pulnix
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
		}
        
        // get spectrum (Opal2k)
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
		}
				
		
		//	Copy all interesting information into worker thread structure if we got this far.
		//  SLAC libraries are NOT thread safe: any event info may get overwritten by the next event() call
		//  Copy all image data into event structure for processing
		eventData->frameNumber = frameNumber;
		eventData->seconds = sec;
		eventData->nanoSeconds = nsec;
		eventData->fiducial = fiducial;
		eventData->runNumber = runNumber;
		eventData->beamOn = beamOn;
		eventData->nPeaks = 0;
		eventData->laserEventCodeOn = 0;
		eventData->laserDelay = 0;
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

		//	Copy raw cspad image data into Cheetah event structure for processing
		//  SLAC libraries are not thread safe: must copy data into event structure for processing
		for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
			
			// CSPAD
			if(strcmp(cheetahGlobal.detector[detID].detectorType, "cspad") == 0 ) {
				uint16_t *quad_data[4];        
				long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
				long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
				long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
				
				
				// Pull out front or back detector depending on detID=0 or 1
				shared_ptr<Psana::CsPad::DataV2> data2;
				if (cheetahGlobal.detector[detID].detectorID == 0) {
					data2 = evt.get(m_srcCspad0, m_key);
				} 
				else if (cheetahGlobal.detector[detID].detectorID == 1) {
					data2 = evt.get(m_srcCspad1, m_key);
				}
				
				// copy data into event structure if successful
				if (data2.get()) {

					// Allocate memory for detector data and set to zero
					for(int quadrant=0; quadrant<4; quadrant++) {
						quad_data[quadrant] = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
					}

					int nQuads = data2->quads_shape()[0];
					
					// loop over elements (quadrants)
					for (int q = 0; q < nQuads; ++ q) {
						const Psana::CsPad::ElementV2& el = data2->quads(q); 
						const ndarray<const int16_t, 3>& data = el.data();
						if(el.quad() < 4){
							// Which quadrant is this?
							int quadrant = el.quad();
							
							// Read 2x1 "sections" into data array in DAQ format, i.e., 2x8 array of asics (two bytes / pixel)
							for (unsigned s = 0; s != data.shape()[0]; ++s) {
								memcpy(&quad_data[quadrant][s*2*asic_nx*asic_ny],&data[s][0][0],2*asic_nx*asic_ny*sizeof(uint16_t));
							}
							// Get temperature on strong back, just in case we want it for anything 
							//float	temperature = std::numeric_limits<float>::quiet_NaN();;
							//eventData->detector[detID].quad_temperature[quadrant] = temperature;
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
					for(int quadrant=0; quadrant<4; quadrant++) 
						free(quad_data[quadrant]);
				}
				else {
				  printf("%li: cspad frame data not available for detector ID %d\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
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
			    printf("%li: cspad 2x2 frame data not available for detector ID %d\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
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
					printf("%li: pnCCD frame data not available (detectorID=%li)\n", frameNumber, cheetahGlobal.detector[detID].detectorID);
					return;
				}
			}
			
			// Didn't find any recognised detectors??
			else {
				printf("Unknown detector type: %s/n", cheetahGlobal.detector[detID].detectorType);
				exit(1);
			}

		}	// end loop over detectors

		
		//	Copy TOF (aqiris) channel into Cheetah event for processing
		//  SLAC libraries are not thread safe: must copy data into event structure for processing
		//eventData->TOFPresent = 0; // DO NOT READ TOF
		eventData->TOFPresent = cheetahGlobal.TOFPresent ;	
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
		    int sampInterval = h.sampInterval();
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
		    for (unsigned i = 0; i < cheetahGlobal.AcqNumSamples; ++ i) {
		      tempTime[i] = timestamp + i*sampInterval;
		      tempVoltage[i] = raw[i]*slope + offset;
		    }
		    //Memcpy is necessary for thread safety.
		    memcpy(eventData->TOFTime, &tempTime[0], cheetahGlobal.AcqNumSamples*sizeof(double));
		    memcpy(eventData->TOFVoltage, &tempVoltage[0], cheetahGlobal.AcqNumSamples*sizeof(double));
		    free(tempTime);
		    free(tempVoltage);
		  }
		  else
		    eventData->TOFPresent = cheetahGlobal.TOFPresent ;	
		}
		
		//	Copy Pulnix camera into Cheetah event for processing
		//	Pulnix 120Hz CCD camera on CXI Questar micrscope
		//	(where the actual camera is CxiSc1 not XppSb3PimCvd)
		//	The choice of CxiEndstation is for a particular camera.  
		//	Here are some of the possible alternatives:
		//		CxiDg1
		//		CxiDg2
		//		CxiDg4
		//		CxiKb1
		//		CxiSc1
		//  SLAC libraries are not thread safe: must copy data into event structure for processing
		eventData->pulnixFail = 1;
		int usePulnix = 0;		// Ignore Pulnix camera
		if (frmData.get() && usePulnix ) {
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

        //	Copy Opal2k camera into Cheetah event for processing
		//	energy spectrum analysis
		//	current spectrum camera is at CxiEndstation.0:Opal2000.1
		//  SLAC libraries are not thread safe: must copy data into event structure for processing
		eventData->specFail = 1;
		int useSpec = 0;		// Ignore Opal camera
		if (specData.get() && useSpec) {
			eventData->specFail = 0;
			eventData->specWidth = specData->width();
			eventData->specHeight = specData->height();
            
			const ndarray<const uint8_t, 2>& data8 = specData->data8();
			if (not data8.empty()) {
				cout << "Opal2k(uint8_t) will not be passed to Cheetah. Complain if you need this!" << endl;
				//eventData->pulnixImage = (uint8_t*) calloc(eventData->pulnixWidth*eventData->pulnixHeight, sizeof(uint8_t));
				//memcpy(eventData->pulnixImage, &data8[0][0], (long)eventData->pulnixWidth*(long)eventData->pulnixHeight*sizeof(uint8_t));
			}
            
			const ndarray<const uint16_t, 2>& data16 = specData->data16();
			if (not data16.empty()) {
				eventData->specImage = (uint16_t*) calloc(eventData->specWidth*eventData->specHeight, sizeof(uint16_t));
				memcpy(eventData->specImage, &data16[0][0], (long)eventData->specWidth*(long)eventData->specHeight*sizeof(uint16_t));
			}  
		}

		
		// Update detector positions
		for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {        
			eventData->detector[detID].detectorZ = detectorPosition[detID];
		}

		// Set CXI name				
		//sprintf(eventData->cxiFilename,"e%d-r%04d.cxi",env.expNum(),eventData->runNumber);
		//sprintf(eventData->cxiFilename,"%s-r%04d.cxi", env.experiment().c_str(),eventData->runNumber);
        
		// Call cheetah
		cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);
	}
	 
		
		
	/// Method which is called at the end of the calibration cycle
	void 
	cheetah_ana_mod::endCalibCycle(Event& evt, Env& env)
	{
	}

	/// Method which is called at the end of the run
	void 
	cheetah_ana_mod::endRun(Event& evt, Env& env)
	{
	  // Wait for all workers to finish
		int p=0;
		int pp=0;
		while(cheetahGlobal.nActiveThreads > 0) {
	    	p = cheetahGlobal.nActiveThreads;
			if ( pp != p){
				pp = p;
				printf("Ending run. Waiting for %li worker threads to finish.\n", cheetahGlobal.nActiveThreads);
				usleep(100000);
			}
	  }
	  writeAccumulatedCXI(&cheetahGlobal);
	  //closeCXIFiles(&cheetahGlobal);
	}

	/// Method which is called once at the end of the job
	///	Clean up all variables associated with libCheetah
	void 
	cheetah_ana_mod::endJob(Event& evt, Env& env)
	{
	  cheetahExit(&cheetahGlobal);
	  
	  time_t endT;
	  time(&endT);
	  double dif = difftime(endT,startT);
	  cout << "time taken: " << dif << " seconds" << endl;
	  exit(1);
	}

} // namespace cheetah_ana_pkg
