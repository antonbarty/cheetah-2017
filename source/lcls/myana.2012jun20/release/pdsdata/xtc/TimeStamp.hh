#ifndef PDS_TIMESTAMP_HH
#define PDS_TIMESTAMP_HH

#include <stdint.h>

namespace Pds {
  class TimeStamp {
  public:
    enum {NumFiducialBits = 17};
    enum {MaxFiducials = (1<<17)-32};
    enum {ErrFiducial = (1<<17)-1};
  public:
    TimeStamp();
    TimeStamp(const TimeStamp&);
    TimeStamp(const TimeStamp&, unsigned control);
    TimeStamp(unsigned ticks, unsigned fiducials, unsigned vector, unsigned control=0);

  public:
    unsigned ticks    () const;
    unsigned fiducials() const;
    unsigned control  () const;
    unsigned vector   () const;

  public:
    TimeStamp& operator= (const TimeStamp&);
    bool       operator==(const TimeStamp&) const;
    bool       operator>=(const TimeStamp&) const;
    bool       operator<=(const TimeStamp&) const;
    bool       operator< (const TimeStamp&) const;
    bool       operator> (const TimeStamp&) const;

  private:
    uint32_t _low;
    uint32_t _high;
  };
}

#endif
