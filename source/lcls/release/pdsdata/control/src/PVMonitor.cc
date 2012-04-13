#include "pdsdata/control/PVMonitor.hh"

#include <string.h>
#include <limits>

using namespace Pds::ControlData;

static const uint32_t NoArray=std::numeric_limits<uint32_t>::max();

PVMonitor::PVMonitor() {}

PVMonitor::PVMonitor(const char* pvname, double loValue, double hiValue) :
  _index  (NoArray),
  _loValue(loValue),
  _hiValue(hiValue)
{
  strncpy(_name, pvname, NameSize);
}

PVMonitor::PVMonitor(const char* pvname, unsigned index, double loValue, double hiValue) :
  _index  (index  ),
  _loValue(loValue),
  _hiValue(hiValue)
{
  strncpy(_name, pvname, NameSize);
}

PVMonitor::PVMonitor(const PVMonitor& m) :
  _index  (m._index  ),
  _loValue(m._loValue),
  _hiValue(m._hiValue)
{
  strncpy(_name, m._name, NameSize);
}

PVMonitor::~PVMonitor() {}

bool PVMonitor::operator<(const PVMonitor& m) const
{
  int nt = strncmp(_name, m._name, NameSize);
  return (nt<0) || (nt==0 && _index < m._index);
}

const char* PVMonitor::name() const { return _name; }

bool PVMonitor::array() const { return _index!=NoArray; }

unsigned PVMonitor::index() const { return _index; }

double PVMonitor::loValue() const { return _loValue; }

double PVMonitor::hiValue() const { return _hiValue; }
