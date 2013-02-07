#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/TimeStamp.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "XtcRun.hh"

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

#include <list>
#include <string>

//#define MEASURE_RATE

#ifdef MEASURE_RATE
#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#endif

using namespace Pds;

static void dump(const Sequence& seq)
{
  char buff[128];
  time_t t = seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x\n",
         buff,
         seq.clock().nanoseconds(),
         seq.stamp().fiducials(),seq.stamp().vector());
}

//    static const double DFID = 1./360.;  // best guess at "360Hz" period
static const double DFID = 2.7773214e-3;  // best guess at "360Hz" period

static double dfiducial(const Sequence& n,
                        const Sequence& o)
{
  //
  //  Time is measured in EVR by 360Hz fiducial clock (17-bits) 
  //    and local clock (after interrupt latency)
  //
  double clkdt = (double(n.clock().seconds    ()) - double(o.clock().seconds    ()));
  clkdt += 1.e-9*(double(n.clock().nanoseconds()) - double(o.clock().nanoseconds()));
  
  const double LFID = DFID*double(TimeStamp::MaxFiducials);
  double fiddt = (double(n.stamp().fiducials  ()) - double(o.stamp().fiducials  ()));
  fiddt       *= DFID;
  
  // Correct integral fiducial wrap
  fiddt += floor( (clkdt-fiddt)/LFID + 0.5 )*LFID;
  return fiddt;
}

//
//  AcqHandler class
//
//    Attempts to relocate Acqiris data to the correct event by matching
//    deltas in Acqiris clock (picoseconds) and event fiducial (~360Hz).
//    Assumes the first run has a real Configure transition to assure
//    synchronicity at the start.  Looks for OutOfOrder damage to begin
//    its re-matching algorithm.  Matching test is limited by relative
//    uncertainty between Acqiris clock and event fiducial clock.
//
//    Special handling added for occurrences of events collected out-of-order;
//    i.e. with a decreasing wall clock time.  In this case, the matching is
//    reset, and the Acqiris data is assumed to be dropped for the next one 
//    event.  These occurrences are not understood.
//

class AcqHandler {
public:
  AcqHandler(const Src& src) : 
    _src(src), _dfid(0), _nsync(0),
    _dt0(0), _dt1(0), _dt2(0),
    _state(Initial)
  {
#ifdef MEASURE_RATE
    const DetInfo& info = static_cast<const DetInfo&>(src);
    _nt = new TTree(DetInfo::name(info.detector()),
                    DetInfo::name(info.detector()));
    _nt->Branch("dfid",&_rate_dfid,"dfid/D",8000);
    _nt->Branch("dts" ,&_rate_dts ,"dts/D" ,8000);
    _nt->Branch("sec" ,&_rate_sec ,"sec/I" ,8000);
    _nt->Branch("nsec",&_rate_nsec,"nsec/I",8000);
#endif
  }
public:
  const Src& src() const { return _src; }
  bool process(const Sequence& seq, const Sequence& prev) 
  {
    if (prev.clock() > seq.clock()) {
      printf("Found old event ");
      dump(seq);
    }
    return false;
  }
  bool process(Xtc& xtc, const Sequence& seq, const Sequence& prev)
  {
    if (_state==Initial) {  // test for out of order
      xtc.damage = Damage(xtc.damage.value()&~(1<<Damage::OutOfOrder));
      { Xtc* txtc = reinterpret_cast<Xtc*>(xtc.payload());
        txtc->damage = Damage(txtc->damage.value()&~(1<<Damage::OutOfOrder)); }
      _sync(xtc,seq);
      _state = Matching;
      return false;
    }

    else { // if (_state==Matching) {  // cache this data and search for a replacement

      //
      //  Don't try to match/synchronize backwards in time
      //
      if (prev.clock() > seq.clock()) {
        printf("Found old event ");  
        dump(seq);
        return false;
      }

      //
      //  Put the current data into the cache
      //
      char* b = new char[xtc.extent];
      memcpy(b, &xtc, xtc.extent);
      _cache.push_back(reinterpret_cast<Xtc*>(b));

      //
      //  Match event with acqiris data cache
      //
      bool lmatch=false;
      for(std::list<Xtc*>::iterator it=_cache.begin(); it!=_cache.end(); it++) {
        Xtc& txtc = *(*it);
        if (_match(txtc, seq)) {
          _sync(txtc, seq);
          txtc.damage = Damage(txtc.damage.value()&~(1<<Damage::OutOfOrder));
          { Xtc* ttc = reinterpret_cast<Xtc*>(txtc.payload());
            ttc->damage = Damage(ttc->damage.value()&~(1<<Damage::OutOfOrder)); }
          memcpy(&xtc, &txtc, txtc.extent);
          _cache.erase(it);
          delete[] reinterpret_cast<char*>(&txtc);
          lmatch=true;
          break;
        }
      }
      if (lmatch) {  // remove any outdated data
        _cache.remove_if(*this);
      }
      else {
        xtc.damage.increase(Damage::OutOfOrder);
        printf("%08x.%08x (%g) no data found for ",
               _src.log(),_src.phy(), dfiducial(seq,_seq)/DFID);
        dump(seq);
        
        if (_cache.size()>1) {
          printf("candidates:\n");
          for(std::list<Xtc*>::iterator it=_cache.begin(); it!=_cache.end(); it++) {
            Xtc& txtc = *(*it);
            if (&txtc != &xtc) {
              uint64_t tacqts = reinterpret_cast<Acqiris::DataDescV1*>
                (reinterpret_cast<const Xtc*>(txtc.payload())->payload())->timestamp(0).value();
              printf("\tdacqts %g\n",double(tacqts-_acqts)*1e-12/DFID);
            }
          }
        }
        
      }
      return lmatch;
    }
  }
  bool operator()(Xtc*& xtc) {
    uint64_t acqts = reinterpret_cast<Acqiris::DataDescV1*>
      (reinterpret_cast<const Xtc*>(xtc->payload())->payload())->timestamp(0).value();
    if (acqts < _acqts) {
      printf("Retiring acqts %llx\n",acqts);
      delete[] reinterpret_cast<char*>(xtc);
      return true;
    }
    return false;
  }
  unsigned nsync() const { return _nsync; }
  void     stats() const {
    printf("stats: diffs %g  mean %.8g (ms) rms %g (ps)\n",
           _dt0, (_dt1/_dt0)*1e-9, sqrt(_dt2/_dt0 - pow(_dt1/_dt0,2)));
  }
private:
  void _sync(const Xtc& xtc, const Sequence& seq) {

    unsigned dfid = seq.stamp().fiducials()-_seq.stamp().fiducials();
    if (dfid > TimeStamp::MaxFiducials)
      dfid += TimeStamp::MaxFiducials;

    uint64_t acqts = reinterpret_cast<Acqiris::DataDescV1*>
      (reinterpret_cast<const Xtc*>(xtc.payload())->payload())->timestamp(0).value();

    if (dfid != _dfid) {
      printf("Sync point : acqts %llx  dacqts %g  dfid %u  at ",
             acqts, double(acqts-_acqts)*1e-12/DFID, dfid);
      dump(seq);

      if (_cache.size()>1) {
        printf("other candidates:\n");
        for(std::list<Xtc*>::iterator it=_cache.begin(); it!=_cache.end(); it++) {
          Xtc& txtc = *(*it);
          if (&txtc != &xtc) {
            uint64_t tacqts = reinterpret_cast<Acqiris::DataDescV1*>
              (reinterpret_cast<const Xtc*>(txtc.payload())->payload())->timestamp(0).value();
            printf("\tdacqts %g\n",double(tacqts-_acqts)*1e-12/DFID);
          }
        }
      }

      _nsync++;
      _dfid = dfid;
    }

#ifdef MEASURE_RATE
    if (_acqts) {
      double dt = double(acqts-_acqts)/double(dfid);
      _dt0++;
      _dt1 += dt;
      _dt2 += dt*dt;

      _rate_dfid = dfid;
      _rate_dts  = double(acqts-_acqts);
      _rate_sec  = seq.clock().seconds();
      _rate_nsec = seq.clock().nanoseconds();
      _nt->Fill();
    }
#endif

    _acqts = acqts;
    _seq   = seq;
  }
  bool _match(const Xtc& xtc, const Sequence& seq) {
    //
    //  Time is measured in Acqiris by picosecond clock
    //
    uint64_t acqts = reinterpret_cast<Acqiris::DataDescV1*>
      (reinterpret_cast<const Xtc*>(xtc.payload())->payload())->timestamp(0).value();
    double acqdt = 1.e-12*double(acqts - _acqts);

    double fiddt = dfiducial(seq,_seq);

    //    The relative clock rate uncertainties limit the synchronicity test
    //    across long timespans.
    const double DCLK = 5.0e-3;  // relative clock rate uncertainty
    bool pass = ( fabs(acqdt - fiddt) < DCLK*acqdt );
    return pass;
  }
private:
  Src             _src;
  std::list<Xtc*> _cache;
  uint64_t        _acqts;
  Sequence        _seq;
  unsigned        _dfid;
  unsigned        _nsync;
  double          _dt0, _dt1, _dt2;
#ifdef MEASURE_RATE
  double          _rate_dfid, _rate_dts;
  unsigned        _rate_sec, _rate_nsec;
  TTree*          _nt;
#endif
  enum State { Initial, Matching };
  State           _state;
};

//
//  A class to collect pointers to each detector's data from the xtc iteration
//
class EventHandler : public XtcIterator {
  enum {Stop, Continue};
public:
  EventHandler() : _nnotreordered(0), _noutoforder(0), _pseq(ClockTime(0,0),TimeStamp(0,0,0)) {}
  ~EventHandler() {
#ifdef MEASURE_RATE
    for(std::list<AcqHandler*>::iterator it=_acqlist.begin(); 
        it!=_acqlist.end(); it++)
      (*it)->stats();
#endif
  }
public:
  void processDg(Dgram* dg) 
  {
    _seq  = dg->seq;
    iterate(&dg->xtc);
    if (_seq.service() != TransitionId::Configure &&
        _seq.clock() > _pseq.clock())
      _pseq = _seq;
  }
public:
  const Sequence& sequence() const { return _seq; }
  unsigned nreordered () const { return _noutoforder-_nnotreordered; }
  unsigned noutoforder() const { return _noutoforder; }
  unsigned nsync() const { return (*_acqlist.begin())->nsync(); }
private:
  int process(Xtc* xtc) 
  {
    if (xtc->extent        <  sizeof(Xtc) ||        // check for corrupt data
        xtc->contains.id() >= TypeId::NumberOf) {
      return Stop;
    }
    else if (xtc->contains.id() == TypeId::Id_Xtc) { // iterate through hierarchy
      iterate(xtc);
    }
    else {                          // handle specific detector data
      const Src& src = xtc->src;

      switch(xtc->contains.id()) {
      case TypeId::Id_AcqConfig:
        { for(std::list<AcqHandler*>::iterator it=_acqlist.begin(); 
              it!=_acqlist.end(); it++)
            if ((*it)->src()==src)
              return Continue;
          _acqlist.push_back(new AcqHandler(src));
          break; }
      case TypeId::Id_AcqWaveform:
        { if ((xtc-1)->damage.value() & (1<<Damage::OutOfOrder))
            _noutoforder++;
          for(std::list<AcqHandler*>::iterator it=_acqlist.begin(); 
              it!=_acqlist.end(); it++)
            if ((*it)->src()==src) {
              (*it)->process(*(xtc-1),_seq,_pseq);
              break;
            }
          if ((xtc-1)->damage.value() & (1<<Damage::OutOfOrder))
            _nnotreordered++;
          break; 
        }
      case TypeId::Any:
        { for(std::list<AcqHandler*>::iterator it=_acqlist.begin(); 
              it!=_acqlist.end(); it++)
            if ((*it)->src()==src) {
              (*it)->process(_seq,_pseq);
              break;
            }
          break;
        }
      default:
        break;
      }
    }
    return Continue;
  }
private:
  unsigned               _nnotreordered;
  unsigned               _noutoforder;
  Sequence               _seq, _pseq;
  std::list<AcqHandler*> _acqlist;
};

class XtcWriter {
public:
  XtcWriter(std::string fbase, const char* outpath, unsigned long long chunksize) : 
    _f(0),
    _chunkindex(0),
    _chunkmax  (chunksize),
    _chunksize (0)
  {
    if (outpath) {
      sprintf(_base,"%s/%s-s00",outpath,fbase.substr(fbase.rfind("/"),fbase.find("-s")).data());
      _open();
    }
  }
  ~XtcWriter() {
    if (_f)
      fclose(_f);
  }
public:
  void write(Dgram* dg) {
    if (_f) {
      unsigned sz = sizeof(Dgram)+dg->xtc.sizeofPayload();
      _chunksize += sz;
      if (_chunksize > _chunkmax) {
        fclose(_f);
        _chunkindex++;
        _open();
        _chunksize = sz;
        if (!_f) return;
      }        
      fwrite(dg, sz, 1, _f);
    }
  }
private:
  void _open() {
    char buffer[256];
    sprintf(buffer,"%s-c%02d.xtc",_base,_chunkindex);
    _fd = open(buffer,O_CREAT | O_EXCL | O_LARGEFILE | O_WRONLY);
    _f = fdopen(_fd,"w");
    if (_f)
      printf("Opened %s\n",buffer);
    else
      printf("Error opening %s\n",buffer);
  }      
private:
  FILE* _f;
  int   _fd;
  unsigned _chunkindex;
  unsigned long long _chunkmax;
  unsigned long long _chunksize;
  char _base[256];
};

void usage(char* progname) 
{
  fprintf(stderr,"Usage: %s -l <filelist> -o <output path> [-c <chunk size>] [-n <maxevts>] [-h]\n", progname);
}

static void dump(Dgram* dg)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x %s extent 0x%x damage %x\n",
         buff,
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
         TransitionId::name(dg->seq.service()),
         dg->xtc.extent, dg->xtc.damage.value());
}

static void dump(Dgram* dg, unsigned ev)
{
  char buff[128];
  time_t t = dg->seq.clock().seconds();
  strftime(buff,128,"%H:%M:%S",localtime(&t));
  printf("%s.%09u %08x/%08x %s extent 0x%x damage %x event %d\n",
         buff,
         dg->seq.clock().nanoseconds(),
         dg->seq.stamp().fiducials(),dg->seq.stamp().vector(),
         TransitionId::name(dg->seq.service()),
         dg->xtc.extent, dg->xtc.damage.value(), ev);
}

void anarun(XtcRun& run, EventHandler* ehandler,
            unsigned &maxevt, const char* outpath, unsigned long long chunksize)
{
  run.init();

  char* buffer = new char[0x2000000];
  Dgram* dg = (Dgram*)buffer;
  printf("Using buffer %p\n",buffer);

  XtcWriter*    writer   = new XtcWriter(run.base(), outpath, chunksize);

  Result r = OK;
  unsigned nevent = 0;
  unsigned nprint = 1;
  unsigned damage;
  unsigned damagemask = 0;
  unsigned ndamage = 0;
  do {
    
    r = run.next(dg);
    if (r == Error)
      break;

    if (dg->seq.service()!=TransitionId::L1Accept) 
      dump(dg);
    else if (nevent%nprint == 0) {
      dump(dg,nevent+1);
      if (nevent==10*nprint)
        nprint *= 10;
    }

    ehandler->processDg(dg);
    writer  ->write    (dg);

    damage = dg->xtc.damage.value();
    if (damage)
      {
        ndamage++;
        damagemask |= damage;
      }

    if (dg->seq.service() == TransitionId::L1Accept)
      {
        if (maxevt)
          maxevt--;
        else
          break;
        nevent++;
      }
  } while(r==OK);
  
  printf("Processed %d events, %d damaged, with damage mask 0x%x.\n",
         nevent, ndamage, damagemask);

  delete[] buffer;
  delete writer  ;
}

int main(int argc, char *argv[])
{
  int c;
  char *filelist  = 0;
  char  filename[200];
  char* outpath   = 0;
  unsigned maxevt = 0xffffffff;
  unsigned long long chunksize = 1ULL<<36; // 64 GBytes
  unsigned first_run = 0;

  while ((c = getopt(argc, argv, "hc:n:l:o:r:")) != -1)
    {
      switch (c)
        {
        case 'h':
          usage(argv[0]);
          exit(0);
        case 'c':
          chunksize = strtoull(optarg, NULL, 0);
          break;
        case 'l':
          filelist = optarg;
          break;
        case 'o':
          outpath = optarg;
          break;
        case 'n':
          maxevt = strtoul(optarg, NULL, 0);
          printf("Will process %d events.\n", maxevt);
          break;
        case 'r':
          first_run = strtoul(optarg, NULL, 0);
          break;
        default:
          usage(argv[0]);
          exit(2);
        }
    }

  if (!filelist)
    {
      usage(argv[0]);
      exit(2);
    }

#ifdef MEASURE_RATE
  char outfile[256];
  sprintf(outfile,"%.*s.root",strrchr(filelist,'.')-filelist,filelist);
  TFile* out = new TFile(outfile, "RECREATE");
#endif

  printf("Opening filelist %s\n", filelist);
  FILE *flist = fopen(filelist, "r");
  if (flist)
    {
      static const int FNAME_LEN=128;
      char* currlistname = (char*)malloc(FNAME_LEN);
      strncpy(currlistname,basename(filelist),FNAME_LEN);
      //
      //  create a sorted list of filenames
      //    (this conveniently groups chunks and slices)
      //
      std::list<std::string> all_files;
      while (fscanf(flist, "%s", filename) != EOF)
        all_files.push_back(std::string(filename));
      all_files.sort();

      EventHandler* ehandler = new EventHandler;

      XtcRun run;
      std::list<std::string>::const_iterator it=all_files.begin();
      run.reset(*it);
      int nfiles=1;
      while(++it!=all_files.end()) {
        if (!run.add_file(*it)) {
          printf("Analyzing files %s [%d]\n", 
                 run.base(),nfiles);
          anarun(run, ehandler, maxevt, 
                 run.run_number()>=first_run ? outpath : NULL, 
                 chunksize);
          run.reset(*it);
          nfiles=0;
        }
        nfiles++;
      }
      printf("Analyzing files %s [%d]\n", 
             run.base(),nfiles);
      anarun(run, ehandler, maxevt, 
             run.run_number()>=first_run ? outpath : NULL, 
             chunksize);

      printf("Found %d sync points\n", ehandler->nsync());
      printf("Recovered %d/%d outoforder.\n", 
             ehandler->nreordered(), ehandler->noutoforder());

      delete ehandler;
    }
  else
    printf("Unable to open list of files %s\n", filelist);

#ifdef MEASURE_RATE
  out->Write();
  out->Close();
#endif

  return 0;
}


