#include "Reference.hh"

#include "ami/data/DescEntry.hh"
#include "ami/data/Entry.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/EntryWaveform.hh"
#include "ami/data/EntryFactory.hh"

#include <sys/uio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace Ami;

#define READ_SIZE(buffer,len,f) {                                       \
    size = fread(buffer,1,len,f);                                       \
    printf("reference read %d bytes\n",int(size));                      \
    if ((unsigned)size != len) {                                        \
      printf("reference read %d/%d bytes\n",int(size),int(len));        \
      abort();                                                          \
    }                                                                   \
  }

Reference::Reference(const char* path) : 
  AbsOperator(AbsOperator::Reference),
  _entry     (0)
{
  strncpy(_path,path,PATHLEN);
}

Reference::Reference(const char*& p, const DescEntry& e) :
  AbsOperator(AbsOperator::Reference)
{
  _extract(p, _path, PATHLEN);
  FILE* f = fopen(_path,"r");
  if (!f) {
    printf("reference failed to open %s\n",_path);
    abort();
  }
  int size;
  READ_SIZE(_buffer,sizeof(DescEntry),f);
  const DescEntry& d = *reinterpret_cast<const DescEntry*>(_buffer);
  if (d.size()>sizeof(_buffer)) {
    printf("reference attempt to read desc size %d from %s\n",d.size(),_path);
    abort();
  }
  READ_SIZE(_buffer+sizeof(DescEntry),d.size()-sizeof(DescEntry),f);
  _entry = EntryFactory::entry(d);
  iovec iov;
  _entry->payload(iov);
  READ_SIZE(iov.iov_base,iov.iov_len,f);
  fclose(f);
}

Reference::~Reference()
{
  if (_entry) delete _entry;
}

DescEntry& Reference::output   () const { return _entry->desc(); }

void*      Reference::_serialize(void* p) const
{
  _insert(p, _path, PATHLEN);
  return p;
}

Entry&     Reference::_operate(const Entry& e) const
{
  _entry->valid(e.time());
  return *_entry;
}
