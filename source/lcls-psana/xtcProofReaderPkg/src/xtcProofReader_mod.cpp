//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class xtcProofReader_mod...
//
// Author List:
//      Chunhong Yoon
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "xtcProofReaderPkg/xtcProofReader_mod.h"
#include "/reg/neh/home3/yoon82/cheetah/source/cheetah.lib/cheetah.h"
//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <string>
//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger/MsgLogger.h"
// to work with detector data include corresponding 
// header from psddl_psana package
#include "psddl_psana/bld.ddl.h"
#include "PSEvt/EventId.h"
#include "psddl_psana/cspad.ddl.h"
#include "psddl_psana/evr.ddl.h"
#include "psddl_psana/acqiris.ddl.h"
#include "psddl_psana/camera.ddl.h"

#include "/reg/neh/home3/yoon82/cheetah/source/lcls/myana/release/pdsdata/cspad/ElementIterator.hh"

#define beamCode 140
#define laserCode 41
#define verbose 1
//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace xtcProofReaderPkg;
using namespace std;
PSANA_MODULE_FACTORY(xtcProofReader_mod)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace xtcProofReaderPkg {
static int count = 0;
static cGlobal cheetahGlobal;
static int laserSwitch = 0;
static int prevLaser = 0;
char detZ[] = "CXI:DS1:MMS:06.RBV";

template <typename T>
bool eventCodePresent(const ndarray<T, 1>& array, unsigned EvrCode){
  //const int nfifo =  array.size();
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
xtcProofReader_mod::xtcProofReader_mod (const std::string& name)
  : Module(name)
  , m_src()
  , m_maxEvents()
  , m_filter()
  , m_count(0)
{
  // get the values from configuration or use defaults
  //m_src = configStr("source", "DetInfo(:Acqiris)");
  m_maxEvents = config("events", 32U);
  m_filter = config("filter", false);
	m_key = configStr("inputKey", "");
	m_src = configStr("source","DetInfo(:Cspad)");
	m_srcEvr = configStr("evrSource","DetInfo(:Evr)");
	m_srcBeam = configStr("beamSource","BldInfo(:EBeam)");
	m_srcFee = configStr("feeSource","BldInfo(:FEEGasDetEnergy)");
	m_srcCav = configStr("cavitySource","BldInfo(:PhaseCavity)");
	m_srcAcq = configStr("acqirisSource","DetInfo(:Acqiris)");
	m_srcCam = configStr("cameraSource","DetInfo()");
}

//--------------
// Destructor --
//--------------
xtcProofReader_mod::~xtcProofReader_mod ()
{
}

/// Method which is called once at the beginning of the job
void 
xtcProofReader_mod::beginJob(Event& evt, Env& env)
{
}

/// Method which is called at the beginning of the run
void 
xtcProofReader_mod::beginRun(Event& evt, Env& env)
{
}

/// Method which is called at the beginning of the calibration cycle
void 
xtcProofReader_mod::beginCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
xtcProofReader_mod::event(Event& evt, Env& env)
{
////////////////////////// MY CODE STARTS HERE /////////////////////
  	int numErrors = 0;
  	count++;
	cout << count << endl;

  	// get RunNumber & EventTime
  	int runNumber = 0;
  	PSTime::Time evtTime;
  	boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  	if (eventId.get()) {
		runNumber = eventId->run();
    		evtTime = eventId->time();
		if (verbose) {
  			cout << "*** runNumber: " << runNumber << endl; 
  			cout << "*** eventTime: " << evtTime << endl;
		}
  	} else {
		numErrors++;
		if (verbose) {
		cout << "/////////////////////////////////" << "\n"
	     << "//				//" << "\n"
	     << "//	ERROR: NO EVENT ID	//" << "\n"
	     << "//				//" << "\n"
	     << "/////////////////////////////////" << endl;
  		}
  	}

  // get number of EvrData & fiducials
  int numEvrData = 0;
  shared_ptr<Psana::EvrData::DataV3> data3 = evt.get(m_srcEvr);
  if (data3.get()) {
	numEvrData = data3->numFifoEvents();
  	const ndarray<const Psana::EvrData::FIFOEvent, 1> array = data3->fifoEvents();
	if (verbose) { 
		cout << "*** fiducial: ";
		for (int i=0; i<numEvrData; i++) {
 			const int fiducial = array[i].timestampHigh(); // array[0] and array[1]
  			cout << fiducial << " ";
		}
		cout << endl;
	}
  } else {
	numErrors++;
	if (verbose) {
        cout << "/////////////////////////////////" << "\n"
             << "//                             //" << "\n"
             << "//     ERROR: NO EVR DATA      //" << "\n"
             << "//                             //" << "\n"
             << "/////////////////////////////////" << endl;
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
  // Soft: 100 ~ 1 Ang
  // Hard: 1 ~ 0.1 Ang
  if (wavelengthA <= 0 || wavelengthA > 100) {
	numErrors++;
	if (verbose) {
        cout << "/////////////////////////////////" << "\n"
             << "//                             //" << "\n"
             << "//   ERROR: WRONG WAVELENGTH   //" << "\n"
             << "//                             //" << "\n"
             << "/////////////////////////////////" << endl;
  	}
  }
  // wavelengthA different to myana (DON'T WORK)

  // get gasdet[4]
  double gmd1, gmd2;
  shared_ptr<Psana::Bld::BldDataFEEGasDetEnergy> fee = evt.get(m_srcFee);
  if (fee.get()) {
	gmd1 = (fee->f_11_ENRC()+fee->f_12_ENRC())/2;
	gmd2 = (fee->f_21_ENRC()+fee->f_22_ENRC())/2;
  	if (verbose) {
		cout << "*** gmd1 , gmd2: " << gmd1 << " , " << gmd2 << endl;  
  	}
  } else {
	numErrors++;
	if (verbose) {
	cout << "/////////////////////////////////" << "\n"
             << "//                             //" << "\n"
             << "//      ERROR: NO FEE GAS      //" << "\n"
             << "//                             //" << "\n"
             << "/////////////////////////////////" << endl;
  	}
  }
  // gmd1,gmd2 different to myana (DON'T WORK) 

  // get PhaseCavity
  shared_ptr<Psana::Bld::BldDataPhaseCavity> cav = evt.get(m_srcCav);
  if (cav.get()) {
	float fitTime1 = cav->fitTime1();
	float fitTime2 = cav->fitTime2();
	float charge1 = cav->charge1();
	float charge2 = cav->charge2();
	if (verbose) {
  	cout << "* fitTime1=" << fitTime1 << "\n"
             << "* fitTime2=" << fitTime2 << "\n"
             << "* charge1=" << charge1 << "\n"
             << "* charge2=" << charge2 << endl;
	}
	if (fitTime1 == 0 || fitTime2 == 0 || charge1 == 0 || charge2 == 0) {
	if (verbose) {
	cout << "+++++++++++++++++++++++++++++++++++" << "\n"
             << "++                               ++" << "\n"
             << "++  WARNING: ZERO PHASE CAVITY   ++" << "\n"
             << "++                               ++" << "\n"
             << "+++++++++++++++++++++++++++++++++++" << endl;
	}
	}
  } else {
	if( verbose) {
  	cout << "+++++++++++++++++++++++++++++++++++" << "\n"
             << "++                               ++" << "\n"
             << "++    WARNING: NO PHASE CAVITY   ++" << "\n"
             << "++                               ++" << "\n"
             << "+++++++++++++++++++++++++++++++++++" << endl;
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
		if (value < -10 || value > 10) {
		if (verbose) {
		cout << "+++++++++++++++++++++++++++++++++++++++" << "\n"
                     << "++                                   ++" << "\n"
                     << "++    WARNING: LARGE LASER DELAY     ++" << "\n"
                     << "++                                   ++" << "\n"
                     << "+++++++++++++++++++++++++++++++++++++++" << endl;
		}
		}
        } else {
	if (verbose) {
	cout << "+++++++++++++++++++++++++++++++++++" << "\n"
             << "++                               ++" << "\n"
             << "++    WARNING: NO TiSa DELAY     ++" << "\n"
             << "++                               ++" << "\n"
             << "+++++++++++++++++++++++++++++++++++" << endl;
	}
	}
  }


  //!! get detector position (Z)
  for(long detID=0; detID<=cheetahGlobal.nDetectors; detID++) {
	shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(detZ);
  	//shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(cheetahGlobal.detector[detID].detectorZpvname);
	//cout << pv->numElements() << endl;
	if (pv && pv->numElements() > 0) {
		const float& value = estore.value(detZ,0);
		//const float& value = estore.value(cheetahGlobal.detector[detID].detectorZpvname,0);
		if (verbose) {
			cout << "***** DetectorPosition[" << detID << "]: " << value << endl;
		}
		if (value < -10 || value > 10) {
		if (verbose) {
		cout << "+++++++++++++++++++++++++++++++++++++++++++++++" << "\n"
             	     << "++                               	      ++" << "\n"
                     << "++    WARNING: LARGE DETECTOR POSITION Z     ++" << "\n"
             	     << "++                               	      ++" << "\n"
             	     << "+++++++++++++++++++++++++++++++++++++++++++++++" << endl;
		}
		}
  	} else {
	 numErrors++;
	 if (verbose) {
	cout << "/////////////////////////////////////////" << "\n"
             << "//                                     //" << "\n"
             << "//      ERROR: NO DETECTOR POS Z       //" << "\n"
             << "//                                     //" << "\n"
             << "/////////////////////////////////////////" << endl;
	}
	}
  }

  //! get beamOn
  bool beamOn = eventCodePresent(data3->fifoEvents(), beamCode);
  if (verbose) {
	cout << "***** beamOn: " << beamOn << endl;
  }
  if (!beamOn) {
	numErrors++;
	if (verbose) {
	cout << "////////////////////////////////////////////" << "\n"
             << "//                                        //" << "\n"
             << "//      ERROR: NO BEAM. GAME OVER!        //" << "\n"
             << "//                                        //" << "\n"
             << "////////////////////////////////////////////" << endl;
  	}
  }

  //! get laserOn
  bool laserOn = eventCodePresent(data3->fifoEvents(), laserCode);
  if (count == 1) {
	// initialize
	prevLaser = laserOn;
	laserSwitch = 1;
  } else {
 	if (prevLaser != laserOn) {
		laserSwitch++;
		prevLaser = laserOn;
  	}
  }
  if (verbose) {
	cout << "*** laserOn: " << laserOn << "\n"
	     << "laserSwitch/count: " << laserSwitch << "/" << count << endl;
  }
  // laserSwitch should be as large as count (50% on and off)

  //!! get CsPadData
  for (long detID=0; detID<=cheetahGlobal.nDetectors; detID++){
	shared_ptr<Psana::CsPad::DataV2> data2 = evt.get(m_src, m_key);
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
  	} else {
	numErrors++;
	if (verbose) {
	cout << "/////////////////////////////////////////" << "\n"
             << "//                                     //" << "\n"
             << "//      ERROR: NO CSPAD DATA V2        //" << "\n"
             << "//                                     //" << "\n"
             << "/////////////////////////////////////////" << endl;
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
		cout << "size: " << size << endl;
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
  } else {
	if (verbose) {
	cout << "++++++++++++++++++++++++++++++++++++" << "\n"
             << "++                                ++" << "\n"
             << "++    WARNING: NO ACQIRIS TOF     ++" << "\n"
             << "++                                ++" << "\n"
             << "++++++++++++++++++++++++++++++++++++" << endl;
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
  } else {
	if (verbose) {
	cout << "++++++++++++++++++++++++++++++++++++++" << "\n"
             << "++                                  ++" << "\n"
             << "++    WARNING: NO PULNIX CAMERA     ++" << "\n"
             << "++                                  ++" << "\n"
             << "++++++++++++++++++++++++++++++++++++++" << endl;
  	}
  }

  // Message
//  if (numErrors > 0) {
//  	cout << "number of Errors: " << numErrors << endl;
//  }
  if (numErrors == 0) {
	cout << "OK" << endl;
  } else {
	cout << "BAD DATAGRAM: " << count << endl;
  }
}
  
/// Method which is called at the end of the calibration cycle
void 
xtcProofReader_mod::endCalibCycle(Event& evt, Env& env)
{
}

/// Method which is called at the end of the run
void 
xtcProofReader_mod::endRun(Event& evt, Env& env)
{
}

/// Method which is called once at the end of the job
void 
xtcProofReader_mod::endJob(Event& evt, Env& env)
{
}

} // namespace xtcProofReaderPkg
