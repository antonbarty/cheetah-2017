
//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class cheetah_ana_mod...
//
// Author List:
//      Chunhong Yoon
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "cheetah_ana_pkg/cheetah_ana_mod.h"
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
static int count = 0;
static cGlobal cheetahGlobal;
static int laserSwitch = 0;
static int prevLaser = 0;

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
cheetah_ana_mod::cheetah_ana_mod (const std::string& name)
  : Module(name)
{
  	// get the values from configuration or use defaults
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
cheetah_ana_mod::~cheetah_ana_mod ()
{
}

/// Method which is called once at the beginning of the job
void 
cheetah_ana_mod::beginJob(Event& evt, Env& env)
{
}

/// Method which is called at the beginning of the run
void 
cheetah_ana_mod::beginRun(Event& evt, Env& env)
{
	/*
         *      Pass new run information to Cheetah
         */
	int runNumber = 0;
  	PSTime::Time evtTime;
  	boost::shared_ptr<PSEvt::EventId> eventId = evt.get();
  	if (eventId.get()) {
        	runNumber = eventId->run();
	}
        cheetahGlobal.runNumber = runNumber;
    	cheetahNewRun(&cheetahGlobal);
	printf("User analysis beginrun() routine called.\n");
        printf("Processing r%04u\n",runNumber);
}

/// Method which is called at the beginning of the calibration cycle
void 
cheetah_ana_mod::beginCalibCycle(Event& evt, Env& env)
{
	cheetahInit(&cheetahGlobal);
}

/// Method which is called with event data, this is the only required 
/// method, all other methods are optional
void 
cheetah_ana_mod::event(Event& evt, Env& env)
{
 
 int numErrors = 0;
  count++;

//if (count==2){exit(1);}  

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

  //cheetahGlobal.runNumber = runNumber;
  // get number of EvrData & fiducials
  int numEvrData = 0;
  int fiducial = 0;
  shared_ptr<Psana::EvrData::DataV3> data3 = evt.get(m_srcEvr);
  if (data3.get()) {
	numEvrData = data3->numFifoEvents();
  	const ndarray<Psana::EvrData::FIFOEvent, 1> array = data3->fifoEvents();
	fiducial = array[0].timestampHigh();
	if (verbose) { 
		cout << "*** fiducial: ";
		for (int i=0; i<numEvrData; i++) {
 			fiducial = array[i].timestampHigh(); // array[0] and array[1]
  			cout << fiducial << " ";
		}
		cout << endl;
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
	cout << "* fEbeamCharge2=" << charge << "\n"
         	<< "* fEbeamL3Energy2=" << L3Energy << "\n"
        	<< "* fEbeamLTUPosX2=" << LTUPosX << "\n"
         	<< "* fEbeamLTUPosY2=" << LTUPosY << "\n"
         	<< "* fEbeamLTUAngX2=" << LTUAngX << "\n"
         	<< "* fEbeamLTUAngY2=" << LTUAngY << "\n"
         	<< "* fEbeamPkCurrBC22=" << PkCurrBC2 << endl;
	}
  }

  double photonEnergyeV=0;
  double wavelengthA=0;

  if (ebeam1.get()){
  // get wavelengthA
  // Calculate the resonant photon energy (ie: photon wavelength) 
  // Get the present peak current in Amps
  double peakCurrent = ebeam1->ebeamPkCurrBC2();
  // Get present beam energy [GeV]
  double DL2energyGeV = 0.001*ebeam1->ebeamL3Energy();
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
  }

  if (ebeam3.get()){
  // get wavelengthA
  // Calculate the resonant photon energy (ie: photon wavelength) 
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
  }

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

  // get PhaseCavity
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

  float detectorPosition[MAX_DETECTORS];
  //!! get detector position (Z)
  for(long detID=0; detID<=cheetahGlobal.nDetectors; detID++) {
  	shared_ptr<Psana::Epics::EpicsPvHeader> pv = estore.getPV(cheetahGlobal.detector[detID].detectorZpvname);
	if (pv && pv->numElements() > 0) {
		const float& value = estore.value(cheetahGlobal.detector[detID].detectorZpvname,0);
		detectorPosition[detID] = value;
		if (verbose) {
			cout << "***** DetectorPosition[" << detID << "]: " << value << endl;
		}
  	}
  }

  //! get beamOn
  bool beamOn = eventCodePresent(data3->fifoEvents(), beamCode);
  if (verbose) {
	cout << "***** beamOn: " << beamOn << endl;
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
  }
	
	 //	Create a new eventData structure in which to place all information
	 
	cEventData	*eventData;
	eventData = cheetahNewEvent(&cheetahGlobal);

	
	 //	Copy all interesting information into worker thread structure if we got this far.
     	 //  SLAC libraries are NOT thread safe: any event info may get overwritten by the next event() call
     	 //  Copy all image data into event structure for processing
    	eventData->frameNumber = count;
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
	eventData->fEbeamL3Energy = L3Energy;	// in MeV
	eventData->fEbeamLTUPosX = LTUPosX;		// in mm
	eventData->fEbeamLTUPosY = LTUPosY;		// in mm
	eventData->fEbeamLTUAngX = LTUAngX;		// in mrad
	eventData->fEbeamLTUAngY = LTUAngY;		// in mrad
	eventData->fEbeamPkCurrBC2 = PkCurrBC2;	// in Amps
	eventData->photonEnergyeV = photonEnergyeV;	// in eV
	eventData->wavelengthA = wavelengthA;			// in Angstrom

	eventData->phaseCavityTime1 = fitTime1;
	eventData->phaseCavityTime2 = fitTime2;
	eventData->phaseCavityCharge1 = charge1;
	eventData->phaseCavityCharge1 = charge2;
	
	eventData->pGlobal = &cheetahGlobal;

	
	 //	Copy raw cspad image data into Cheetah event structure for processing
     	 //  SLAC libraries are not thread safe: must copy data into event structure for processing
	int fail=0;
	for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {
        	uint16_t *quad_data[4];        
            		nevents++;
            		long    pix_nn = cheetahGlobal.detector[detID].pix_nn;
			long    asic_nx = cheetahGlobal.detector[detID].asic_nx;
			long    asic_ny = cheetahGlobal.detector[detID].asic_ny;
            		long    nasics_x = cheetahGlobal.detector[detID].nasics_x;
            		long    nasics_y = cheetahGlobal.detector[detID].nasics_y;
            		//uint16_t    *quad_data[4];
            
            		// Allocate memory for detector data and set to zero
            		for(int quadrant=0; quadrant<4; quadrant++) {
                		quad_data[quadrant] = (uint16_t*) calloc(pix_nn, sizeof(uint16_t));
                		//memset(quad_data[quadrant], 0, pix_nn*sizeof(uint16_t));
            		}

			// loop over elements (quadrants)
			shared_ptr<Psana::CsPad::DataV2> data2 = evt.get(m_src, m_key);
			int nQuads = data2->quads_shape()[0];
			
			//const Pds::CsPad::ElementHeader* element;
			//while(( element=iter.next() )) {
			for (int q = 0; q < nQuads; ++ q) {
				const Psana::CsPad::ElementV2& el = data2->quads(q); 
				const ndarray<int16_t, 3>& data = el.data();
				//if(element->quad() < 4) {
				if(el.quad() < 4){
					// Which quadrant is this?
					//int quadrant = element->quad();
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
        
			
			 //	Assemble data from all four quadrants into one large array (rawdata layout)
             		 //      Memcpy is necessary for thread safety.
			 
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
    	//}	

	
	 //	Copy TOF (aqiris) channel into Cheetah event for processing
     	 //  SLAC libraries are not thread safe: must copy data into event structure for processing
	eventData->TOFPresent = 0;
	//eventData->TOFPresent = cheetahGlobal.TOFPresent ;	
	if (cheetahGlobal.TOFPresent==1){
		cout << "cheetahGlobal.TOFPresent" << endl;
		double *tempTOFTime;
		double *tempTOFVoltage;
		double tempTrigTime = 0;
		//Memcpy is necessary for thread safety.
		eventData->TOFtrigtime = tempTrigTime;
		eventData->TOFTime = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
		eventData->TOFVoltage = (double*) malloc(cheetahGlobal.AcqNumSamples*sizeof(double));
		memcpy(eventData->TOFTime, tempTOFTime, cheetahGlobal.AcqNumSamples*sizeof(double));
		memcpy(eventData->TOFVoltage, tempTOFVoltage, cheetahGlobal.AcqNumSamples*sizeof(double));
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
	 //
//	int pulnixWidth, pulnixHeight;
//	unsigned short	*pulnixImage;
	//DetInfo pulnixInfo(0,DetInfo::CxiSc1, 0, DetInfo::TM6740, 0);
	//eventData->pulnixFail = getTm6740Value(pulnixInfo, pulnixWidth, pulnixHeight, pulnixImage);
	//if ( eventData->pulnixFail == 0 )
	//{
		//Memcpy is necessary for thread safety.
//        eventData->pulnixWidth = pulnixWidth;
//        eventData->pulnixHeight = pulnixHeight;
//        eventData->pulnixImage = (unsigned short*) calloc((long)pulnixWidth*(long)pulnixHeight, sizeof(unsigned short));
//        memcpy(eventData->pulnixImage, pulnixImage, (long)pulnixWidth*(long)pulnixHeight*sizeof(unsigned short));
	//}
	eventData->pulnixFail = 1;

	// Update detector positions
	for(long detID=0; detID<cheetahGlobal.nDetectors; detID++) {        
		//eventData->detector[detID].detectorZ;
	       	eventData->detector[detID].detectorZ = detectorPosition[detID];
	}

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
}

/// Method which is called once at the end of the job
void 
cheetah_ana_mod::endJob(Event& evt, Env& env)
{
	/*
         *      Clean up all variables associated with libCheetah
         */
    	cheetahExit(&cheetahGlobal);
}

} // namespace cheetah_ana_pkg
