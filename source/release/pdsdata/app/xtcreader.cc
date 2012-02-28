#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/acqiris/TdcConfigV1.hh"
#include "pdsdata/acqiris/TdcDataV1.hh"
#include "pdsdata/ipimb/ConfigV1.hh"
#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/ipimb/ConfigV2.hh"
#include "pdsdata/ipimb/DataV2.hh"
#include "pdsdata/encoder/ConfigV1.hh"
#include "pdsdata/encoder/DataV1.hh"
#include "pdsdata/encoder/DataV2.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/fccd/FccdConfigV1.hh"
#include "pdsdata/fccd/FccdConfigV2.hh"
#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/opal1k/ConfigV1.hh"
#include "pdsdata/pulnix/TM6740ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV2.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/evr/IOConfigV1.hh"
#include "pdsdata/evr/ConfigV1.hh"
#include "pdsdata/evr/ConfigV2.hh"
#include "pdsdata/evr/ConfigV3.hh"
#include "pdsdata/evr/ConfigV4.hh"
#include "pdsdata/evr/ConfigV5.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"
#include "pdsdata/control/PVMonitor.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/epics/EpicsXtcSettings.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/FrameV1.hh"
#include "pdsdata/princeton/InfoV1.hh"
#include "pdsdata/cspad/MiniElementV1.hh"
#include "pdsdata/cspad/ElementV1.hh"
#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/lusi/IpmFexConfigV1.hh"
#include "pdsdata/lusi/IpmFexConfigV2.hh"
#include "pdsdata/lusi/IpmFexV1.hh"
#include "pdsdata/lusi/DiodeFexConfigV1.hh"
#include "pdsdata/lusi/DiodeFexConfigV2.hh"
#include "pdsdata/lusi/DiodeFexV1.hh"
#include "pdsdata/lusi/PimImageConfigV1.hh"
#include "pdsdata/pulnix/TM6740ConfigV1.hh"
#include "pdsdata/pulnix/TM6740ConfigV2.hh"

static unsigned eventCount = 0;

using namespace Pds;

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth, long long int lliOffset) : XtcIterator(xtc), _depth(depth), _lliOffset(lliOffset) {}

  void process(const DetInfo& d, const Camera::FrameV1& f) {
    printf("*** Processing frame object\n");
  }
  void process(const DetInfo&, const Acqiris::DataDescV1&) {
    printf("*** Processing acqiris data object\n");
  }
  void process(const DetInfo&, const Acqiris::ConfigV1&) {
    printf("*** Processing Acqiris config object\n");
  }
  void process(const DetInfo& i, const Acqiris::TdcDataV1& d) {
    printf("*** Processing acqiris TDC data object for %s\n",
     DetInfo::name(i));
    const Acqiris::TdcDataV1* p = &d;
    //  Data is terminated with an AuxIOMarker (Memory bank switch)
    while(!(p->source() == Acqiris::TdcDataV1::AuxIO &&
      static_cast<const Acqiris::TdcDataV1::Marker*>(p)->type() < 
      Acqiris::TdcDataV1::Marker::AuxIOMarker)) {
      switch(p->source()) {
      case Acqiris::TdcDataV1::Comm:
  printf("common start %d\n",
         static_cast<const Acqiris::TdcDataV1::Common*>(p)->nhits());
  break;
      case Acqiris::TdcDataV1::AuxIO:
  break;
      default:
  { 
    const Acqiris::TdcDataV1::Channel& c = 
      *static_cast<const Acqiris::TdcDataV1::Channel*>(p);
    if (!c.overflow())
      printf("ch %d : 0x%x ticks, %g ns\n",
       p->source(), c.ticks(), c.ticks()*50e-12);
    break;
  }
      }
      p++;
    }
  }
  void process(const DetInfo& i, const Acqiris::TdcConfigV1& c) {
    printf("*** Processing Acqiris TDC config object for %s\n",
     DetInfo::name(i));
    for(unsigned j=0; j<Acqiris::TdcConfigV1::NChannels; j++) {
      const Acqiris::TdcChannel& ch = c.channel(j);
      printf("chan %d : %s, slope %c, level %gv\n",
             ch.channel(),
             ch.mode ()==Acqiris::TdcChannel::Inactive?"inactive":"active",
             ch.slope()==Acqiris::TdcChannel::Positive?'+':'-',
             ch.level());
    }
  }
  void process(const DetInfo&, const Ipimb::DataV1&) {
    printf("*** Processing ipimb data object\n");
  }
  void process(const DetInfo&, const Ipimb::ConfigV1&) {
    printf("*** Processing Ipimb config object\n");
  }
  void process(const DetInfo&, const Ipimb::DataV2&) {
    printf("*** Processing ipimb data object\n");
  }
  void process(const DetInfo&, const Ipimb::ConfigV2&) {
    printf("*** Processing Ipimb config object\n");
  }
  void process(const DetInfo&, const Encoder::DataV1&) {
    printf("*** Processing encoder DataV1 object\n");
  }
  void process(const DetInfo&, const Encoder::DataV2&) {
    printf("*** Processing encoder DataV2 object\n");
  }
  void process(const DetInfo&, const Encoder::ConfigV1&) {
    printf("*** Processing Encoder config object\n");
  }
  void process(const DetInfo&, const Opal1k::ConfigV1&) {
    printf("*** Processing Opal1000 config object\n");
  }
  void process(const DetInfo&, const Camera::FrameFexConfigV1&) {
    printf("*** Processing frame feature extraction config object\n");
  }
  void process(const DetInfo&, const FCCD::FccdConfigV1&) {
    printf("*** Processing FCCD ConfigV1 object\n");
  }
  void process(const DetInfo&, const FCCD::FccdConfigV2&) {
    printf("*** Processing FCCD ConfigV2 object\n");
  }
  void process(const DetInfo&, const Camera::TwoDGaussianV1& o) {
    printf("*** Processing 2DGauss object\n");
  }
  void process(const DetInfo& det, const PNCCD::ConfigV1& config) {
    if ( det.detId() != 0 )
    {
      printf( "myLevelIter::process(...,PNCCD::ConfigV1&): pnCCD detector Id (%d) is not 0\n", det.detId() );
      return;
    }
    if ( det.devId() < 0 || det.devId() > 1)
    {
      printf( "myLevelIter::process(...,PNCCD::ConfigV1&): pnCCD device Id (%d) is out of range (0..1)\n", det.devId() );
      return;
    }
    
    _pnCcdCfgListV1[det.devId()] = config;
    printf("*** Processing pnCCD config.  Number of Links: %d, PayloadSize per Link: %d\n",
           config.numLinks(),config.payloadSizePerLink());
  }  
  void process(const DetInfo& det, const PNCCD::ConfigV2& config) {
    if ( det.detId() != 0 )
    {
      printf( "myLevelIter::process(...,PNCCD::ConfigV2&): pnCCD detector Id (%d) is not 0\n", det.detId() );
      return;
    }
    if ( det.devId() < 0 || det.devId() > 1)
    {
      printf( "myLevelIter::process(...,PNCCD::ConfigV2&): pnCCD device Id (%d) is out of range (0..1)\n", det.devId() );
      return;
    }

    _pnCcdCfgListV2[det.devId()] = config;
    printf("*** Processing pnCCD config.  Number of Links: %u, PayloadSize per Link: %u\n",
           config.numLinks(),config.payloadSizePerLink());
    printf("\tNumber of Channels %u, Number of Rows %u, Number of SubModule Channels %u\n\tNumber of SubModule Rows %u, Number of SubModules, %u\n",
        config.numChannels(),config.numRows(), config.numSubmoduleChannels(),config.numSubmoduleRows(),config.numSubmodules());
    printf("\tCamex Magic 0x%x, info %s, Timing File Name %s\n", config.camexMagic(),config.info(),config.timingFName());
  }
  void process(const DetInfo& det, const PNCCD::FrameV1* f) {
    if ( det.detId() != 0 )
    {
      printf( "myLevelIter::process(...,PNCCD::FrameV1*): pnCCD detector Id (%d) is not 0\n", det.detId() );
      return;
    }
    if ( det.devId() < 0 || det.devId() > 1)
    {
      printf( "myLevelIter::process(...,PNCCD::FrameV1*): pnCCD device Id (%d) is out of range (0..1)\n", det.devId() );
      return;
    }
    
    printf("*** Processing pnCCD Frame\n");
  }  
  void process(const DetInfo&, const ControlData::ConfigV1& config) {
    printf("*** Processing Control config object\n");    
    
    printf( "Control PV Number = %d, Monitor PV Number = %d\n", config.npvControls(), config.npvMonitors() );
    for(unsigned int iPvControl=0; iPvControl < config.npvControls(); iPvControl++) {      
      const Pds::ControlData::PVControl& pvControlCur = config.pvControl(iPvControl);
      if (pvControlCur.array())
        printf( "%s[%d] = ", pvControlCur.name(), pvControlCur.index() );
      else
        printf( "%s = ", pvControlCur.name() );
      printf( "%lf\n", pvControlCur.value() );
    }
    
    for(unsigned int iPvMonitor=0; iPvMonitor < config.npvMonitors(); iPvMonitor++) {      
      const Pds::ControlData::PVMonitor& pvMonitorCur = config.pvMonitor(iPvMonitor);
      if (pvMonitorCur.array())
        printf( "%s[%d]  ", pvMonitorCur.name(), pvMonitorCur.index() );
      else
        printf( "%s  ", pvMonitorCur.name() );
      printf( "Low %lf  High %lf\n", pvMonitorCur.loValue(), pvMonitorCur.hiValue() );
    }
          
  }  
  void process(const DetInfo&, const EpicsPvHeader& epicsPv)
  {    
    printf("*** Processing Epics object\n");
    epicsPv.printPv();
    printf( "\n" );
  }
  void process(const DetInfo&, const BldDataFEEGasDetEnergy& bldData) {
    printf("*** Processing FEEGasDetEnergy object\n");
    bldData.print();
    printf( "\n" );    
  }  
  void process(const DetInfo&, const BldDataEBeamV0& bldData) {
    printf("*** Processing EBeamV0 object\n");
    bldData.print();
    printf( "\n" );    
  }  
  void process(const DetInfo&, const BldDataEBeam& bldData) {
    printf("*** Processing EBeam object\n");
    bldData.print();
    printf( "\n" );    
  }  
  void process(const DetInfo&, const BldDataPhaseCavity& bldData) {
    printf("*** Processing PhaseCavity object\n");
    bldData.print();
    printf( "\n" );    
  } 
  void process(const DetInfo&, const BldDataIpimbV0& bldData) {
    printf("*** Processing Bld-Ipimb V0 object\n");
    bldData.print();
    printf( "\n" );    
  } 

  void process(const DetInfo&, const BldDataIpimb& bldData) {
    printf("*** Processing Bld-Ipimb V1 object\n");
    bldData.print();
    printf( "\n" );    
  } 
  
  void process(const DetInfo&, const EvrData::IOConfigV1&) {
    printf("*** Processing EVR IOconfig V1 object\n");
  }
  void process(const DetInfo&, const EvrData::ConfigV1&) {
    printf("*** Processing EVR config V1 object\n");
  }
  void process(const DetInfo&, const EvrData::ConfigV2&) {
    printf("*** Processing EVR config V2 object\n");
  }
  void process(const DetInfo&, const EvrData::ConfigV3&) {
    printf("*** Processing EVR config V3 object\n");
  }
  void process(const DetInfo&, const EvrData::ConfigV4&) {
    printf("*** Processing EVR config V4 object\n");
  }
  void process(const DetInfo&, const EvrData::ConfigV5&) {
    printf("*** Processing EVR config V5 object\n");
  }
  void process(const DetInfo&, const EvrData::DataV3& data) {
    printf("*** Processing Evr Data object\n");
    eventCount++;    

    printf( "# of Fifo Events: %u\n", data.numFifoEvents() );
    for ( unsigned int iEventIndex=0; iEventIndex< data.numFifoEvents(); iEventIndex++ ) {
      const EvrData::DataV3::FIFOEvent& event = data.fifoEvent(iEventIndex);
      printf( "[%02u] Event Code %u  TimeStampHigh 0x%x  TimeStampLow 0x%x\n",
        iEventIndex, event.EventCode, event.TimestampHigh, event.TimestampLow );
      if (event.EventCode == 162)
        printf ("Blank shot eventcode 162 found at eventNo: %u \n",eventCount); 
    }    
    printf( "\n" );    
  }  
  void process(const DetInfo&, const Princeton::ConfigV1&) {
    printf("*** Processing Princeton ConfigV1 object\n");
  }
  void process(const DetInfo&, const Princeton::FrameV1&) {
    printf("*** Processing Princeton FrameV1 object\n");
  }
  void process(const DetInfo&, const Princeton::InfoV1&) {
    printf("*** Processing Princeton InfoV1 object\n");
  }
  void process(const DetInfo&, const CsPad::MiniElementV1&) {
    printf("*** Processing CsPad MiniElementV1 object\n");
  }
  void process(const DetInfo&, const CsPad::ElementV1&) {
    printf("*** Processing CsPad ElementV1 object\n");
  }
  void process(const DetInfo&, const CsPad::ConfigV1&) {
    printf("*** Processing CsPad ElementV1 object\n");
  }
  void process(const DetInfo&, const Lusi::IpmFexConfigV1&) {
    printf("*** Processing LUSI IpmFexConfigV1 object\n");
  }
  void process(const DetInfo&, const Lusi::IpmFexConfigV2&) {
    printf("*** Processing LUSI IpmFexConfigV2 object\n");
  }
  void process(const DetInfo&, const Lusi::IpmFexV1&) {
    printf("*** Processing LUSI IpmFexV1 object\n");
  }
  void process(const DetInfo&, const Lusi::DiodeFexConfigV1&) {
    printf("*** Processing LUSI DiodeFexConfigV1 object\n");
  }
  void process(const DetInfo&, const Lusi::DiodeFexConfigV2&) {
    printf("*** Processing LUSI DiodeFexConfigV2 object\n");
  }
  void process(const DetInfo&, const Lusi::DiodeFexV1&) {
    printf("*** Processing LUSI DiodeFexV1 object\n");
  }
  void process(const DetInfo &, const Lusi::PimImageConfigV1 &)
  {
    printf("*** Processing LUSI PimImageConfigV1 object\n");
  }  
  void process(const DetInfo &, const Pulnix::TM6740ConfigV1 &)
  {
    printf("*** Processing Pulnix TM6740ConfigV1 object\n");
  }
  void process(const DetInfo &, const Pulnix::TM6740ConfigV2 &)
  {
    printf("*** Processing Pulnix::TM6740ConfigV2 object\n");
  }  
  int process(Xtc* xtc) {
    unsigned      i         =_depth; while (i--) printf("  ");
    Level::Type   level     = xtc->src.level();
    printf("%s level  offset %Ld (0x%Lx), payload size %d contains: %s: ",
      Level::name(level), _lliOffset, _lliOffset, xtc->sizeofPayload(), TypeId::name(xtc->contains.id()));
    long long lliOffsetPayload = _lliOffset + sizeof(Xtc);
    _lliOffset += sizeof(Xtc) + xtc->sizeofPayload();
     
    const DetInfo& info = *(DetInfo*)(&xtc->src);
    if (level==Level::Source) {
      printf("%s,%d  %s,%d\n",
             DetInfo::name(info.detector()),info.detId(),
             DetInfo::name(info.device()),info.devId());
    } else {
      ProcInfo& info = *(ProcInfo*)(&xtc->src);
      printf("IpAddress 0x%x ProcessId 0x%x\n",info.ipAddr(),info.processId());
    }
    if (level < 0 || level >= Level::NumberOfLevels )
    {
        printf("Unsupported Level %d\n", (int) level);
        return Continue;
    }    
    switch (xtc->contains.id()) {
    case (TypeId::Id_Xtc) : {
      myLevelIter iter(xtc,_depth+1, lliOffsetPayload);
      iter.iterate();
      break;
    }
    case (TypeId::Id_Frame) :
      process(info, *(const Camera::FrameV1*)(xtc->payload()));
      break;
    case (TypeId::Id_AcqWaveform) :
      process(info, *(const Acqiris::DataDescV1*)(xtc->payload()));
      break;
    case (TypeId::Id_AcqConfig) :
    {      
      unsigned version = xtc->contains.version();
      switch (version) {
      case 1:
        process(info,*(const Acqiris::ConfigV1*)(xtc->payload()));
        break;
      default:
        printf("Unsupported acqiris configuration version %d\n",version);
        break;
      }
      break;      
    }
    case (TypeId::Id_AcqTdcConfig) :
      process(info, *(const Acqiris::TdcConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_AcqTdcData) :
      process(info, *(const Acqiris::TdcDataV1*)(xtc->payload()));
      break;
    case (TypeId::Id_IpimbData) :
      {
	unsigned version = xtc->contains.version();
	switch (version) {
	case 1:
	  process(info, *(const Ipimb::DataV1*)(xtc->payload()));
	  break;
	case 2:
	  process(info, *(const Ipimb::DataV2*)(xtc->payload()));
	  break;
	default:
	  printf("Unsupported ipimb configuration version %d\n",version);
	  break;
	}
      }
      break;
    case (TypeId::Id_IpimbConfig) :
    {      
      unsigned version = xtc->contains.version();
      switch (version) {
      case 1:
        process(info,*(const Ipimb::ConfigV1*)(xtc->payload()));
        break;
      case 2:
        process(info,*(const Ipimb::ConfigV2*)(xtc->payload()));
        break;
      default:
        printf("Unsupported ipimb configuration version %d\n",version);
        break;
      }
      break;      
    }
    case (TypeId::Id_EncoderData) :
    {      
      unsigned version = xtc->contains.version();
      switch (version) {
      case 1:
        process(info,*(const Encoder::DataV1*)(xtc->payload()));
        break;
      case 2:
        process(info,*(const Encoder::DataV2*)(xtc->payload()));
        break;
      default:
        printf("Unsupported encoder data version %d\n",version);
        break;
      }
      break;      
    }
    case (TypeId::Id_EncoderConfig) :
    {      
      unsigned version = xtc->contains.version();
      switch (version) {
      case 1:
        process(info,*(const Encoder::ConfigV1*)(xtc->payload()));
        break;
      default:
        printf("Unsupported encoder configuration version %d\n",version);
        break;
      }
      break;      
    }
    case (TypeId::Id_TwoDGaussian) :
      process(info, *(const Camera::TwoDGaussianV1*)(xtc->payload()));
      break;
    case (TypeId::Id_Opal1kConfig) :
      process(info, *(const Opal1k::ConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_FrameFexConfig) :
      process(info, *(const Camera::FrameFexConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_pnCCDconfig) :
      {
      unsigned version = xtc->contains.version();
      switch (version) {
        case 1:
          process(info, *(const PNCCD::ConfigV1*)(xtc->payload()));
          break;
        case 2:
          process(info, *(const PNCCD::ConfigV2*)(xtc->payload()));
          break;
        default:
          printf("Unsupported pnCCD configuration version %d\n",version);
      }
      break;
      }
    case (TypeId::Id_pnCCDframe) :
      {
      process(info, (const PNCCD::FrameV1*)(xtc->payload()));
      break;
      }
    case (TypeId::Id_EvrIOConfig) :
      {      
      process(info, *(const EvrData::IOConfigV1*)(xtc->payload()));
      break;
      }
    case (TypeId::Id_EvrConfig) :
    {      
      unsigned version = xtc->contains.version();
      switch (version) {
      case 1:
        process(info, *(const EvrData::ConfigV1*)(xtc->payload()));
        break;
      case 2:
        process(info, *(const EvrData::ConfigV2*)(xtc->payload()));
        break;
      case 3:
        process(info, *(const EvrData::ConfigV3*)(xtc->payload()));
        break;
      case 4:
        process(info, *(const EvrData::ConfigV4*)(xtc->payload()));
        break;
      case 5:
        process(info, *(const EvrData::ConfigV5*)(xtc->payload()));
        break;
      default:
        printf("Unsupported evr configuration version %d\n",version);
        break;
      }
      break;      
    }
    case (TypeId::Id_EvrData) :
    {
      process(info, *(const EvrData::DataV3*) xtc->payload() );
      break;        
    }
    case (TypeId::Id_ControlConfig) :
      process(info, *(const ControlData::ConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_Epics) :      
    {
      int iVersion = xtc->contains.version();
      if ( iVersion != EpicsXtcSettings::iXtcVersion ) 
      {
          printf( "Xtc Epics version (%d) is not compatible with reader supported version (%d)", iVersion, EpicsXtcSettings::iXtcVersion );
          break;
      }
      process(info, *(const EpicsPvHeader*)(xtc->payload()));
      break;
    }
    /*
     * BLD data
     */
    case (TypeId::Id_FEEGasDetEnergy) :
    {
      process(info, *(const BldDataFEEGasDetEnergy*) xtc->payload() );
      break;        
    }
    case (TypeId::Id_EBeam) :
    {
      switch(xtc->contains.version()) {
      case 0:
        process(info, *(const BldDataEBeamV0*) xtc->payload() );
        break; 
      case 1:
        process(info, *(const BldDataEBeam*) xtc->payload() );
        break; 
      default:
        break;
      }       
      break;
    }    
    case (TypeId::Id_PhaseCavity) :
    {
      process(info, *(const BldDataPhaseCavity*) xtc->payload() );
      break;        
    }
    case (TypeId::Id_SharedIpimb) :
    {
     switch(xtc->contains.version()) {
      case 0:
        process(info, *(const BldDataIpimbV0*) xtc->payload() );
        break; 
      case 1:
        process(info, *(const BldDataIpimb*) xtc->payload() );
        break; 
      default:
        break;
      }       
      break;       
    } 
    case (TypeId::Id_PrincetonConfig) :
    {
      process(info, *(const Princeton::ConfigV1*)(xtc->payload()));
      break;
    }
    case (TypeId::Id_PrincetonFrame) :
    {
      process(info, *(const Princeton::FrameV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_PrincetonInfo) :
    {
      process(info, *(const Princeton::InfoV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_Cspad2x2Element) :
    {
      process(info, *(const CsPad::MiniElementV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_CspadElement) :
    {
      process(info, *(const CsPad::ElementV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_CspadConfig) :
    {
      process(info, *(const CsPad::ConfigV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_IpmFexConfig) :
    {
      switch(xtc->contains.version()) {
      case 1:
        process(info, *(const Lusi::IpmFexConfigV1*)(xtc->payload()));
        break;
      case 2:
        process(info, *(const Lusi::IpmFexConfigV2*)(xtc->payload()));
        break;
      default:
        printf("Unsupported IpmFexConfig version %d\n",xtc->contains.version());
        break;
      }
    }    
    case (TypeId::Id_IpmFex) :
    {
      process(info, *(const Lusi::IpmFexV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_DiodeFexConfig) :
    {
      switch(xtc->contains.version()) {
      case 1:
        process(info, *(const Lusi::DiodeFexConfigV1*)(xtc->payload()));
        break;
      case 2:
        process(info, *(const Lusi::DiodeFexConfigV2*)(xtc->payload()));
        break;
      default:
        printf("Unsupported DiodeFexConfig version %d\n",xtc->contains.version());
        break;
      }
    }    
    case (TypeId::Id_DiodeFex) :
    {
      process(info, *(const Lusi::DiodeFexV1*)(xtc->payload()));
      break;
    }    
    case (TypeId::Id_TM6740Config):
    {
      switch (xtc->contains.version())
      {
      case 1:
        process(info, *(const Pulnix::TM6740ConfigV1 *) xtc->payload());
        break;
      case 2:
        process(info, *(const Pulnix::TM6740ConfigV2 *) xtc->payload());
        break;
      default:
        printf("Unsupported TM6740Config version %d\n", xtc->contains.version());            
        break;
      }        
      break;
    }
    case (TypeId::Id_PimImageConfig):
    {
      process(info, *(const Lusi::PimImageConfigV1 *) (xtc->payload()));
      break;
    }          
    default :
      printf("Unsupported TypeId %s (value = %d)\n", TypeId::name(xtc->contains.id()), (int) xtc->contains.id());
      break;
    }
    return Continue;
  }
private:
  unsigned       _depth;
  long long int  _lliOffset;

  /* static private data */
  static PNCCD::ConfigV1 _pnCcdCfgListV1[2];
  static PNCCD::ConfigV2 _pnCcdCfgListV2[2];
};

PNCCD::ConfigV1 myLevelIter::_pnCcdCfgListV1[2] = { PNCCD::ConfigV1(), PNCCD::ConfigV1() };
PNCCD::ConfigV2 myLevelIter::_pnCcdCfgListV2[2] = { PNCCD::ConfigV2(), PNCCD::ConfigV2() };

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  int parseErr = 0;

  while ((c = getopt(argc, argv, "hf:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      xtcname = optarg;
      break;
    default:
      parseErr++;
    }
  }
  
  if (!xtcname) {
    usage(argv[0]);
    exit(2);
  }

  int fd = open(xtcname, O_RDONLY | O_LARGEFILE);
  if (fd < 0) {
    perror("Unable to open file %s\n");
    exit(2);
  }

  XtcFileIterator iter(fd,0x2000000);
  Dgram* dg;
  long long int lliOffset = lseek64( fd, 0, SEEK_CUR );  
  while ((dg = iter.next())) {
    printf("%s transition: time 0x%x/0x%x, offset %Ld (0x%Lx), payloadSize %d\n",TransitionId::name(dg->seq.service()),
           dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(), lliOffset, lliOffset, dg->xtc.sizeofPayload());
    myLevelIter iter(&(dg->xtc),0, lliOffset + sizeof(Xtc) + sizeof(*dg) - sizeof(dg->xtc));
    iter.iterate();
    lliOffset = lseek64( fd, 0, SEEK_CUR ); // get the file offset for the next iteration
  }

  ::close(fd);
  return 0;
}
