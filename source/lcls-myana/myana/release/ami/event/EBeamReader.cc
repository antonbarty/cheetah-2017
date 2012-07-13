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
  _index(-1),
  _nvars(0)
{
}

EBeamReader::~EBeamReader()
{
}

void   EBeamReader::_calibrate(Pds::TypeId id,
                               const void* payload, 
                               const Pds::ClockTime& t) 
{}

void   EBeamReader::_configure(Pds::TypeId id,
                               const void* payload, 
                               const Pds::ClockTime& t) 
{
  _index = _cache.add("BLD:EBEAM:Q");
  _cache.add("BLD:EBEAM:L3E");
  _cache.add("BLD:EBEAM:LTUX");
  _cache.add("BLD:EBEAM:LTUY");
  _cache.add("BLD:EBEAM:LTUXP");
  _cache.add("BLD:EBEAM:LTUYP");

  int index;
  switch(id.version()) {
  case 0: break;
  case 1:
    index = _cache.add("BLD:EBEAM:PKCURRBC2");
    break;
  case 2:
    index = _cache.add("BLD:EBEAM:PKCURRBC2");
    index = _cache.add("BLD:EBEAM:ENERGYBC2");
    break;
  case 3:
  default:
    index = _cache.add("BLD:EBEAM:PKCURRBC1");
    index = _cache.add("BLD:EBEAM:ENERGYBC1");
    index = _cache.add("BLD:EBEAM:PKCURRBC2");
    index = _cache.add("BLD:EBEAM:ENERGYBC2");
    break;
  }
  _nvars = index-_index+1;
}

void   EBeamReader::_event    (Pds::TypeId id,
                               const void* payload, 
                               const Pds::ClockTime& t)
{
  if (_index>=0) {
    unsigned index = _index;
    switch(id.version()) {
    case 0:
      {
        const Pds::BldDataEBeamV0& bld = 
          *reinterpret_cast<const Pds::BldDataEBeamV0*>(payload);
        _cache.cache(index++,bld.fEbeamCharge);
        _cache.cache(index++,bld.fEbeamL3Energy);
        _cache.cache(index++,bld.fEbeamLTUPosX);
        _cache.cache(index++,bld.fEbeamLTUPosY);
        _cache.cache(index++,bld.fEbeamLTUAngX);
        _cache.cache(index++,bld.fEbeamLTUAngY);
        break;
      }
    case 1:
      {
        const Pds::BldDataEBeamV1& bld = 
          *reinterpret_cast<const Pds::BldDataEBeamV1*>(payload);
        _cache.cache(index++,bld.fEbeamCharge);
        _cache.cache(index++,bld.fEbeamL3Energy);
        _cache.cache(index++,bld.fEbeamLTUPosX);
        _cache.cache(index++,bld.fEbeamLTUPosY);
        _cache.cache(index++,bld.fEbeamLTUAngX);
        _cache.cache(index++,bld.fEbeamLTUAngY);
        _cache.cache(index++,bld.fEbeamPkCurrBC2);
        break;
      }
    case 2:
      {
        const Pds::BldDataEBeamV2& bld = 
          *reinterpret_cast<const Pds::BldDataEBeamV2*>(payload);
        _cache.cache(index++,bld.fEbeamCharge);
        _cache.cache(index++,bld.fEbeamL3Energy);
        _cache.cache(index++,bld.fEbeamLTUPosX);
        _cache.cache(index++,bld.fEbeamLTUPosY);
        _cache.cache(index++,bld.fEbeamLTUAngX);
        _cache.cache(index++,bld.fEbeamLTUAngY);
        _cache.cache(index++,bld.fEbeamPkCurrBC2);
        _cache.cache(index++,bld.fEbeamEnergyBC2);
        break;
      }
    case 3:
    default:
      {
        const Pds::BldDataEBeamV3& bld = 
          *reinterpret_cast<const Pds::BldDataEBeamV3*>(payload);
        _cache.cache(index++,bld.fEbeamCharge);
        _cache.cache(index++,bld.fEbeamL3Energy);
        _cache.cache(index++,bld.fEbeamLTUPosX);
        _cache.cache(index++,bld.fEbeamLTUPosY);
        _cache.cache(index++,bld.fEbeamLTUAngX);
        _cache.cache(index++,bld.fEbeamLTUAngY);
        _cache.cache(index++,bld.fEbeamPkCurrBC1);
        _cache.cache(index++,bld.fEbeamEnergyBC1);
        _cache.cache(index++,bld.fEbeamPkCurrBC2);
        _cache.cache(index++,bld.fEbeamEnergyBC2);
        break;
      }
    }
  }
}

void   EBeamReader::_damaged  ()
{
  if (_index>=0) {
    unsigned index = _index;
    for(int i=0; i<_nvars; i++)
      _cache.cache(index++,-1,true);
  }
}

//  No Entry data
unsigned     EBeamReader::nentries() const { return 0; }
const Entry* EBeamReader::entry   (unsigned) const { return 0; }
void         EBeamReader::reset   () { _index=-1; }

