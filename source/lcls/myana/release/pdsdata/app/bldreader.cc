
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/XtcFileIterator.hh"
#include "pdsdata/xtc/Level.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/bld/bldData.hh"

using namespace Pds;

static const double damaged = -1.e9;
class bldData {
public:
  void reset() { 
    gasdet   = 0;
    ebeam    = 0;
    ebeamV0  = 0;
    ebeamV1  = 0;
    phasecav = 0;
  }
  void dump() const {
    printf("%d\t%d\t%d\t",
	   seconds,
	   nanoseconds,
	   pulseId);
    if (gasdet)   printf("%g\t%g\t%g\t%g\t", 
			 gasdet->f_11_ENRC,
			 gasdet->f_12_ENRC,
			 gasdet->f_21_ENRC,
			 gasdet->f_22_ENRC);
    else          printf("%g\t%g\t%g\t%g\t", 
			 damaged,
			 damaged,
			 damaged,
			 damaged);
    if (ebeam)        printf("%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t",
			     ebeam->fEbeamCharge,
			     ebeam->fEbeamL3Energy,
			     ebeam->fEbeamLTUPosX,
			     ebeam->fEbeamLTUPosY,
			     ebeam->fEbeamLTUAngX,
			     ebeam->fEbeamLTUAngY,
			     ebeam->fEbeamPkCurrBC2,
			     ebeam->fEbeamEnergyBC2,
			     ebeam->fEbeamPkCurrBC1,
			     ebeam->fEbeamEnergyBC1);
    else if (ebeamV0) printf("%g\t%g\t%g\t%g\t%g\t%g\t%g\t",
			     ebeamV0->fEbeamCharge,
			     ebeamV0->fEbeamL3Energy,
			     ebeamV0->fEbeamLTUPosX,
			     ebeamV0->fEbeamLTUPosY,
			     ebeamV0->fEbeamLTUAngX,
			     ebeamV0->fEbeamLTUAngY,
			     damaged);
    else if (ebeamV1) printf("%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t",
			     ebeamV1->fEbeamCharge,
			     ebeamV1->fEbeamL3Energy,
			     ebeamV1->fEbeamLTUPosX,
			     ebeamV1->fEbeamLTUPosY,
			     ebeamV1->fEbeamLTUAngX,
			     ebeamV1->fEbeamLTUAngY,
                             ebeamV1->fEbeamPkCurrBC2,
			     damaged);
    else              printf("%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t",
			     damaged,
			     damaged,
			     damaged,
			     damaged,
			     damaged,
			     damaged,
			     damaged,
                             damaged,
                             damaged,
                             damaged);
    if (phasecav) printf("%g\t%g\t%g\t%g\n", 
			 phasecav->fFitTime1,
			 phasecav->fFitTime2,
			 phasecav->fCharge1,
			 phasecav->fCharge2);
    else          printf("%g\t%g\t%g\t%g\n", 
			 damaged,
			 damaged,
			 damaged,
			 damaged);
  }
  void header() const {
    static const char* headers[] = { "seconds",
				     "nanoseconds",
				     "pulseId",
				     "GDET:FEE:11:ENRC[mJ]",
				     "GDET:FEE:12:ENRC[mJ]",
				     "GDET:FEE:21:ENRC[mJ]",
				     "GDET:FEE:22:ENRC[mJ]",
				     "ebeamCharge[nC]",
				     "ebeamL3Energy[MeV]",
				     "ebeamLTUPosX[mm]",
				     "ebeamLTUPosY[mm]",
				     "ebeamLTUAngX[mrad]",
				     "ebeamLTUAngY[mrad]",
				     "ebeamPkCurrBC2[Amp]",
                                     "ebeamBC2Energy[mm]",
                                     "ebeamPkCurrBC1[Amp]",
                                     "ebeamBC1Energy[mm]",
				     "PhCav:FitTime1[ps]",
				     "PhCav:FitTime2[ps]",
				     "PhCav:Charge1[pC]",
				     "PhCav:Charge2[pC]",
				     NULL };
    for(const char** h = headers; *h != NULL; h++)
      printf("%s\t",*h);
    printf("\n");
  }
  unsigned                      seconds;
  unsigned                      nanoseconds;
  unsigned                      pulseId;
  const BldDataFEEGasDetEnergy* gasdet;
  const BldDataEBeamV0*         ebeamV0;
  const BldDataEBeamV1*         ebeamV1;
  const BldDataEBeam*           ebeam;
  const BldDataPhaseCavity*     phasecav;
};

static bldData bld;

class myLevelIter : public XtcIterator {
public:
  enum {Stop, Continue};
  myLevelIter(Xtc* xtc, unsigned depth) : XtcIterator(xtc), _depth(depth) {}

  int process(Xtc* xtc) {
    if (xtc->contains.id() == TypeId::Id_Xtc) {
      myLevelIter iter(xtc,_depth+1);
      iter.iterate();
    }
    else if (xtc->damage.value())
      ;
    else if (xtc->src.level() == Level::Reporter) {
      const BldInfo& info = static_cast<const BldInfo&>(xtc->src);
      switch(info.type()) {
      case BldInfo::EBeam          : 
	switch(xtc->contains.version()) {
	case 0:
	  bld.ebeamV0  = reinterpret_cast<const BldDataEBeamV0*>      (xtc->payload()); 
	  break;
	case 1:
	  bld.ebeamV1    = reinterpret_cast<const BldDataEBeamV1*>      (xtc->payload()); 
	  break;
	case 2:
	  bld.ebeam    = reinterpret_cast<const BldDataEBeam*>        (xtc->payload()); 
	  break;
	default:
	  break;
	}
	break;
      case BldInfo::PhaseCavity    : 
	bld.phasecav = reinterpret_cast<const BldDataPhaseCavity*>    (xtc->payload()); 
	break;
      case BldInfo::FEEGasDetEnergy: 
	bld.gasdet   = reinterpret_cast<const BldDataFEEGasDetEnergy*>(xtc->payload()); 
	break;
      default:
	break;
      }
    }
    return Continue;
  }
private:
  unsigned _depth;
};

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> -b <begin time> -e <end time> [-h]\n", progname);
  fprintf(stderr,"  time is expressed as YYYYMMDD_HH:MM:SS (UTC);\n");
  fprintf(stderr,"  e.g. \"20091101_14:09:00\"  = Nov 1, 2009, 2:09pm (UTC)\n");
}

bool parse_time(const char* arg, ClockTime& clk)
{
  struct tm t;
  char* r=strptime(optarg, "%Y%m%d_%H:%M:%S", &t);
  if (*r) {
    printf("Error parsing time %s\n",arg);
    return false;
  }
  time_t tt = mktime(&t);
  clk = ClockTime(tt,0);
  return true;
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  int parseErr = 0;
  ClockTime begin(0,0);
  ClockTime end(-1,-1);

  while ((c = getopt(argc, argv, "hf:b:e:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      xtcname = optarg;
      break;
    case 'b':
      if (!parse_time(optarg,begin)) parseErr++;
      break;
    case 'e':
      if (!parse_time(optarg,end  )) parseErr++;
      break;
    default:
      parseErr++;
    }
  }
  
  if (!xtcname || parseErr) {
    usage(argv[0]);
    exit(2);
  }

  int fd = open(xtcname,O_RDONLY | O_LARGEFILE);
  if (fd < 0) {
    perror("Unable to open file %s\n");
    exit(2);
  }

  bld.header();
  XtcFileIterator iter(fd,0x900000);
  Dgram* dg;
  unsigned long long bytes=0;
  while ((dg = iter.next())) {

    if (dg->seq.service() != TransitionId::L1Accept) continue;

    if (!(dg->seq.clock() > begin)) continue;
    if (  dg->seq.clock() > end   ) break;

    bld.reset();
    bld.seconds     = dg->seq.clock().seconds();
    bld.nanoseconds = dg->seq.clock().nanoseconds();
    bld.pulseId     = dg->seq.stamp().fiducials();

    myLevelIter iter(&(dg->xtc),0);
    iter.iterate();
    bytes += sizeof(*dg) + dg->xtc.sizeofPayload();

    bld.dump();
  }

  close(fd);
  return 0;
}
