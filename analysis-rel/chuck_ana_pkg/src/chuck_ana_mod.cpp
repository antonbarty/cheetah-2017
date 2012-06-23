//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class chuck_ana_mod...
//
// Author List:
//      Chunhong Yoon
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "chuck_ana_pkg/chuck_ana_mod.h"
#include "/reg/neh/home/yoon82/cheetah/source/cheetah.lib/cheetah.h"
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

#include "/reg/neh/home/yoon82/cheetah/source/lcls/myana/release/pdsdata/cspad/ElementIterator.hh"

#define beamCode 140
#define laserCode 41
#define verbose 1
//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace chuck_ana_pkg;
using namespace std;
PSANA_MODULE_FACTORY(chuck_ana_mod)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace chuck_ana_pkg {
static int count = 0;
static cGlobal cheetahGlobal;
static int laserSwitch = 0;
static int prevLaser = 0;

//char detZ[] = "CXI:SC1:MZM:06";
char detZ[] = "CXI:DS1:MMS:06.RBV";

//void print(std::ostream& str, unsigned i, const Psana::EvrData::FIFOEvent& f)
//  {
//    str << "\n    fifo event #" << i
//        <<  " timestampHigh=" << f.timestampHigh()
//        <<  " timestampLow=" << f.timestampLow()
//        << " eventCode=" << f.eventCode();
//  }

//template <typename T>
//  void print_array(std::ostream& str, const ndarray<T, 1>& array) {
//    for (unsigned i = 0; i < array.size(); ++ i) {
//      print(str, i, array[i]);
//    }
//  }

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
chuck_ana_mod::chuck_ana_mod (const std::string& name)
  : Module(name)
{
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
chuck_ana_mod::~chuck_ana_mod ()
{
}

// Method called at the beginning of the calibration cycle
void
chuck_ana_mod::beginCalibCycle(Event& evt, Env& env)
{
	// Initialize libCheetah
	//cheetahInit(&cheetahGlobal);
}

// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
chuck_ana_mod::event(Event& evt, Env& env)
{
  int numErrors = 0;
  count++;

//if (count==10){exit(1);}
//  if (count==325 || count==435){
//
//  } else {
//	return;
	//exit(1);
//  }  

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
  	const ndarray<Psana::EvrData::FIFOEvent, 1> array = data3->fifoEvents();
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
  shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam1 = evt.get(m_srcBeam);
  if (ebeam1.get()) {
	float charge = ebeam1->ebeamCharge();
	float L3Energy = ebeam1->ebeamL3Energy();
	float LTUPosX = ebeam1->ebeamLTUPosX();
	float LTUPosY = ebeam1->ebeamLTUPosY();
	float LTUAngX = ebeam1->ebeamLTUAngX();
	float LTUAngY = ebeam1->ebeamLTUAngY();
	float PkCurrBC2 = ebeam1->ebeamPkCurrBC2();
	if (verbose) {
	cout << "* fEbeamCharge1=" << charge << "\n"
         	<< "* fEbeamL3Energy1=" << L3Energy << "\n"
        	<< "* fEbeamLTUPosX1=" << LTUPosX << "\n"
         	<< "* fEbeamLTUPosY1=" << LTUPosY << "\n"
         	<< "* fEbeamLTUAngX1=" << LTUAngX << "\n"
         	<< "* fEbeamLTUAngY1=" << LTUAngY << "\n"
         	<< "* fEbeamPkCurrBC21=" << PkCurrBC2 << endl;
	}
	if ( charge < 0 || L3Energy < 0 || LTUPosX < 0 || LTUPosY < 0
 	     || LTUAngX < 0 || LTUAngY < 0 || PkCurrBC2 < 0) {
	if (verbose) {
	cout << "+++++++++++++++++++++++++++++++++++" << "\n"
             << "++                               ++" << "\n"
             << "++ WARNING: NEGATIVE EBEAM PARAM ++" << "\n"
             << "++                               ++" << "\n"
             << "+++++++++++++++++++++++++++++++++++" << endl;
	}
	}
  } else {
	//numErrors++;
	//if (verbose) {
        //cout << "/////////////////////////////////" << "\n"
        //     << "//                             //" << "\n"
        //     << "//     ERROR: NO EBEAM 1       //" << "\n"
        //     << "//                             //" << "\n"
        //     << "/////////////////////////////////" << endl;
  	//}
  }

  shared_ptr<Psana::Bld::BldDataEBeamV3> ebeam3 = evt.get(m_srcBeam);
  if (ebeam3.get()) {
	float charge = ebeam3->ebeamCharge();
	float L3Energy = ebeam3->ebeamL3Energy();
	float LTUPosX = ebeam3->ebeamLTUPosX();
	float LTUPosY = ebeam3->ebeamLTUPosY();
	float LTUAngX = ebeam3->ebeamLTUAngX();
	float LTUAngY = ebeam3->ebeamLTUAngY();
	float PkCurrBC2 = ebeam3->ebeamPkCurrBC2();
	if (verbose) {
	cout << "* fEbeamCharge2=" << charge << "\n"
         	<< "* fEbeamL3Energy2=" << L3Energy << "\n"
        	<< "* fEbeamLTUPosX2=" << LTUPosX << "\n"
         	<< "* fEbeamLTUPosY2=" << LTUPosY << "\n"
         	<< "* fEbeamLTUAngX2=" << LTUAngX << "\n"
         	<< "* fEbeamLTUAngY2=" << LTUAngY << "\n"
         	<< "* fEbeamPkCurrBC22=" << PkCurrBC2 << endl;
	}
	if ( charge < 0 || L3Energy < 0 || LTUPosX < 0 || LTUPosY < 0
 	     || LTUAngX < 0 || LTUAngY < 0 || PkCurrBC2 < 0) {
	if (verbose) {
	cout << "+++++++++++++++++++++++++++++++++++" << "\n"
             << "++                               ++" << "\n"
             << "++ WARNING: NEGATIVE EBEAM PARAM3++" << "\n"
             << "++                               ++" << "\n"
             << "+++++++++++++++++++++++++++++++++++" << endl;
	}
	}
  } else {
	numErrors++;
	if (verbose) {
        cout << "/////////////////////////////////" << "\n"
             << "//                             //" << "\n"
             << "//     ERROR: NO EBEAM 3       //" << "\n"
             << "//                             //" << "\n"
             << "/////////////////////////////////" << endl;
  	}
  }

  if (ebeam3.get()){
  // get wavelengthA
  double photonEnergyeV;
  double wavelengthA;
  /* Calculate the resonant photon energy (ie: photon wavelength) */
  // Get the present peak current in Amps
  double peakCurrent = ebeam3->ebeamPkCurrBC2();
  // Get present beam energy [GeV]
  double DL2energyGeV = 0.001*ebeam3->ebeamL3Energy();
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
  }

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
        const ndarray<Psana::Acqiris::TimestampV1, 1>& timestamps = elem.timestamp();
	const ndarray<int16_t, 2>& waveforms = elem.waveforms();
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

      	const ndarray<uint8_t, 2>& data8 = frmData->data8();
      	if (not data8.empty()) {
        cout << " data8=[" << int(data8[0][0])
             << ", " << int(data8[0][1])
             << ", " << int(data8[0][2]) << ", ...]";
      	}

      	const ndarray<uint16_t, 2>& data16 = frmData->data16();
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
} // event
} // namespace chuck_ana_pkg
