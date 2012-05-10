#ifndef Ami_Cds_hh
#define Ami_Cds_hh

#include <sys/uio.h>

#include "ami/data/Desc.hh"
#include "ami/service/Port.hh"
#include "ami/service/Semaphore.hh"

#include <list>

namespace Ami {

  class Entry;

  class Cds {
  public:
    Cds(const char* name);
    ~Cds();
  
    unsigned       add         (Entry* entry);
    void           add         (Entry* entry, unsigned signature);
    void           remove      (Entry* entry);
    const Entry*   entry       (int signature) const;
    Entry*         entry       (int signature);
    unsigned short totalentries() const { return _entries.size(); }
    unsigned short totaldescs  () const { return _entries.size()+1; }

    void reset      ();
    void showentries() const;

    //  serialize
    unsigned description() const { return totaldescs(); }
    unsigned payload    () const { return totalentries(); }
    void     description(iovec*) const;
    void     payload    (iovec*) const;

    void     invalidate_payload();

    Semaphore& payload_sem() const { return _payload_sem; }
  private:
    void adjust();

  private:
    typedef std::list<Entry*> EnList;
    Desc              _desc;
    EnList            _entries;
    mutable Semaphore _payload_sem;
    unsigned          _signature;
  };
};

#endif
