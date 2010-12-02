#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/ipimb/ConfigV1.hh"
#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/camera/FrameFexConfigV1.hh"
#include "pdsdata/camera/TwoDGaussianV1.hh"
#include "pdsdata/evr/IOConfigV1.hh"
#include "pdsdata/evr/ConfigV1.hh"
#include "pdsdata/evr/ConfigV2.hh"
#include "pdsdata/evr/ConfigV3.hh"
#include "pdsdata/evr/ConfigV4.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/opal1k/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/encoder/ConfigV1.hh"
#include "pdsdata/encoder/DataV1.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"
#include "pdsdata/control/PVMonitor.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/epics/EpicsXtcSettings.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/FrameV1.hh"

#include "XtcMonitorClient.hh"

static PNCCD::ConfigV1 cfg;

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth) : XtcIterator(xtc), _depth(depth) {}

  void process(const DetInfo& d, const Camera::FrameV1& f) {
    printf("*** Processing frame object\n");
  }
  void process(const DetInfo&, const Acqiris::DataDescV1&) {
    printf("*** Processing acqiris data object\n");
  }
  void process(const DetInfo&, const Acqiris::ConfigV1&) {
    printf("*** Processing Acqiris config object\n");
  }
  void process(const DetInfo&, const Ipimb::DataV1&) {
    printf("*** Processing ipimb data object\n");
  }
  void process(const DetInfo&, const Ipimb::ConfigV1&) {
    printf("*** Processing Ipimb config object\n");
  }
  void process(const DetInfo&, const Encoder::DataV1&) {
    printf("*** Processing Encoder data object\n");
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
  void process(const DetInfo&, const Camera::TwoDGaussianV1& o) {
    printf("*** Processing 2DGauss object\n");
  }
  void process(const DetInfo&, const PNCCD::ConfigV1& config) {
    cfg = config;
    printf("*** Processing pnCCD config.  Number of Links: %d, PayloadSize per Link: %d\n",
           cfg.numLinks(),cfg.payloadSizePerLink());
  }
//   void process(const DetInfo& di, PnccdFrameHeaderType* frh) {
//     enum {numberOfLinks=4, payloadPerLink=(1<<19)+16};
//     uint8_t* pb = reinterpret_cast<uint8_t*>(frh);
//     PnccdFrameHeaderType* fp;
//     for (uint32_t i=0; i<numberOfLinks; i++) {
//       fp = reinterpret_cast<PnccdFrameHeaderType*>(pb);
//       printf("\tpnCCD frame: %08X %08X %08X %08X\n", fp->specialWord, fp->frameNumber,
//           fp->TimeStampHi, fp->TimeStampLo);
//       unsigned* pu = (unsigned*)(fp+1);
//       printf("\tdata begins: %08X %08X %08X %08X %08X\n", pu[0], pu[1], pu[2], pu[3], pu[4]);
//       pb += payloadPerLink;
//     }
//   }
  void process(const DetInfo& d, const PNCCD::FrameV1& f) {
    for (unsigned i=0;i<cfg.numLinks();i++) {
      printf("*** Processing pnCCD frame number %x segment %d\n",f.frameNumber(),i);
      printf("  pnCCD frameHeader: %08X, %u, %u, %u\n", f.specialWord(), f.frameNumber(),
          f.timeStampHi(), f.timeStampLo());
      const uint16_t* data = f.data();
      unsigned last  = f.sizeofData(cfg); 
      printf("First data words: 0x%4.4x 0x%4.4x\n",data[0],data[1]);
      printf("Last  data words: 0x%4.4x 0x%4.4x\n",data[last-2],data[last-1]);
    }
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
  void process(const DetInfo&, const EvrData::ConfigV1&) {
    printf("*** Processing EVR config V1 object\n");
  }
  void process(const DetInfo&, const EvrData::IOConfigV1&) {
    printf("*** Processing EVR IOconfig V1 object\n");
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
  void process(const DetInfo&, const EvrData::DataV3& data) {
    printf("*** Processing Evr Data object\n");
    
    printf( "# of Fifo Events: %u\n", data.numFifoEvents() );
    
    for ( unsigned int iEventIndex=0; iEventIndex< data.numFifoEvents(); iEventIndex++ )
    {
      const EvrData::DataV3::FIFOEvent& event = data.fifoEvent(iEventIndex);
      printf( "[%02u] Event Code %u  TimeStampHigh 0x%x  TimeStampLow 0x%x\n",
        iEventIndex, event.EventCode, event.TimestampHigh, event.TimestampLow );
    }
    
    printf( "\n" );    
  }  
  void process(const DetInfo&, const Princeton::ConfigV1&) {
    printf("*** Processing Princeton ConfigV1 object\n");
  }
  void process(const DetInfo&, const Princeton::FrameV1&) {
    printf("*** Processing Princeton FrameV1 object\n");
  }
  int process(Xtc* xtc) {
    unsigned i=_depth; while (i--) printf("  ");
    Level::Type level = xtc->src.level();
    printf("%s level contains: %s: ",Level::name(level), TypeId::name(xtc->contains.id()));
    const DetInfo& info = *(DetInfo*)(&xtc->src);
    if (level==Level::Source) {
      printf("%s %d %s %d",
             DetInfo::name(info.detector()),info.detId(),
             DetInfo::name(info.device()),info.devId());
    } else {
      ProcInfo& info = *(ProcInfo*)(&xtc->src);
      printf("IpAddress 0x%x ProcessId 0x%x",info.ipAddr(),info.processId());
    }
    if (xtc->damage.value()) {
      printf(", damage 0x%x", xtc->damage.value());
    }
    printf("\n");
    switch (xtc->contains.id()) {
    case (TypeId::Id_Xtc) : {
      myLevelIter iter(xtc,_depth+1);
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
      }
      break;
    case (TypeId::Id_IpimbData) :
      process(info, *(const Ipimb::DataV1*)(xtc->payload()));
      break;
    case (TypeId::Id_IpimbConfig) :
      {
        unsigned version = xtc->contains.version();
        switch (version) {
        case 1:
          process(info,*(const Ipimb::ConfigV1*)(xtc->payload()));
          break;
        default:
          printf("Unsupported ipimb configuration version %d\n",version);
          break;
        }
      }
      break;
    case (TypeId::Id_EncoderData) :
      process(info, *(const Encoder::DataV1*)(xtc->payload()));
      break;
    case (TypeId::Id_EncoderConfig) :
      {
        unsigned version = xtc->contains.version();
        switch (version) {
        case 1:
          process(info,*(const Encoder::ConfigV1*)(xtc->payload()));
          break;
        default:
          printf("Unsupported Encoder configuration version %d\n",version);
          break;
        }
      }
      break;
    case (TypeId::Id_TwoDGaussian) :
      process(info, *(const Camera::TwoDGaussianV1*)(xtc->payload()));
      break;
    case (TypeId::Id_Opal1kConfig) :
      process(info, *(const Opal1k::ConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_FrameFexConfig) :
      process(info, *(const Camera::FrameFexConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_pnCCDframe) :
      process(info, *(const PNCCD::FrameV1*)(xtc->payload()));
      break;
    case (TypeId::Id_pnCCDconfig) :
      process(info, *(const PNCCD::ConfigV1*)(xtc->payload()));
      break;
    case (TypeId::Id_EvrIOConfig) :
      process(info, *(const EvrData::IOConfigV1*)(xtc->payload()));
      break;
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
    }    
    case (TypeId::Id_PhaseCavity) :
    {
      process(info, *(const BldDataPhaseCavity*) xtc->payload() );
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
//     case (TypeId::Id_pnCCDconfig) :
//       process(info, (fileHeaderType*) xtc->payload());
//       break;
//     case (TypeId::Id_pnCCDframe) :
//       process(info, (PnccdFrameHeaderType*) xtc->payload());
//       break;
    default :
      break;
    }
    return Continue;
  }
private:
  unsigned _depth;
};

class MyXtcMonitorClient : public XtcMonitorClient {
  public:
    MyXtcMonitorClient() {
    }
    virtual int processDgram(Dgram* dg) {
      printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x, damage 0x%x\n",TransitionId::name(dg->seq.service()),
       dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload(), dg->xtc.damage.value());
      myLevelIter iter(&(dg->xtc),0);
      iter.iterate();
      return 0;
    };
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s [-t <partitionTag>] [-h]\n", progname);
}

int main(int argc, char* argv[]) {
  int c=0;
  unsigned client=0;
  bool serialized=false;
  const char* partitionTag = 0;
  XtcMonitorClient myClient;
  char* endPtr;

  while ((c = getopt(argc, argv, "?ht:c:s")) != -1) {
    switch (c) {
    case '?':
    case 'h':
      usage(argv[0]);
      exit(0);
    case 't':
      partitionTag = optarg;
      break;
    case 'c':
      client = strtoul(optarg,&endPtr,0);
      break;
    case 's':
      serialized = true;
      break;
    default:
      usage(argv[0]);
    }
  }
  if (partitionTag==0)
    usage(argv[0]);
  else
    fprintf(stderr, "myClient returned: %d\n", 
      myClient.run(partitionTag,client,serialized ? client : 0));

  return 1;
}
