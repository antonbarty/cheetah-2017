#include "pdsdata/cspad/ElementIterator.hh"

#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/cspad/ConfigV2.hh"
#include "pdsdata/cspad/ConfigV3.hh"
#include "pdsdata/cspad/ElementV1.hh"
#include "pdsdata/cspad/ElementV2.hh"

#include "pdsdata/xtc/Xtc.hh"

#include <stdio.h>
#include <stdlib.h>

using namespace Pds::CsPad;

ElementIterator::ElementIterator() :
  _elem(0), _end(0), _qmask(0)
{
}

ElementIterator::ElementIterator(const ConfigV1& c, const Xtc& xtc) :
  _elem((const ElementHeader*)(xtc.payload())), 
  _end ((const ElementHeader*)(xtc.payload()+xtc.sizeofPayload())),
  _qmask(c.quadMask())
{
  if (xtc.contains.id()!=Pds::TypeId::Id_CspadElement ||
      xtc.contains.version()!=1) {
    printf("Pds::CsPad::ElementIterator wrong type %x/%x\n",
 	   xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
  else {
    unsigned amask(c.asicMask());
    for(int iq=0; iq<4; iq++) {
      if (_qmask & (1<<iq))
	_smask[iq] = amask==1 ? 0x3 : 0xff;
      else
	_smask[iq] = 0;
    }
  }
}

ElementIterator::ElementIterator(const ConfigV2& c, const Xtc& xtc) :
  _elem((const ElementHeader*)(xtc.payload())), 
  _end ((const ElementHeader*)(xtc.payload()+xtc.sizeofPayload())),
  _qmask(c.quadMask())
{
  if (xtc.contains.id()!=Pds::TypeId::Id_CspadElement) {
    printf("Pds::CsPad::ElementIterator wrong type %x/%x\n",
 	   xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
  else if (xtc.contains.version()==1) {
    unsigned amask(c.asicMask());
    for(int iq=0; iq<4; iq++) {
      if (_qmask & (1<<iq))
	_smask[iq] = amask==1 ? 0x3 : 0xff;
      else
	_smask[iq] = 0;
    }
  }
  else if (xtc.contains.version()==2) {
    for(int iq=0; iq<4; iq++)
      _smask[iq] = c.roiMask(iq);
  }
  else {
    printf("Pds::CsPad::ElementIterator wrong type %x/%x\n",
 	   xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
}

ElementIterator::ElementIterator(const ConfigV3& c, const Xtc& xtc) :
  _elem((const ElementHeader*)(xtc.payload())),
  _end ((const ElementHeader*)(xtc.payload()+xtc.sizeofPayload())),
  _qmask(c.quadMask())
{
  if (xtc.contains.id()!=Pds::TypeId::Id_CspadElement) {
    printf("Pds::CsPad::ElementIterator wrong type %x/%x\n",
       xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
  else if (xtc.contains.version()==1) {
    unsigned amask(c.asicMask());
    for(int iq=0; iq<4; iq++) {
      if (_qmask & (1<<iq))
    _smask[iq] = amask==1 ? 0x3 : 0xff;
      else
    _smask[iq] = 0;
    }
  }
  else if (xtc.contains.version()==2) {
    for(int iq=0; iq<4; iq++)
      _smask[iq] = c.roiMask(iq);
  }
  else {
    printf("Pds::CsPad::ElementIterator wrong type %x/%x\n",
       xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
}

ElementIterator::ElementIterator(const ElementIterator& c) :
  _elem      (c._elem      ),
  _end       (c._end       ),
  _qmask     (c._qmask     ),
  _smaskc    (c._smaskc    ),
  _section   (c._section   ),
  _section_id(c._section_id)
{
  for(int i=0; i<4; i++)
    _smask[i]=c._smask[i];
}

const ElementHeader* ElementIterator::next()
{
  if (_qmask==0 || _elem>=_end) return 0;

  unsigned iq = _elem->quad();
  _qmask &= ~(1<<iq);

  _smaskc = _smask[iq];
  if (_smaskc) {
    _section    = reinterpret_cast<const Section*>(_elem+1);
    _section_id = 0; 
    while ((_smaskc&(1<<_section_id))==0)
      _section_id++;
  }
  else {
    printf("Quad %d without sectors: qmask %x\n", iq, _qmask);
    const unsigned* d = reinterpret_cast<const unsigned*>(_elem);
    for(int i=0; i<8; i++)
      printf(" %08x",*d++);
    printf("\n");
    abort();
  }

  //  advance _elem 
  const ElementHeader* e = _elem;
  if (_qmask) {
    const Section*  s = reinterpret_cast<const Section*>(_elem+1);
    for(unsigned sm=_smask[iq]; sm; sm>>=1)
      if (sm&1) s++;
    const uint16_t* u = reinterpret_cast<const uint16_t*>(s);
    _elem = reinterpret_cast<const ElementHeader*>(u+2);
  }

  return e;
}

const Section* ElementIterator::next(unsigned& id)
{
  if (_smaskc==0) return 0;

  _smaskc &= ~(1<<_section_id);

  const Section* s = _section;
  id = _section_id;

  // advance _section
  if (_smaskc) {
    _section++;
    while ((_smaskc&(1<<++_section_id))==0) 
      ;
  }

  return s;
}
