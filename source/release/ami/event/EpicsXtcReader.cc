#include "EpicsXtcReader.hh"

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/Level.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/DetInfo.hh"

#include "pdsdata/epics/EpicsPvData.hh"

#include <stdio.h>

namespace Ami {
  class EpicsInfo : public Pds::Src {
  public:
    EpicsInfo() : Pds::Src(Pds::Level::Segment) { _phy=1; }
  };
};

using namespace Ami;

EpicsXtcReader::EpicsXtcReader(FeatureCache& f)  : 
  EventHandler(Pds::DetInfo(0, Pds::DetInfo::EpicsArch, 0, DetInfo::NoDevice, 0),
    //  EventHandler(EpicsInfo(),
	       Pds::TypeId::Id_Epics,
	       Pds::TypeId::Id_Epics),
  _cache(f)
{
}

EpicsXtcReader::~EpicsXtcReader()
{
}

void   EpicsXtcReader::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void   EpicsXtcReader::_configure(const void* payload, const Pds::ClockTime& t)
{
  const Pds::EpicsPvHeader& pvData = *reinterpret_cast<const Pds::EpicsPvHeader*>(payload);

  if (pvData.iDbrType >= DBR_CTRL_SHORT &&
      pvData.iDbrType <= DBR_CTRL_DOUBLE) {
    const Pds::EpicsPvCtrlHeader& ctrl = static_cast<const Pds::EpicsPvCtrlHeader&>(pvData);

    if (pvData.iPvId < 0) {
      printf("EpicsXtcReader found pv %s id %d.  Ignoring.\n",ctrl.sPvName,pvData.iPvId);
      return;
    }

    if (pvData.iPvId >= MaxPvs) {
      printf("EpicsXtcReader found pv %s id %d > %d.  Ignoring.\n",ctrl.sPvName,pvData.iPvId,MaxPvs);
      return;
    }

    int index = -1;
    if (ctrl.iNumElements>1) {
      char buffer[64];
      strncpy(buffer,ctrl.sPvName,64);
      char* iptr = buffer+strlen(buffer);
      for(unsigned i=0; i<unsigned(ctrl.iNumElements); i++) {
	sprintf(iptr,"[%d]",i);
	index = _cache.add(buffer);
      }
      index -= ctrl.iNumElements-1;
    }
    else {
      index = _cache.add(ctrl.sPvName);
    }

    if (ctrl.iPvId < MaxPvs)
      _index[ctrl.iPvId] = index;
    else
      printf("PV %s truncated from list\n",ctrl.sPvName);
  }
}

#define CASETOVAL(timetype,valtype) case timetype: {			\
    const EpicsPvTime<valtype>& p = static_cast<const EpicsPvTime<valtype>&>(pvData); \
    const EpicsDbrTools::DbrTypeFromInt<valtype>::TDbr* v = &p.value;	\
    for(int i=0; i<pvData.iNumElements; i++)				\
      _cache.cache(index++, *v++);					\
    break; }

void   EpicsXtcReader::_event    (const void* payload, const Pds::ClockTime& t)
{
  const EpicsPvHeader& pvData = *reinterpret_cast<const EpicsPvHeader*>(payload);

  if (pvData.iPvId <  0 || 
      pvData.iPvId >= MaxPvs)
    return;

  if (pvData.iDbrType < DBR_CTRL_STRING) {
    int index = _index[pvData.iPvId];
    switch(pvData.iDbrType) {
    CASETOVAL(DBR_TIME_SHORT ,DBR_SHORT)
    CASETOVAL(DBR_TIME_FLOAT ,DBR_FLOAT)
    CASETOVAL(DBR_TIME_ENUM  ,DBR_ENUM)
    CASETOVAL(DBR_TIME_LONG  ,DBR_LONG)
    CASETOVAL(DBR_TIME_DOUBLE,DBR_DOUBLE)
    default: 
      break;
    }
  }
}

void   EpicsXtcReader::_damaged  ()
{
  for(unsigned i=0; i<MaxPvs; i++)
    if (_index[i]>=0)
      _cache.cache(_index[i], 0, true);
}

//  No Entry data
unsigned     EpicsXtcReader::nentries() const { return 0; }
const Entry* EpicsXtcReader::entry   (unsigned) const { return 0; }
void         EpicsXtcReader::reset   () 
{
  for(unsigned i=0; i<MaxPvs; i++)
    _index[i] = -1;
}
