#include "pdsdata/xtc/ClockTime.hh"


Pds::ClockTime::ClockTime() {}
Pds::ClockTime::ClockTime(const ClockTime& t) : _low(t._low), _high(t._high) {}
Pds::ClockTime::ClockTime(unsigned sec, unsigned nsec) : _low(nsec), _high(sec) {}

Pds::ClockTime& Pds::ClockTime::ClockTime::operator=(const ClockTime& input)
{
  _low  = input._low;
  _high = input._high;
  return *this;
}

bool Pds::ClockTime::ClockTime::operator>(const ClockTime& t) const
{
  return (_high > t._high) || (_high == t._high && _low > t._low);
}

bool Pds::ClockTime::ClockTime::operator==(const ClockTime& t) const
{
  return (_high == t._high) && (_low == t._low);
}
