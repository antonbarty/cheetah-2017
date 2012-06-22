#ifndef Pds_Cspad_ElementIterator_hh
#define Pds_Cspad_ElementIterator_hh

//
//  class ElementIterator
//
//  A class to iterate through the CSPAD detector data
//
//  Each "Element" represents one quadrant of a complete detector
//  and they are arranged as follows (viewed from upstream):
//  +---+---+
//  | 0 | 1 |
//  +---+---+
//  | 3 | 2 |
//  +---+---+
//
//  Each "Element" is composed of 8 "Section"s arranged as follows:
//  +---+---+-------+
//  |   |   |   6   |
//  + 5 | 4 +-------+
//  |   |   |   7   |
//  +---+---+---+---+   (for quadrant 0)
//  |   2   |   |   |
//  +-------+ 0 | 1 |
//  |   3   |   |   |
//  +-------+---+---+
//  The layout of each successive quadrant is rotated 90 degrees clockwise
//  with respect to the previous quadrant.
//
//  Each "Section" is composed of 2*194 rows by 185 columns with the following 
//  orientations (for quadrant 0):
//    Sections 0,1: row index increases from bottom to top, column index increases from left to right
//    Sections 2,3: row index increases from left to right, column index increases from top to bottom
//    Sections 4,5: row index increases from top to bottom, column index increases from right to left
//    Sections 6,7: row index increases from left to right, column index increases from top to bottom
//  Again, the orientations of the Sections for quadrant 1 are rotated 90 degrees clockwise 
//  and so on for each successive quadrant.
//

#include "Detector.hh"
#include "pdsdata/cspad/CspadCompressor.hh"

#include <stdint.h>

namespace Pds {

  class Xtc;

  namespace CsPad {

    class ConfigV1;
    class ConfigV2;
    class ConfigV3;
    class ConfigV4;
    class ElementHeader;
    
    class Section {
    public:
      uint16_t pixel[ColumnsPerASIC][2*MaxRowsPerASIC];
    };

    class CompressedSectionHeader {
    public:
      uint32_t  u32Flag;
      uint32_t  u32CheckSum;
      uint32_t  u32Width;
      uint32_t  u32Height;
      uint32_t  u32Depth;
      uint32_t  u32DataSize; // Total size of the following image data (not including this header)
    };
    
    class ElementIterator {
    public:
      ElementIterator();
      ElementIterator(const ConfigV1&, const Xtc&);
      ElementIterator(const ConfigV2&, const Xtc&);
      ElementIterator(const ConfigV3&, const Xtc&);
      ElementIterator(const ConfigV4&, const Xtc&);
      ElementIterator(const ElementIterator&);
      ~ElementIterator();
    public:
      //  Iterate to the next Element/quadrant (0..3)
      const ElementHeader*  next();
      //  Iterate to the next Section (0..7) within the current quadrant
      const Section*        next(unsigned& sectionID);
      uint32_t              getQuadWord();
    private:
      const ElementHeader* _elem;
      const ElementHeader* _end;
      unsigned             _qmask;      
      unsigned             _smask[4];
      unsigned             _smaskc;
      bool                 _bCompressed;
      Section*             _section;
      unsigned             _section_id;
      Section*             _sectionBuf1;
      Section*             _sectionBuf2;
      const CompressedSectionHeader*
                           _compressedSection;
      CspadCompressor*     _pDecompressor;
      uint32_t             _uQuadWord;
      bool                 _bLastQuadWordSet;
    };
  };
};

#endif
