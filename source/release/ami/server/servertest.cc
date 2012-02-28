#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "ami/service/Task.hh"
#include "ami/service/Port.hh"
#include "ami/service/Ins.hh"

#include "ami/server/Factory.hh"
#include "ami/server/Server.hh"
#include "ami/server/ServerManager.hh"

#include "ami/data/Cds.hh"
#include "ami/data/ChannelID.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/FeatureCache.hh"

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/ClockTime.hh"

typedef Pds::DetInfo DI;

static float randomnumber(float low, float hig)
{
  float res = low + rand()*(hig-low)/RAND_MAX;
  return res;
}

static void randomgss(float& x, float& y)
{
  float r = sqrt(-2*log(randomnumber(0.,1.)));
  float f = randomnumber(0.,2*M_PI);
  x = r*cos(f);
  y = r*sin(f);
}

namespace Ami {
  class MyFactory : public Factory {
  public:
    MyFactory(ServerManager& srv) : 
      _cds("Factory"),
      _srv(srv)
    {
      { DI src(0,DI::NoDetector,0,DI::Acqiris,0);
	DescWaveform wfd(src,0,ChannelID::name(src,0),"x","y",200,-0.5,199.5);
	wfd.signature(1);
	_wf = new EntryWaveform(wfd); }
      { DI src(0,DI::NoDetector,0,DI::Opal1000,0);
	DescImage    imd(src,0,ChannelID::name(src,0),256,256,4,4);
	imd.signature(2);
	_im = new EntryImage(imd); }
      _cds.add(_wf);
      _cds.add(_im);
    }
    ~MyFactory() 
    {
      delete _wf;
      delete _im;
    }
  public:
    FeatureCache& features() { return _features; }
    Cds& discovery() { return _cds; }
    Cds& hidden   () { return _ocds; }
    void configure(unsigned id, const Message& cfg, const char* b, Cds& cds)
    {
      printf("Factory::configure payload %d %p\n",cfg.payload(),b);
//       { const unsigned* p = reinterpret_cast<const unsigned*>(b);
// 	for(unsigned k=0; k<cfg.payload()>>2; k++)
// 	  printf("%08x%c", *p++, (k&7)==7 ? '\n' : ' ');
//       }
      const char* e = b + cfg.payload();
      while( b < e ) {
	const ConfigureRequest& req = reinterpret_cast<const ConfigureRequest&>(*b);
	printf("config %s inp %d outp %d\n", 
	       req.state()==ConfigureRequest::Create ? "Create" : "Destroy",
	       req.input(),req.output());
	b += req.size();
      }
    }
    void analyze() {
      timespec tp;
      clock_gettime(CLOCK_REALTIME, &tp);
      Pds::ClockTime clk(tp.tv_sec,tp.tv_nsec);
      //  Fill the waveform
      { float f = randomnumber(0,10);
	for(unsigned j=0; j<_wf->desc().nbins(); j++)
	  _wf->content(cos(double(j)*0.1+f), j);
	_wf->valid(clk); }
      //  Fill the image
      { float x,y;
	randomgss(x,y);
	for(unsigned j=0; j<_im->desc().nbinsx(); j++) {
	  float ex = sin(double(j)*.1+x);
	  for(unsigned k=0; k<_im->desc().nbinsy(); k++) {
	    float ey = sin(double(k)*.1+y);
	    _im->content(unsigned((ex*ey+1)*100), j,k);
	  }
	}
	_im->valid(clk); }
    }
    void discover() { _srv.discover(); }
    void wait_for_configure() {}
  private:
    FeatureCache   _features;
    Cds            _cds;
    Cds            _ocds;
    ServerManager  _srv;
    EntryWaveform* _wf;
    EntryImage*    _im;
  };
};

using namespace Ami;

int main(int argc, char **argv) 
{
  unsigned interface = 0;
  in_addr inp;
  if (inet_aton(argv[1], &inp))
    interface = ntohl(inp.s_addr);

  ServerManager srv(interface,0xefff2000);
  MyFactory     factory(srv);
  srv.serve(factory);

  timespec tv;
  tv.tv_sec  = 0;
  tv.tv_nsec = 100000000;
  while(1) {
    nanosleep(&tv,0);

    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    Pds::ClockTime clk(tp.tv_sec, tp.tv_nsec);
//     fmServer.accumulate(clk);
//     wfServer.accumulate(clk);
  }

  srv.dont_serve();

  return 0;
}
