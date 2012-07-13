#include "SharedIpimbReader.hh"

#include "pdsdata/xtc/Dgram.hh"
#include "pdsdata/xtc/Level.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/bld/bldData.hh"

#include <stdio.h>

using namespace Ami;

  SharedIpimbReader::SharedIpimbReader(const Pds::BldInfo& bldInfo, FeatureCache& f)  : 
    EventHandler(bldInfo, Pds::TypeId::Id_SharedIpimb, Pds::TypeId::Id_SharedIpimb),
    _cache(f)
{
}

SharedIpimbReader::~SharedIpimbReader()
{
}

void  SharedIpimbReader::_calibrate(const void* payload, const Pds::ClockTime& t) {}
void  SharedIpimbReader::_configure(const void* payload, const Pds::ClockTime& t) 
{

  char buffer[64];
  strncpy(buffer,Pds::BldInfo::name(static_cast<const Pds::BldInfo&>(info())),60);
  char* iptr = buffer+strlen(buffer);
  
  unsigned i=0;
  sprintf(iptr,":DATA[0]");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":DATA[1]");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":DATA[2]");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":DATA[3]");  _index[i] = _cache.add(buffer);  i++;

  sprintf(iptr,":FEX:CH0");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":FEX:CH1");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":FEX:CH2");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":FEX:CH3");  _index[i] = _cache.add(buffer);  i++;
  
  sprintf(iptr,":FEX:SUM");  _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":FEX:XPOS"); _index[i] = _cache.add(buffer);  i++;
  sprintf(iptr,":FEX:YPOS"); _index[i] = _cache.add(buffer);  i++;

}

void SharedIpimbReader::_event (const void* payload, const Pds::ClockTime& t)
{
  const Pds::BldDataIpimb& bld = 
      *reinterpret_cast<const Pds::BldDataIpimb*>(payload);

  _cache.cache(_index[0], bld.ipimbData.channel0Volts());
  _cache.cache(_index[1], bld.ipimbData.channel1Volts());
  _cache.cache(_index[2], bld.ipimbData.channel2Volts());
  _cache.cache(_index[3], bld.ipimbData.channel3Volts());
  
  _cache.cache(_index[4], bld.ipmFexData.channel[0]);
  _cache.cache(_index[5], bld.ipmFexData.channel[1]);
  _cache.cache(_index[6], bld.ipmFexData.channel[2]);
  _cache.cache(_index[7], bld.ipmFexData.channel[3]);
  
  _cache.cache(_index[8], bld.ipmFexData.sum);
  _cache.cache(_index[9], bld.ipmFexData.xpos);
  _cache.cache(_index[10],bld.ipmFexData.ypos);


}

void SharedIpimbReader::_damaged  ()
{
  _cache.cache(_index[0], 0, true);
  _cache.cache(_index[1], 0, true);
  _cache.cache(_index[2], 0, true);
  _cache.cache(_index[3], 0, true);
  _cache.cache(_index[4], 0, true);
  _cache.cache(_index[5], 0, true);
  _cache.cache(_index[6], 0, true);
  _cache.cache(_index[7], 0, true); 
  _cache.cache(_index[8], 0, true);
  _cache.cache(_index[9], 0, true);
  _cache.cache(_index[10],0, true);  
}

//  No Entry data
unsigned     SharedIpimbReader::nentries() const { return 0; }
const Entry* SharedIpimbReader::entry   (unsigned) const { return 0; }
void         SharedIpimbReader::reset   () { }

