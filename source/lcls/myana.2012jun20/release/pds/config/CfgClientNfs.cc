#include "CfgClientNfs.hh"

#include "pds/config/CfgPath.hh"
#include "pds/utility/Transition.hh"
#include "pdsdata/xtc/Src.hh"
#include "pdsdata/xtc/TypeId.hh"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

using namespace Pds;

CfgClientNfs::CfgClientNfs( const Src& src ) :
  _src( src )
{
}

const Src& CfgClientNfs::src() const
{ return _src; }

void CfgClientNfs::initialize(const Allocation& alloc)
{
  strcpy(_path, alloc.dbpath());
}

int CfgClientNfs::fetch(const Transition& tr, 
			const TypeId&     id, 
			void*             dst,
			unsigned          maxSize)
{

  char filename[128];
  sprintf(filename,"%s/%s",_path,CfgPath::path(tr.env().value(),_src,id).c_str());
  int fd = ::open(filename,O_RDONLY);
  if (fd < 0) {
    printf("CfgClientNfs::fetch error opening %s : %s\n",
	   filename, strerror(errno));
    return 0;
  }

  int len = ::read(fd, dst, maxSize);
  ::close(fd);

  return len;
}
