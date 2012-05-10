#ifndef Pds_CfgClientNet_hh
#define Pds_CfgClientNet_hh

#include "pdsdata/xtc/Src.hh"

namespace Pds {

  class Transition;
  class TypeId;

  class CfgClientNet {
  public:
    CfgClientNet( const Src& src,
		  unsigned platform );
    ~CfgClientNet() {}

    void setDbName(const char*);
    
    int fetch(const Transition& tr, 
	      const TypeId&     id, 
	      char*             dst);

  private:
    enum { DbNameSize=16 };
    Src      _src;
    unsigned _platform;
    char     _dbName[DbNameSize];
  };

};

#endif

    
