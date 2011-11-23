#include "EBeamReader.hh"

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/Level.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/bld/bldData.hh"

#include <stdio.h>

using namespace Ami;

EBeamReader::EBeamReader(FeatureCache& f)  : 
  EventHandler(Pds::BldInfo(0,Pds::BldInfo::EBeam),
	       Pds::TypeId::Id_EBeam,
	       Pds::TypeId::Id_EBeam),
  _cache(f),
  _index(-1)
{
}

EBeamReader::~EBeamReader()
{
}

void   EBeamReader::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   EBeamReader::_configure(const void* payload, const Pds::ClockTime& t) 
{
  _index = _cache.add("BLD:EBEAM:Q");
  _cache.add("BLD:EBEAM:L3E");
  _cache.add("BLD:EBEAM:LTUX");
  _cache.add("BLD:EBEAM:LTUY");
  _cache.add("BLD:EBEAM:LTUXP");
  _cache.add("BLD:EBEAM:LTUYP");
  _cache.add("BLD:EBEAM:PKCURRBC2");
}

void   EBeamReader::_event    (const void* payload, const Pds::ClockTime& t)
{
  if (_index>=0) {
    const Pds::BldDataEBeam& bld = 
      *reinterpret_cast<const Pds::BldDataEBeam*>(payload);
    unsigned index = _index;
//     const double* p = &bld.fEbeamCharge;
//     const double* e = &bld.fEbeamPkCurrBC2;
//     unsigned mask   =  bld.uDamageMask;
//     while(p <= e) {
//       if (mask&1)
// 	_cache.cache(index,-1,true);
//       else
// 	_cache.cache(index,*p);
//       mask >>= 1;
//       index++;
//       p++;
//     }	
    _cache.cache(index++,bld.fEbeamCharge);
    _cache.cache(index++,bld.fEbeamL3Energy);
    _cache.cache(index++,bld.fEbeamLTUPosX);
    _cache.cache(index++,bld.fEbeamLTUPosY);
    _cache.cache(index++,bld.fEbeamLTUAngX);
    _cache.cache(index++,bld.fEbeamLTUAngY);
    _cache.cache(index++,bld.fEbeamPkCurrBC2);
  }
}

void   EBeamReader::_damaged  ()
{
  if (_index>=0) {
    unsigned index = _index;
    _cache.cache(index++,-1,true);
    _cache.cache(index++,-1,true);
    _cache.cache(index++,-1,true);
    _cache.cache(index++,-1,true);
    _cache.cache(index++,-1,true);
    _cache.cache(index++,-1,true);
    _cache.cache(index  ,-1,true);
  }
}

//  No Entry data
unsigned     EBeamReader::nentries() const { return 0; }
const Entry* EBeamReader::entry   (unsigned) const { return 0; }
void         EBeamReader::reset   () { _index=-1; }

