#ifndef Pds_CfgCache_hh
#define Pds_CfgCache_hh

#include "pds/config/CfgClientNfs.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/Damage.hh"

namespace Pds {
  class Allocation;
  class Transition;
  class InDatagram;

  class CfgCache {
  public:
    CfgCache(const Src&    src,
	     const TypeId& id,
	     int           size);
    virtual ~CfgCache();
  public:
    bool        changed() const;
    bool        scanning() const;
    const void* current() const;
    void        record (InDatagram*) const;
  public:
    void        init   (const Allocation&);
    int         fetch  (Transition*);
    void        next   ();
    Damage&     damage ();
  private:
    virtual int _size  (void*) const = 0;
  private:
    CfgClientNfs _config;
    TypeId       _type;
    Xtc          _configtc;
    unsigned     _bsize;
    char*        _buffer;
    char*        _cur_config;
    char*        _end_config;
    bool         _changed;
    bool         _scanning;
  };
};

#endif
