#include "pdsdata/control/ConfigV1.hh"

#include "pdsdata/control/PVControl.hh"
#include "pdsdata/control/PVMonitor.hh"

using namespace Pds::ControlData;

enum { EventsMask   = 0x3fffffff,
       UsesDuration = 0x40000000,
       UsesEvents   = 0x80000000 };

static void appendChannels(const std::list<PVControl>& pvcs, 
			   const std::list<PVMonitor>& pvms, 
			   ConfigV1* s)
{
  std::list<PVControl> sorted_pvcs (pvcs); sorted_pvcs.sort();
  std::list<PVMonitor> sorted_pvms (pvms); sorted_pvms.sort();

  PVControl* c = reinterpret_cast<PVControl*>(s+1);
  for(std::list<PVControl>::const_iterator iter = sorted_pvcs.begin();
      iter != sorted_pvcs.end(); ++iter)
    *c++ = *iter;
  PVMonitor* m = reinterpret_cast<PVMonitor*>(c);
  for(std::list<PVMonitor>::const_iterator iter = sorted_pvms.begin();
      iter != sorted_pvms.end(); ++iter)
    *m++ = *iter;
}

ConfigV1::ConfigV1() {}

ConfigV1::ConfigV1(Initialize) :
  _control    (0),
  _npvControls(0),
  _npvMonitors(0)
{
}

ConfigV1::ConfigV1(const std::list<PVControl>& pvcs,const std::list<PVMonitor>& pvms) :
  _control    (0),
  _duration   (0,0),
  _npvControls(pvcs.size()),
  _npvMonitors(pvms.size())
{
  appendChannels(pvcs,pvms,this);
}

ConfigV1::ConfigV1(const std::list<PVControl>& pvcs,const std::list<PVMonitor>& pvms,
		   const ClockTime& t) :
  _control    (UsesDuration),
  _duration   (t),
  _npvControls(pvcs.size()),
  _npvMonitors(pvms.size())
{
  appendChannels(pvcs,pvms,this);
}

ConfigV1::ConfigV1(const std::list<PVControl>& pvcs,const std::list<PVMonitor>& pvms,
		   unsigned events ) :
  _control    (UsesEvents | (events&EventsMask)),
  _duration   (0,0),
  _npvControls(pvcs.size()),
  _npvMonitors(pvms.size())
{
  appendChannels(pvcs,pvms,this);
}

ConfigV1::ConfigV1(const ConfigV1& s) :
  _control    (s._control),
  _npvControls(s._npvControls),
  _npvMonitors(s._npvMonitors)
{
  PVControl* c = reinterpret_cast<PVControl*>(this+1);
  for(unsigned k=0; k<_npvControls; k++)
    *c++ = s.pvControl(k);
  PVMonitor* m = reinterpret_cast<PVMonitor*>(c);
  for(unsigned k=0; k<_npvMonitors; k++)
    *m++ = s.pvMonitor(k);
}

ConfigV1::~ConfigV1() {}

bool             ConfigV1::uses_duration()       const
{ return _control & UsesDuration; }

bool             ConfigV1::uses_events  ()       const
{ return _control & UsesEvents; }

const ClockTime& ConfigV1::duration   ()         const 
{ return _duration; }

unsigned         ConfigV1::events     ()         const 
{ return _control & EventsMask; }

unsigned         ConfigV1::npvControls()         const
{ return _npvControls; }

const PVControl& ConfigV1::pvControl  (unsigned i) const
{ return reinterpret_cast<const PVControl*>(this+1)[i]; }

unsigned         ConfigV1::npvMonitors()         const
{ return _npvMonitors; }

const PVMonitor& ConfigV1::pvMonitor  (unsigned i) const
{ return reinterpret_cast<const PVMonitor*>(&pvControl(_npvControls))[i]; }

unsigned         ConfigV1::size       ()         const
{ return sizeof(ConfigV1) + _npvControls*sizeof(PVControl) + _npvMonitors*sizeof(PVMonitor); }
