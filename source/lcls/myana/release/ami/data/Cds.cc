#include <string.h>

#include "Cds.hh"
#include "Entry.hh"
#include "DescEntry.hh"
#include "ami/service/Semaphore.hh"

//#define DBUG

using namespace Ami;

Cds::Cds(const char* name) :
  _desc       (name),
  _payload_sem(Semaphore::FULL),
  _signature  (0)
{
  _desc.signature(_signature++);
}

Cds::~Cds()
{
  reset();
}

unsigned Cds::add(Entry* entry) 
{
  unsigned s = _signature++; 
  add(entry,s);
  return s; 
}

void Cds::add(Entry* entry, unsigned signature)
{
  entry->desc().signature(signature);
  _entries.push_back(entry);

#ifdef DBUG
  printf("Cds %s added entry %s (%p) type %d signature %d nentries %d\n",
   	 _desc.name(),entry->desc().name(),entry,entry->desc().type(),signature,totalentries());
#endif
}

void Cds::remove(Entry* entry)
{
  if (!entry)
    return;

  _entries.remove(entry);

#ifdef DBUG
  printf("Cds %s removed entry %s (%p) type %d signature %d nentries %d\n",
  	 _desc.name(),entry->desc().name(),entry,entry->desc().type(),entry->desc().signature(),totalentries());
#endif
}

void Cds::reset()
{
  for (EnList::iterator it=_entries.begin(); it!=_entries.end(); it++) {
    Entry* entry = *it;

#ifdef DBUG    
    printf("Cds %s clearing entry %s type %d signature %d\n",
     	   _desc.name(),entry->desc().name(),entry->desc().type(),entry->desc().signature());
#endif
    delete entry;
  }
  _entries.clear();
}

const Entry*   Cds::entry       (int signature) const
{
  for (EnList::const_iterator it=_entries.begin(); it!=_entries.end(); it++) {
    const Entry* en = *it;
    if (en->desc().signature()==signature)
      return en;
  }
  return 0;
}

Entry*         Cds::entry       (int signature)
{
  for (EnList::iterator it=_entries.begin(); it!=_entries.end(); it++) {
    Entry* en = *it;
    if (en->desc().signature()==signature)
      return en;
  }
  return 0;
}

#include <stdio.h>
#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

void Cds::showentries() const
{
  printf("Serving %d entries:\n", totalentries());
  for (EnList::const_iterator it=_entries.begin(); it!=_entries.end(); it++) {
    const Entry* en = *it;
    printf("  [%2d] %s\n", en->desc().signature(), en->desc().name());
  }
}

void Cds::description(iovec* iov) const
{
  Cds* cthis = const_cast<Cds*>(this);
  cthis->_desc.nentries(totalentries());
  iov->iov_base = (void*)&cthis->_desc;
  iov->iov_len  = sizeof(Desc);
  iov++;
  for (EnList::const_iterator it=_entries.begin(); it!=_entries.end(); it++, iov++) {
    const Entry* en = *it;
    iov->iov_base = (void*)&en->desc();
    iov->iov_len = en->desc().size();
  }
}

void Cds::payload(iovec* iov) const
{
  for (EnList::const_iterator it=_entries.begin(); it!=_entries.end(); it++, iov++)
    (*it)->payload(*iov);
}

void Cds::invalidate_payload()
{
  for (EnList::const_iterator it=_entries.begin(); it!=_entries.end(); it++)
    (*it)->invalid();
}
