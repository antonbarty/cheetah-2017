#ifndef Pds_CfgClientNfs_hh
#define Pds_CfgClientNfs_hh

#include "pdsdata/xtc/Src.hh"

namespace Pds {

  class Allocation;
  class Transition;
  class TypeId;

  class CfgClientNfs {
  public:
    CfgClientNfs( const Src& src );
    ~CfgClientNfs() {}

    const Src& src() const;

    void initialize(const Allocation&);

    int fetch(const Transition& tr, 
	      const TypeId&     id, 
	      void*             dst,
	      unsigned          maxSize=0x100000);

  private:
    enum { PathSize=128 };
    Src      _src;
    char     _path[PathSize];
  };

};

#endif

    
