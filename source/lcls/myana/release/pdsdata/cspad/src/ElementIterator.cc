#include "pdsdata/cspad/ElementIterator.hh"

#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/cspad/ConfigV2.hh"
#include "pdsdata/cspad/ConfigV3.hh"
#include "pdsdata/cspad/ConfigV4.hh"
#include "pdsdata/cspad/ElementV1.hh"
#include "pdsdata/cspad/ElementV2.hh"
#include "pdsdata/xtc/Xtc.hh"

#include <stdio.h>
#include <stdlib.h>

using namespace Pds::CsPad;

ElementIterator::ElementIterator() :
  _elem(0), _end(0), _qmask(0), _bCompressed(false), _section(NULL), _section_id(0), 
  _sectionBuf1(NULL), _sectionBuf2(NULL), _compressedSection(NULL), 
  _pDecompressor(NULL), _uQuadWord(0), _bLastQuadWordSet(false)
{
}

ElementIterator::ElementIterator(const ConfigV1& c, const Xtc& xtc) :
  _elem((const ElementHeader*)(xtc.payload())), 
  _end ((const ElementHeader*)(xtc.payload()+xtc.sizeofPayload())),
  _qmask(c.quadMask()), _bCompressed(false), _section(NULL), _section_id(0), 
  _sectionBuf1(NULL), _sectionBuf2(NULL), _compressedSection(NULL), 
  _pDecompressor(NULL), _uQuadWord(0), _bLastQuadWordSet(false)
{
  if (xtc.contains.id()!=Pds::TypeId::Id_CspadElement ||
      xtc.contains.version()!=1) {
    printf("Pds::CsPad::ElementIterator wrong type (V1) %x/%x\n",
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
  _qmask(c.quadMask()), _bCompressed(false), _section(NULL), _section_id(0),
  _sectionBuf1(NULL), _sectionBuf2(NULL), _compressedSection(NULL), 
  _pDecompressor(NULL), _uQuadWord(0), _bLastQuadWordSet(false)
{
  if (xtc.contains.id()!=Pds::TypeId::Id_CspadElement) {
    printf("Pds::CsPad::ElementIterator wrong type (V2) %x/%x\n",
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
    printf("Pds::CsPad::ElementIterator wrong type (V2a) %x/%x\n",
     xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
}

ElementIterator::ElementIterator(const ConfigV3& c, const Xtc& xtc) :
  _elem((const ElementHeader*)(xtc.payload())),
  _end ((const ElementHeader*)(xtc.payload()+xtc.sizeofPayload())),
  _qmask(c.quadMask()), _bCompressed(false), _section(NULL), _section_id(0), 
  _sectionBuf1(NULL), _sectionBuf2(NULL), _compressedSection(NULL), 
  _pDecompressor(NULL), _uQuadWord(0), _bLastQuadWordSet(false)
{
  if (  xtc.contains.id()!=Pds::TypeId::Id_CspadElement &&
        xtc.contains.id()!=Pds::TypeId::Id_CspadCompressedElement ) {
    printf("Pds::CsPad::ElementIterator wrong type (V3) %x/%x\n", xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
    return;
  }
  
  if (xtc.contains.id()==Pds::TypeId::Id_CspadElement) {
    if (xtc.contains.version()==1) {
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
      printf("Pds::CsPad::ElementIterator wrong type (V3a) %x/%x\n",
         xtc.contains.id(),xtc.contains.version());
      _elem = 0;
      _end  = 0;
    }
    
    return;
  }
  
  /*
   * Remaining case: xtc.contains.id() == Pds::TypeId::Id_CspadCompressedElement
   */

  // only support version 1: Data must have been shuffled
  if (xtc.contains.version()==2) {
    _bCompressed  = true;
    _sectionBuf1  = new Section();
    _sectionBuf2  = new Section();
    _pDecompressor  = new CspadCompressor();
    for(int iq=0; iq<4; iq++)
    {
      //!! debug only
      //printf("smark %d = 0x%x\n", iq, c.roiMask(iq));
      _smask[iq] = c.roiMask(iq);      
    }
  }
  else {
    printf("Pds::CsPad::ElementIterator wrong type (V3b) %x/%x\n",
       xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }   
}

ElementIterator::ElementIterator(const ConfigV4& c, const Xtc& xtc) :
  _elem((const ElementHeader*)(xtc.payload())),
  _end ((const ElementHeader*)(xtc.payload()+xtc.sizeofPayload())),
  _qmask(c.quadMask()), _bCompressed(false), _section(NULL), _section_id(0),
  _sectionBuf1(NULL), _sectionBuf2(NULL), _compressedSection(NULL),
  _pDecompressor(NULL), _uQuadWord(0), _bLastQuadWordSet(false)
{
  if (  xtc.contains.id()!=Pds::TypeId::Id_CspadElement &&
        xtc.contains.id()!=Pds::TypeId::Id_CspadCompressedElement ) {
    printf("Pds::CsPad::ElementIterator wrong type (V4) %x/%x\n", xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
    return;
  }

  if (xtc.contains.id()==Pds::TypeId::Id_CspadElement) {
    if (xtc.contains.version()==1) {
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
      printf("Pds::CsPad::ElementIterator wrong type (V3a) %x/%x\n",
         xtc.contains.id(),xtc.contains.version());
      _elem = 0;
      _end  = 0;
    }

    return;
  }

  /*
   * Remaining case: xtc.contains.id() == Pds::TypeId::Id_CspadCompressedElement
   */

  // only support version 1: Data must have been shuffled
  if (xtc.contains.version()==2) {
    _bCompressed  = true;
    _sectionBuf1  = new Section();
    _sectionBuf2  = new Section();
    _pDecompressor  = new CspadCompressor();
    for(int iq=0; iq<4; iq++)
    {
      //!! debug only
      //printf("smark %d = 0x%x\n", iq, c.roiMask(iq));
      _smask[iq] = c.roiMask(iq);
    }
  }
  else {
    printf("Pds::CsPad::ElementIterator wrong type (V3b) %x/%x\n",
       xtc.contains.id(),xtc.contains.version());
    _elem = 0;
    _end  = 0;
  }
}
ElementIterator::ElementIterator(const ElementIterator& c) :
  _elem       (c._elem      ),
  _end        (c._end       ),
  _qmask      (c._qmask     ),
  _smaskc     (c._smaskc    ),
  _bCompressed(c._bCompressed),
  _section    (c._section),
  _section_id (c._section_id),
  _compressedSection
              (c._compressedSection),
  _uQuadWord  (c._uQuadWord),
  _bLastQuadWordSet
              (c._bLastQuadWordSet)
{
  for(int i=0; i<4; i++)
    _smask[i]=c._smask[i];
  
  if (_bCompressed)
  {
    _sectionBuf1    = new Section();
    _sectionBuf2    = new Section();
    _pDecompressor  = new CspadCompressor();    
    
    if (c._sectionBuf1 != NULL)
      memcpy( _sectionBuf1, c._sectionBuf1, sizeof(*_sectionBuf1));
    if (c._sectionBuf2 != NULL)
      memcpy( _sectionBuf2, c._sectionBuf2, sizeof(*_sectionBuf2));
  }
}

ElementIterator::~ElementIterator()
{  
  if (_bCompressed)
  {
    delete _sectionBuf1;
    delete _sectionBuf2;
    delete _pDecompressor;
  }
}

const ElementHeader* ElementIterator::next()
{
  if (_qmask==0 || _elem>=_end) return 0;

  unsigned iq = _elem->quad();
  _qmask &= ~(1<<iq);

  _smaskc = _smask[iq];
  if (_smaskc) {
    if (_bCompressed)
    {
      _compressedSection = reinterpret_cast<const CompressedSectionHeader*>(_elem+1);
        
      void*                         image = NULL;
      CspadCompressor::ImageParams  params;
      const int status = _pDecompressor->decompress( _compressedSection,
        sizeof(CompressedSectionHeader) + _compressedSection->u32DataSize, image, params );
      if( 0 != status) {
        printf("Pds::CsPad::ElementIterator::next(): Decompression failed for quad %d (remain mask 0x%x) mask %x data size %d\n",
            iq, _qmask, _smaskc, _compressedSection->u32DataSize);
            
        memset( _sectionBuf1, 0, sizeof(Section) );
        _section_id = 0; 
        while ((_smaskc&(1<<_section_id))==0)
          _section_id++;      
      }      
      else
      {      
        memcpy( _sectionBuf1, image, sizeof(Section) );
        
        _section    = _sectionBuf1;
        _section_id = 0; 
        while ((_smaskc&(1<<_section_id))==0)
          _section_id++;            
      }
    }
    else
    {
      _section    = (Section*)(_elem+1);
      _section_id = 0; 
      while ((_smaskc&(1<<_section_id))==0)
        _section_id++;
    }
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
  if (_qmask || !_bLastQuadWordSet) {
    if (_bCompressed)
    {
      const CompressedSectionHeader* 
        s = reinterpret_cast<const CompressedSectionHeader*>(_elem+1);
      for(unsigned sm=_smask[iq]; sm; sm>>=1)
        if (sm&1) 
        {
          s = reinterpret_cast<const CompressedSectionHeader*>(
            (const char*) (s+1) + s->u32DataSize );
        }
        
      const uint32_t* u = reinterpret_cast<const uint32_t*>(s);
      _uQuadWord = *u;      
      //printf( "#0 Quadword 0x%x pElem %p\n", *(uint32_t*) u, _elem); //!!debug
      
      if (_qmask)
        _elem = reinterpret_cast<const ElementHeader*>(u+1);
      else
        _bLastQuadWordSet = true;
    }
    else
    {
      const Section*  s = reinterpret_cast<const Section*>(_elem+1);
      for(unsigned sm=_smask[iq]; sm; sm>>=1)
        if (sm&1) s++;
        
      const uint32_t* u = reinterpret_cast<const uint32_t*>(s);
      _uQuadWord = *u;      
      //printf( "#0 Quadword 0x%x pElem %p\n", *(uint32_t*) u, _elem); //!!debug
      
      if (_qmask)
        _elem = reinterpret_cast<const ElementHeader*>(u+1);      
      else
        _bLastQuadWordSet = true;
    }
  }

  return e;
}

const Section* ElementIterator::next(unsigned& id)
{
  if (_smaskc==0) return NULL;

  _smaskc &= ~(1<<_section_id);

  const Section* s = _section;
  id = _section_id;  

  // advance _section
  if (_smaskc) {      
    if (_bCompressed)
    {
      if (_section == _sectionBuf1)
        _section = _sectionBuf2;
      else
        _section = _sectionBuf1;
        
      _compressedSection = (CompressedSectionHeader*) ( (char*) (_compressedSection+1) + 
        _compressedSection->u32DataSize );      
      void*                         image = NULL;
      CspadCompressor::ImageParams  params;
      const int status = _pDecompressor->decompress( _compressedSection,
        sizeof(CompressedSectionHeader) + _compressedSection->u32DataSize, image, params );
      if( 0 != status) {
          printf("Pds::CsPad::ElementIterator::next(): Decompression failed for section %d mask %x data size %d\n",
            id, _smaskc, _compressedSection->u32DataSize);
          return NULL;
      }
      
      memcpy( _section, image, sizeof(Section) );      
      while ((_smaskc&(1<<++_section_id))==0) 
        ;
    }
    else
    {
      _section++;
      while ((_smaskc&(1<<++_section_id))==0) 
        ;
    }
  }

  return s;
}

uint32_t ElementIterator::getQuadWord()
{
  return _uQuadWord;
}
