#ifndef Pds_ENTRYRef_HH
#define Pds_ENTRYRef_HH

#include "ami/data/Entry.hh"
#include "ami/data/DescRef.hh"

#include <math.h>

namespace Ami {

  class EntryRef : public Entry {
  public:
    EntryRef(const Pds::DetInfo& info, unsigned channel, const char* name, const char* ytitle);
    EntryRef(const DescRef& desc);

    virtual ~EntryRef();

    //  The contents are organized as 
    const void*  data() const;
    void   set (const void*);

    // Implements Entry
    virtual const DescRef& desc() const;
    virtual DescRef& desc();

  private:
    DescRef _desc;

  private:
    const void* _y;
  };

  inline const void* EntryRef::data() const { return _y; }
  inline void        EntryRef::set (const void* y) { _y=y; }

  inline const DescRef& EntryRef::desc() const {return _desc;}
  inline       DescRef& EntryRef::desc()       {return _desc;}

};


#endif
