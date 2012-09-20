//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class chuck_ana_mod... XTC proof reader
//
// Author List:
//      Chunhong Yoon	06/2012		chun.hong.yoon@cfel.de
//  	Copyright (c) 2012 CFEL. All rights reserved.
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "chuck_ana_pkg/chuck_ana_mod.h"

#include "/reg/neh/home3/yoon82/cheetah/source/cheetah.lib/cheetah.h"
//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
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
//#include "psddl_psana/xtc.ddl.h"

#include "/reg/neh/home3/yoon82/cheetah/source/lcls/myana/release/pdsdata/cspad/ElementIterator.hh"

#define beamCode 140
#define laserCode 41
#define verbose 1

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

// This declares this class as psana module
using namespace chuck_ana_pkg;
PSANA_MODULE_FACTORY(chuck_ana_mod)

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace chuck_ana_pkg {
static int count = 0;
static cGlobal cheetahGlobal;

void print(std::ostream& str, unsigned i, const Psana::EvrData::FIFOEvent& f)
  {
    str << "\n    fifo event #" << i
        <<  " timestampHigh=" << f.timestampHigh()
        <<  " timestampLow=" << f.timestampLow()
        << " eventCode=" << f.eventCode();
  }

template <typename T>
  void print_array(std::ostream& str, const ndarray<T, 1>& array) {
    for (unsigned i = 0; i < array.size(); ++ i) {
      print(str, i, array[i]);
    }
  }

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
	// Initialise libCheetah
	//cheetahInit(&cheetahGlobal);
}

// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
chuck_ana_mod::event(Event& evt, Env& env)
{
  int numErrors = 0;
  count++;
cout << count << endl;
/*
  if (count==2){
	exit(1);
  }  
  std::cout << "count = " << count << std::endl;
*/

  // get event time
  PSTime::Time evtTime;
  boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (eventId.get()) {
    evtTime = eventId->time();
    //std::string t_str = (eventId->time()).asStringFormat("%Y-%m-%d %H:%M:%s%f%z");
  }
  
  // get beam data
  boost::shared_ptr<Psana::Bld::BldDataEBeamV1> ebeam = evt.get(m_srcBeam);
  if (ebeam.get()) {
    MsgLog(name(), info, "time: " << evtTime << ", charge: " << ebeam->ebeamCharge() << ", energy: " 
<< ebeam->ebeamL3Energy());
  }

  // config
  shared_ptr<Psana::CsPad::ConfigV3> config3 = env.configStore().get(m_src);
  if (config3.get()) {
        WithMsgLog(name(), info, str) {
                str << "\n eventCode = " << config3->eventCode();
        }
  }

  // get event ID
 // shared_ptr<PSEvt::EventId> eventId = evt.get();
  if (!eventId.get()){
	MsgLog(name(), info, "event ID not found");
  } else {
	MsgLog(name(), info, "event ID: " << *eventId);
  }

  // get fifo
  shared_ptr<Psana::EvrData::DataV3> data3 = evt.get(m_srcEvr);
  if (data3.get()) {
    WithMsgLog(name(), info, str) {
      str << "Chuck Encoder::DataV3: numFifoEvents=" << data3->numFifoEvents();
     ::print_array(str, data3->fifoEvents());
    }
  } else {
      std::cout << "Enter the dragon" << std::endl;
  }

std::cout << "==========================================" << std::endl;

  // get run number
  int runNumber = eventId->run();
  std::cout << "*** runNumber: " << runNumber << std::endl;

  // get time
  std::cout << "*** getTime: " << evtTime << std::endl;

  // get fiducials
  const ndarray<Psana::EvrData::FIFOEvent, 1> array = data3->fifoEvents();
  const int fiducial = array[0].timestampHigh(); // array[0] and array[1]
  std::cout << "*** fiducial: " << fiducial << std::endl;

  // get numEvrData
  int numEvrData = data3->numFifoEvents();
  std::cout << "*** numEvrData: " << numEvrData << std::endl;

  // get EvrData
  if (data3.get()) {
	for(int i = 0; i < numEvrData; i++ ) { // sizeof(array) = 16
    		std::cout << "*** getEvrData (i,eventCode,fiducial,timeStamp): "  << i << "," << array[i].eventCode() << "," << array[i].timestampHigh() << "," << array[i].timestampLow() << std::endl; 
	}
  }

  // get EBeam
  // EBeamV0 ~ EBeamV3
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
  /* Calculate the resonant photon energy (ie: photon wavelength) */
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
  std::cout << "***** wavelengthA: " << wavelengthA << std::endl;

  // get gasdet[4]
  double gmd1, gmd2;
  shared_ptr<Psana::Bld::BldDataFEEGasDetEnergy> fee = evt.get(m_srcFee);
  if (fee.get()) {
	gmd1 = (fee->f_11_ENRC()+fee->f_12_ENRC())/2;
	gmd2 = (fee->f_21_ENRC()+fee->f_22_ENRC())/2;
	std::cout << "*** gasdet[4] = " << fee->f_11_ENRC()
          << " " << fee->f_12_ENRC()
          << " " << fee->f_21_ENRC()
          << " " << fee->f_22_ENRC() << std::endl;
  	std::cout << "gmd1,gmd2: " << gmd1 << "," << gmd2 << std::endl;  
  }
  // gmd1,gmd2 different to myana  

  // get PhaseCavity
  shared_ptr<Psana::Bld::BldDataPhaseCavity> cav = evt.get(m_srcCav);
  if (cav.get()) {
  std::cout << "\n  fitTime1=" << cav->fitTime1()
          << "\n  fitTime2=" << cav->fitTime2()
          << "\n  charge1=" << cav->charge1()
          << "\n  charge2=" << cav->charge2();
  }
  std::cout << "*** getPhaseCavity: " << cav.get() << std::endl;
  
  //? get PvFloat (laserDelay only for Neutze TiSa delay)
   
  //!! get detector position (Z)

  //! get beamOn
  bool beamOn = eventCodePresent(data3->fifoEvents(), beamCode);
  std::cout << "beamOn: " << beamOn << std::endl;

  //! get laserOn
  bool laserOn = eventCodePresent(data3->fifoEvents(), laserCode);  
  std::cout << "laserOn: " <<  laserOn << std::endl;

  //!! get CsPadData
  std::cout << "nDetectors: " << cheetahGlobal.nDetectors << std::endl;
  for (long detID=0; detID<cheetahGlobal.nDetectors; detID++){
	uint16_t *quad_data[4];
	Pds::CsPad::ElementIterator iter;
	
  }


  //? get Acqiris
  shared_ptr<Psana::Acqiris::DataDescV1> acq = evt.get(m_srcAcq);

  // get Pulnix
  shared_ptr<Psana::Camera::FrameV1> cam = evt.get(m_srcCam);

}
  
} // namespace chuck_ana_pkg
