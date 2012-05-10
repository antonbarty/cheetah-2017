#ifndef Ami_EventHandler_hh
#define Ami_EventHandler_hh

#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/XtcIterator.hh"

#include <list>

namespace Pds {
  class ClockTime;
};

namespace Ami {
  class Entry;

  class EventHandler {
  public:
    EventHandler(const Pds::Src&     info,
		 Pds::TypeId::Type   data_type,
		 Pds::TypeId::Type   config_type);
    EventHandler(const Pds::Src&     info,
		 const std::list<Pds::TypeId::Type>& data_type,
		 Pds::TypeId::Type   config_type);
    EventHandler(const Pds::Src&     info,
		 Pds::TypeId::Type   data_type,
		 const std::list<Pds::TypeId::Type>& config_type);
    EventHandler(const Pds::Src&     info,
		 const std::list<Pds::TypeId::Type>& data_type,
		 const std::list<Pds::TypeId::Type>& config_type);
    virtual ~EventHandler();
  public:
    virtual void   _configure(const void* payload, const Pds::ClockTime& t) = 0;
    virtual void   _calibrate(const void* payload, const Pds::ClockTime& t) = 0;
    virtual void   _event    (const void* payload, const Pds::ClockTime& t) = 0;
    virtual void   _damaged  () = 0;
  public:
    virtual void   _configure(Pds::TypeId, 
			      const void* payload, const Pds::ClockTime& t);
    virtual void   _calibrate(Pds::TypeId, 
			      const void* payload, const Pds::ClockTime& t);
    virtual void   _event    (Pds::TypeId,
                              const void* payload, const Pds::ClockTime& t);
  public:
    virtual unsigned     nentries() const = 0;
    virtual const Entry* entry            (unsigned) const = 0;
    virtual const Entry* hidden_entry     (unsigned) const { return 0; }
    virtual void         reset   () = 0;
  public:
    const Pds::Src&     info() const { return _info; }
    const Pds::TypeId::Type&  data_type() const { return _data_type.front(); }
    const std::list<Pds::TypeId::Type>& data_types() const { return _data_type; }
    const Pds::TypeId::Type&  config_type() const { return _config_type.front(); }
    const std::list<Pds::TypeId::Type>& config_types() const { return _config_type; }
  public:
    static void enable_full_resolution(bool);
  protected:
    bool _full_resolution() const;
  private:
    Pds::Src                     _info;
    std::list<Pds::TypeId::Type> _data_type;
    std::list<Pds::TypeId::Type> _config_type;
  };
};

#endif
